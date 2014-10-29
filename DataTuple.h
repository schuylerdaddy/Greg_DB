#ifndef DBMS_DT_
#define DBMS_DT_

#include <string>
#include <fstream>
#include "FileIO.h"

using namespace std;

class TupString{
public:
	char str[TUPLE_SIZE];
	TupString(char* other){ memcpy(str, other, TUPLE_SIZE); }
};

class DataTuple{
public:
	int dkey;
	char data[TUPLE_SIZE];

	DataTuple();
	//DataTuple(int k, std::string d){ key = k; memcpy(data, d.c_str(), 104);};
	DataTuple(int k, char* d){ dkey = k; memcpy(data, d, TUPLE_SIZE); };
    DataTuple(char* d);
	TupString genTupBytes();
	DataTuple loadTup(std::fstream f);
    bool operator ==(DataTuple);
    bool operator !=(DataTuple);
};


#endif