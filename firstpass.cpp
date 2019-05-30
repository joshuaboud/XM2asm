#include "firstpass.hpp"

// Globals

FPState fpstate;

bool END_OF_FIRST_PASS;

int lineNum, memLoc;	//	line number counter and memory location

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
				fpstate = END;
				break;
			}
			// Get next record:
			getline(source, record);
			lineNum++;
			pushRecord(records, lineNum, record);
			recordStream.clear();
			recordStream.str(record);
			checkFirstToken(recordStream, token, tblSub);
			break;
		case CHECK_INST_DIR:
			
			break;
		case CHECK_DIR:
			
			break;
		case CHECK_INST:
			cout << commands[tblSub].name << endl;
			break;
		case END:
			END_OF_FIRST_PASS = true;
			break;
		}
	}
}

void checkFirstToken(istringstream & record, string & token, int & tblSub){
	record >> token; // Get first token
	if(token[0] == ';'){ // ignore comments, no state change
		return;
	}else if((tblSub = checkTable(token)) > -1){ // if true, inst or dir
		switch(commands[tblSub].type){
		case DIR:
			fpstate = CHECK_DIR;
			return;
		case INST:
			fpstate = CHECK_INST;
			return;
		}
	}
}
