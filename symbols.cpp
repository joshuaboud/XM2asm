#include "symbols.hpp"

// Globals

Symbol * symtbl;

Symbol * checkTable(Symbol * head, string & mnemonic){
	Symbol * itr = head;
	while(itr != NULL){
		if (mnemonic.compare(itr->name) == 0) // mnem same
			break;
		itr = itr->next;
	}
	return itr; // if not found, will return NULL ptr
}

bool validLabel(string str){
	if(str.length() > 31)
		return false;
	// if first letter is NOT between 'A' and 'Z' or between 'a' and 'z'
	if(!(('A' <= str[0] && str[0] <= 'Z') || ('a' <= str[0] && str[0] <= 'z')))
		return false;
	// check if rest of letters are alphanumeric or '_'
	for(char i : str){
		if(!(('A' <= i && i <= 'Z') || ('a' <= i && i <= 'z') || \
		('0' <= i && i <= '9') || (i == '_')))
			return false;
	}
	// if we made it here, it's a valid label
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
	for(int i = 0; i < 8; i++){
		pushSymbol(head, registers[i]);
	}
}

void pushSymbol(Symbol *& head, Symbol & sym){
	Symbol * ptr = new Symbol;
	ptr->name = sym.name;
	ptr->type = sym.type;
	ptr->value = sym.value;
	ptr->next = head;
	head = ptr;
}

void printSymTbl(ostream & os, Symbol * head){
	Symbol * itr = head;
	if(itr == NULL){
		os << "Symtbl empty!" << endl;
		return;
	}
	while(itr != NULL){
		os << itr;
		itr = itr->next;
	}
}

ostream & operator<<(ostream & os, Symbol * sym){
	os << setw(31) << sym->name << " | ";
	os << setw(3);
	(sym->type == REG)? os << "REG" : os << "LBL";
	os << " | " << sym->value;
	os << endl;
	return os;
}
