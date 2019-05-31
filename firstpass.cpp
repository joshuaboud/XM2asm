/* File name: firstpass.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: State machine that runs through each record of the source
 * 			file stream passed to it, builds a symbol table, and verifies
 * 			that there are no errors. All state changes are controlled
 * 			in this file.
 * Last Modified: 2019-05-30
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
			// enter statements:
			// Get next record:
			getline(source, record);
			if((record == "") && source.eof()){
				END_OF_FIRST_PASS = true;
				break;
			}
			
			lineNum++;
			recordStream.clear();
			recordStream.str(record);
			// do:
			checkFirstToken(recordStream, token, tblSub);
			break;
		case CHECK_INST_DIR:
			checkInstOrDir(recordStream, token, tblSub);
			break;
		case CHECK_DIR:
			cout << "CHECK_DIR" << endl;
			END_OF_FIRST_PASS = true;
			break;
		case CHECK_INST:
			checkInst(recordStream, token, tblSub, memLoc);
			// exit statements:
			memLoc += 2;
			break;
		}
	}
	// first pass exit actions:
	printRecords(cout, records);
	printSymTbl(cout, symtbl);
	if(ERROR_FLAG){
		cout << "Errors were detected. Stopping after first pass." << endl;
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
	}else if(validLabel(token) == 0){
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
			pushRecord(records, lineNum, record.str());
			fpstate = CHECK_INST_DIR;
			return;
		}
	}
}

void checkInstOrDir(istringstream & record, string & token, int & tblSub){
	token = getNextToken(record); // Get first token
	if(token.empty()){ // end of record
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
			// operand is a register
			err = "ERROR: Instruction takes constant as first operand.";
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
	case LBL10:
		if((sym = checkTable(symtbl, operand)) == NULL && validLabel(operand) != 1){
			// not a label or valid label to make forward reference
			err = "ERROR: Operand must be LABEL10.";
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
	case LBL13:
		
		break;
	case CEX:
		
		break;
	case SA:
		
		break;
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

