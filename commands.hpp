#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <iostream>
#include <iomanip>

#define CMD_TBL_SIZE 72

using namespace std;

enum Command_type { DIR, INST };
enum Operand_type { NONE, CR_R, R_R, C_R, C, Opt};

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

#endif
