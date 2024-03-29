/* File name: record.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Necessary includes and declarations for record.cpp.
 * 			Include this file to access the global record list.
 * Last Modified: 2019-06-29
 */

#ifndef RECORD_H
#define RECORD_H

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include "error.hpp"

struct Record { // doubly linked list node
	int lineNum;
	int memLoc;
	unsigned short opcode;
	int cmdSubScr;
	std::string record;
	ErrorEnum error;
	struct Record * next;
	struct Record * prev;
};

extern Record * records; // buffer of records to be printed
extern Record * records_end;  // for printing FIFO

void initRecords(Record *& head);
// Initializes head and records_end to NULL

void pushRecord(Record *& head, int lineNum, std::string rec, \
ErrorEnum error = NO_ERR, int tblSub = -1, int memLoc = -1, unsigned short opcode = -1);
// Pushes record ptr to head of record list, if records_end is still
// pointing to NULL, points it to the new head to keep track of the
// bottom of the list. If error is not empty, then the global error flag
// is set.

std::ostream & operator<<(std::ostream & os, const Record * rec);
// Prints fields of record on output stream.

void printRecords(std::ostream & os);
// Iterates over list of records, printing each one from back to front.

void destroyRecords(Record * head);
// Frees memory taken by record list.

std::string getNextToken(std::istringstream & record);
// Extracts next token out of stream, handles comments and errors

#endif
