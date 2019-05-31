/* File name: commands.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Defines list of commands (instructions + directives), along
 * 			with definitions of functions to be used with instructions
 * 			or directives.
 * Last Modified: 2019-05-30
 */

#include "commands.hpp"

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
	{"BC",		INST,	1,	LBL10},
	{"BEQ",		INST,	1,	LBL10},
	{"BGE", 	INST,	1,	LBL10},
	{"BHS",		INST,	1,	LBL10},
	{"BIC",		INST,	2,	CR_R},
	{"BIC.B",	INST,	2,	CR_R},
	{"BIC.W",	INST,	2,	CR_R},
	{"BIS",		INST,	2,	CR_R},
	{"BIS.B",	INST,	2,	CR_R},
	{"BIS.W",	INST,	2,	CR_R},
	{"BIT",		INST,	2,	CR_R},
	{"BIT.B",	INST,	2,	CR_R},
	{"BIT.W",	INST,	2,	CR_R},
	{"BL",		INST,	1,	LBL13},
	{"BLO", 	INST,	1,	LBL10},
	{"BLT", 	INST,	1,	LBL10},
	{"BN",		INST,	1,	LBL10},
	{"BNC",		INST,	1,	LBL10},
	{"BNE", 	INST,	1,	LBL10},
	{"BNZ", 	INST,	1,	LBL10},
	{"BRA", 	INST,	1,	LBL10},
	{"BSS",		DIR,	1,	C},
	{"BYTE",	DIR,	1,	C},
	{"BZ",		INST,	1,	LBL10},
	{"CEX",		INST,	3,	CEX}, // special code
	{"CMP",		INST,	2,	CR_R},
	{"CMP.B",	INST,	2,	CR_R},
	{"CMP.W",	INST,	2,	CR_R},
	{"DADD",	INST,	2,	CR_R},
	{"DADD.B",	INST,	2,	CR_R},
	{"DADD.W",	INST,	2,	CR_R},
	{"END",		DIR,	1,	Opt},
	{"EQU",		DIR,	1,	C},
	{"LD",		INST,	2,	R_R},
	{"LD.B",	INST,	2,	R_R},
	{"LD.W",	INST,	2,	R_R},
	{"LDR",		INST,	2,	R_R},
	{"LDR.B",	INST,	2,	R_R},
	{"LDR.W",	INST,	2,	R_R},
	{"MOV",		INST,	2,	CR_R},
	{"MOV.B",	INST,	2,	CR_R},
	{"MOV.W",	INST,	2,	CR_R},
	{"MOVH",	INST,	2,	C_R},
	{"MOVL",	INST,	2,	C_R},
	{"MOVLS",	INST,	2,	C_R},
	{"MOVLZ",	INST,	2,	C_R},
	{"ORG",		DIR,	1,	C},
	{"RRC",		INST,	1,	R},
	{"RRC.B",	INST,	1,	R},
	{"RRC.W",	INST,	1,	R},
	{"SRA",		INST,	1,	R},
	{"SRA.B",	INST,	1,	R},
	{"SRA.W",	INST,	1,	R},
	{"ST",		INST,	2,	R_R},
	{"ST.B",	INST,	2,	R_R},
	{"ST.W",	INST,	2,	R_R},
	{"STR",		INST,	2,	R_R},
	{"STR.B",	INST,	2,	R_R},
	{"STR.W",	INST,	2,	R_R},
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

int checkTable(Command (& tbl)[CMD_TBL_SIZE], string & mnemonic){
	int low = 0, high = CMD_TBL_SIZE - 1;
	while(low <= high){
		int mid = low + (high - low)/ 2;
		if (strcmp(mnemonic.c_str(), tbl[mid].name.c_str()) == 0){ // mnem same
			return mid;
		} else if (strcmp(mnemonic.c_str(), tbl[mid].name.c_str()) < 0){ // mnem is lower
			high = mid - 1;
		} else if (strcmp(mnemonic.c_str(), tbl[mid].name.c_str()) > 0){ // mnem is higher
			low = mid + 1;
		}
	}
	return -1;
}

bool validConstant(string operand){
	int i = 1;
	if(operand[1] == '-') // if negative start at position 2
		i++;
	switch(operand[0]){
	case '#': // decimal
		for(i; i < operand.length(); i++){
			// if digit is NOT between '0' and '9'
			if(!('0' <= operand[i] && operand[i] <= '9'))
				return false;
		}
		return true;
	case '$': // hexadecimal
		for(i; i < operand.length(); i++){
			// if digit is NOT (between '0' and '9' or between 'A' and 'F')
			if(!(('0' <= operand[i] && operand[i] <= '9') || \
			('A' <= operand[i] && operand[i] <= 'F')))
				return false;
		}
		break;
	default: // does not start with # or $: invalid operand
		return false;
	}
	// if we made it here, operand is valid.
	return true;
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
		delimiterPos = operands.find(' '); // maybe followed by space delim garbage
		if(delimiterPos == string::npos){ // no space
			operand = operands;
			operands.erase(0, operands.length());
			return operand;
		}
	}
	
	// else make substring of operands containing first operand
	operand = operands.substr(0, delimiterPos);
	// erase first operand + delim from operands
	operands.erase(0, operand.length() + 1);
	
	return operand;
}

