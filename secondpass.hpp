/* File name: secondpass.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Necessary includes and declarations for secondpass.cpp.
 * 			Inlude this file to call the second pass state machine or
 * 			change the state variable.
 * Last Modified: 2019-07-01
 */
 
#ifndef SECONDPASS_H
#define SECONDPASS_H

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "record.hpp"
#include "symbols.hpp"
#include "commands.hpp"
#include "opcode.hpp"
#include "main.hpp" // for START (starting memloc)
#define NO_MEMLOC -1

enum SPState { CHK_FIRST_TOK, PROC_DIR, PROC_INST_OPS,
	GEN_S_RECS };
	
extern SPState spstate;

void secondPassStateMachine(std::string baseFileName);
// Controls each state function according to the state variable

void chkFirstTok(Record * record, std::istringstream & recordStream);
// finds instruction or directive of record, changes state to 
// procInstOps or procDir, respectively

void procDir(Record * record, std::istringstream & recordStream);
// for byte and word, generates "opcode", i.e. data

void procInstOps(Record * record, std::istringstream & recordStream);
// calls opcode generating function from opcode.cpp for instruction
// in record passed

void genSRecs(std::string baseFileName);
// generates an S Record file from the opcodes in the list of Record
// structs

#endif
