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
