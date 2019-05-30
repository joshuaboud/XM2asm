#ifndef COMMANDS_H
#define COMMANDS_H

#define CMD_CNT 47

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

enum Command_type { DIR, INST };
enum Operand_type { NONE, CR_R, R_R, C_R, C, Opt};
enum Symbol_type { LBL, REG };

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

struct Symbol {
	string name;
	Symbol_type type;
	uint16_t value;
};

extern Command commands[CMD_CNT]; // array of instructions & directives

int checkTable(string & mnemonic);
// Returns positional subscript of command array element,
// or -1 if element not found. Uses binary search.

void pushRecord(vector<Record> & vec, int lineNum, string rec, \
string error = "", int memLoc = -1);
// Wrapper for std::vector::push_back() to handle record struct Record
// if memLoc is -1 (default), memory counter will not be printed on list
// file.

template <typename T> 
ostream & operator<<(ostream & os, const vector<T> & v);

ostream & operator<<(ostream & os, const Record & rec);

ostream & operator<<(ostream & os, const Symbol & sym);

#endif
