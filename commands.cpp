#include "commands.hpp"

vector<Command> commands = {
//	Mnemonic  Ins/Dir OpCnt	OpType
	{"ADD", 	INST,	2,	CR_R},
	{"ADDC",	INST,	2,	CR_R},
	{"ALIGN",	DIR,	0,	NONE},
	{"AND",		INST,	2,	CR_R},
	{"BC",		INST,	0,	NONE},
	{"BEQ",		INST,	0,	NONE},
	{"BGE", 	INST,	0,	NONE},
	{"BHS",		INST,	0,	NONE},
	{"BIC",		INST,	2,	CR_R},
	{"BIS",		INST,	2,	CR_R},
	{"BIT",		INST,	2,	CR_R},
	{"BL",		INST,	0,	NONE},
	{"BLO", 	INST,	0,	NONE},
	{"BN",		INST,	0,	NONE},
	{"BNC",		INST,	0,	NONE},
	{"BNE", 	INST,	0,	NONE},
	{"BNZ", 	INST,	0,	NONE},
	{"BRA", 	INST,	0,	NONE},
	{"BSS",		DIR,	1,	C},
	{"BTL", 	INST,	0,	NONE},
	{"BYTE",	DIR,	1,	C},
	{"BZ",		INST,	0,	NONE},
//	{"CEX",		,		,},			// ?
	{"CMP",		INST,	2,	CR_R},
	{"DADD",	INST,	2,	CR_R},
	{"END",		DIR,	1,	Opt},
	{"EQU",		DIR,	1,	C},
	{"LD",		INST,	2,	R_R},
	{"LDR",		INST,	2,	R_R},
	{"MOV",		INST,	2,	CR_R},
	{"MOVH",	INST,	2,	C_R},
	{"MOVL",	INST,	2,	C_R},
	{"MOVLS",	INST,	2,	C_R},
	{"MOVLZ",	INST,	2,	C_R},
	{"ORG",		DIR,	1,	C},
	{"RRC",		INST,	1,	R_R},
	{"SRA",		INST,	1,	R_R},
	{"ST",		INST,	2,	R_R},
	{"STR",		INST,	2,	R_R},
	{"SUB",		INST,	2,	CR_R},
	{"SUBC",	INST,	2,	CR_R},
//	{"SVC",		,		,},			// ?"
	{"SWAP",	INST,	2,	R_R},
	{"SWPB",	INST,	1,	R_R},
	{"SXT",		INST,	1,	R_R},
	{"WORD",	DIR,	1,	C},
	{"XOR",		INST,	2,	CR_R}};

int checkTable(vector<Command> & vec, string & mnemonic){
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


void pushRecord(vector<Record> & vec, int lineNum, string rec, \
string error, int memLoc){
	Record temp;
	temp.lineNum = lineNum;
	temp.memLoc = memLoc;
	temp.record = rec;
	temp.error = error;
	vec.push_back(temp);
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

template <typename T>
ostream & operator<<(ostream & os, const vector<T> & v){
	for(auto i : v){
		os << i << endl;
	}
	os << endl;
	return os;
}

ostream & operator<<(ostream & os, const Record & rec){
	os << rec.lineNum << setw(10);
	(rec.memLoc >= 0)? os << rec.memLoc : os << "";
	os << rec.record;
	(rec.error.empty())? os << endl : os << rec.error << endl;
	return os;
}

ostream & operator<<(ostream & os, const Symbol & sym){
	os << setw(31) << sym.name << " | ";
	os << setw(3);
	(sym.type == REG)? os << "REG" : os << "LBL";
	os << " | " << sym.value;
	return os;
}
