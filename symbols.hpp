/* File name: symbols.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Necessary includes and declarations for symbols.cpp.
 * 			Include this file to access the global symbol table.
 * Last Modified: 2019-06-29
 */

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <iostream>
#include <iomanip>
#include <string>

#include "error.hpp"

enum Symbol_type { LBL, REG, UNK };

struct Symbol { // doubly linked list node
	// doubly linked because when I print it, I'd like for the symbols
	// to print in the order they are found in the source file (FIFO)
	std::string name;
	Symbol_type type;
	short value;
	struct Symbol * next;
	struct Symbol * prev;
};

extern Symbol * symtbl; // doubly linked list to hold symbols
extern Symbol * symtbl_end; // see above, print FIFO


Symbol * checkTable(Symbol * head, std::string name);
// Returns pointer to found element by name,
// or NULL if element not found. Uses iterative search.

bool validLabel(std::string str);
// Returns true if str == alphabetic + 0{alphanumeric}30
// where alphabetic is [ A - Z | a - z | _ ] and
// alphanumeric is [ alphabetic | 0 - 9 ].
// Returns false if str is not a valid label.

void initSymTbl(Symbol *& head);
// Initializes symtbl.

void pushSymbol(Symbol *& head, Symbol & sym);
// Creates symbol pointer from sym, pushes to head of symtbl linked list,
// replacing head node.

void printSymTbl(std::ostream & os);
// Prints symbol table to output stream from back to front. Back of
// table is global Symbol pointer symtbl_end.

void destroySymTbl(Symbol * head);
// Frees memory taken by symtbl.

std::ostream & operator<<(std::ostream & os, Symbol * sym);
// Prints fields of symbol struct to output stream .

#endif
