/* File name: main.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Open the source module, list file, initialize global tables,
 * and call state machines for first and second pass. Create list file
 * after first pass completes.
 * Last Modified: 2019-06-31
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
	
	// Prepare base file name
	std::string srcName(argv[1]);
	// Truncate file extension
	std::string baseFileName = srcName.substr(0, srcName.find_last_of('.'));
	
	// call first state machine
	firstPassStateMachine(source);
	
	// first pass exit actions:
	// create list file
	printListFile(baseFileName, timestamp, srcName, FP);
	
	
	if(!ERROR_FLAG){
		// call second pass here
		secondPassStateMachine(baseFileName);
		printListFile(baseFileName, timestamp, srcName, SP);
	}
	
	// free symtbl
	destroySymTbl(symtbl);
	// free records
	destroyRecords(records);
	
	source.close();
	
	return 0;
}

void printListFile(std::string baseFileName, time_t timestamp,
std::string srcName, Pass pass){
	#ifdef DEBUG
	#define listFile cout // print to console instead of list file
	#else
	std::string listFileName = baseFileName + ".lis";
	std::ofstream listFile;
	// Open list file for writing
	listFile.open(listFileName, std::ios::out | std::ios::trunc);
	if(!listFile){
		std::cout << "Error opening list file! Exiting..." << std::endl;
		exit(1);
	}
	#endif
	listFile << "X-Makina Assembler V 1.1" << std::endl;
	listFile << "File opened: \"" << srcName << "\"" << std::endl;
	listFile << "Time of execution: " << ctime(& timestamp) << std::endl;
	printRecords(listFile);
	listFile << std::endl;
	printSymTbl(listFile);
	listFile << "Starting memory location: 0x" << std::hex << START <<
	std::endl;
	
	switch(pass){
	case FP:
		if(ERROR_FLAG){
			listFile << "Errors were detected. Stopping after first pass." <<
			std::endl;
			// finished with errors
			std::cout << "First pass finished with one or more errors." <<
			std::endl;
			std::cout << "Check \"" << listFileName << "\" for details." <<
			std::endl;
		}else{
			listFile << "First pass finished with no errors." << std::endl;
			std::cout << "First pass finished with no errors." << std::endl;
		}
		break;
	case SP:
		if(ERROR_FLAG){
			listFile << "Errors were detected during second pass." <<
			std::endl;
			// finished with errors
			std::cout << "Second pass finished with one or more errors." <<
			std::endl;
			std::cout << "Check \"" << listFileName << "\" for details." <<
			std::endl;
		}else{
			listFile << "First and second pass finished with no errors." <<
			std::endl;
			std::cout << "Second pass finished with no errors." << std::endl;
		}
		break;
	default:
		break;
	}
	
	#ifndef DEBUG
	listFile.close();
	#endif
}
