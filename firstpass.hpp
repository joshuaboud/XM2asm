#ifndef FIRSTPASS_H
#define FIRSTPASS_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

#include "commands.hpp"
#include "symbols.hpp"
#include "record.hpp"

enum FPState { CHECK_FIRST_TOKEN, CHECK_INST_DIR, CHECK_DIR, \
	CHECK_INST };	//	state machine control for first pass
	
extern FPState fpstate;

extern bool END_OF_FIRST_PASS;

void firstPassStateMachine(ifstream & source);
// enter state machine

void checkFirstToken(istringstream & source, string & token, int & tblSub);
// reads first token of record, decide state change

#endif
