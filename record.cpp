/* File name: record.cpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Defines functions to be used with records and the record
 * 			linked list, as well as defines the beginning and end of the
 * 			record linked list.
 * Last Modified: 2019-06-02
 */

#include "record.hpp"

// holds all records for printing into list file:
Record * records; // head of record list
Record * records_end; // rear of record list for printing FIFO

void initRecords(Record *& head){
	head = NULL;
	records_end = NULL;
}

void pushRecord(Record *& head, int lineNum, string rec, \
string error, int memLoc){
	if(!error.empty())
		ERROR_FLAG = true;
	Record * temp = new Record;
	temp->lineNum = lineNum;
	temp->memLoc = memLoc;
	temp->record = rec;
	temp->error = error;
	temp->next = head;
	temp->prev = NULL;
	if(head != NULL) // guard seg fault
		head->prev = temp;
	head = temp;
	if(records_end == NULL){
		records_end = head; // init list end pointer
	}
}

ostream & operator<<(ostream & os, const Record * rec){
	os << setw(3) << rec->lineNum;
	if(rec->memLoc >= 0){
		os << "   0x" << setfill('0') << setw(4); // print as 0xNNNN padded w/ 0
		os << hex << rec->memLoc << dec << " \t"; // print formatted hex
		os << setfill(' '); // clear setw fill
	}else{
		os << "          \t";
	}
	os << rec->record;
	(rec->error.empty())? os << endl : os << "\t!!! " << rec->error << endl;
	return os;
}

void printRecords(ostream & os){
	Record * itr = records_end;
	while(itr != NULL){
		os << itr;
		itr = itr->prev;
	}
}

void destroyRecords(Record * head){
	Record * itr = head;
	Record * temp;
	if(itr == NULL){
		return;
	}
	while(itr != NULL){
		temp = itr;
		itr = itr->next;
		delete temp; // analogous to free()
	}
}

string getNextToken(istringstream & record){
	string token;
	
	// to handle ' ' (space):
	record >> std::ws; // eat whitespace to peek at '
	if(record.peek() == '\''){ // next token will be char
		getline(record, token, ';'); // grab until ';' or newline
		return token;
	}
	
	record >> token;
	if(token.empty() || token[0] == ';'){
		// end of record
		record.clear(); // clear error bits to allow continued reading
		return "";
	}
	return token;
}
