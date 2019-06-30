/* File name: error.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Provide a table of errors with enumeration
 * Last Modified: 2019-06-29
 */

#ifndef ERROR_H
#define ERROR_H

#include <string>

#define NUM_OF_ERRORS 24

extern std::string Error[NUM_OF_ERRORS];

typedef enum { NO_ERR = -1, EMPTY_RECORD, INV_LBL, DUP_LBL, LBL_REG,
	MULT_LBL, MISS_OP, UNEV_ADDR, INV_OP1, INV_OP2, INV_CONST,
	NOT_REG, NOT_VAL, CEX1, INV_OP3, OUT_BOUND, UNK_OP, EXTR_OP, GARB,
	POS_VAL, END_ERR, FWD_REF, SEC_PASS_NO_CMD, UNK_SYM, BRA_UNEV } ErrorEnum;

#endif
