#ifndef DBMS_BPT
#define DBMS_BPT

#include "DataNode.h"
#include "IndexNode.h"

class BPlustree
{
private:
    int linkedProgressionId;
    int tupleIdx =0;
    int treeIdx;
    Cache* cache;
public:
	BPlustree();
	BPlustree(const char*);
	~BPlustree();

    bool GetNextLinkedTuple(DataTuple& tup);
    bool EndofLinkList();
	void PrintLinkedList();
	void TreeDump();
    void ShowBlocks();

    vector<Attribute> GetSchemaAttributes();
	int rootId;
	void insert(const int, DataTuple);
	int arity;
	int dataLinkRid;
	void delKey(const int);
	bool search(int key,DataTuple& dt);
	void updateSystemCat();
	void loadTree();
	void clear();
	void close();
    
    Node* AssembleNode(FactoryNode n);
};
#endif