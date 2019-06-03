/* File name: commands.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Defines list of commands (instructions + directives), along
 * 			with definitions of functions to be used with instructions
 * 			or directives.
 * Last Modified: 2019-06-02
 */

#include "commands.hpp"

// list of every instruction and directive, alphabetical order for
// quick binary search capabilities.
Command commands[CMD_TBL_SIZE] = {
//	Mnemonic  Ins/Dir OpCnt	OpType
	{"ADD", 	INST,	2,	CR_R},
	{"ADD.B", 	INST,	2,	CR_R},
	{"ADD.W", 	INST,	2,	CR_R},
	{"ADDC",	INST,	2,	CR_R},
	{"ADDC.B",	INST,	2,	CR_R},
	{"ADDC.W",	INST,	2,	CR_R},
	{"ALIGN",	DIR,	0,	NONE},
	{"AND",		INST,	2,	CR_R},
	{"AND.B",	INST,	2,	CR_R},
	{"AND.W",	INST,	2,	CR_R},
	{"BC",		INST,	1,	BRA},
	{"BEQ",		INST,	1,	BRA},
	{"BGE", 	INST,	1,	BRA},
	{"BHS",		INST,	1,	BRA},
	{"BIC",		INST,	2,	CR_R},
	{"BIC.B",	INST,	2,	CR_R},
	{"BIC.W",	INST,	2,	CR_R},
	{"BIS",		INST,	2,	CR_R},
	{"BIS.B",	INST,	2,	CR_R},
	{"BIS.W",	INST,	2,	CR_R},
	{"BIT",		INST,	2,	CR_R},
	{"BIT.B",	INST,	2,	CR_R},
	{"BIT.W",	INST,	2,	CR_R},
	{"BL",		INST,	1,	BRA13},
	{"BLO", 	INST,	1,	BRA},
	{"BLT", 	INST,	1,	BRA},
	{"BN",		INST,	1,	BRA},
	{"BNC",		INST,	1,	BRA},
	{"BNE", 	INST,	1,	BRA},
	{"BNZ", 	INST,	1,	BRA},
	{"BRA", 	INST,	1,	BRA},
	{"BSS",		DIR,	1,	C},
	{"BYTE",	DIR,	1,	C},
	{"BZ",		INST,	1,	BRA},
	{"CEX",		INST,	3,	CEX}, // special code
	{"CMP",		INST,	2,	CR_R},
	{"CMP.B",	INST,	2,	CR_R},
	{"CMP.W",	INST,	2,	CR_R},
	{"DADD",	INST,	2,	CR_R},
	{"DADD.B",	INST,	2,	CR_R},
	{"DADD.W",	INST,	2,	CR_R},
	{"END",		DIR,	1,	Opt},
	{"EQU",		DIR,	1,	C},
	{"LD",		INST,	2,	LD},
	{"LD.B",	INST,	2,	LD},
	{"LD.W",	INST,	2,	LD},
	{"LDR",		INST,	3,	LDR},
	{"LDR.B",	INST,	3,	LDR},
	{"LDR.W",	INST,	3,	LDR},
	{"MOV",		INST,	2,	CR_R},
	{"MOV.B",	INST,	2,	CR_R},
	{"MOV.W",	INST,	2,	CR_R},
	{"MOVH",	INST,	2,	V_R},
	{"MOVL",	INST,	2,	V_R},
	{"MOVLS",	INST,	2,	V_R},
	{"MOVLZ",	INST,	2,	V_R},
	{"ORG",		DIR,	1,	V},
	{"RRC",		INST,	1,	R},
	{"RRC.B",	INST,	1,	R},
	{"RRC.W",	INST,	1,	R},
	{"SRA",		INST,	1,	R},
	{"SRA.B",	INST,	1,	R},
	{"SRA.W",	INST,	1,	R},
	{"ST",		INST,	2,	ST},
	{"ST.B",	INST,	2,	ST},
	{"ST.W",	INST,	2,	ST},
	{"STR",		INST,	3,	STR},
	{"STR.B",	INST,	3,	STR},
	{"STR.W",	INST,	3,	STR},
	{"SUB",		INST,	2,	CR_R},
	{"SUB.B",	INST,	2,	CR_R},
	{"SUB.W",	INST,	2,	CR_R},
	{"SUBC",	INST,	2,	CR_R},
	{"SUBC.B",	INST,	2,	CR_R},
	{"SUBC.W",	INST,	2,	CR_R},
	{"SVC",		INST,	1,	SA},
	{"SWAP",	INST,	2,	R_R},
	{"SWPB",	INST,	1,	R},
	{"SXT",		INST,	1,	R},
	{"WORD",	DIR,	1,	C},
	{"XOR",		INST,	2,	CR_R},
	{"XOR.B",	INST,	2,	CR_R},
	{"XOR.W",	INST,	2,	CR_R}};

