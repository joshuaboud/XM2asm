/* File name: main.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Open the source module, list file, initialize global tables,
 * and call state machines for first and second pass. Create list file
 * after first pass completes.
 * Last Modified: 2019-06-02
 */

// uncomment following line to send output stdout
//#define DEBUG

#include "main.hpp"

unsigned int START = 0; // Starting memory location for loader

int main(int argc, char ** argv){
	// timestamp for list file
	time_t timestamp = chrono::system_clock::to_time_t(chrono::system_clock::now());
	
	initSymTbl(symtbl);
	initRecords(records);
	
	// File streams for input and output
	ifstream source;
	// Ensure file has opened properly
	if(argc != 2){
		cout << "Must open with XM2 source module! Exiting..." << endl;
		exit(1);
	}
	source.open(argv[1]);
	if(!source){
		cout << "File does not exist! Exiting..." << endl;
		exit(1);
	}
	
	// Prepare name of list file
	string srcName(argv[1]);
	// Truncate file extension
	string listFileName = srcName.substr(0, srcName.find('.'));
	listFileName += ".lis"; // append .lis
	
	#ifdef DEBUG
	#define listFile cout // print to console instead of list file
	#else
	ofstream listFile;
	// Open list file for writing
	listFile.open(listFileName);
	if(!listFile){
		cout << "Error opening list file! Exiting..." << endl;
		exit(1);
	}
	#endif
	
	// call first state machine
	firstPassStateMachine(source);
	
	// first pass exit actions:
	// create list file
	listFile << "X-Makina Assembler V 2.0" << endl;
	listFile << "File opened: \"" << argv[1] << "\"" << endl;
	listFile << "Time of execution: " << ctime(& timestamp) << endl;
	printRecords(listFile);
	listFile << endl;
	printSymTbl(listFile);
	listFile << "Starting memory location: 0x" << hex << START << endl;
	if(ERROR_FLAG){
		listFile << "Errors were detected. Stopping after first pass." << endl;
	}
	
	if(!ERROR_FLAG){
		cout << "First pass finished with no errors." << endl;
		listFile << "First pass finished with no errors." << endl;
		// call second pass here
	}else{
		// finished with errors
		cout << "First pass finished with one or more errors." << endl;
		cout << "Check \"" << listFileName << "\" for details." << endl;
	}
	
	// free symtbl
	destroySymTbl(symtbl);
	// free records
	destroyRecords(records);
	
	source.close();
	
	#ifndef DEBUG
	listFile.close();
	#endif
	
	return 0;
}
