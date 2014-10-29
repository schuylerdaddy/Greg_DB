
#ifndef DBMS_INode
#define DBMS_INode

#include "Node.h"
#include "DataNode.h"

class IndexNode:public Node
{
private:
	void insertKeyAndNodeInPlace(int pos,int, Node* child,bool);
	void insertKeyAndNodeInPlace(int pos, int,Node*, Node* child,bool);
	IndexNode* split(bool even);
	void PrintList(){}
	int minMax();
	int anchorKeyIdx;
    Cache* cache;
public:
	IndexNode(Cache*);
    IndexNode(FactoryNode,Cache*);
    void SetCache(Cache* c){cache =c;}
	//void* operator new(size_t siz){
	//	void* n = std::malloc(siz);
	//	std::ofstream outData;
	//	outData.open("allocatedNodes.txt",std::ios_base::app);
	//	if (!outData)
	//	{
	//		std::cerr << "cannot open file." << std::endl;
	//	}
	//	outData << "*" << n << endl;
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
	//	outData << "*" << p << " "<<reinterpret_cast<Node*>(p)->rid<<endl;
	//	outData.close();
	//	std::free(p);
	//}
	int insert(int,DataTuple);
	void TreeDump(int);
	~IndexNode()
	{
      /*  if(childNodes != NULL)
            for (int idx = 0; idx < keyCount+1; ++idx)
            {
                if(childNodes[idx] != NULL && childNodes[idx] != nullptr)
                    delete childNodes[idx];
            }*/
	}
	int delKey(int, Node*, Node*, Node*, Node*, int);
	int swapKey(int key);
	int swapKey(int key, bool);
	int removeChildNode(Node*);
	DataTuple getTuple(int){ return DataTuple(); }
	void setTuple(DataTuple, int){}
	int getFirst();
	char* genBytes(int);
	FactoryNode genBytes();
	bool search(int,DataTuple&);
	void commit(Node*, Node*, Node*, Node*);
    Node* AssembleNode(FactoryNode);
};  

#endif