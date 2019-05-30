#include "symbols.hpp"

// Globals

vector<Symbol> symtbl = \
		{	{"R0", REG, 0},
			{"R1", REG, 1},
			{"R2", REG, 2},
			{"R3", REG, 3},
			{"R4", REG, 4},
			{"R5", REG, 5},
			{"R6", REG, 6},
			{"R7", REG, 7}	};

int checkTable(vector<Symbol> & vec, string & mnemonic){
	int low = 0, high = vec.size() - 1;
	while(low <= high){
		int mid = low + (high - low)/ 2;
		if (mnemonic.compare(vec[mid].name) == 0){ // mnem same
			return mid;
		} else if (mnemonic.compare(vec[mid].name) < 0){ // mnem is lower
			high = mid - 1;
		} else if (mnemonic.compare(vec[mid].name) > 0){ // mnem is higher
			low = mid + 1;
		}
	}
	return -1;
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

ostream & operator<<(ostream & os, const Symbol & sym){
	os << setw(31) << sym.name << " | ";
	os << setw(3);
	(sym.type == REG)? os << "REG" : os << "LBL";
	os << " | " << sym.value;
	return os;
}
