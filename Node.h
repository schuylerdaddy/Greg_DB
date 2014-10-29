
#ifndef DBMS_Node
#define DBMS_Node

#include <iostream>
#include <iomanip>
#include "DataTuple.h"
#include "Cache.h"
#include <fstream>
#include <string>

class Node{
 public:    
	 int rid;
	 int nextRid;
	 int keyCount;
	 int minKey;
	 int maxKey;
	 int height;
	 int level;
	 int keys[MAX_KEY];
     Cache* cache;
	 int childRid[MAX_KEY + 1];
	 
	 //methods

	 virtual void TreeDump(int)=0;
	 virtual void PrintList() = 0;
	 virtual int insert(int, DataTuple) = 0;
	 virtual int minMax() = 0;
	 virtual ~Node(){}
	 virtual int delKey(int, Node*, Node*, Node*, Node*, int) = 0;
	 virtual int swapKey(int key) = 0;
	 virtual int swapKey(int key, bool) = 0;
	 virtual int removeChildNode(Node*) = 0;
	 virtual DataTuple getTuple(int) = 0;
	 virtual void setTuple(DataTuple, int) = 0;
	 virtual int getFirst() = 0;
	 virtual char* genBytes(int) = 0;
	 virtual FactoryNode genBytes() = 0;
	 virtual bool search(int,DataTuple&)=0;
};

#endif