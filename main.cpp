#include "main.hpp"

int main(int argc, char ** argv){
	ifstream source;
	
	initSymTbl(symtbl);
	initRecords(records);
	
	//	Ensure file has opened properly
	if(argc != 2){
		cout << "Must open with XM2 source module! Exiting...\n";
		exit(1);
	}
	source.open(argv[1]);
	if(!source){
		cout << "File does not exist! Exiting...\n";
		exit(1);
	}
	
	// call state machine
	firstPassStateMachine(source);
	
	return 0;
}
