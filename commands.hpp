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
enum Symbol_type { LBL, REG, UNK };

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

extern vector<Command> commands; // array of instructions & directives

int checkTable(vector<Command> & vec, string & mnemonic);
int checkTable(vector<Symbol> & vec, string & mnemonic);
// Returns positional subscript of array element by name,
// or -1 if element not found. Uses binary search.

void pushRecord(vector<Record> & vec, int lineNum, string rec, \
string error = "", int memLoc = -1);
// Wrapper for std::vector::push_back() to handle record struct Record
// if memLoc is -1 (default), memory counter will not be printed on list
// file.

bool validLabel(string str);
// Returns true if str == alphabetic + 0{alphanumeric}30
// where alphabetic is [ A - Z | a - z | _ ] and
// alphanumeric is [ alphabetic | 0 - 9 ]

template <typename T> 
ostream & operator<<(ostream & os, const vector<T> & v);

ostream & operator<<(ostream & os, const Record & rec);

ostream & operator<<(ostream & os, const Symbol & sym);

#endif
