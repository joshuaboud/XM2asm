/* File name: commands.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Necessary includes and definitions for commands.cpp.
 * 			Include this file to access the global command table
 * 			and its related functions.
 * Last Modified: 2019-05-30
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

#include <string.h> // for strcmp

#define CMD_TBL_SIZE 83

enum Command_type { DIR, INST };
enum Operand_type { NONE, CR_R, R_R, C_R, C, R, Opt, LBL10, LBL13, CEX, SA};

struct Command {
	string name;
	Command_type type;
	int opCnt;
	Operand_type ops;
};

extern Command commands[CMD_TBL_SIZE]; // array of instructions & directives

int checkTable(Command (& tbl)[CMD_TBL_SIZE], string & mnemonic);
// Returns positional subscript of array element by name,
// or -1 if element not found. Uses binary search.

bool validConstant(string operand);

string getOperand(string & operands);
// Extracts operands delimited by commas until end of record is reached.

#endif
