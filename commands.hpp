/* File name: commands.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Necessary includes and declarations for commands.cpp.
 * 			Include this file to access the global command table
 * 			and its related functions.
 * Last Modified: 2019-06-03
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

#include <string.h> // for strcmp

#define CMD_TBL_SIZE 83

enum Command_type { DIR, INST };
enum Operand_type { NONE, CR_R, R_R, C, R, Opt, V, CEX,\
	 SA, LD, ST, LDR, STR, BRA, BRA13, V_R };

struct Command {
	string name;
	Command_type type;
	int opCnt;
	Operand_type ops;
};

extern Command commands[CMD_TBL_SIZE]; // array of instructions & directives

int checkTable(Command (& tbl)[CMD_TBL_SIZE], string mnemonic);
// Returns positional subscript of array element by name,
// or -1 if element not found. Uses binary search.

bool validValue(string operand);
bool validValue(uint16_t operand);
// Returns true if operand passed is a valid value:
// [ # + (-) + { [0..9] } | $ + (-) + { [0..A] } | ' + char + ' ]
// Overloaded for checking against symbol - always returns true because
// if it's a value in the symtbl, it is a valid value.

int extractValue(string operand);
//	Returns integer value of string passed.

bool validConstant(string operand);
bool validConstant(uint16_t operand);
// Returns true if operand is "#0", "#1", "#2", "#4", "#8", "#16", "#32",
// "#-1", "$0", "$1", "$2", "$4", "$8", "$10", "$20", or "$FF". Else
// returns false. Overloaded for case where checking against symbol,
// where symbol->value is of type uint16_t.

string getOperand(string & operands);
// Extracts operands delimited by commas until end of record is reached.
// Erases extracted operand from string of operands, so each time it is
// called, the next operand will be extracted until the end of the string
// is reached, similar to strtok() from the C standard library.

#endif
