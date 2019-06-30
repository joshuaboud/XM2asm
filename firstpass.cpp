/* File name: firstpass.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: State machine that runs through each record of the source
 * 			file stream passed to it, builds a symbol table, and verifies
 * 			that there are no errors. All state changes are controlled
 * 			in this file.
 * Last Modified: 2019-06-03
 */

#include "firstpass.hpp"

#define CEX_OOB(x) (x < 0 || x > 7) // CEX op 2 out of bounds

#define SA_OOB(x) (x < 0 || x > 15)

// Globals

FPState fpstate; // state variable

bool END_OF_FIRST_PASS; // signals stop of state machine
bool ERROR_FLAG; // is set when pushRecord() is passed a non-empty error
// message. When the first pass is complete, the assembler will not
// continue to the second pass if this flag is set.

int lineNum; // line number of source module
int memLoc; // memory location counter

void firstPassStateMachine(std::ifstream & source){
	// Initializes and controls state functions of state machine
	END_OF_FIRST_PASS = false;
	ERROR_FLAG = false;
	fpstate = CHECK_FIRST_TOKEN;
	lineNum = 0;
	memLoc = 0;
	
	std::string record; // record of source module is read into this
	std::istringstream recordStream; // converted to stream to extract tokens
	std::string token; // token of record, space delimited
	
	std::string label; // hold label for directive, used to pass label for EQU
	
	int tblSub;	//	command table subscript
	
	while(!END_OF_FIRST_PASS){
		switch(fpstate){
		case CHECK_FIRST_TOKEN:
			// Get next record:
			if(source.eof()){ // file not terminated with \n
				END_OF_FIRST_PASS = true;
				break;
			}
			getline(source, record);
			if((record == "") && source.eof()){ // file terminated with \n
				END_OF_FIRST_PASS = true;
				break;
			}
			lineNum++;
			recordStream.clear(); // clear stream error bits (EOF)
			recordStream.str(record);
			// do:
			checkFirstToken(recordStream, token, tblSub);
			break;
		case CHECK_INST_DIR: // after identifying label goto chkDir|Inst
			// save label for going to check_dir
			label = token;
			checkInstOrDir(recordStream, token, tblSub);
			break;
		case CHECK_DIR: // verify operand of directive
			checkDir(recordStream, token, tblSub, memLoc, label);
			// clear label holder
			label = "";
			break;
		case CHECK_INST: // verify operand(s) of instruction
			checkInst(recordStream, token, tblSub, memLoc);
			// each instruction takes 2 bytes of memory
			memLoc += 2;
			break;
		}
	}
}

void checkFirstToken(std::istringstream & record, std::string & token, int & tblSub){
	// Grabs first token from record, decides what state to jump to
	// after identifying token. If errors are encountered, the whole record
	// is saved with an error description and the state machine moves onto
	// the next record.
	token = getNextToken(record);
	bool labelResult = validLabel(token);
	ErrorEnum err = NO_ERR; // default to no error
	if(record.str()[0] == ';'){ // just a comment
		pushRecord(records, lineNum, record.str());
		return;
	}if(token.empty()){ // empty record
		err = EMPTY_RECORD;
		pushRecord(records, lineNum, record.str(), err);
		return;
	}if((tblSub = checkTable(commands, token)) != CMD_NOT_FOUND){
		switch(commands[tblSub].type){ // ^if true, inst or dir
		case DIR:
			fpstate = CHECK_DIR;
			return;
		case INST:
			fpstate = CHECK_INST;
			return;
		default:
			// shouldn't be neither
			err = EMPTY_RECORD;
			pushRecord(records, lineNum, record.str(), err);
			return;
		}
	}if(!labelResult){ // invalid label
		err = INV_LBL;
		pushRecord(records, lineNum, record.str(), err);
		return;
	}if(labelResult){ // must be valid label
		Symbol * symPtr;
		if((symPtr = checkTable(symtbl, token)) != NULL){ // in sym table
			switch(symPtr->type){
			case LBL: // double defined label
				err = DUP_LBL;
				pushRecord(records, lineNum, record.str(), err);
				return;
			case REG: // label is R0 - R7
				err = LBL_REG;
				pushRecord(records, lineNum, record.str(), err);
				return;
			case UNK: // forward reference, fill in data
				symPtr->value = memLoc;
				symPtr->type = LBL;
				fpstate = CHECK_INST_DIR;
				return;
			}
		} else { // add label to symbol table
			Symbol temp;
			temp.name = token;
			temp.type = LBL;
			temp.value = memLoc;
			pushSymbol(symtbl, temp);
			fpstate = CHECK_INST_DIR;
			return;
		}
	} else { // invalid label
		err = INV_LBL;
		pushRecord(records, lineNum, record.str(), err);
		return;
	}
}

