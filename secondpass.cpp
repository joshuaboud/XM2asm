/* File name: secondpass.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Second pass state machine
 * Last Modified: 2019-07-01
 */

#include "secondpass.hpp"

#define IS_BYTE(x) ((x & ~0xFF) == 0)
#define IS_WORD(x) ((x & ~0xFFFF) == 0)
#define HIGH_BYTE(x) ((x >> 8) & 0xFF)
#define LOW_BYTE(x) (x & 0xFF)
#define BYTE_MASK(x) (x & 0xFF)
#define WORD_MASK(x) (x & 0xFFFF)

// 32 + 2 because the memory location is not included in max bytes
#define SREC_BYTECNT 32 + 2

SPState spstate;

bool END_OF_SECOND_PASS;

void secondPassStateMachine(std::string baseFileName){
	spstate = CHK_FIRST_TOK;
	END_OF_SECOND_PASS = false;
	
	Record * record = NULL;
	std::istringstream recordStream;
	
	while(!END_OF_SECOND_PASS){
		switch(spstate){
		case CHK_FIRST_TOK:
			if(!record){
				// start of second pass, start at first record
				record = records_end;
			} else {
				// get next record
				record = record->next;
				if(!record){ // end of records reached
					spstate = GEN_S_RECS; // create s records
					break;
				}
			}
			chkFirstTok(record, recordStream);
			break;
		case PROC_DIR:
			procDir(record, recordStream);
			break;
		case PROC_INST_OPS:
			procInstOps(record, recordStream);
			break;
		case GEN_S_RECS:
			genSRecs(baseFileName);
			break;
		default:
			break;
		}
	}
}

void chkFirstTok(Record * record, std::istringstream & recordStream){
	// open record string as string stream
	recordStream.clear(); // unset eof bit
	recordStream.str(record->record); // init stream
	
	// get first token
	std::string token = getNextToken(recordStream);
	
	if(checkTable(symtbl,token) != NULL){
		// token is a label, skip it
		token = getNextToken(recordStream);
	}
	if(token[0] == ';'){
		// comment, skip record
		return;
	}
	// save table subscript in record struct
	record->cmdSubScr = checkTable(commands, token);
	if(record->cmdSubScr != CMD_NOT_FOUND){
		switch(commands[record->cmdSubScr].type){
		case INST:
			spstate = PROC_INST_OPS;
			return;
		case DIR:
			spstate = PROC_DIR;
			return;
		default:
			record->error = SEC_PASS_NO_CMD;
			ERROR_FLAG = true;
			return;
		}
	}
}

void procDir(Record * record, std::istringstream & recordStream){
	// grab operands
	std::string operands = getNextToken(recordStream);
	std::string operand = getOperand(operands);
	
	Symbol * symPtr;
	switch(record->cmdSubScr){
	case BYTE:
	case WORD:
	{
		int value;
		symPtr = checkTable(symtbl, operand);
		if(symPtr != NULL){
			// symbol found
			value = symPtr->value;
		}else{
			// regular value
			value = extractValue(operand);
		}
		if((record->cmdSubScr == BYTE && IS_BYTE((unsigned short)value)) ||
		(record->cmdSubScr == WORD && IS_WORD((unsigned short)value))){
			record->opcode = value;
		}else{
			// out of bounds error
			record->error = OUT_BOUND;
		}
		break;
	}
	default:
		break;
	}
	spstate = CHK_FIRST_TOK;
	if(record->error != NO_ERR){
		ERROR_FLAG = true;
	}
	return;
}

void procInstOps(Record * record, std::istringstream & recordStream){
	// grab operands
	std::string operands = getNextToken(recordStream);
	
	switch(commands[record->cmdSubScr].ops){
	case CR_R:
	case R_R:
		arith(record, operands);
		break;
	case V_R:
		reginit(record, operands);
		break;
	case R:
		singr(record, operands);
		break;
	case BRA_:
	case BRA13:
		bra(record, operands);
		break;
	case LD_:
	case ST_:
		mem(record, operands);
		break;
	case STR_:
	case LDR_:
		memr(record, operands);
		break;
	case SA:
		svc(record, operands);
		break;
	case CEX_:
		cex(record, operands);
		break;
	default:
		record->error = SEC_PASS_NO_CMD;
		spstate = CHK_FIRST_TOK;
		break;
	}
	if(record->error != NO_ERR){
		ERROR_FLAG = true;
	}
	return;
}

