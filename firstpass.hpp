/* File name: firstpass.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Necessary includes and declarations for firstpass.cpp.
 * 			Inlude this file to call the first pass state machine.
 * Last Modified: 2019-06-03
 */

#ifndef FIRSTPASS_H
#define FIRSTPASS_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "commands.hpp"
#include "symbols.hpp"
#include "record.hpp"
#include "error.hpp"
#include "operands.hpp"
#include "main.hpp" // to modify starting location

enum FPState { CHECK_FIRST_TOKEN, CHECK_INST_DIR, CHECK_DIR, \
	CHECK_INST };	//	state machine control for first pass

void firstPassStateMachine(std::ifstream & source);
// enter state machine

void checkFirstToken(std::istringstream & record, std::string & token, int & tblSub);
// Reads first token of record, decide state change after identifying token
// as a label, an instruction, a directive, a comment, or an error.

void checkInstOrDir(std::istringstream & record, std::string & token, int & tblSub);
// Receives table index of inst or dir, changes state to process inst
// or dir. If not found, push error for label after label and return.

void checkInst(std::istringstream & record, std::string & token, int & tblSub, int & memLoc);
// Verifies operand(s) of instruction and adds to list of records with location counter.
// Starts off with empty error string. If an error is detected, the string is
// updated to contain a description of the error. At the end of the function,
// the record is pushed into the list of records. The error flag is set in the
// pushRecord() function based on whether or not the error string is empty.

void checkDir(std::istringstream & record, std::string & token, int & tblSub, int & memLoc, std::string label);
// Executes directive, reports any errors in operands or otherwise, and 
// adds to list of records. Modifies location counter for applicable directives.
// The error flag is set in the pushRecord() function based on whether or 
// not the error string is empty.

#endif
