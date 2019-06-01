/* File name: firstpass.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: State machine that runs through each record of the source
 * 			file stream passed to it, builds a symbol table, and verifies
 * 			that there are no errors. All state changes are controlled
 * 			in this file.
 * Last Modified: 2019-05-31
 */

#include "firstpass.hpp"

// Globals

FPState fpstate;

bool END_OF_FIRST_PASS;
bool ERROR_FLAG;

int lineNum;
uint16_t memLoc;	//	line number counter and memory location

void firstPassStateMachine(ifstream & source){
	END_OF_FIRST_PASS = false;
	ERROR_FLAG = false;
	fpstate = CHECK_FIRST_TOKEN;
	lineNum = 0;
	memLoc = 0;
	
	string record;
	istringstream recordStream;
	string token;
	
	int tblSub;	//	command table subscript
	
	while(!END_OF_FIRST_PASS){
		switch(fpstate){
		case CHECK_FIRST_TOKEN:
			// Get next record:
			getline(source, record);
			if((record == "") && source.eof()){
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
			checkInstOrDir(recordStream, token, tblSub);
			break;
		case CHECK_DIR: // verify operand of directive
			cout << "CHECK_DIR" << endl;
			END_OF_FIRST_PASS = true;
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
	token = getNextToken(record);
	if(record.str()[0] == ';'){ // just a comment
		pushRecord(records, lineNum, record.str());
		return;
	}else if(token.empty()){ // empty record
		string err = "ERROR: Empty record.";
		pushRecord(records, lineNum, record.str(), err);
		return;
	}else if(validLabel(token) != 1){
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
	} else { // must be valid label
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
				pushRecord(records, lineNum, record.str());
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
	string err;
	Symbol * sym;
	string operand;
	
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
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}else if(sym == NULL && validLabel(operand) == 1){ // valid forward ref
			// add label to symtbl as unkown type
			Symbol temp;
			temp.name = operand;
			temp.type = UNK;
			temp.value = 0;
			pushSymbol(symtbl, temp);
		}else if(sym != NULL && sym->type == LBL){ // if known label
			if(!validConstant(sym->value)){ // check that value is const
				err = "ERROR: Instruction takes constant as first operand.";
				pushRecord(records, lineNum, record.str(), err, memLoc);
				fpstate = CHECK_FIRST_TOKEN;
				return;
			}
		}
		// if here, first operand is verified as a constant, register, or label.
		// verify second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) == NULL || sym->type != REG){
			// if operand is not a register
			err = "ERROR: Instruction takes register as second operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operands followed by non-comment garbage.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		// valid operands
		// push with no error
		pushRecord(records, lineNum, record.str(), "", memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	case R_R:
		// verify first operand
		if((sym = checkTable(symtbl, operand)) == NULL || sym->type != REG){
			// if operand is not a register
			err = "ERROR: Instruction takes register as first operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		// verify second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) == NULL || sym->type != REG){
			// if operand is not a register
			err = "ERROR: Instruction takes register as second operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operands followed by non-comment garbage.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		// valid operands
		// push with no error
		pushRecord(records, lineNum, record.str(), "", memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	case C_R:
		// first operand
		if((sym = checkTable(symtbl, operand)) == NULL && !validConstant(operand)){
			// if operand is not a label/UNK and not a valid constant
			err = "ERROR: Instruction takes constant as first operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}else if(sym != NULL && sym->type == REG){
			// operand is a register
			err = "ERROR: Instruction takes constant as first operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}else if(sym != NULL && sym->type == LBL){ // known label
			if(!validConstant(sym->value)){ // check that value is const
				err = "ERROR: Instruction takes constant as first operand.";
				pushRecord(records, lineNum, record.str(), err, memLoc);
				fpstate = CHECK_FIRST_TOKEN;
				return;
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
		if((sym = checkTable(symtbl, operand)) == NULL || sym->type != REG){
			// if operand is not a register
			err = "ERROR: Instruction takes register as second operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operands followed by non-comment garbage.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		// valid operands
		// push with no error
		pushRecord(records, lineNum, record.str(), "", memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	case C:
		if((sym = checkTable(symtbl, operand)) == NULL && !validConstant(operand)){
			// if operand is not a label/UNK and not a valid constant
			err = "ERROR: Instruction takes constant as operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}else if(sym != NULL && sym->type == REG){
			// if operand is a register
			err = "ERROR: Instruction takes constant as first operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}else if(sym != NULL && !validConstant(sym->value)){ // known label
			// if label value is not a constant
			err = "ERROR: Instruction takes constant as first operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
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
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		// valid operands
		// push with no error
		pushRecord(records, lineNum, record.str(), "", memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	case R:
		if((sym = checkTable(symtbl, operand)) == NULL || sym->type != REG){
			// if operand is not a register
			err = "ERROR: Instruction takes register as operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes one operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		// valid operands
		// push with no error
		pushRecord(records, lineNum, record.str(), "", memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	case V: // value
		if(!validValue(operand)){ // if not a valid value
			err = "ERROR: Operand must be value.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}else if((sym = checkTable(symtbl, operand)) == NULL && \
		validLabel(operand) != 1 && !validValue(operand)){
			// not a label or valid label to make forward reference
			err = "ERROR: Operand must be value.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
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
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes one operand.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operand followed by non-comment garbage.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		// valid operands
		// push with no error
		pushRecord(records, lineNum, record.str(), "", memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	case CEX:
		
		break;
	case SA:
		
		break;
	case ST:
		// check that first operand is register
		if((sym = checkTable(symtbl, operand)) == NULL || sym->type != REG){
			// if operand is not a register
			err = "ERROR: Instruction takes register as first operand. (Offset only on second)";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
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
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type != REG){
			// if in symtbl but not as register
			err = "ERROR: Operand must be register.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}else if(sym == NULL){
			// not in symtbl, definitely not register
			err = "ERROR: Operand must be register.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operands followed by non-comment garbage.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		// valid operands
		// push with no error
		pushRecord(records, lineNum, record.str(), "", memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
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
		if((sym = checkTable(symtbl, operand)) != NULL && sym->type != REG){
			// if in symtbl but not as register
			err = "ERROR: Operand must be register.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}else if(sym == NULL){
			// not in symtbl, definitely not register
			err = "ERROR: Operand must be register.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		// check second operand
		operand = getOperand(token);
		if((sym = checkTable(symtbl, operand)) == NULL || sym->type != REG){
			// if operand is not a register
			err = "ERROR: Instruction takes register as second operand. (Offset only on first)";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if((operand = getOperand(token)) != ""){ // extraneous operand(s)
			err = "ERROR: Instruction only takes two operands.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		if(!token.empty()){ // operands followed by non-comment garbage
			err = "ERROR: Operands followed by non-comment garbage.";
			pushRecord(records, lineNum, record.str(), err, memLoc);
			fpstate = CHECK_FIRST_TOKEN;
			return;
		}
		// valid operands
		// push with no error
		pushRecord(records, lineNum, record.str(), "", memLoc);
		fpstate = CHECK_FIRST_TOKEN;
		return;
	}
}

string getNextToken(istringstream & record){
	string token;
	record >> token;
	if(token.empty() || token[0] == ';'){
		// end of record
		record.clear(); // clear error bits to allow continued reading
		return "";
	}
	return token;
}