void checkInstOrDir(std::istringstream & record, std::string & token, int & tblSub){
	token = getNextToken(record); // Get first token
	ErrorEnum err = NO_ERR;
	if(token.empty()){ // end of record
		// save label
		pushRecord(records, lineNum, record.str());
		fpstate = CHECK_FIRST_TOKEN;
		return;
	}if((tblSub = checkTable(commands, token)) != CMD_NOT_FOUND){ // inst or dir
		switch(commands[tblSub].type){
		case DIR:
			fpstate = CHECK_DIR;
			return;
		case INST:
			fpstate = CHECK_INST;
			return;
		default:
			break;
		}
	}
	err = MULT_LBL;
	pushRecord(records, lineNum, record.str(), err);
	fpstate = CHECK_FIRST_TOKEN;
	return;
}

void checkInst(std::istringstream & record, std::string & token,
int & tblSub, int & memLoc){
	token = getNextToken(record); // should hold all operands
	
	ErrorEnum err = NO_ERR; // default to no error
	
	Symbol * sym;
	std::string operand;
	
	bool cexPassed;
	
	if(token.empty()){
		err = MISS_OP;
		pushRecord(records, lineNum, record.str(), err, memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	}
	if(memLoc % 2 != 0){
		err = UNEV_ADDR;
		pushRecord(records, lineNum, record.str(), err, memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	}
	
	// get first operand
	operand = getOperand(token);
	
	bool labelResult;
	bool valueResult;
	bool constResult;
	
	switch(commands[tblSub].ops){
	case CR_R:
		labelResult = validLabel(operand);
		constResult = validConstant(operand);
		// verify first operand
		if((sym = checkTable(symtbl, operand)) == NULL &&
		!(labelResult || constResult)){
			// if not a register or label and not a valid label or constant
			err = INV_OP1;
		}else if(sym == NULL && labelResult){ // valid forward ref
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && \
		!(labelResult || constResult)){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = INV_OP1;
		}else if(sym != NULL && sym->type != REG){ // if known label
			if(!validConstant(sym->value)){ // check that value is const
				err = INV_OP1;
			}
		}
		// if here, first operand is verified as a constant, register, or label.
		// verify second operand
		operand = getOperand(token);
		labelResult = validLabel(operand);
		
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unkown
			err = INV_OP2;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = INV_OP2;
		}
		break;
	case R_R:
		labelResult = validLabel(operand);
		// verify first operand
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unkown
			err = INV_OP1;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && labelResult != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = INV_OP1;
		}
		// verify second operand
		operand = getOperand(token);
		labelResult = validLabel(operand);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unkown
			err = INV_OP2;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = INV_OP2;
		}
		break;
	case C:
		constResult = validConstant(operand);
		labelResult = validLabel(operand);
		if((sym = checkTable(symtbl, operand)) == NULL && !constResult){
			// if operand is not a label/UNK and not a valid constant
			err = INV_CONST;
		}else if(sym != NULL && sym->type == REG){
			// if operand is a register
			err = INV_CONST;
		}else if(sym != NULL && !validConstant(sym->value)){ // known label
			// if label value is not a constant
			err = INV_CONST;
		}else if(sym == NULL && labelResult){
			// add to symbol table as forward reference
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}
		break;
	case R:
		labelResult = validLabel(operand);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = NOT_REG;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = NOT_REG;
		}
		break;
	case V: // value
		valueResult = validValue(operand);
		labelResult = validLabel(operand);
		if(!valueResult && !labelResult){
			// if not a valid value and not a valid label
			err = NOT_VAL;
		}else if((sym = checkTable(symtbl, operand)) == NULL &&
		!labelResult && !valueResult){
			// not a label or valid label to make forward reference
			err = NOT_VAL;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = NOT_VAL;
		}
		break;
	case CEX_:
		// see if first operand is in list of CEX flags
		cexPassed = false;
		for(int i = 0; i < CEX_FLAG_CNT; i++){
			if(operand == cexFlags[i].flag){
				cexPassed = true;
				break;
			}
		}
		if(!cexPassed){
			// failed check against valid optype1 list
			err = CEX1;
		}
		// check second operand is value
		operand = getOperand(token);
		valueResult = validValue(operand);
		labelResult = validLabel(operand);
		if(!valueResult && !labelResult){
			// if not a valid value and not a valid label
			err = INV_OP2;
		}else if((sym = checkTable(symtbl, operand)) == NULL &&
		!labelResult && !valueResult){
			// not a label or valid label to make forward reference
			err = INV_OP2;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = INV_OP2;
		}else if(sym != NULL && sym->type == LBL &&
		CEX_OOB(sym->value)){
			// value of symbol out of bounds
			err = OUT_BOUND;
		}else if(valueResult){ // valid value but...
			int val = extractValue(operand);
			if(CEX_OOB(val)){
				// value out of bounds
				err = OUT_BOUND;
			}
		}
		// check third operand
		operand = getOperand(token);
		valueResult = validValue(operand);
		labelResult = validLabel(operand);
		if(!valueResult && !labelResult){
			// if not a valid value and not a valid label
			err = INV_OP3;
		}else if((sym = checkTable(symtbl, operand)) == NULL &&
		!labelResult && !valueResult){
			// not a label or valid label to make forward reference
			err = INV_OP3;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = INV_OP3;
		}else if(sym != NULL && sym->type == LBL &&
		CEX_OOB(sym->value)){
			// value of symbol out of bounds
			err = OUT_BOUND;
		}else if(valueResult){ // valid value but...
			int val = extractValue(operand);
			if(CEX_OOB(val)){
				// value out of bounds
				err = OUT_BOUND;
			}
		}
		break;
	case SA:
		labelResult = validLabel(operand);
		valueResult = validValue(operand);
		if(!labelResult && !valueResult){
			// if not a valid value and not a valid label
			err = NOT_VAL;
		}else if((sym = checkTable(symtbl, operand)) == NULL &&
		!labelResult && !valueResult){
			// not a label or valid label to make forward reference
			err = NOT_VAL;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = NOT_VAL;
		}else if(sym != NULL && SA_OOB(sym->value)){
			// value out of bounds
			err = OUT_BOUND;
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}else if(valueResult){
			int val = extractValue(operand);
			if(SA_OOB(val)){
				// value out of bounds
				err = OUT_BOUND;
			}
		}
		break;
	case ST_:
		labelResult = validLabel(operand);
		// check that first operand is register
		if((sym = checkTable(symtbl, operand)) == NULL && sym->type == LBL){
			// if operand is not a register
			err = INV_OP1;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label
			err = INV_OP1;
		}
		// check second operand
		operand = getOperand(token);
		// may be pre/post increment/decrement
		if(operand[0] == '+' || operand[0] == '-'){
			// pre increment/decrement
			// cut off + or -
			operand = operand.substr(1, operand.length() - 1);
		}
		if(operand[operand.length() - 1] == '+' || \
		operand[operand.length() - 1] == '-'){
			// post increment/decrement
			// cut off + or -
			operand = operand.substr(0, operand.length() - 1);
		}
		// check symtbl with (extracted) register name
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = INV_OP2;
		}else if(sym == NULL && (labelResult = validLabel(operand))){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = INV_OP2;
		}
		break;
	case LD_:
		// may be pre/post increment/decrement
		if(operand[0] == '+' || operand[0] == '-'){
			// pre increment/decrement
			// cut off + or -
			operand = operand.substr(1, operand.length() - 1);
		}
		if(operand[operand.length() - 1] == '+' || \
		operand[operand.length() - 1] == '-'){
			// post increment/decrement
			// cut off + or -
			operand = operand.substr(0, operand.length() - 1);
		}
		// check symtbl with (extracted) register name
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = INV_OP1;
		}else if(sym == NULL && (labelResult = validLabel(operand))){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label
			err = INV_OP1;
		}
		// check second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = INV_OP2;
		}else if(sym == NULL && (labelResult = validLabel(operand))){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label
			err = INV_OP2;
		}
		break;
	case LDR_:
		// check first operand
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = INV_OP1;
		}else if(sym == NULL && (labelResult = validLabel(operand))){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = INV_OP1;
		}
		// check second operand
		operand = getOperand(token);
		valueResult = validValue(operand);
		labelResult = validLabel(operand);
		if(!valueResult && !labelResult){
			// if not a valid value and not a valid label
			err = INV_OP2;
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		!labelResult && !valueResult){
			// not a label or valid label to make forward reference
			err = INV_OP2;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = INV_OP2;
		}
		// check third operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = INV_OP3;
		}else if(sym == NULL && (labelResult = validLabel(operand))){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = INV_OP3;
		}
		break;
	case STR_:
		// check first operand
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = INV_OP1;
		}else if(sym == NULL && (labelResult = validLabel(operand))){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = INV_OP1;
		}
		// check second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = INV_OP2;
		}else if(sym == NULL && (labelResult = validLabel(operand))){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = INV_OP2;
		}
		// check third operand
		operand = getOperand(token);
		valueResult = validValue(operand);
		labelResult = validLabel(operand);
		if(!valueResult && !labelResult){
			// if not a valid value and not a valid label
			err = INV_OP3;
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		!labelResult && !valueResult){
			// not a label or valid label to make forward reference
			err = INV_OP3;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = INV_OP3;
		}
		break;
	case BRA_:
		valueResult = validValue(operand);
		labelResult = validLabel(operand);
		if(!valueResult && !labelResult){
			// if not a valid value and not a valid label
			err = NOT_VAL;
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		!labelResult && !valueResult){
			// not a label or valid label to make forward reference
			err = NOT_VAL;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = NOT_VAL;
		}else if(sym != NULL && sym->type == LBL && \
		(sym->value > 1022 || sym->value < -1024 || sym->value % 2 != 0)){
			// value of symbol out of bounds or odd
			err = OUT_BOUND;
		}
		break;
	case BRA13:
		valueResult = validValue(operand);
		labelResult = validLabel(operand);
		if(!valueResult && !labelResult){
			// if not a valid value and not a valid label
			err = NOT_VAL;
		}else if((sym = checkTable(symtbl, operand)) == NULL &&
		!labelResult && !valueResult){
			// not a valid label to make forward reference
			err = NOT_VAL;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = NOT_VAL;
		}
		break;
	case V_R:
		valueResult = validValue(operand);
		labelResult = validLabel(operand);
		// check first operand
		if(!valueResult && !labelResult){
			// if not a valid value and not a valid label
			err = INV_OP1;
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		!labelResult && !valueResult){
			// not a label or valid label to make forward reference
			err = INV_OP1;
		}else if(sym == NULL && labelResult){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = INV_OP1;
		}
		//check second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type != REG){
			// if operand is not a register
			err = INV_OP2;
		}else if(sym == NULL && (labelResult = validLabel(operand))){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && !labelResult){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = INV_OP2;
		}
		break;
	default:
		err = UNK_OP;
		break;
	}
	// check if there are too many operands
	if((operand = getOperand(token)) != ""){ // extraneous operand(s)
		err = EXTR_OP;
	}
	if(!record.eof()){ // operands followed by non-comment garbage
		std::string comments;
		record >> std::ws; // eat whitespace
		getline(record, comments);
		if(comments[0] != ';'){
			err = GARB;
		}
	}
	// end of state, push record with either empty err or error description
	pushRecord(records, lineNum, record.str(), err, tblSub, memLoc);
	fpstate = CHECK_FIRST_TOKEN;
	return;
}

