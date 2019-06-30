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
	time_t timestamp = 
	std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	
	initSymTbl(symtbl);
	initRecords(records);
	
	// File streams for input and output
	std::ifstream source;
	// Ensure file has opened properly
	if(argc != 2){
		std::cout << "Must open with XM2 source module! Exiting..." << std::endl;
		exit(1);
	}
	source.open(argv[1]);
	if(!source){
		std::cout << "File does not exist! Exiting..." << std::endl;
		exit(1);
	}
	
	// Prepare name of list file
	std::string srcName(argv[1]);
	// Truncate file extension
	std::string listFileName = srcName.substr(0, srcName.find_last_of('.'));
	listFileName += ".lis"; // append .lis
	
	#ifdef DEBUG
	#define listFile cout // print to console instead of list file
	#else
	std::ofstream listFile;
	// Open list file for writing
	listFile.open(listFileName);
	if(!listFile){
		std::cout << "Error opening list file! Exiting..." << std::endl;
		exit(1);
	}
	#endif
	
	// call first state machine
	firstPassStateMachine(source);
	
	// first pass exit actions:
	// create list file
	listFile << "X-Makina Assembler V 2.0" << std::endl;
	listFile << "File opened: \"" << srcName << "\"" << std::endl;
	listFile << "Time of execution: " << ctime(& timestamp) << std::endl;
	printRecords(listFile);
	listFile << std::endl;
	printSymTbl(listFile);
	listFile << "Starting memory location: 0x" << std::hex << START << std::endl;
	if(ERROR_FLAG){
		listFile << "Errors were detected. Stopping after first pass." << std::endl;
	}
	
	if(!ERROR_FLAG){
		std::cout << "First pass finished with no errors." << std::endl;
		listFile << "First pass finished with no errors." << std::endl;
		// call second pass here
	}else{
		// finished with errors
		std::cout << "First pass finished with one or more errors." << std::endl;
		std::cout << "Check \"" << listFileName << "\" for details." << std::endl;
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
