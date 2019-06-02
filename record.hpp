/* File name: record.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Necessary includes and definitions for record.cpp.
 * 			Include this file to access the global record list.
 * Last Modified: 2019-05-31
 */

#ifndef RECORD_H
#define RECORD_H

#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

#include <stdio.h> // for printing hex mem location

struct Record {
	int lineNum;
	int memLoc;
	string record;
	string error;
	struct Record * next;
	struct Record * prev;
};

extern Record * records; // buffer of records to be printed
extern Record * records_end;  // for printing the right way around
extern bool ERROR_FLAG; // flag will be set on finding an error
// this flag stops execution of the second pass

void initRecords(Record *& head);
// Initializes head and records_end to NULL

void pushRecord(Record *& head, int lineNum, string rec, \
string error = "", int memLoc = -1);
// Pushes record ptr to head of record list, if records_end is still
// pointing to NULL, points it to the new head to keep track of the
// bottom of the list.

ostream & operator<<(ostream & os, const Record * rec);
// Prints fields of record on output stream.

void printRecords(ostream & os, Record * head);
// Iterates over list of records, printing each one.

void destroyRecords(Record * head);
// Frees memory taken by record list.

string getNextToken(istringstream & record);
// Extracts next token out of stream, handles comments and errors

#endif
