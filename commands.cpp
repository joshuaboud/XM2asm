/* File name: commands.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Defines list of commands (instructions + directives), along
 * 			with definitions of functions to be used with instructions
 * 			or directives.
 * Last Modified: 2019-06-03
 */

#include "commands.hpp"

// list of every instruction and directive, alphabetical order for
// quick binary search capabilities.
// Mnem		type  opCnt	opType	opSz	baseOp
Command commands[CMD_TBL_SIZE] = {
{"ADD",		INST,	2,	CR_R,	W,	0x4000	},
{"ADD.B",	INST,	2,	CR_R,	B,	0x4000	},
{"ADD.W",	INST,	2,	CR_R,	W,	0x4000	},
{"ADDC",	INST,	2,	CR_R,	W,	0x4100	},
{"ADDC.B",	INST,	2,	CR_R,	B,	0x4100	},
{"ADDC.W",	INST,	2,	CR_R,	W,	0x4100	},
{"ALIGN",	DIR,	0,	NONE,	W,		0	},
{"AND",		INST,	2,	CR_R,	W,	0x4700	},
{"AND.B",	INST,	2,	CR_R,	B,	0x4700	},
{"AND.W",	INST,	2,	CR_R,	W,	0x4700	},
{"BC",		INST,	1,	BRA_,	W,	0x2800	},
{"BEQ",		INST,	1,	BRA_,	W,	0x2000	},
{"BGE",		INST,	1,	BRA_,	W,	0x3400	},
{"BHS",		INST,	1,	BRA_,	W,	0x2800	},
{"BIC",		INST,	2,	CR_R,	W,	0x4900	},
{"BIC.B",	INST,	2,	CR_R,	B,	0x4900	},
{"BIC.W",	INST,	2,	CR_R,	W,	0x4900	},
{"BIS",		INST,	2,	CR_R,	W,	0x4A00	},
{"BIS.B",	INST,	2,	CR_R,	B,	0x4A00	},
{"BIS.W",	INST,	2,	CR_R,	W,	0x4A00	},
{"BIT",		INST,	2,	CR_R,	W,	0x4800	},
{"BIT.B",	INST,	2,	CR_R,	B,	0x4800	},
{"BIT.W",	INST,	2,	CR_R,	W,	0x4800	},
{"BL",		INST,	1,	BRA13,	W,	0x0000	},
{"BLO",		INST,	1,	BRA_,	W,	0x2C00	},
{"BLT",		INST,	1,	BRA_,	W,	0x3800	},
{"BN",		INST,	1,	BRA_,	W,	0x3000	},
{"BNC",		INST,	1,	BRA_,	W,	0x2C00	},
{"BNE",		INST,	1,	BRA_,	W,	0x2400	},
{"BNZ",		INST,	1,	BRA_,	W,	0x2400	},
{"BRA",		INST,	1,	BRA_,	W,	0x3C00	},
{"BSS",		DIR,	1,	C,		W,		0	},
{"BYTE",	DIR,	1,	C,		B,		0	},
{"BZ",		INST,	1,	BRA_,	W,	0x2000	},
{"CEX",		INST,	3,	CEX_,	W,	0x5C00	},
{"CMP",		INST,	2,	CR_R,	W,	0x4500	},
{"CMP.B",	INST,	2,	CR_R,	B,	0x4500	},
{"CMP.W",	INST,	2,	CR_R,	W,	0x4500	},
{"DADD",	INST,	2,	CR_R,	W,	0x4400	},
{"DADD.B",	INST,	2,	CR_R,	B,	0x4400	},
{"DADD.W",	INST,	2,	CR_R,	W,	0x4400	},
{"END",		DIR,	1,	Opt,	W,		0	},
{"EQU",		DIR,	1,	V,		W,		0	},
{"LD",		INST,	2,	LD_,	W,	0x5000	},
{"LD.B",	INST,	2,	LD_,	B,	0x5000	},
{"LD.W",	INST,	2,	LD_,	W,	0x5000	},
{"LDR",		INST,	3,	LDR_,	W,	0x8000	},
{"LDR.B",	INST,	3,	LDR_,	B,	0x8000	},
{"LDR.W",	INST,	3,	LDR_,	W,	0x8000	},
{"MOV",		INST,	2,	CR_R,	W,	0x4B00	},
{"MOV.B",	INST,	2,	CR_R,	B,	0x4B00	},
{"MOV.W",	INST,	2,	CR_R,	W,	0x4B00	},
{"MOVH",	INST,	2,	V_R,	W,	0x7800	},
{"MOVL",	INST,	2,	V_R,	W,	0x6000	},
{"MOVLS",	INST,	2,	V_R,	W,	0x7000	},
{"MOVLZ",	INST,	2,	V_R,	W,	0x6800	},
{"ORG",		DIR,	1,	V,		W,		0	},
{"RRC",		INST,	1,	R,		W,	0x4E00	},
{"RRC.B",	INST,	1,	R,		B,	0x4E00	},
{"RRC.W",	INST,	1,	R,		W,	0x4E00	},
{"SRA",		INST,	1,	R,		W,	0x4D00	},
{"SRA.B",	INST,	1,	R,		B,	0x4D00	},
{"SRA.W",	INST,	1,	R,		W,	0x4D00	},
{"ST",		INST,	2,	ST_,	W,	0x5400	},
{"ST.B",	INST,	2,	ST_,	B,	0x5400	},
{"ST.W",	INST,	2,	ST_,	W,	0x5400	},
{"STR",		INST,	3,	STR_,	W,	0xC000	},
{"STR.B",	INST,	3,	STR_,	B,	0xC000	},
{"STR.W",	INST,	3,	STR_,	W,	0xC000	},
{"SUB",		INST,	2,	CR_R,	W,	0x4200	},
{"SUB.B",	INST,	2,	CR_R,	B,	0x4200	},
{"SUB.W",	INST,	2,	CR_R,	W,	0x4200	},
{"SUBC",	INST,	2,	CR_R,	W,	0x4300	},
{"SUBC.B",	INST,	2,	CR_R,	B,	0x4300	},
{"SUBC.W",	INST,	2,	CR_R,	W,	0x4300	},
{"SVC",		INST,	1,	SA,		W,	0x5800	},
{"SWAP",	INST,	2,	R_R,	W,	0x4C00	},
{"SWPB",	INST,	1,	R,		W,	0x4F00	},
{"SXT",		INST,	1,	R,		W,	0x4F80	},
{"WORD",	DIR,	1,	C,		W,		0	},
{"XOR",		INST,	2,	CR_R,	W,	0x4600	},
{"XOR.B",	INST,	2,	CR_R,	B,	0x4600	},
{"XOR.W",	INST,	2,	CR_R,	W,	0x4600	}};

