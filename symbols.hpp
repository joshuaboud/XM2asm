#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

enum Symbol_type { LBL, REG, UNK };

struct Symbol {
	string name;
	Symbol_type type;
	uint16_t value;
	struct Symbol * next;
};

extern Symbol * symtbl;

Symbol * checkTable(Symbol * head, string & mnemonic);
// Returns pointer to found element by name,
// or NULL if element not found. Uses iterative search.

bool validLabel(string str);
// Returns true if str == alphabetic + 0{alphanumeric}30
// where alphabetic is [ A - Z | a - z | _ ] and
// alphanumeric is [ alphabetic | 0 - 9 ].

void initSymTbl(Symbol *& head);
// Initializes symtbl.

void pushSymbol(Symbol *& head, Symbol & sym);
// Pushed symbol pointer to head of symtbl.

void printSymTbl(ostream & os, Symbol * head);
// Prints symbol table to output stream.

ostream & operator<<(ostream & os, Symbol * sym);
// Prints fields of symbol to output stream.

#endif
