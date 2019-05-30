#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

enum Command_type { DIR, INST };
enum Operand_type { NONE, CR_R, R_R, C_R, C, Opt};

struct Record {
	int lineNum;
	int memLoc;
	string record;
	string error;
};

struct Command {
	string name;
	Command_type type;
	int opCnt;
	Operand_type ops;
};

extern vector<Command> commands; // array of instructions & directives

int checkTable(vector<Command> & vec, string & mnemonic);
// Returns positional subscript of array element by name,
// or -1 if element not found. Uses binary search.

void pushRecord(vector<Record> & vec, int lineNum, string rec, \
string error = "", int memLoc = -1);
// Wrapper for std::vector::push_back() to handle record struct Record
// if memLoc is -1 (default), memory counter will not be printed on list
// file.

template <typename T> 
ostream & operator<<(ostream & os, const vector<T> & v);

ostream & operator<<(ostream & os, const Record & rec);

#endif
