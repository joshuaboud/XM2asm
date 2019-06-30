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

void pushRecord(Record *& head, int lineNum, std::string rec, \
ErrorEnum error, int tblSub, int memLoc, unsigned short opcode){
	if(error != NO_ERR) // an error was passed
		ERROR_FLAG = true;
	Record * temp = new Record;
	temp->lineNum = lineNum;
	temp->cmdSubScr = tblSub;
	temp->memLoc = memLoc;
	temp->record = rec;
	temp->error = error;
	temp->opcode = opcode;
	temp->prev = head;
	temp->next = NULL;
	if(head != NULL) // guard seg fault
		head->next = temp;
	head = temp;
	if(records_end == NULL){
		records_end = head; // init list end pointer
	}
}

std::ostream & operator<<(std::ostream & os, const Record * rec){
	os << std::setw(3) << rec->lineNum;
	if(rec->memLoc >= 0){
		os << "   0x" << std::setfill('0'); // print as 0xNNNN padded w/ 0
		// print formatted hex
		os << std::uppercase; // A-F instead of a-f
		os << std::hex << std::setw(4) << rec->memLoc;
		os << std::hex << " 0x" << std::setw(4) << rec->opcode << std::dec << " \t";
		os << std::setfill(' '); // clear setw fill
	}else{
		os << "             \t";
	}
	os << rec->record;
	if(rec->error == -1){
		os << std::endl;
	}else{
		os << "\t!!! ERROR: " << Error[rec->error] << std::endl;
	}
	return os;
}

void printRecords(std::ostream & os){
	Record * itr = records_end;
	while(itr != NULL){
		os << itr;
		itr = itr->next;
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
		itr = itr->prev;
		delete temp; // analogous to free()
	}
}

std::string getNextToken(std::istringstream & record){
	std::string token = "";
	
	// to handle ' ' (space):
	record >> std::ws; // eat whitespace to peek at '
	if(record.peek() == '\''){ // next token will be char
		token.push_back(record.get()); // store first '
		if(record.peek() == '\\'){
			token.push_back(record.get()); // escaped char
		}
		token.push_back(record.get()); // store char
		if(record.peek() == '\''){
			token.push_back(record.get()); // store terminating '
		} else {
			record.clear(); // error, not a char
			return "";
		}
		if(!record.eof()){ // if not end of record, push back the rest
			std::string temp;
			record >> temp;
			token.append(temp);
		}
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
