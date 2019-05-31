/* File name: main.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Open source module, initialize global tables and call state 
 * 			machines for first and second pass
 * Last Modified: 2019-05-30
 */

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
	
	cout << commands[83].name << endl;
	
	// call state machine
	firstPassStateMachine(source);
	
	if(!ERROR_FLAG){
		// call second pass here
	}
	
	// free symtbl
	destroySymTbl(symtbl);
	
	return 0;
}
