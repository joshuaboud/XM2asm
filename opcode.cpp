/* File name: opcode.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Functions to generate opcodes
 * Last Modified: 2019-06-31
 */

#include "opcode.hpp"

#define IS_BYTE(x) ((x & ~0xFF) == 0)
#define IS_WORD(x) ((x & ~0xFFFF) == 0)
#define IN_BOUNDS_BRA10(x) (x >= -1024 && x <= 1022)
#define IN_BOUNDS_BRA13(x) (x >= -8192 && x <= 8190)
#define IN_BOUNDS_MEMR(x) (x >= -64 && x <= 63)
#define IN_BOUNDS_SVC(x) (x >= 0 && x <= 15)
#define IN_BOUNDS_CEX(x) (x >= 0 && x <= 7)
#define HIGH_BYTE(x) ((x >> 8) & 0xFF)
#define LOW_BYTE(x) (x & 0xFF)

void arith(Record * record, std::string & operands){
	std::string operand = getOperand(operands);
	Symbol * symPtr;
	union arith_op tempOp;
	// set base opcode, wb built in
	tempOp.opcode = commands[record->cmdSubScr].baseOp;
	// fill in rest
	if((symPtr = checkTable(symtbl,operand)) != NULL){
		// operand in symtbl
		switch(symPtr->type){
		case REG:
			// set rc bit to register
			tempOp.bf.rc = r;
			tempOp.bf.src = symPtr->value;
			break;
		case LBL:
		{
			// set rc bit to const
			tempOp.bf.rc = c;
			// save constant encoding
			int tempConst = decodeConst(symPtr->value);
			if(tempConst == NOT_FOUND){
				// invalid constant
				record->error = INV_CONST;
				spstate = CHK_FIRST_TOK;
				return;
			}else{
				tempOp.bf.src = tempConst;
			}
			break;
		}
		default:
			// unknown symbol type
			record->error = UNK_SYM;
			break;
		}
	}else{ // regular constant
		tempOp.bf.rc = c;
		int tempConst = decodeConst(extractValue(operand));
		if(tempConst == NOT_FOUND){
			// invalid constant
			record->error = INV_CONST;
			spstate = CHK_FIRST_TOK;
			return;
		}else{
			tempOp.bf.src = tempConst;
		}
	}
	// get next operand, should be register
	operand = getOperand(operands);
	symPtr = checkTable(symtbl,operand);
	if(symPtr != NULL && symPtr->type == REG){
		// symbol found
		tempOp.bf.dst = symPtr->value;
	}else{
		// error if sym not found or sym not reg
		record->error = INV_OP2;
		spstate = CHK_FIRST_TOK;
		return;
	}
	// save opcode in record
	record->opcode = tempOp.opcode;
	spstate = CHK_FIRST_TOK;
}

void reginit(Record * record, std::string & operands){
	std::string operand = getOperand(operands);
	Symbol * symPtr;
	union reginit_op tempOp;
	tempOp.opcode = commands[record->cmdSubScr].baseOp;
	// first operand
	int val;
	if((symPtr = checkTable(symtbl,operand)) != NULL){
		// symbol
		val = symPtr->value;
	}else{ // regular value
		val = extractValue(operand);
	}
	if(IS_WORD(val)){
		switch(record->cmdSubScr){
		case MOVH:
			tempOp.bf.byte = HIGH_BYTE(val);
			break;
		default:
			tempOp.bf.byte = LOW_BYTE(val);
			break;
		}
	}else{ // operand out of bounds
		record->error = OUT_BOUND;
		spstate = CHK_FIRST_TOK;
		return;
	}
	// second operand
	operand = getOperand(operands);
	symPtr = checkTable(symtbl,operand);
	tempOp.bf.dst = symPtr->value;
	record->opcode = tempOp.opcode;
	spstate = CHK_FIRST_TOK;
}

void singr(Record * record, std::string & operands){
	std::string operand = getOperand(operands);
	Symbol * symPtr;
	union singr_op tempOp;
	tempOp.opcode = commands[record->cmdSubScr].baseOp;
	symPtr = checkTable(symtbl,operand);
	if(symPtr != NULL){
		tempOp.bf.dst = symPtr->value;
	}else{ // reg not found
		record->error = INV_OP1;
	}
	record->opcode = tempOp.opcode;
	spstate = CHK_FIRST_TOK;
}

void mem(Record * record, std::string & operands){
	std::string operand = getOperand(operands);
	Symbol * symPtr;
	union mem_op tempOp;
	tempOp.opcode = commands[record->cmdSubScr].baseOp;
	if(commands[record->cmdSubScr].ops == ST_){
		symPtr = checkTable(symtbl,operand);
		tempOp.bf.src = symPtr->value;
		operand = getOperand(operands);
	}
	
	if(operand[0] == '+'){
		operand = operand.substr(1,operand.length()-1); // pop off +
		tempOp.bf.prpo = pr;
		tempOp.bf.inc = true;
	}else if(operand[0] == '-'){
		operand = operand.substr(1,operand.length()-1); // pop off -
		tempOp.bf.prpo = pr;
		tempOp.bf.dec = true;
	}
	if(operand.back() == '+'){
		operand.pop_back(); // pop off +
		tempOp.bf.prpo = po;
		tempOp.bf.inc = true;
	}else if(operand.back() == '-'){
		operand.pop_back(); // pop off -
		tempOp.bf.prpo = po;
		tempOp.bf.dec = true;
	}
	
	symPtr = checkTable(symtbl,operand);
	
	if(commands[record->cmdSubScr].ops == ST_){
		tempOp.bf.dst = symPtr->value;
	}else{ // LD
		tempOp.bf.src = symPtr->value;
		operand = getOperand(operands);
		symPtr = checkTable(symtbl,operand);
		tempOp.bf.dst = symPtr->value;
	}
	
	record->opcode = tempOp.opcode;
	
	spstate = CHK_FIRST_TOK;
}

