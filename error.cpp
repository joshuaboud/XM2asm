/* File name: error.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Provide a table of errors with enumeration
 * Last Modified: 2019-06-29
 */

#include "error.hpp"

bool ERROR_FLAG; // is set when pushRecord() is passed a non-empty error
// message. When the first pass is complete, the assembler will not
// continue to the second pass if this flag is set.

std::string Error[NUM_OF_ERRORS] = {
	"Empty record.",
	"Invalid label.",
	"Duplicate label definition.",
	"Label cannot be register.",
	"Label cannot follow label on same record.",
	"Missing operand(s).",
	"Instruction not on even mem addr. Use ALIGN.",
	"Invalid first operand.",
	"Invalid second operand.",
	"Operand must be constant.",
	"Operand must be register.",
	"Operand must be value.",
	"Instruction takes CEX flag as first operand.",
	"Invalid third operand.",
	"Operand value out of bounds.",
	"Unkown operand type (this should not happen).",
	"Extraneous operand(s).",
	"Non-comment garbage.",
	"Operand must be positive value.",
	"END operand must refer to label or mem location.",
	"Operand cannot be forward reference.",
	"Second pass cannot find INST or DIR.",
	"Unknown symbol in second pass.",
	"Branch to uneven address."
};
