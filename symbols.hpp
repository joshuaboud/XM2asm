/* File name: symbols.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Necessary includes and declarations for symbols.cpp.
 * 			Include this file to access the global symbol table.
 * Last Modified: 2019-06-02
 */

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

#include <stdio.h> // for printing mem location in hex in symtbl

enum Symbol_type { LBL, REG, UNK };

struct Symbol { // doubly linked list node
	string name;
	Symbol_type type;
	uint16_t value;
	struct Symbol * next;
	struct Symbol * prev;
};

extern Symbol * symtbl; // doubly linked list to hold symbols
extern Symbol * symtbl_end; // for printing the right way around

extern bool ERROR_FLAG;

Symbol * checkTable(Symbol * head, string name);
// Returns pointer to found element by name,
// or NULL if element not found. Uses iterative search.

int validLabel(string str);
// Returns 1 if str == alphabetic + 0{alphanumeric}30
// where alphabetic is [ A - Z | a - z | _ ] and
// alphanumeric is [ alphabetic | 0 - 9 ].
// Returns 2 if alphabetic + 0{alphanumeric}30 + .[B|W],
// which is not a valid label, but still a valid instruction.
// Returns 0 if str is not a valid label.

void initSymTbl(Symbol *& head);
// Initializes symtbl.

void pushSymbol(Symbol *& head, Symbol & sym);
// Creates symbol pointer from sym, pushes to head of symtbl linked list,
// replacing head node.

void printSymTbl(ostream & os);
// Prints symbol table to output stream from back to front. Back of
// table is global Symbol pointer symtbl_end.

void destroySymTbl(Symbol * head);
// Frees memory taken by symtbl.

ostream & operator<<(ostream & os, Symbol * sym);
// Prints fields of symbol struct to output stream .

#endif
