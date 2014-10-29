//
//  FileIO.cpp
//  RAEngine
//
//  Created by GregMac on 7/1/14.
//  Copyright (c) 2014 Mac. All rights reserved.
//

#include "FileIO.h"
#include"Factory.h"
#include <string>
#include <algorithm>
#include <iterator>
#include <regex>
#include <thread>
#include <numeric>

#define BLK_SZ 512

std::fstream FileIO::tableCat;
std::fstream FileIO::attCat;
mutex FileIO::attMutex;
mutex FileIO::tableMutex;
vector<DBFile> FileIO::binaryFiles = vector<DBFile>();
bool FileIO::initialized;
map<string,int> AttUtil::AttributeSizes = map<string,int>();

const char* f;

void FileIO::Initialize()
{
    AttUtil::AttributeSizes["int"] = 4;
    AttUtil::AttributeSizes["V20"] = 21;
}

string FileIO::LookUpCatalog(const char* fName, int& blockCount)
{
    lock_guard<mutex> lock(tableMutex);
        ifstream tfile = ifstream("Table.cat");
        if (tfile)
        {
            tableCat.open("Table.cat", ios::in | ios::out | ios::binary);
            if(!tfile)
                throw runtime_error("Could not open Table.cat");
            string getter = "\n\t";
            string temp = "*.*";
            while (getline(tableCat, getter) && Trim(getter) != fName)
                temp = Trim(getter);
            if (tableCat.eof())
                throw runtime_error("File Not found in Table.Cat");
            else
                tableCat>>blockCount;
            tableCat.close();
            return temp;
        }
    throw runtime_error("Error occurred in proccessing Table.cat lookup");
}

vector<Attribute> FileIO::DefineData(string tableName)
{
    lock_guard<mutex> lock(attMutex);
	ifstream afile("Attr.cat");
    vector<Attribute> fields = vector<Attribute>();
	if (afile)
	{
		attCat.open("Attr.cat", ios::in | ios::out | ios::binary);
		string getter = "\n\t";
		while (getline(attCat, getter))
		{
			if (Trim(getter) == tableName)
			{
				Attribute atr = Attribute();
				getline(attCat, getter);
				memcpy(atr.field, Trim(getter).c_str(), 20);
				getline(attCat, getter);
				memcpy(atr.type, Trim(getter).c_str(), 20);
				getline(attCat, getter);
				if (!regex_match(Trim(getter), regex("[0-9]")))
					throw runtime_error("Expected position match:read non-integer value");
				atr.pos = stoi( Trim(getter));
				fields.push_back(atr);
			}
		}
		if (fields.size() <1)
			throw runtime_error("Unable to find attributes for " + tableName);
		sort(fields.begin(), fields.end(), [](Attribute a, Attribute b){return a.pos < b.pos; });
		//set tupleData
        
        attCat.close();
		return fields;
	}
	throw runtime_error("Error occured opening Attr.cat");
}

int FileIO::Connect(const char* fName, int blockNum)
{
    /*if (fName == "")
        return -1;
    try{
        
        fields = vector<Attribute>();
        blockSize = BLK_SZ;
        dbNew = false;
    }
    catch(exception e)
    {
        throw e;
    }*/
    
    int fileHandleIdx = (int)binaryFiles.size();
	ifstream ifile = ifstream(fName);
	if (ifile) {
        
		//check for cat files if don't exist create new
        fstream* fHandle = new fstream();
		fHandle->open(fName, ios::in | ios::out | ios::binary);
		ifile.close();
        DBFile dbHandle = DBFile(fHandle);
        dbHandle.blockSize = BLOCK_SIZE;
        dbHandle.isNew =false;
        dbHandle.blockNum = blockNum;
        binaryFiles.push_back(dbHandle);

	}
	else
	{
        //new
		ifile.close();
		unsigned int u = 1u;

        fstream* fHandle = new fstream();
		fHandle->open(fName, ios::in | ios::out | ios::binary | ios::trunc);
        
		fHandle->clear();
		fHandle->seekp(sizeof(SystemCat), ios::beg);
        
        DBFile dbHandle = DBFile(fHandle);
        
		fHandle->write(reinterpret_cast<char*>(&u),1);
		int range = blockNum % 8 == 0 ? (blockNum / 8) : blockNum +1/ 8 +2;
		u = 0u;
		for (int i = 0; i < range;++i)
			fHandle->write(reinterpret_cast<char*>(&u), 1);
        
        dbHandle.blockSize = BLOCK_SIZE;
        dbHandle.isNew = true;
        dbHandle.blockNum = blockNum;
        binaryFiles.push_back(dbHandle);
	}
    return fileHandleIdx;
}

void FileIO::ResetFile(int hndlIdx)
{
    binaryFiles[hndlIdx].fileData->close();
	binaryFiles[hndlIdx].fileData->open(f, ios::in | ios::out | ios::binary | ios::trunc);
}

FactoryNode FileIO::read(int hndlIdx, int rid)
{
	binaryFiles[hndlIdx].fileData->clear();
    int blockSize = binaryFiles[hndlIdx].blockSize;
	binaryFiles[hndlIdx].fileData->seekg(rid*blockSize, ios::beg);
    FactoryNode fn = FactoryNode();
    binaryFiles[hndlIdx].fileData->read(reinterpret_cast<char*>(&fn), sizeof(FactoryNode));
    return fn;
}

