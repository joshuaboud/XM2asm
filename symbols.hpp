#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
using namespace std;

enum Symbol_type { LBL, REG, UNK };

struct Symbol {
	string name;
	Symbol_type type;
	uint16_t value;
};

extern vector<Symbol> symtbl;	//	symbol table

int checkTable(vector<Symbol> & vec, string & mnemonic);
// Returns positional subscript of array element by name,
// or -1 if element not found. Uses binary search.

bool validLabel(string str);
// Returns true if str == alphabetic + 0{alphanumeric}30
// where alphabetic is [ A - Z | a - z | _ ] and
// alphanumeric is [ alphabetic | 0 - 9 ]

ostream & operator<<(ostream & os, const Symbol & sym);

#endif
