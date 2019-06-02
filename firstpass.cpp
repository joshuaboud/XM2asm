/* File name: firstpass.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: State machine that runs through each record of the source
 * 			file stream passed to it, builds a symbol table, and verifies
 * 			that there are no errors. All state changes are controlled
 * 			in this file.
 * Last Modified: 2019-06-01
 */

#include "firstpass.hpp"

// Globals

FPState fpstate;

bool END_OF_FIRST_PASS;
bool ERROR_FLAG;

int lineNum;
uint16_t memLoc;	//	line number counter and memory location

void firstPassStateMachine(ifstream & source){
	// Initializes and controls state functions of state machine
	END_OF_FIRST_PASS = false;
	ERROR_FLAG = false;
	fpstate = CHECK_FIRST_TOKEN;
	lineNum = 0;
	memLoc = 0;
	
	string record;
	istringstream recordStream;
	string token;
	
	string label; // hold label for directive
	
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

void checkFirstToken(istringstream & record, string & token, int & tblSub){
	// Grabs first token from record, decides what state to jump to
	// after identifying token. If errors are encountered, the whole record
	// is saved with an error description and the state machine moves onto
	// the next record.
	token = getNextToken(record);
	if(record.str()[0] == ';'){ // just a comment
		pushRecord(records, lineNum, record.str());
		return;
	}else if(token.empty()){ // empty record
		string err = "ERROR: Empty record.";
		pushRecord(records, lineNum, record.str(), err);
		return;
	}else if(validLabel(token) == 0){ // invalid label, valid inst
		string err = "ERROR: Invalid label.";
		pushRecord(records, lineNum, record.str(), err);
		return;
	}else if((tblSub = checkTable(commands, token)) > -1){
		switch(commands[tblSub].type){ // ^if true, inst or dir
		case DIR:
			fpstate = CHECK_DIR;
			return;
		case INST:
			fpstate = CHECK_INST;
			return;
		}
	}else if(validLabel(token) == 1){ // must be valid label
		Symbol * symPtr;
		if((symPtr = checkTable(symtbl, token)) != NULL){ // in sym table
			string err;
			switch(symPtr->type){
			case LBL: // double defined label
				err = "ERROR: Duplicate label definition.";
				pushRecord(records, lineNum, record.str(), err);
				return;
			case REG: // label is R0 - R7
				err = "ERROR: Label cannot be register.";
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
		string err = "ERROR: Invalid label.";
		pushRecord(records, lineNum, record.str(), err);
		return;
	}
	// end of first pass, back to main
}

void checkInstOrDir(istringstream & record, string & token, int & tblSub){
	token = getNextToken(record); // Get first token
	if(token.empty()){ // end of record
		// save label
		pushRecord(records, lineNum, record.str());
		fpstate = CHECK_FIRST_TOKEN;
		return;
	}else if((tblSub = checkTable(commands, token)) != -1){ // inst or dir
		switch(commands[tblSub].type){
		case DIR:
			fpstate = CHECK_DIR;
			return;
		case INST:
			fpstate = CHECK_INST;
			return;
		}
	} else { // error, cannot have label following label
		string err = "ERROR: Label cannot follow label on same record.";
		pushRecord(records, lineNum, record.str(), err);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	}
}

void checkInst(istringstream & record, string & token, int & tblSub, uint16_t & memLoc){
	token = getNextToken(record); // should hold all operands
	
	string err = ""; // default to no error
	
	Symbol * sym;
	string operand;
	
	// for CEX:
	string cexOp1[17] = {"EQ","NE","CS","HS","CC","LO","MI","PL","VS","VC",\
	"HI","LS","GE","LT","GT","LE","AL"};
	bool cexOp;
	
	if(token.empty()){
		err = "ERROR: Missing operand(s).";
		pushRecord(records, lineNum, record.str(), err, memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	}
	if(memLoc % 2 != 0){
		err = "ERROR: Instruction not on even mem addr. Use ALIGN.";
		pushRecord(records, lineNum, record.str(), err, memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	}
	
	// get first operand
	operand = getOperand(token);
	
	switch(commands[tblSub].ops){
	case CR_R:
		// verify first operand
		if((sym = checkTable(symtbl, operand)) == NULL && \
		!(validLabel(operand) == 1 || validConstant(operand))){
			// if not a register or label and not a valid label or constant
			err = "ERROR: Instruction takes constant or register as first operand.";
		}else if(sym == NULL && validLabel(operand) == 1){ // valid forward ref
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && \
		!(validLabel(operand) == 1 || validConstant(operand))){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in first operand.";
		}else if(sym != NULL && sym->type == LBL){ // if known label
			if(!validConstant(sym->value)){ // check that value is const
				err = "ERROR: Instruction takes constant as first operand.";
			}
		}
		// if here, first operand is verified as a constant, register, or label.
		// verify second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unkown
			err = "ERROR: Instruction takes register as second operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in second operand.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operands followed by non-comment garbage.";
		}
		break;
	case R_R:
		// verify first operand
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unkown
			err = "ERROR: Instruction takes register as first operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in first operand.";
		}
		// verify second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unkown
			err = "ERROR: Instruction takes register as second operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in second operand.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operands followed by non-comment garbage.";
		}
		break;
	case C_R:
		// first operand
		if((sym = checkTable(symtbl, operand)) == NULL && !validConstant(operand)){
			// if operand is not a label/UNK and not a valid constant
			err = "ERROR: Instruction takes constant as first operand.";
		}else if(sym != NULL && sym->type == REG){
			// operand is a register
			err = "ERROR: Instruction takes constant as first operand.";
		}else if(sym != NULL && sym->type == LBL){ // known label
			if(!validConstant(sym->value)){ // check that value is const
				err = "ERROR: Instruction takes constant as first operand.";
			}
		}else if(sym == NULL && validLabel(operand) == 1){ // valid forward ref
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}
		// first operand is either constant or label
		// verify second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type != REG){
			// if operand is not a register
			err = "ERROR: Instruction takes register as second operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in second operand.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operands followed by non-comment garbage.";
		}
		break;
	case C:
		if((sym = checkTable(symtbl, operand)) == NULL && !validConstant(operand)){
			// if operand is not a label/UNK and not a valid constant
			err = "ERROR: Instruction takes constant as operand.";
		}else if(sym != NULL && sym->type == REG){
			// if operand is a register
			err = "ERROR: Instruction takes constant as first operand.";
		}else if(sym != NULL && !validConstant(sym->value)){ // known label
			// if label value is not a constant
			err = "ERROR: Instruction takes constant as first operand.";
		}else if(sym == NULL && validLabel(operand)){
			// add to symbol table as forward reference
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes one operand.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
		}
		break;
	case R:
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = "ERROR: Instruction takes register as operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in operand.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes one operand.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
		}
		break;
	case V: // value
		if(!validValue(operand) && validLabel(operand) != 1){
			// if not a valid value and not a valid label
			err = "ERROR: Operand must be value.";
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		validLabel(operand) != 1 && !validValue(operand)){
			// not a label or valid label to make forward reference
			err = "ERROR: Operand must be value.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = "ERROR: Operand must be value.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes one operand.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
		}
		break;
	case CEX:
		cexOp = false;
		for(int i = 0; i < 17; i++){
			if(operand == cexOp1[i]){
				cexOp = true;
				break;
			}
		}
		if(!cexOp){
			// failed check against valid optype1's
			err = "ERROR: Instruction takes CEX flag as first operand.";
		}
		// check second operand
		operand = getOperand(token);
		if(!validValue(operand) && validLabel(operand) != 1){
			// if not a valid value and not a valid label
			err = "ERROR: Second operand must be value.";
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		validLabel(operand) != 1 && !validValue(operand)){
			// not a label or valid label to make forward reference
			err = "ERROR: Operand must be value or valid label.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = "ERROR: Operand must be value.";
		}else if(sym != NULL && sym->type == LBL && \
		(sym->value > 7 || sym->value < 0)){
			// value of symbol out of bounds
			err = "ERROR: Operand must be value between 0 and 7 (inclusive).";
		}else if(validValue(operand) && \
		(extractValue(operand) > 7 || extractValue(operand) < 0)){
			// value out of bounds
			err = "ERROR: Operand must be value between 0 and 7 (inclusive).";
		}
		// check third operand
		operand = getOperand(token);
		if(!validValue(operand) && validLabel(operand) != 1){
			// if not a valid value and not a valid label
			err = "ERROR: Second operand must be value.";
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		validLabel(operand) != 1 && !validValue(operand)){
			// not a label or valid label to make forward reference
			err = "ERROR: Operand must be value or valid label.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = "ERROR: Operand must be value.";
		}else if(sym != NULL && sym->type == LBL && \
		(sym->value > 7 || sym->value < 0)){
			// value of symbol out of bounds
			err = "ERROR: Operand must be value between 0 and 7 (inclusive).";
		}else if(validValue(operand) && \
		(extractValue(operand) > 7 || extractValue(operand) < 0)){
			// value out of bounds
			err = "ERROR: Operand must be value between 0 and 7 (inclusive).";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes three operands.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
		}
		break;
	case SA:
		if(!validValue(operand) && validLabel(operand) != 1){
			// if not a valid value and not a valid label
			err = "ERROR: Operand must be value.";
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		validLabel(operand) != 1 && !validValue(operand)){
			// not a label or valid label to make forward reference
			err = "ERROR: Operand must be value.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = "ERROR: Operand must be value.";
		}else if(sym != NULL && (sym->value < 0 || sym->value > 15)){
			// value out of bounds
			err = "ERROR: Value must be between 0 and 15 (inclusive).";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}else if(validValue(operand) && \
		(extractValue(operand) < 0 || extractValue(operand) > 15)){
			// value out of bounds
			err = "ERROR: Value must be between 0 and 15 (inclusive).";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operands followed by non-comment garbage.";
		}
		break;
	case ST:
		// check that first operand is register
		if((sym = checkTable(symtbl, operand)) == NULL && sym->type == LBL){
			// if operand is not a register
			err = "ERROR: Instruction takes register as first operand. (Offset only on second)";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in first operand. (Offset only on second)";
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
			err = "ERROR: Instruction takes register as second operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in operand.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operands followed by non-comment garbage.";
		}
		break;
	case LD:
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
			err = "ERROR: Instruction takes register as first operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in operand.";
		}
		// check second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = "ERROR: Instruction takes register as second operand. (Offest only on first)";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in operand.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operands followed by non-comment garbage.";
		}
		break;
	case LDR:
		// check first operand
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = "ERROR: Instruction takes register as first operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in operand.";
		}
		// check second operand
		operand = getOperand(token);
		if(!validValue(operand) && validLabel(operand) != 1){
			// if not a valid value and not a valid label
			err = "ERROR: Second operand must be value.";
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		validLabel(operand) != 1 && !validValue(operand)){
			// not a label or valid label to make forward reference
			err = "ERROR: Second operand must be value.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = "ERROR: Second operand must be value.";
		}
		// check third operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = "ERROR: Instruction takes register as third operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in operand.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes three operands.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
		}
		break;
	case STR:
		// check first operand
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = "ERROR: Instruction takes register as first operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in operand.";
		}
		// check second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type == LBL){
			// if operand is not a register or unk
			err = "ERROR: Instruction takes register as second operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in operand.";
		}
		// check third operand
		operand = getOperand(token);
		if(!validValue(operand) && validLabel(operand) != 1){
			// if not a valid value and not a valid label
			err = "ERROR: Third operand must be value.";
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		validLabel(operand) != 1 && !validValue(operand)){
			// not a label or valid label to make forward reference
			err = "ERROR: Third operand must be value.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = "ERROR: Third operand must be value.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes three operands.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
		}
		break;
	case BRA:
		if(!validValue(operand) && validLabel(operand) != 1){
			// if not a valid value and not a valid label
			err = "ERROR: Operand must be value.";
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		validLabel(operand) != 1 && !validValue(operand)){
			// not a label or valid label to make forward reference
			err = "ERROR: Operand must be value.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = "ERROR: Operand must be value.";
		}else if(sym != NULL && sym->type == LBL && \
		(sym->value > 1022 || sym->value < -1024 || sym->value % 2 != 0)){
			// value of symbol out of bounds or odd
			err = "ERROR: Operand must be value between -1024 and 1022 (inclusive) and even.";
		}else if(validValue(operand) && \
		(extractValue(operand) > 1022 || extractValue(operand) < -1024 ||\
		extractValue(operand) % 2 != 0)){
			// value out of bounds or odd
			err = "ERROR: Operand must be value between -1024 and 1022 (inclusive) and even.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes one operand.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
		}
		break;
	case BRA13:
		if(!validValue(operand) && validLabel(operand) != 1){
			// if not a valid value and not a valid label
			err = "ERROR: Operand must be value.";
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		validLabel(operand) != 1 && !validValue(operand)){
			// not a label or valid label to make forward reference
			err = "ERROR: Operand must be value.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = "ERROR: Operand must be value.";
		}else if(sym != NULL && sym->type == LBL && \
		(sym->value > 8190 || sym->value < -8192 || sym->value % 2 != 0)){
			// value of symbol out of bounds or odd
			err = "ERROR: Operand must be value between -8192 and 8190 (inclusive) and even.";
		}else if(validValue(operand) && \
		(extractValue(operand) > 8190 || extractValue(operand) < -8192 ||\
		extractValue(operand) % 2 != 0)){
			// value out of bounds
			err = "ERROR: Operand must be value between -8192 and 8190 (inclusive) and even.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes one operand.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
		}
		break;
	case V_R:
		// check first operand
		if(!validValue(operand) && validLabel(operand) != 1){
			// if not a valid value and not a valid label
			err = "ERROR: First operand must be value.";
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		validLabel(operand) != 1 && !validValue(operand)){
			// not a label or valid label to make forward reference
			err = "ERROR: First operand must be value.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == REG){
			// if symbol is in symtbl but not a label or unk
			err = "ERROR: First operand must be value.";
		}
		//check second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type != REG){
			// if operand is not a register
			err = "ERROR: Instruction takes register as second operand.";
		}else if(sym == NULL && validLabel(operand) == 1){
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym == NULL && validLabel(operand) != 1){ // invalid lbl
			// if not a register or label and not a valid label or constant
			err = "ERROR: Invalid label in second operand.";
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
		}
		break;
	default:
		err = "This shouldn't happen, instruction has unhandled operand type.";
		break;
	}
	// end of state, push record with either empty err or error description
	pushRecord(records, lineNum, record.str(), err, memLoc);
	fpstate = CHECK_FIRST_TOKEN;
	return;
}

