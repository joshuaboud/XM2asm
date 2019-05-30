#ifndef FIRSTPASS_H
#define FIRSTPASS_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

#include "commands.hpp"

enum FPState { CHECK_FIRST_TOKEN, CHECK_INST_DIR, CHECK_DIR, \
	CHECK_INST, END };	//	state machine control for first pass
	
extern FPState fpstate;

extern bool END_OF_FIRST_PASS;

extern vector<Record> records;	//	buffer of records to be printed

void firstPassStateMachine(ifstream & source);

void checkFirstToken(istringstream & source, string & token, int & tblSub);

#endif
