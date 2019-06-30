#include "secondpass.hpp"

SPState spstate;

bool END_OF_SECOND_PASS;

void secondPassStateMachine(){
	spstate = CHK_FIRST_TOK;
	END_OF_SECOND_PASS = false;
	
	Record * record = NULL;
	std::istringstream recordStream;
	
	while(!END_OF_SECOND_PASS){
		switch(spstate){
		case CHK_FIRST_TOK:
			if(!record){
				record = records_end;
			} else {
				record = record->next;
				if(!record){
					END_OF_SECOND_PASS = true;
					break;
				}
			}
			chkFirstTok(record, recordStream);
			break;
		case PROC_DIR:
			procDir();
			break;
		case PROC_INST_OPS:
			procInstOps();
			break;
		case GEN_OP_CODE:
			genOpCode();
			break;
		case GEN_S_RECS:
			genSRecs();
			break;
		default:
			break;
		}
	}
}

void chkFirstTok(Record * record, std::istringstream & recordStream){
	std::cout << record->record << std::endl;
}

void procDir(){
	
}

void procInstOps(){
	
}

void genOpCode(){
	
}

void genSRecs(){
	
}