void checkDir(istringstream & record, string & token, int & tblSub, uint16_t & memLoc, string label){
	token = getNextToken(record); // should hold operand, if any
	string operand;
	int value;
	Symbol * sym;
	Symbol * lblSym; // for EQU to copy labels
	string err = "";
	if(commands[tblSub].name == "ALIGN"){
		if(memLoc%2 != 0){
			// memLoc odd, increment by 1
			memLoc++;
		}
	}else if(commands[tblSub].name == "BSS"){
		// if a label is present, it will already be assiciated with
		// block from checkFirstToken().
		operand = getOperand(token);
		if(validValue(operand) && extractValue(operand) >= 0){
			value = extractValue(operand);
			memLoc += value;
		}else if((sym = checkTable(symtbl, operand)) != NULL &&\
		sym->type == LBL && sym->value >= 0){
			value = sym->value;
			memLoc += value;
		}else{ // invalid label, unk, reg, invalid value, or negative
			err = "ERROR: Operand must be positive value of BSS size.";
		}
	}else if(commands[tblSub].name == "BYTE"){
		operand = getOperand(token);
		if(validValue(operand) && !(extractValue(operand) & 0xFF00)){
			// valid value and 8 bits or less (no bits set higher than bit 7)
			//memLoc += 1; // increase counter by one byte // moved to end of BYTE
		}else if((sym = checkTable(symtbl, operand)) != NULL &&\
		sym->type == LBL && !(sym->value & 0xFF00)){
			// same idea, no bits set above bit 7
			//memLoc += 1; // increase counter by one byte // ``
		}else{ // invalid label, unk, reg, invalid value
			err = "ERROR: Operand must be 8 bit value.";
		}
		memLoc += 1; // increment memLoc regardless to not cause false errors
	}else if(commands[tblSub].name == "END"){
		// this has optional operand
		operand = getOperand(token);
		if(!operand.empty()){
			// has operand: make sure valid
			sym = checkTable(symtbl, operand);
			err = ""; // ensure err is empty
			if(sym != NULL && (sym->type != LBL || sym->value < 0)){
				// if operand is in symbol table, and either it's not a
				// label or not positive
				err = "ERROR: Operand after END must refer to label or mem location.";
			}else if(sym != NULL && sym->type == LBL && sym->value > 0){
				// if operand is in symbol table, sym type is a label,
				// and value is positive
				START = sym->value; // set starting point to label's value
			}else if(sym == NULL && \
			(!validValue(operand) || extractValue(operand) < 0)){
				// operand not a symbol and
				// if operand is not a valid constant or if operand is a
				// negative value
				err = "ERROR: Operand after END must be valid memory location.";
			}else if(sym == NULL && validConstant(operand) && \
			((value = extractValue(operand)) > 0)){
				// if operand is not a symbol, is a valid value,
				// and the value is positive
				START = value;
			}
		}
		// finish first pass, ignoring any subsequent records
		END_OF_FIRST_PASS = true;
	}else if(commands[tblSub].name == "EQU"){
		operand = getOperand(token);
		if(operand.empty()){
			// no operand
			err = "ERROR: Directive EQU requires an operand.";
		}
		if(label.empty()){
			// no label passed
			err = "ERROR: Directive EQU requires a label.";
		}else{ // label provided
			if((sym = checkTable(symtbl, operand)) != NULL && \
			sym->type != UNK){
				// copy symbol with new name
				// modify value of label
				lblSym = checkTable(symtbl, label); // grab Symbol ptr
				lblSym->type = sym->type; // copy type
				lblSym->value = sym->value; // copy value
			}else if(sym != NULL && sym->type == UNK){
				err = "ERROR: Cannot equate to unkown label.";
			}else if(sym == NULL && validValue(operand)){
				// operand is valid value
				// modify label with operand as value
				lblSym = checkTable(symtbl, label); // grab Symbol ptr
				lblSym->type = LBL; // copy type
				lblSym->value = extractValue(operand); // copy value
			}else{
				// operand must be invalid label or value
				err = "ERROR: Operand must be valid symbol or value.";
			}
		}
	}else if(commands[tblSub].name == "ORG"){
		operand = getOperand(token);
		if(operand.empty()){
			// no operand
			err = "ERROR: Directive ORG requires an operand.";
		}
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type != LBL){
			// in symbol table but not a label
			err = "ERROR: Operand cannot be register or unkown symbol.";
		}else if(sym != NULL && sym->type == LBL && sym->value < 0){
			// in symbol table and label, but negative
			err = "ERROR: ORG requires positive value.";
		}else if(sym == NULL && \
		(!validValue(operand) || extractValue(operand) < 0)){
			// not in symbol table, but is either not a valid value
			// or is negative
			err = "ERROR: ORG requires positive value.";
		}else{
			// update mem location
			memLoc = extractValue(operand);
		}
	}else if(commands[tblSub].name == "WORD"){
		if(memLoc % 2 != 0){
			// 16 bit value should fall on even byte
			err = "ERROR: 16 bit word should fall on even byte. Use ALIGN.";
		}
		operand = getOperand(token);
		if(validValue(operand) && !(extractValue(operand) & 0xFFFF0000)){
			// valid value and 16 bits or less (no bits set higher than bit 15)
			//memLoc += 2; // increase counter by two bytes // moved to end of WORD
		}else if((sym = checkTable(symtbl, operand)) != NULL &&\
		sym->type == LBL && !(sym->value & 0xFFFF0000)){
			// same idea, no bits set above bit 15
			//memLoc += 2; // increase counter by two bytes // ``
		}else{ // invalid label, unk, reg, invalid value
			err = "ERROR: Operand must be 16 bit value.";
		}
		memLoc += 2; // increment memLoc regardless to not cause false errors
	}
	if((operand = getOperand(token)) != ""){ // extraneous operand(s)
		err = "ERROR: Too many operand(s).";
	}
	if(!token.empty()){ // operands followed by non-comment garbage
		err = "ERROR: Statement followed by non-comment garbage.";
	}
	pushRecord(records, lineNum, record.str(), err);
	fpstate = CHECK_FIRST_TOKEN;
	return;
}
