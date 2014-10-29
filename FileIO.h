//
//  FileIO.h
//  RAEngine
//
//  Created by GregMac on 7/1/14.
//  Copyright (c) 2014 Mac. All rights reserved.
//

#ifndef __RAEngine__FileIO__
#define __RAEngine__FileIO__

#define MAX_KEY 4
#define TUPLE_SIZE 108
#define BLOCK_SIZE 512
#define BLOCK_COUNT 100

#include <fstream>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

enum DataType{none,integer,v20};

struct SystemCat{
	int blockSize;
	int blockCount;
	int usedBlocks;
	int rootLoc;
	int HeadLoc;
};

struct Attribute
{
	char field[20];
	char type[20];
	int pos;
    size_t offset;
};

class AttUtil
{
public:
    static size_t GetSchemaSize(vector<Attribute>);
    static size_t CombineScemas(vector<Attribute>&,vector<Attribute>);
    static map<string,int> AttributeSizes;
};

class FactoryNode
{
public:
    char type;
    int RID;
    int kCount;
    int kVal[MAX_KEY];
    int hgt;
    int nxt;
    int children[MAX_KEY + 1];
    char tuple[MAX_KEY][TUPLE_SIZE];
};

/*class FacString{
public:
    int rid;
	char str[512];
	FacString(int r, char* other){ memcpy(str, other, BLOCK_SIZE); rid = r; }
};*/

class DBFile{
public:
    DBFile(){}
    DBFile(fstream* s){fileData = s;}
    int factoryIdx;
    fstream* fileData;
    int blockSize;
    int blockNum;
    bool cached;
    bool isNew;
    std::vector<Attribute> fields;
    std::string TableName;
};

class FileIO
{
private	:
    FileIO();
    static vector<DBFile> binaryFiles;
    static mutex tableMutex;
    static mutex attMutex;
    static bool initialized;
public:
    static void Initialize();
    static int Connect(const char* fName,int);
	static std::fstream tableCat;
	static std::fstream attCat;
    
	static FactoryNode read(int,int rid);
	static void write(int,char[BLOCK_SIZE], int rid);
	static SystemCat readCatalog(int);
	static void writeCatalog(int,int root, int head, int used, int max);
	static void writeActiveBits(int,char*,int);
	static char* readActiveBits(int,int);
    static void cleanup(int);
	static FactoryNode AssembleNode(char*);
	static void ResetFile(int);
	static string LookUpCatalog(const char*, int&);
	static vector<Attribute> DefineData(string);
	static string Trim(string s);
    static int BlockNum(int facIdx);
    static bool IsNewDB(int facIdx);
    static void ToggleCaching(int fileIdx, bool on);
};

#endif /* defined(__RAEngine__FileIO__) */