int checkTable(Command (& tbl)[CMD_TBL_SIZE], string mnemonic){
	// force mnemonic to upper case: examples given by Dr. Hughes have
	// lower case instructions (?)
	for(int i = 0; i < (int)mnemonic.length(); i++){
		mnemonic[i] = toupper(mnemonic[i]);
	}
	
	// binary search:
	int low = 0, high = CMD_TBL_SIZE - 1;
	while(low <= high){
		int mid = low + (high - low)/ 2;
		if (strcmp(mnemonic.c_str(), tbl[mid].name.c_str()) == 0){
			// mnem same
			return mid;
		} else if (strcmp(mnemonic.c_str(), tbl[mid].name.c_str()) < 0){
			// mnem is lower
			high = mid - 1;
		} else if (strcmp(mnemonic.c_str(), tbl[mid].name.c_str()) > 0){
			// mnem is higher
			low = mid + 1;
		}
	}
	// if not found, return -1
	return -1;
}

bool validValue(string operand){
	int i = 1; // cannot declare var inside switch, start at 1 to
	// skip the '#' or '$'
	char ch; // temp char variable
	
	// for escaped chars:
	char escapees[8] = {'b','t','n','r','0','\\','\'','\"'};
	
	if(operand[1] == '-') // if negative start at position 2
		i++;
	switch(operand[0]){
	case '#': // decimal
		for(; i < (int)operand.length(); i++){ // i = 1 or 2
			// if digit is NOT between '0' and '9'
			if(!('0' <= operand[i] && operand[i] <= '9'))
				return false;
		}
		break; // jump to return true
	case '$': // hexadecimal
		for(; i < (int)operand.length(); i++){ // i = 1 or 2
			// if digit is NOT (between '0' and '9' or between 'A' and 'F')
			if(!(('0' <= operand[i] && operand[i] <= '9') || \
			('A' <= operand[i] && operand[i] <= 'F') || \
			('a' <= operand[i] && operand[i] <= 'f')))
				return false;
		}
		break; // jump to return true
	case '\'': // char
		ch = operand[1]; // get first char after \'
		if(operand.length() == 3){ // probably normal char e.g. {\',ch,\'}
			if(!(' ' <= ch && ch <= '~')) // if ch is not valid char
				return false; // not valid if char not in unescaped ascii table
		}else if(operand.length() == 4){ // probably escaped char e.g. {\',\\,ch,\'}
			// When C++ parses in a file with an escaped character, it does
			// not take a raw escaped char. It escapes the backslash
			// automatically as '\\' as its own char.
			if(ch != '\\'){ // if not an escaped char
				return false;
			}else{ // check if next char is valid escape char
				ch = operand[2]; // get next char after '\\'
				for(i = 0; i < 8; i++){
					if(ch == escapees[i])
						return true; // char is valid escape char
				}
				// if we make it past the for loop,
				// char is not a valid escaped char
				return false;
			}
		}else{ // too many characters in string for char
			return false;
		}
		break;  // jump to return true
	default: // does not start with # or $, is not char: invalid value
		return false;
	}
	// if we made it here, value is valid.
	return true;
}

bool validValue(uint16_t operand){
	// if in symtbl, always valid
	return true;
}

int extractValue(string operand){
	int value;
	istringstream ss(operand); // open string as input stream
	char type = ss.get(); // pop first character, either #, $, '
	switch(type){
	case '#':
		ss >> dec >> value;
		break;
	case '$':
		ss >> hex >> value;
		break;
	}
	return value;
}

bool validConstant(string operand){
	string constants[16] = { "#0", "#1", "#2", "#4", "#8", "#16", "#32",\
		 "#-1", "$0", "$1", "$2", "$4", "$8", "$10", "$20", "$FF"};
	for(int i = 0; i < 16; i++){
		if(operand == constants[i]){ // operand is valid constant
			return true;
		}
	}
	// operand is not valid constant
	return false;
}

bool validConstant(uint16_t operand){
	uint16_t constants[8] = { 0, 1, 2, 4, 8, 16, 32, 0xFF };
	for(int i = 0; i < 8; i++){
		if(operand == constants[i]){ // operand is valid constant
			return true;
		}
	}
	// operand is not valid constant
	return false;
}

string getOperand(string & operands){
	if(operands.empty()){
		// no operand
		return "";
	}
	
	string operand; // to return
	
	// find position of first comma
	int delimiterPos = operands.find(',');
	
	if(delimiterPos == string::npos){ // no comma
		switch(operands[0]){
		case '\'': // in case operand is ' ' (space)
			// keep until last \', inclusive
			delimiterPos = operands.find_last_of('\'') + 1;
			break;
		default:
			delimiterPos = operands.find(' '); // maybe followed by space delim garbage
			if(delimiterPos == string::npos){ // no space
				operand = operands;
				operands.erase(0, operands.length());
				return operand;
			}
		}
	}
	
	// else make substring of operands containing first operand
	operand = operands.substr(0, delimiterPos);
	// erase first operand + delim from operands
	operands.erase(0, operand.length() + 1);
	
	return operand;
}

