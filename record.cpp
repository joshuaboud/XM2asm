#include "record.hpp"

// holds all records for printing into list file:
Record * records;

void initRecords(Record *& head){
	head = NULL;
}

void pushRecord(Record *& head, int lineNum, string rec, \
string error, int memLoc){
	Record * temp = new Record;
	temp->lineNum = lineNum;
	temp->memLoc = memLoc;
	temp->record = rec;
	temp->error = error;
	temp->next = head;
	head = temp;
}

ostream & operator<<(ostream & os, const Record * rec){
	os << rec->lineNum << setw(10);
	(rec->memLoc >= 0)? os << rec->memLoc : os << "";
	os << rec->record;
	(rec->error.empty())? os << endl : os << ' ' << rec->error << endl;
	return os;
}

void printRecords(ostream & os, Record * head){
	Record * itr = head;
	while(itr != NULL){
		os << itr;
		itr = itr->next;
	}
}
