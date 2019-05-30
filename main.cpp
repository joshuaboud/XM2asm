#include "main.hpp"

// Globals

vector<Symbol> symtbl = \
		{	{"R0", REG, 0},
			{"R1", REG, 1},
			{"R2", REG, 2},
			{"R3", REG, 3},
			{"R4", REG, 4},
			{"R5", REG, 5},
			{"R6", REG, 6},
			{"R7", REG, 7}	};
			
// holds all records for printing into list file:
vector<Record> records;

int main(int argc, char ** argv){
	ifstream source;
	
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
