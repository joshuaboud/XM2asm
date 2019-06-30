/* File name: operands.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Functions and tables for operand checking and encoding
 * Last Modified: 2019-06-29
 */

#include "operands.hpp"

CEX_Flag cexFlags[CEX_FLAG_CNT] = {	
	{"EQ",0x0},
	{"NE",0x1},
	{"CS",0x2},
	{"HS",0x2},
	{"CC",0x3},
	{"LO",0x3},
	{"MI",0x4},
	{"PL",0x5},
	{"VS",0x6},
	{"VC",0x7},
	{"HI",0x8},
	{"LS",0x9},
	{"GE",0xA},
	{"LT",0xB},
	{"GT",0xC},
	{"LE",0xD},
	{"AL",0xE}
};

char escapeChars[ESC_CHARS][2] = {
	{'b','\b'},
	{'t','\t'},
	{'n','\n'},
	{'r','\r'},
	{'0','\0'},
	{'\\','\\'},
	{'\'','\''},
	{'\"','\"'}
};

bool validValue(std::string operand){
	int i = 1; // start at 1 to skip # or $
	char ch; // temp char variable
	
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
					if(ch == escapeChars[i][ESC_KEY])
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
				if(operand[1] == escapeChars[i][ESC_KEY]){
					// find operand in escapees list
					value = (int)escapeChars[i][ESC_VAL];
					break;
				}
			}
			break;
		}
		break;
	}
	std::cout << "ExtractValue: " << operand << " = " << value << std::endl;
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
	short constants[CONST_CNT] = { 0, 1, 2, 4, 8, 16, 32, -1 };
	for(int i = 0; i < CONST_CNT; i++){
		if(operand == constants[i]){ // operand is valid constant
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

int decodeConst(int val){
	switch(val){
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
		return 2;
	case 4:
		return 3;
	case 8:
		return 4;
	case 16:
		return 5;
	case 32:
		return 6;
	case -1:
		return 7;
	default:
		return -1;
	}
}
