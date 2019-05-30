#include "firstpass.hpp"

// Globals

FPState fpstate;

bool END_OF_FIRST_PASS;

int lineNum;
uint16_t memLoc;	//	line number counter and memory location

void firstPassStateMachine(ifstream & source){
	END_OF_FIRST_PASS = false;
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
			if(source.eof()){
				END_OF_FIRST_PASS = true;
				break;
			}
			// Get next record:
			getline(source, record);
			lineNum++;
			recordStream.clear();
			recordStream.str(record);
			checkFirstToken(recordStream, token, tblSub);
			break;
		case CHECK_INST_DIR:
			cout << "CHECK_INST_DIR" << endl;
			END_OF_FIRST_PASS = true;
			break;
		case CHECK_DIR:
			cout << "CHECK_DIR" << endl;
			END_OF_FIRST_PASS = true;
			break;
		case CHECK_INST:
			cout << "CHECK_INST" << endl;
			END_OF_FIRST_PASS = true;
			break;
		}
	}
}

void checkFirstToken(istringstream & record, string & token, int & tblSub){
	record >> token; // Get first token
	if(token[0] == ';'){ // ignore comments, no state change
		return;
	}else if(!validLabel(token)){
		string err = "ERROR: Invalid label.";
		pushRecord(records, lineNum, record.str(), err);
		return;
	}else if((tblSub = checkTable(commands, token)) > -1){
		switch(commands[tblSub].type){ // if true, inst or dir
		case DIR:
			fpstate = CHECK_DIR;
			return;
		case INST:
			fpstate = CHECK_INST;
			return;
		}
	} else { // must be valid label
		int symPos;
		if((symPos = checkTable(symtbl, token)) != -1){ // in sym table
			string err;
			switch(symtbl[symPos].type){
			case LBL: // double defined label
				err = "ERROR: Duplicate label definition.";
				pushRecord(records, lineNum, record.str(), err);
				return;
			case REG: // label is R0 - R7
				err = "ERROR: Label cannot be register.";
				pushRecord(records, lineNum, record.str(), err);
				return;
			case UNK: // forward reference, fill in data
				symtbl[symPos].value = memLoc;
				symtbl[symPos].type = LBL;
				fpstate = CHECK_INST_DIR;
				return;
			}
		} else { // add label to symbol table
			Symbol temp = { token, LBL, memLoc };
			symtbl.push_back(temp);
			fpstate = CHECK_INST_DIR;
			return;
		}
	}
}