int checkTable(Command (& tbl)[CMD_TBL_SIZE], std::string mnemonic){
	// force mnemonic to upper case: examples given by Dr. Hughes have
	// lower case instructions (?)
	for(int i = 0; i < (int)mnemonic.length(); i++){
		mnemonic[i] = toupper(mnemonic[i]);
	}
	
	// binary search:
	int low = 0, high = CMD_TBL_SIZE - 1;
	while(low <= high){
		int mid = low + (high - low)/ 2;
		if (strcmp(mnemonic.c_str(), tbl[mid].mnem.c_str()) == 0){
			// mnem same
			return mid;
		} else if (strcmp(mnemonic.c_str(), tbl[mid].mnem.c_str()) < 0){
			// mnem is lower
			high = mid - 1;
		} else if (strcmp(mnemonic.c_str(), tbl[mid].mnem.c_str()) > 0){
			// mnem is higher
			low = mid + 1;
		}
	}
	// if not found, return -1
	return -1;
}

bool validValue(std::string operand){
	int i = 1; // start at 1 to skip # or $
	char ch; // temp char variable
	
	// for escaped chars:
	char escapees[ESC_CHARS] = {'b','t','n','r','0','\\','\'','\"'};
	
	switch(operand[0]){
	case '#': // decimal
		if(operand[1] == '-') // if negative start at position 2
			i++;
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
		if(operand.length() == CHAR_LEN){ // probably normal char e.g. {\',ch,\'}
			if(!(' ' <= ch && ch <= '~')) // if ch is not valid char
				return false; // not valid if char not in unescaped ascii table
		}else if(operand.length() == ESC_CHAR_LEN){ // probably escaped char e.g. {\',\\,ch,\'}
			// When C++ parses in a file with an escaped character, it does
			// not take a raw escaped char. It escapes the backslash
			// automatically as '\\' as its own char.
			if(ch != '\\'){ // if not an escaped char
				return false;
			}else{ // check if next char is valid escape char
				ch = operand[2]; // get next char after '\\'
				for(i = 0; i < ESC_CHARS; i++){
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

int extractValue(std::string operand){
	int value;
	std::istringstream ss(operand); // open string as input stream
	char type = ss.get(); // pop first character, either #, $, '
	char escapees[ESC_CHARS][2] =  {{'b','\b'},
									{'t','\t'},
									{'n','\n'},
									{'r','\r'},
									{'0','\0'},
									{'\\','\\'},
									{'\'','\''},
									{'\"','\"'}};
	switch(type){
	case '#':
		ss >> std::dec >> value;
		break;
	case '$':
		ss >> std::hex >> value;
		break;
	case '\'':
		// replace operand with just what's in quotes
		operand = operand.substr(1,operand.length()-2); // cut off 's
		switch(operand.length()){
		case 1: // not escaped
			value = (int)operand.c_str()[0]; // get char value as int
			break;
		case 2: // escaped
			for(int i = 0; i < ESC_CHARS; i++){
				if(operand[1] == escapees[i][0]){
					// find operand in escapees list
					value = (int)escapees[i][1];
					break;
				}
			}
			break;
		}
		break;
	}
	return value;
}

bool validConstant(std::string operand){
	std::string constants[19] = { "#0", "#1", "#2", "#4", "#8", "#16", "#32",\
		 "#-1", "$0", "$1", "$2", "$4", "$8", "$10", "$20", "$FF", "$ff", "$FFFF", "$ffff"};
	for(int i = 0; i < 19; i++){
		if(operand == constants[i]){ // operand is valid constant
			return true;
		}
	}
	// operand is not valid constant
	return false;
}

bool validConstant(short operand){
	unsigned short constants[CONST_CNT] = { 0, 1, 2, 4, 8, 16, 32, 0xFFFF };
	for(int i = 0; i < CONST_CNT; i++){
		if((unsigned short)operand == constants[i]){ // operand is valid constant
			return true;
		}
	}
	// operand is not valid constant
	return false;
}

std::string getOperand(std::string & operands){
	if(operands.empty()){
		// no operand
		return "";
	}
	
	std::string operand; // to return
	
	// find position of first comma
	int delimiterPos = operands.find(',');
	
	if(delimiterPos == std::string::npos){ // no comma
		switch(operands[0]){
		case '\'': // in case operand is ' ' (space)
			// keep until last \', inclusive
			delimiterPos = operands.find_last_of('\'') + 1;
			break;
		default:
			delimiterPos = operands.find(' '); // maybe followed by space delim garbage
			if(delimiterPos == std::string::npos){ // no space
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
