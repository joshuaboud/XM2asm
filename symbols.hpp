/* File name: symbols.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Necessary includes and definitions for symbols.cpp.
 * 			Include this file to access the global symbol table.
 * Last Modified: 2019-05-30
 */

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

#include <stdio.h> // for printing mem location in hex in symtbl

enum Symbol_type { LBL, REG, UNK };

struct Symbol {
	string name;
	Symbol_type type;
	uint16_t value;
	struct Symbol * next;
	struct Symbol * prev;
};

extern Symbol * symtbl; // doubly linked list to hold symbols
extern Symbol * symtbl_end; // for printing the right way around

extern bool ERROR_FLAG;

Symbol * checkTable(Symbol * head, string & name);
// Returns pointer to found element by name,
// or NULL if element not found. Uses iterative search.

int validLabel(string str);
// Returns 1 if str == alphabetic + 0{alphanumeric}30
// where alphabetic is [ A - Z | a - z | _ ] and
// alphanumeric is [ alphabetic | 0 - 9 ].
// Returns 2 if alphabetic + 0{alphanumeric}30 + .[B|W],
// which is not a valid label, but still a valid instruction.

void initSymTbl(Symbol *& head);
// Initializes symtbl.

void pushSymbol(Symbol *& head, Symbol & sym);
// Pushed symbol pointer to head of symtbl.

void printSymTbl(ostream & os, Symbol * head);
// Prints symbol table to output stream.

void destroySymTbl(Symbol * head);
// Frees memory taken by symtbl.

ostream & operator<<(ostream & os, Symbol * sym);
// Prints fields of symbol to output stream.

#endif
