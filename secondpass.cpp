#include "secondpass.hpp"

#define IS_BYTE(x) ((x & ~0xFF) == 0)

SPState spstate;

bool END_OF_SECOND_PASS;

void secondPassStateMachine(){
	spstate = CHK_FIRST_TOK;
	END_OF_SECOND_PASS = false;
	
	Record * record = NULL;
	std::istringstream recordStream;
	
	int tblSub;
	
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
			chkFirstTok(record, recordStream, tblSub);
			break;
		case PROC_DIR:
			procDir(record, recordStream);
			break;
		case PROC_INST_OPS:
			procInstOps(record, recordStream, tblSub);
			break;
		case GEN_S_RECS:
			genSRecs();
			break;
		default:
			break;
		}
	}
}

void chkFirstTok(Record * record, std::istringstream & recordStream,
int & tblSub){
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
	tblSub = record->cmdSubScr;
	if(tblSub != CMD_NOT_FOUND){
		// save table subscript in record struct
		record->cmdSubScr = tblSub;
		switch(commands[tblSub].type){
		case INST:
			spstate = PROC_INST_OPS;
			return;
		case DIR:
			spstate = PROC_DIR;
			return;
		default:
			record->error = SEC_PASS_NO_CMD;
			return;
		}
	}
}

void procDir(Record * record, std::istringstream & recordStream){
	spstate = CHK_FIRST_TOK;
	return;
}

void procInstOps(Record * record, std::istringstream & recordStream,
int & tblSub){
	// grab operands
	std::string operands = getNextToken(recordStream);
	std::string operand = getOperand(operands);
	
	Symbol * symPtr;
	
	switch(commands[tblSub].ops){
	case CR_R:
	case R_R:
	{
		union arith_op tempOp;
		// set base opcode, wb built in
		tempOp.opcode = commands[tblSub].baseOp;
		// fill in rest
		if((symPtr = checkTable(symtbl,operand)) != NULL){
			// operand in symtbl
			switch(symPtr->type){
			case REG:
				// set rc bit to register
				tempOp.bf.rc = r;
				tempOp.bf.src = symPtr->value;
				break;
			case LBL:
			{
				// set rc bit to const
				tempOp.bf.rc = c;
				// save constant encoding
				int tempConst = decodeConst(symPtr->value);
				if(tempConst == -1){
					// invalid constant
					record->error = INV_CONST;
					spstate = CHK_FIRST_TOK;
					return;
				}else{
					tempOp.bf.src = tempConst;
				}
				break;
			}
			default:
				// unknown symbol type
				record->error = UNK_SYM;
				break;
			}
		}else{ // regular constant
			tempOp.bf.rc = c;
			int tempConst = decodeConst(extractValue(operand));
			if(tempConst == -1){
				// invalid constant
				record->error = INV_CONST;
				spstate = CHK_FIRST_TOK;
				return;
			}else{
				tempOp.bf.src = tempConst;
			}
		}
		// get next operand, should be register
		operand = getOperand(operands);
		symPtr = checkTable(symtbl,operand);
		if(symPtr != NULL && symPtr->type == REG){
			// symbol found
			tempOp.bf.dst = symPtr->value;
		}else{
			// error if sym not found or sym not reg
			record->error = INV_OP2;
			spstate = CHK_FIRST_TOK;
			return;
		}
		// save opcode in record
		record->opcode = tempOp.opcode;
		spstate = CHK_FIRST_TOK;
		return;
	}
	case V_R:
	{
		union reginit_op tempOp;
		tempOp.opcode = commands[tblSub].baseOp;
		// first operand
		int val;
		if((symPtr = checkTable(symtbl,operand)) != NULL){
			// symbol
			val = symPtr->value;
		}else{ // regular value
			int val = extractValue(operand);
		}
		if(IS_BYTE(val)){
			tempOp.bf.byte = symPtr->value;
		}else{ // operand out of bounds
			record->error = OUT_BOUND;
			spstate = CHK_FIRST_TOK;
			return;
		}
		// second operand
		operand = getOperand(operands);
		symPtr = checkTable(symtbl,operand);
		tempOp.bf.dst = symPtr->value;
		record->opcode = tempOp.opcode;
		spstate = CHK_FIRST_TOK;
		return;
	}
	case R:
	{
		union singr_op tempOp;
		tempOp.opcode = commands[tblSub].baseOp;
		symPtr = checkTable(symtbl,operand);
		if(symPtr != NULL){
			tempOp.bf.dst = symPtr->value;
		}else{ // reg not found
			record->error = INV_OP1;
		}
		record->opcode = tempOp.opcode;
		spstate = CHK_FIRST_TOK;
		return;
	}
	case BRA_:
	case BRA13:
	{
		int label;
		symPtr = checkTable(symtbl,operand);
		if(symPtr != NULL){
			// symbol found
			label = symPtr->value;
		}else{
			// non-label value
			label = extractValue(operand);
		}
		
		// encode offset
		int dist = label - (record->memLoc + 2);
		if(dist % 2 != 0){
			// branch to uneven address
			record->error = BRA_UNEV;
		}
		int offset = dist >> 1;
		if(commands[tblSub].ops == BRA_ && -1024 <= offset <= 1022){
			union bra10_op tempOp;
			tempOp.opcode = commands[tblSub].baseOp;
			tempOp.bf.off = offset;
			record->opcode = tempOp.opcode;
		}else if (commands[tblSub].ops == BRA13 &&
		-8192 <= offset <= 8190){
			union bra13_op tempOp;
			tempOp.opcode = commands[tblSub].baseOp;
			tempOp.bf.off = offset;
			record->opcode = tempOp.opcode;
		}else{
			record->error = OUT_BOUND;
		}
		spstate = CHK_FIRST_TOK;
		std::cout << "BRA OFF: " << offset << std::endl;
		return;
	}
	case LD_:
	case ST_:
		spstate = CHK_FIRST_TOK;
		return;
	case LDR_:
	case STR_:
		spstate = CHK_FIRST_TOK;
		return;
	case SA:
		spstate = CHK_FIRST_TOK;
		return;
	case CEX:
		spstate = CHK_FIRST_TOK;
		return;
	default:
		record->error = SEC_PASS_NO_CMD;
		spstate = CHK_FIRST_TOK;
		return;
	}
}

void genSRecs(){
	std::cout << "Done second pass." << std::endl;
	END_OF_SECOND_PASS = true;
	return;
}
