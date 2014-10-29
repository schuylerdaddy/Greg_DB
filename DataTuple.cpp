#include "DataTuple.h"

DataTuple::DataTuple()
{
	dkey = -99;
	for (int i = 0; i < TUPLE_SIZE; ++i)
		data[i] = '\0';
}

DataTuple::DataTuple(char* bytes)
{
    if(bytes==NULL)
    {
        return;
    }
	memcpy(reinterpret_cast<void*>(&dkey),bytes ,sizeof(dkey));
	memcpy(data,bytes,TUPLE_SIZE);
}

TupString DataTuple::genTupBytes()
{
	char buff[TUPLE_SIZE];
	memcpy(buff, data, TUPLE_SIZE);
	return TupString(buff);
}

DataTuple DataTuple::loadTup(std::fstream f)
{
	DataTuple dt = DataTuple();
	f.read(reinterpret_cast<char*>(&dt.dkey), sizeof( int));
	f.read(reinterpret_cast<char*>(dt.data), TUPLE_SIZE);
	return dt;
}

bool DataTuple::operator==(DataTuple dt)
{
    return strncpy(this->data,dt.data,TUPLE_SIZE);
}

bool DataTuple::operator!=(DataTuple dt)
{
    return !strncpy(this->data,dt.data,TUPLE_SIZE);
}