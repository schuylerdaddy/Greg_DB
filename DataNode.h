#ifndef DBMS_DNode
#define DBMS_DNode

#include "DataTuple.h"
#include "Cache.h"
#include "Node.h"

class DataNode:public Node{
private:
	void Sort(int*, DataTuple*, int);
	DataTuple tuples [MAX_KEY];
    Cache* cache;
public:
	DataNode(Cache*);
    DataNode(FactoryNode,Cache*);
    void SetCache(Cache* c){cache =c;}
	//void* operator new(size_t siz){
	//	void* n = std::malloc(siz); 
	//	std::ofstream outData;
	//	outData.open("allocatedNodes.txt", std::ios_base::app);
	//	if (!outData)
	//	{
	//		std::cerr << "cannot open file." << std::endl;
	//	}
	//	outData << "&" << n << endl;;
	//	outData.close();
	//	return n;
	//}
	//void operator delete(void* p)
	//{
	//	std::ofstream outData;
	//	outData.open("deletedNodes.txt", std::ios_base::app);
	//	if (!outData)
	//	{
	//		std::cerr << "cannot open file." << std::endl;
	//	}
	//	outData << "&" << p << " "<< reinterpret_cast<Node*>(p)->rid<<endl;
	//	outData.close();
	//	std::free(p);
	//}

	int insert(int, DataTuple);
	int minMax();

	void PrintList();
	void TreeDump(int);
	~DataNode(); 
	int delKey(int, Node*, Node*, Node*, Node*, int);
	int removeChildNode(Node*){ return -99; }
	void removeKey(int keyIdx);
	int swapKey(int key);
	int swapKey(int key, bool) { return -1; }
	DataTuple getTuple(int i){ return tuples[i]; }
	void setTuple(DataTuple d, int i){ tuples[i] = d; }
	int getFirst(){ return keys[0]; }
	char* genBytes(int);
	FactoryNode genBytes();
	bool search(int,DataTuple&);
	void commit(Node*, Node*, Node*, Node*);
    Node* AssembleNode(FactoryNode);
};

#endif