void bra(Record * record, std::string & operands){
	std::string operand = getOperand(operands);
	Symbol * symPtr;
	int label;
	symPtr = checkTable(symtbl,operand);
	if(symPtr != NULL){
		// symbol found
		label = symPtr->value;
	}else{
		// non-label value
		label = extractValue(operand);
	}
	
	// encode offset
	int dist = label - (record->memLoc + 2);
	if(dist % 2 != 0){
		// branch to uneven address
		record->error = BRA_UNEV;
	}
	int offset = dist >> 1;
	if(commands[record->cmdSubScr].ops == BRA_ && IN_BOUNDS_BRA10(offset)){
		union bra10_op tempOp;
		tempOp.opcode = commands[record->cmdSubScr].baseOp;
		tempOp.bf.off = offset;
		record->opcode = tempOp.opcode;
	}else if (commands[record->cmdSubScr].ops == BRA13 &&
	IN_BOUNDS_BRA13(offset)){
		union bra13_op tempOp;
		tempOp.opcode = commands[record->cmdSubScr].baseOp;
		tempOp.bf.off = offset;
		record->opcode = tempOp.opcode;
	}else{
		record->error = OUT_BOUND;
	}
	spstate = CHK_FIRST_TOK;
}

void memr(Record * record, std::string & operands){
	std::string operand = getOperand(operands);
	Symbol * symPtr;
	union memr_op tempOp;
	int value;
	tempOp.opcode = commands[record->cmdSubScr].baseOp;
	symPtr = checkTable(symtbl,operand);
	// save first operand (register) in source
	tempOp.bf.src = symPtr->value;
	// get second operand
	operand = getOperand(operands);
	
	if(commands[record->cmdSubScr].ops == STR_){
		symPtr = checkTable(symtbl,operand);
		// store dest if instruction is STR
		tempOp.bf.dst = symPtr->value;
		// get next operand
		operand = getOperand(operands);
	}
	
	// process offset value
	symPtr = checkTable(symtbl,operand);
	if(symPtr != NULL){
		// symbol found
		value = symPtr->value;
	}else{
		// regular value
		value = extractValue(operand);
	}
	if(IN_BOUNDS_MEMR(value)){
		tempOp.bf.off = value;
	}else{
		// offset out of range
		record->error = OUT_BOUND;
	}
	
	if(commands[record->cmdSubScr].ops == LDR_){
		// store dest if LDR
		// get next operand
		operand = getOperand(operands);
		symPtr = checkTable(symtbl,operand);
		tempOp.bf.dst = symPtr->value;
	}
	
	record->opcode = tempOp.opcode;
	
	spstate = CHK_FIRST_TOK;
}

void svc(Record * record, std::string & operands){
	std::string operand = getOperand(operands);
	Symbol * symPtr;
	union svc_op tempOp;
	tempOp.opcode = commands[record->cmdSubScr].baseOp;
	int value;
	
	// see if symbol
	symPtr = checkTable(symtbl, operand);
	if(symPtr != NULL){
		// symbol found
		value = symPtr->value;
	}else{
		// regular value
		value = extractValue(operand);
	}
	
	if(IN_BOUNDS_SVC(value)){
		tempOp.bf.sa = value;
	}else{
		record->error = OUT_BOUND;
	}
	
	record->opcode = tempOp.opcode;
	
	spstate = CHK_FIRST_TOK;
}

void cex(Record * record, std::string & operands){
	std::string operand = getOperand(operands);
	Symbol * symPtr;
	union cex_op tempOp;
	tempOp.opcode = commands[record->cmdSubScr].baseOp;
	
	int cond = decodeCEX(operand);
	int t, f;
	
	if(cond != -1){
		// flag found
		tempOp.bf.c = cond;
	}else{
		// invalid flag
		record->error = CEX1;
		spstate = CHK_FIRST_TOK;
		return;
	}
	
	// get next operand
	operand = getOperand(operands);
	
	// check if symbol
	symPtr = checkTable(symtbl, operand);
	if(symPtr != NULL){
		// symbol found
		t = symPtr->value;
	}else{
		// regular value
		t = extractValue(operand);
	}
	
	// get next operand
	operand = getOperand(operands);
	
	// check if symbol
	symPtr = checkTable(symtbl, operand);
	if(symPtr != NULL){
		// symbol found
		f = symPtr->value;
	}else{
		// regular value
		f = extractValue(operand);
	}
	
	if(IN_BOUNDS_CEX(t) && IN_BOUNDS_CEX(f)){
		tempOp.bf.t = t;
		tempOp.bf.f = f;
	}else{
		// out of bounds
		record->error = OUT_BOUND;
	}
	
	record->opcode = tempOp.opcode;
	
	spstate = CHK_FIRST_TOK;
}
