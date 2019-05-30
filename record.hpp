#ifndef RECORD_H
#define RECORD_H

#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

struct Record {
	int lineNum;
	int memLoc;
	string record;
	string error;
	struct Record * next;
};

extern Record * records;	//	buffer of records to be printed

void initRecords(Record *& head);

void pushRecord(Record *& head, int lineNum, string rec, \
string error = "", int memLoc = -1);
// Wrapper for std::vector::push_back() to handle record struct Record
// if memLoc is -1 (default), memory counter will not be printed on list
// file.

ostream & operator<<(ostream & os, const Record * rec);
// Prints fields of record on output stream.

void printRecords(ostream & os, Record * head);
// Iterates over list of records, printing each one.

#endif