void checkDir(std::istringstream & record, std::string & token, int & tblSub, 
int & memLoc, std::string label){
	token = getNextToken(record); // should hold operand, if any
	std::string operand;
	int value;
	int memLoc_ = -1; // will change to memLoc for BYTE and WORD
	unsigned short opcode; // value for BYTE and WORD
	Symbol * sym;
	Symbol * lblSym; // for EQU to copy labels
	ErrorEnum err = NO_ERR;
	switch(tblSub){
	case ALIGN:
		if(memLoc%2 != 0){
			// memLoc odd, increment by 1
			memLoc++;
		}
		break;
	case BSS:
		memLoc_ = memLoc; // save memLoc
		// if a label is present, it will already be assiciated with
		// block from checkFirstToken().
		operand = getOperand(token);
		if(validValue(operand) && (value = extractValue(operand)) >= 0){
			memLoc += value;
		}else if((sym = checkTable(symtbl, operand)) != NULL &&\
		sym->type == LBL && sym->value >= 0){
			value = sym->value;
			memLoc += value;
		}else{ // invalid label, unk, reg, invalid value, or negative
			err = POS_VAL;
		}
		opcode = 0x0;
		break;
	case BYTE:
		memLoc_ = memLoc; // save memLoc
		operand = getOperand(token);
		if(validValue(operand) && !((value = extractValue(operand)) & ~0xFF)){
			// valid value and 8 bits or less (no bits set higher than bit 7)
			//memLoc += 1; // increase counter by one byte // moved to end of BYTE
			opcode = value;
		}else if((sym = checkTable(symtbl, operand)) != NULL &&
		sym->type != REG && !(sym->value & ~0xFF)){
			// same idea, no bits set above bit 7
			//memLoc += 1; // increase counter by one byte // ``
			opcode = sym->value;
		}else if(sym == NULL && validLabel(operand)){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else{ // invalid label, unk, reg, invalid value
			err = OUT_BOUND;
		}
		memLoc += 1; // increment memLoc regardless to not cause false errors
		break;
	case END:
		// this has optional operand
		operand = getOperand(token);
		if(!operand.empty()){
			// has operand: make sure valid
			sym = checkTable(symtbl, operand);
			if(sym != NULL && (sym->type != LBL || sym->value < 0)){
				// if operand is in symbol table, and either it's not a
				// label or not positive
				err = END_ERR;
			}else if(sym != NULL && sym->type == LBL && sym->value > 0){
				// if operand is in symbol table, sym type is a label,
				// and value is positive
				START = sym->value; // set starting point to label's value
			}else if(sym == NULL && \
			(!validValue(operand) || (value = extractValue(operand)) < 0)){
				// operand not a symbol and
				// if operand is not a valid constant or if operand is a
				// negative value
				err = END_ERR;
			}else if(sym == NULL && validValue(operand) && \
			(value  > 0)){
				// if operand is not a symbol, is a valid value,
				// and the value is positive
				START = value;
			}
		}
		// finish first pass, ignoring any subsequent records
		END_OF_FIRST_PASS = true;
		break;
	case EQU:
		operand = getOperand(token);
		if(operand.empty()){
			// no operand
			err = MISS_OP;
		}
		if(label.empty()){
			// no label passed
			err = MISS_OP;
		}else{ // label provided
			if((sym = checkTable(symtbl, operand)) != NULL && \
			sym->type != UNK){
				// copy symbol with new name
				// modify value of label
				lblSym = checkTable(symtbl, label); // grab Symbol ptr
				lblSym->type = sym->type; // copy type
				lblSym->value = sym->value; // copy value
			}else if(sym != NULL && sym->type == UNK){
				err = FWD_REF;
			}else if(sym == NULL && validValue(operand)){
				// operand is valid value
				// modify label with operand as value
				lblSym = checkTable(symtbl, label); // grab Symbol ptr
				lblSym->type = LBL; // copy type
				lblSym->value = extractValue(operand); // copy value
			}else{
				// operand must be invalid label or value
				err = NOT_VAL;
			}
		}
		break;
	case ORG:
		operand = getOperand(token);
		if(operand.empty()){
			// no operand
			err = MISS_OP;
		}
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type != LBL){
			// in symbol table but not a label
			err = FWD_REF;
		}else if(sym != NULL && sym->type == LBL && sym->value < 0){
			// in symbol table and label, but negative
			err = POS_VAL;
		}else if(sym == NULL && \
		(!validValue(operand) || extractValue(operand) < 0)){
			// not in symbol table, but is either not a valid value
			// or is negative
			err = POS_VAL;
		}else{
			// update mem location
			memLoc = extractValue(operand);
		}
		break;
	case WORD:
		memLoc_ = memLoc; // save memLoc
		operand = getOperand(token);
		if(validValue(operand) && !((value = extractValue(operand)) & ~0xFFFF)){
			// valid value and 16 bits or less (no bits set higher than bit 15)
			//memLoc += 2; // increase counter by two bytes // moved to end of WORD
			opcode = value;
		}else if((sym = checkTable(symtbl, operand)) != NULL &&\
		sym->type != REG && !(sym->value & ~0xFFFF)){
			// same idea, no bits set above bit 15
			//memLoc += 2; // increase counter by two bytes // ``
			opcode = sym->value;
		}else if(sym == NULL && validLabel(operand)){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else{ // invalid label, unk, reg, invalid value
			err = NOT_VAL;
		}
		memLoc += 2; // increment memLoc regardless to not cause false errors
		break;
	default:
		break;
	}
	if((operand = getOperand(token)) != ""){ // extraneous operand(s)
		err = EXTR_OP;
	}
	if(!token.empty()){ // operands followed by non-comment garbage
		err = GARB;
	}
	pushRecord(records, lineNum, record.str(), err, tblSub, memLoc_, opcode);
	fpstate = CHECK_FIRST_TOKEN;
	return;
}
