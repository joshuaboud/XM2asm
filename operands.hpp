/* File name: operands.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Functions and tables for operand checking and encoding
 * Last Modified: 2019-06-29
 */

#ifndef OPERANDS_H
#define OPERANDS_H

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#define CONST_CNT 8
#define CHAR_LEN 3
#define ESC_CHAR_LEN 4

#define CEX_FLAG_CNT 17
typedef struct{
	std::string flag;
	int encoding;
} CEX_Flag;
extern CEX_Flag cexFlags[CEX_FLAG_CNT];

#define ESC_CHARS 8
enum {ESC_KEY, ESC_VAL};
extern char escapeChars[ESC_CHARS][2];

bool validValue(std::string operand);
// Returns true if operand passed is a valid value:
// [ # + (-) + { [0..9] } | $ + { [0..9 | A..F] } | ' + char + ' ]

int extractValue(std::string operand);
//	Returns integer value of string passed.

bool validConstant(std::string operand);
bool validConstant(short operand);
// Returns true if operand is "#0", "#1", "#2", "#4", "#8", "#16", "#32",
// "#-1", "$0", "$1", "$2", "$4", "$8", "$10", "$20", or "$FF". Else
// returns false. Overloaded for case where checking against symbol,
// where symbol->value is of type short.

std::string getOperand(std::string & operands);
// Extracts operands delimited by commas until end of record is reached.
// Erases extracted operand from string of operands, so each time it is
// called, the next operand will be extracted until the end of the string
// is reached, similar to strtok() from the C standard library.

int decodeConst(int val);
// Returns 3 bit encoding for passed constant as an integer

#endif
