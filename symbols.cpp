/* File name: symbols.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Defines functions to be used with symbols and the symbol
 * 			table, as well as defines the beginning and end of the
 * 			symbol table linked list.
 * Last Modified: 2019-06-02
 */

#include "symbols.hpp"

// Globals

Symbol * symtbl; // head of linked list of symbols
Symbol * symtbl_end; // hold end to allow first in first out printing

Symbol * checkTable(Symbol * head, string name){
	// iteratively searches for symbol by name, returns ptr to symbol
	Symbol * itr = head;
	while(itr != NULL){
		if (name.compare(itr->name) == 0) // same
			break;
		itr = itr->next;
	}
	return itr; // if not found, will return NULL ptr
}

int validLabel(string str){
	int flag = 0; // 0 false, 1 true, 2 special case for INST.[B|W]
	if(str.length() > 31)
		return flag;
	// if first letter is NOT between 'A' and 'Z' or between 'a' and 'z'
	if(!(('A' <= str[0] && str[0] <= 'Z') || ('a' <= str[0] && str[0] <= 'z')))
		return flag;
	// check if rest of letters are alphanumeric or '_'
	for(char i : str){
		if(!(('A' <= i && i <= 'Z') || ('a' <= i && i <= 'z') || \
		('0' <= i && i <= '9') || (i == '_') || (i == '.'))){
			flag = 0;
			return flag;
		}
		if(i == '.'){ // special case of valid INST.[B|W], not valid label
			flag = 2;
		}
	}
	// if we made it here, it's a valid label or instruction
	if(flag != 2) // if not special case above,
		flag = 1; // return valid label
	return flag;
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

void printSymTbl(ostream & os){
	Symbol * itr = symtbl_end; // start at bottom
	if(itr == NULL){
		os << "Symtbl empty!" << endl;
		return;
	}
	
	os << setw(28) << "" << "      SYMBOL TABLE" << endl;
	os << setw(28) << "" << "------------------------" << endl;
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

ostream & operator<<(ostream & os, Symbol * sym){
	os << setw(31) << sym->name << " | ";
	os << setw(3);
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
	os << " | " << setw(6) << sym->value; // print as dec
	os << " (0x" << hex << sym->value << dec << ")"; // print as hex
	if(sym->type == UNK)
		os << " !!! ERROR: Unkown Symbol";
	os << endl;
	return os;
}