void genSRecs(std::string baseFileName){
	if(ERROR_FLAG){
		END_OF_SECOND_PASS = true;
		return;
	}
	
	std::ofstream srec(baseFileName + ".xme");
	if(!srec){
		std::cout << "Error creating S-Record file." << std::endl;
		return;
	}
	// generate S0 record name
	// max s-rec length is 32, subtract 2 bytes for memloc
	// Truncating filename to this length or less.
	std::string s0name = baseFileName.substr(0,SREC_BYTECNT - 2);
	
	int byteCnt = 0;
	char chkSum = 0;
	
	// s0 record
	byteCnt = 2; // 0000 memloc
	for(char c : s0name){;
		byteCnt++;
		chkSum += c;
	}
	byteCnt++; // chkSum
	chkSum += byteCnt;
	chkSum = ~chkSum;
	// print s0
	srec << "S0" << std::setw(2) << std::setfill('0') << std::hex <<
	byteCnt << "0000" << s0name <<
	std::setw(2) << std::setfill('0') << BYTE_MASK((int)chkSum) << std::endl;
	
	// print s1's
	char sBuff[SREC_BYTECNT];
	int prevMemLoc;
	bool done = false;
	Record * record = records_end; //start at first record
	// find first record with a memory location
	while(record->memLoc == NO_MEMLOC && record->next != NULL){
		// skip non inst or data recs
		record = record->next;
	}
	if(record != NULL){
		prevMemLoc = record->memLoc;
	}else{
		return;
	}
	byteCnt = 0;
	sBuff[byteCnt++] = HIGH_BYTE(record->memLoc);
	chkSum = HIGH_BYTE(record->memLoc);
	sBuff[byteCnt++] = LOW_BYTE(record->memLoc);
	chkSum += LOW_BYTE(record->memLoc);
	while(!done){
		while(record != NULL && record->memLoc == NO_MEMLOC){
			// skip non inst or data recs
			record = record->next;
		}
		if((record == NULL || record->memLoc > prevMemLoc + 2) ||
		byteCnt >= SREC_BYTECNT){
			// flush srec
			srec << "S1" << std::hex << std::setw(2) <<
			std::setfill('0') << byteCnt + 1; // plus one for chksum
			for(int i = 0; i < byteCnt; i++){
				srec << std::hex << std::setw(2) << std::setfill('0') <<
				BYTE_MASK((int)sBuff[i]);
			}
			byteCnt++;
			chkSum += byteCnt;
			chkSum = ~chkSum;
			srec << std::hex << std::setw(2) << std::setfill('0') <<
			BYTE_MASK((int)chkSum) << std::endl;
			// reinit sBuff etc
			byteCnt = 0;
			chkSum = 0;
			if(record != NULL){
				sBuff[byteCnt++] = HIGH_BYTE(record->memLoc);
				chkSum += HIGH_BYTE(record->memLoc);
				sBuff[byteCnt++] = LOW_BYTE(record->memLoc);
				chkSum += LOW_BYTE(record->memLoc);
				prevMemLoc = record->memLoc;
			}else{
				done = true;
				break;
			}
		}
		if(record->cmdSubScr == BYTE){
			sBuff[byteCnt++] = LOW_BYTE(record->opcode);
			chkSum += LOW_BYTE(record->opcode);
		}else{
			sBuff[byteCnt++] = LOW_BYTE(record->opcode);
			chkSum += LOW_BYTE(record->opcode);
			sBuff[byteCnt++] = HIGH_BYTE(record->opcode);
			chkSum += HIGH_BYTE(record->opcode);
		}
		prevMemLoc = record->memLoc;
		record = record->next;
	}
	
	// S9 record
	chkSum = 0;
	srec << "S903";
	chkSum += 03 + LOW_BYTE(START) + HIGH_BYTE(START);
	chkSum = ~chkSum;
	srec << std::hex << std::setw(4) << std::setfill('0') <<
	WORD_MASK((int)START) << std::setw(2) << BYTE_MASK((int)chkSum) <<
	std::endl;
	
	srec.close();
	
	END_OF_SECOND_PASS = true;
	return;
}
