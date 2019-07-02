/* File name: commands.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Necessary includes and declarations for commands.cpp.
 * 			Include this file to access the global command table
 * 			and its related functions.
 * Last Modified: 2019-06-29
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>

#include <string.h> // for strcmp

#include "operands.hpp"

#define CMD_TBL_SIZE 83
#define CMD_NOT_FOUND -1

enum Command_type { DIR, INST };
enum Operand_type { NONE, CR_R, R_R, C, R, Opt, V, CEX_,\
	 SA, LD_, ST_, LDR_, STR_, BRA_, BRA13, V_R };
enum Opsz { W = 0, B };

struct Command {
	std::string mnem;
	Command_type type;
	int opCnt;
	Operand_type ops;
	Opsz wb;
	unsigned short baseOp;
};

extern Command commands[CMD_TBL_SIZE]; // array of instructions & directives

typedef enum {
	ADD, ADD_B, ADD_W, ADDC, ADDC_B, ADDC_W, ALIGN, AND, AND_B, AND_W, 
	BC, BEQ, BGE, BHS, BIC, BIC_B, BIC_W, BIS, BIS_B, BIS_W, BIT, BIT_B, 
	BIT_W, BL, BLO, BLT, BN, BNC, BNE, BNZ, BRA, BSS, BYTE, BZ, CEX, 
	CMP, CMP_B, CMP_W, DADD, DADD_B, DADD_W, END, EQU, LD, LD_B, LD_W, 
	LDR, LDR_B, LDR_W, MOV, MOV_B, MOV_W, MOVH, MOVL, MOVLS, MOVLZ, ORG, 
	RRC, RRC_B, RRC_W, SRA, SRA_B, SRA_W, ST, ST_B, ST_W, STR, STR_B, 
	STR_W, SUB, SUB_B, SUB_W, SUBC, SUBC_B, SUBC_W, SVC, SWAP, SWPB, 
	SXT, WORD, XOR, XOR_B, XOR_W
} CommandEnum;

int checkTable(Command (& tbl)[CMD_TBL_SIZE], std::string mnemonic);
// Returns positional subscript of array element by name,
// or -1 if element not found. Uses binary search.

#endif
