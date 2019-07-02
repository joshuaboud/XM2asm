/* File name: symbols.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Defines functions to be used with symbols and the symbol
 * 			table, as well as defines the beginning and end of the
 * 			symbol table linked list.
 * Last Modified: 2019-06-29
 */

#include "symbols.hpp"

#define MAX_LBL_LENGTH 31

#define ALPHABETIC(x) (('A' <= x && x <= 'Z') || \
('a' <= x && x <= 'z') || (x == '_'))
		
#define ALPHANUMERIC(x) (ALPHABETIC(x) || ('0' <= x && x <= '9'))

// Globals

Symbol * symtbl; // head of linked list of symbols
Symbol * symtbl_end; // hold end to allow first in first out printing

Symbol * checkTable(Symbol * head, std::string name){
	// iteratively searches for symbol by name, returns ptr to symbol
	Symbol * itr = head;
	while(itr != NULL){
		if (name.compare(itr->name) == 0) // same
			break;
		itr = itr->next;
	}
	return itr; // if not found, will return NULL ptr
}

bool validLabel(std::string str){
	if(str.length() > MAX_LBL_LENGTH)
		return false;
	// if first letter is NOT between 'A' and 'Z' or between 'a' and 'z'
	if(!ALPHABETIC(str[0]))
		return false;
	// check if rest of letters are alphanumeric or '_'
	for(char i : str){
		if(!ALPHANUMERIC(i)){
			return false;
		}
	}
	// if we made it here, it's a valid label or instruction
	return true;
}

void initSymTbl(Symbol *& head){
	Symbol registers[8] = \
		{	{"R0", REG, 0},
			{"R1", REG, 1},
			{"R2", REG, 2},
			{"R3", REG, 3},
			{"R4", REG, 4},
			{"R5", REG, 5},
			{"R6", REG, 6},
			{"R7", REG, 7}	};
	head = NULL;
	pushSymbol(head, registers[0]); // init with first reg
	head->prev = NULL;
	symtbl_end = head; // symtbl_end points to bottom of list
	for(int i = 1; i < 8; i++){
		pushSymbol(head, registers[i]);
	}
}

void pushSymbol(Symbol *& head, Symbol & sym){
	Symbol * ptr = new Symbol;
	ptr->name = sym.name;
	ptr->type = sym.type;
	ptr->value = sym.value;
	ptr->next = head;
	ptr->prev = NULL;
	if(head != NULL) // guard seg fault
		head->prev = ptr;
	head = ptr;
}

void printSymTbl(std::ostream & os){
	Symbol * itr = symtbl_end; // start at bottom
	if(itr == NULL){
		os << "Symtbl empty!" << std::endl;
		return;
	}
	
	os << std::setw(28) << "" << "      SYMBOL TABLE" << std::endl;
	os << std::setw(28) << "" << "------------------------" << std::endl;
	while(itr != NULL){
		os << itr;
		itr = itr->prev; // move towards top
	}
}

void destroySymTbl(Symbol * head){
	Symbol * itr = head;
	Symbol * temp;
	if(itr == NULL){
		return;
	}
	while(itr != NULL){
		temp = itr;
		itr = itr->next;
		delete temp; // analogous to free()
	}
}

std::ostream & operator<<(std::ostream & os, Symbol * sym){
	os << std::setw(31) << sym->name << " | ";
	os << std::setw(3);
	switch(sym->type){
	case REG:
		os << "REG";
		break;
	case LBL:
		os << "LBL";
		break;
	case UNK:
		os << "UNK";
		ERROR_FLAG = true;
		break;
	}
	os << " | " << std::setw(6) << sym->value; // print as dec
	os << " (0x" << std::hex << sym->value << std::dec << ")"; // print as hex
	if(sym->type == UNK)
		os << " !!! ERROR: Unkown Symbol";
	os << std::endl;
	return os;
}