void FileIO::write(int hndlIdx, char* bytes, int rid)
{
	binaryFiles[hndlIdx].fileData->clear();
	binaryFiles[hndlIdx].fileData->flush();
    int blockSize = binaryFiles[hndlIdx].blockSize;
	binaryFiles[hndlIdx].fileData->seekp(rid*blockSize, ios::beg);
	binaryFiles[hndlIdx].fileData->write(bytes, BLOCK_SIZE);
}

SystemCat FileIO::readCatalog(int hndlIdx)
{
	SystemCat cat = SystemCat();
	binaryFiles[hndlIdx].fileData->clear();
	binaryFiles[hndlIdx].fileData->seekg(0, ios::beg);
	binaryFiles[hndlIdx].fileData->read(reinterpret_cast<char*>(&cat), sizeof(SystemCat));
    
	binaryFiles[hndlIdx].blockSize = cat.blockSize;
	return cat;
}

void FileIO::writeCatalog(int hndlIdx,int root, int head, int used, int max)
{
	binaryFiles[hndlIdx].fileData->clear();
	binaryFiles[hndlIdx].fileData->seekp(0, ios::beg);
	SystemCat cat = SystemCat();
	cat.blockCount = binaryFiles[hndlIdx].blockNum;
	cat.blockSize = binaryFiles[hndlIdx].blockSize;
	cat.usedBlocks = used;
	cat.HeadLoc = head;
	cat.rootLoc = root;
	binaryFiles[hndlIdx].fileData->write(reinterpret_cast<char*>(&cat), sizeof(SystemCat));
}

void FileIO::writeActiveBits(int hndlIdx, char * b, int size)
{
	binaryFiles[hndlIdx].fileData->clear();
	binaryFiles[hndlIdx].fileData->seekp(sizeof(SystemCat), ios::beg);
    
    //for cache
	if (binaryFiles[hndlIdx].cached)
	{
		for (int i = 0; i < size / 8; ++i)
			binaryFiles[hndlIdx].fileData->write(reinterpret_cast<char*>(b+i), 1);
		if (size%8 != 0)
			binaryFiles[hndlIdx].fileData->write(reinterpret_cast<char*>(b + (size/8+1)), 1);
		return;
	}
    
	unsigned int bits = 0;
	int bitCtr = 0;
	for (int i = 0; i < size; ++i)
	{
		bits |= b[i]!='0' ? 1 << (i % 8) : 0;
		bitCtr++;
		if (bitCtr == 8)
		{
			binaryFiles[hndlIdx].fileData->write(reinterpret_cast<char*>(&bits), 1);
			bits = 0;
			bitCtr = 0;
		}
	}
	if (bitCtr != 8)
	{
		binaryFiles[hndlIdx].fileData->write(reinterpret_cast<char*>(&bits), 1);
	}
}

void FileIO::cleanup(int hndlIdx)
{
	binaryFiles[hndlIdx].fileData->close();
}

char* FileIO::readActiveBits(int hndlIdx, int size)
{
	binaryFiles[hndlIdx].fileData->clear();
	binaryFiles[hndlIdx].fileData->seekg(sizeof(SystemCat), ios::beg);
    char byte = ' ';
    int num = (size%8 == 0)? size/8 : size/8 + 1;
    char* bits= new char[size];
    int bitCtr = 0;
    
	if (binaryFiles[hndlIdx].cached)
	{
		for (int i = 0; i<num && bitCtr<size; ++i)
		{
			binaryFiles[hndlIdx].fileData->read(&byte, 1);
			bits[i] = byte;
			bitCtr++;
		}
		return bits;
	}
    for(int i=0;i<num && bitCtr<size;++i)
    {
        binaryFiles[hndlIdx].fileData->read(&byte, 1);
        for(int j =0; j<8 && bitCtr<size;++j)
        {
            bits[bitCtr] = (1<<j & byte)? '1' :'0';
            bitCtr++;
        }
    }
	return bits;
}

int FileIO::BlockNum(int facIdx)
{
    return binaryFiles[facIdx].blockNum;
}

bool FileIO::IsNewDB(int facIdx)
{
    return binaryFiles[facIdx].isNew;
}

FactoryNode FileIO::AssembleNode(char* data)
{
    
    FactoryNode n = FactoryNode();
    memcpy(&n, data, BLOCK_SIZE);
    return n;
}

string FileIO::Trim(string s)
{
	s = regex_replace(s,regex("^[\\r\\n\\t]+"),"");
	return regex_replace(s, regex("[\\r\\n\\t]+$"), "");
}

void FileIO::ToggleCaching(int fileIdx, bool on)
{
    binaryFiles[fileIdx].cached = on;
}

size_t AttUtil::GetSchemaSize(vector<Attribute> attr)
{
    return accumulate(attr.begin(),attr.end(),0,[](int y,Attribute x)->size_t{return y + x.offset;});
}

size_t AttUtil::CombineScemas(vector<Attribute>& perm,vector<Attribute> temp)
{
    for(auto a : temp)
        perm.push_back(a);
    return GetSchemaSize(perm);
}