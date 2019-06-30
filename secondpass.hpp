/* File name: secondpass.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Necessary includes and declarations for secondpass.cpp.
 * 			Inlude this file to call the second pass state machine or
 * 			change the state variable.
 * Last Modified: 2019-06-30
 */
 
#ifndef SECONDPASS_H
#define SECONDPASS_H

#include <string>
#include <sstream>
#include <iostream>

#include "record.hpp"

enum SPState { CHK_FIRST_TOK, PROC_DIR, PROC_INST_OPS, GEN_OP_CODE,
	GEN_S_RECS };

void secondPassStateMachine();

void chkFirstTok(Record * record, std::istringstream & recordStream);

void procDir();

void procInstOps();

void genOpCode();

void genSRecs();

#endif
