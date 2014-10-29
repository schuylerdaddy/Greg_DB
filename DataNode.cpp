
#include <iostream>
#include "DataNode.h"

DataNode::DataNode(Cache* c)
{
	height = 0;
	keyCount = 0;
	cache = c;
	 minKey = MAX_KEY/2;
	 maxKey = MAX_KEY;
	 nextRid = -1;
}

DataNode::DataNode(FactoryNode n, Cache* c)
{
	nextRid = n.nxt;
	rid = n.RID;
    maxKey = MAX_KEY;
    minKey = MAX_KEY/2;
    cache = c;
    keyCount = n.kCount;
	height = n.hgt;
    
	for (int i = 0; i < keyCount; ++i)
	{
		keys[i] = n.kVal[i];
        tuples[i] = n.tuple[i];
	}
}

DataNode::~DataNode()
{
}

int DataNode::insert(int keyValue, DataTuple tup)
{
	DataNode* rNode= nullptr;

	for (int idx = 0; idx < keyCount; idx++)
	{
		if (keys[idx] == keyValue) //replace data and return
		{
			tuples[idx] = tup;
			return -1;
		}
			
	}

	if (keyCount == maxKey)
	{
		minKey = (maxKey / 2);
		int minMax = keys[minKey];

		//split
		rNode = new DataNode(cache);
		
		for (int idx = minKey; idx < maxKey; idx++)
		{
			rNode->keys[idx-minKey] = keys[idx];
			rNode->tuples[idx-minKey] = tuples[idx];
		}

		rNode->keyCount = minKey;
		this->keyCount = minKey;
		//place new key

		if (keyValue < minMax)
		{
			keys[minKey] = keyValue;
			tuples[minKey] = tup;
			keyCount++;
			Sort(this->keys,this->tuples, keyCount);
		}
		else
		{
			rNode->keys[minKey] = keyValue;
			rNode->tuples[minKey] = tup;
			rNode->keyCount++;
			Sort(rNode->keys, rNode->tuples, rNode->keyCount);
		}

		//rNode->next = this->next;
		//rNode->nextRid = this->nextRid;
		//this->nextRid = rNode->rid;
		//this->next = rNode;	
	}
	else
	{
		keys[keyCount] = keyValue;
		tuples[keyCount] = tup;
		keyCount++;
		Sort(this->keys, this->tuples, this->keyCount);
	}
	int ret = -1;
	if (rNode != nullptr)
	{
		rNode->rid = cache->GetAvailableRIDLRU(rNode->genBytes());
		rNode->nextRid = this->nextRid;
		this->nextRid = rNode->rid;
		ret = rNode == nullptr ? -1 : rNode->rid;
		cache->WriteToCacheLRU(rNode->rid,rNode->genBytes());
	}
	cache->WriteToCacheLRU(this->rid,this->genBytes(), false);
	return ret;
}

void DataNode::Sort(int* keyArray, DataTuple* tupArray, int numKeys)
{
	for (int idx =numKeys-1; idx > 0; idx--)
	{
		if (keyArray[idx] < keyArray[idx - 1])
		{
			int tempKey = keyArray[idx-1];
			DataTuple tempTup = tupArray[idx - 1];
			keyArray[idx - 1] = keyArray[idx];
			tupArray[idx - 1] = tupArray[idx];
			keyArray[idx] = tempKey;
			tupArray[idx] = tempTup;
		}
	}
}

void DataNode::PrintList()
{
	std::cout << "[";
	if (keyCount != 0)
	for (int idx = 0; idx < keyCount; idx++)
	{
		std::cout << keys[idx];
		if (idx + 1 < keyCount)
			cout << ",";
	}
	std::cout << "]";
	if (this->nextRid > 0)
	{
		cout << ",";
		Node* temp = AssembleNode(cache->GetNodeLRU(nextRid));
		temp->PrintList();
		delete temp;
	}
}

void DataNode::TreeDump(int depth)
{
	cout << std::setw(depth * 2) << "";
	std::cout << "[";
	if (keyCount != 0)
	for (int idx = 0; idx < keyCount; idx++)
	{
		std::cout << keys[idx];
		if (idx + 1 < keyCount)
			cout << ",";
	}
	std::cout << "] " <<rid<< endl;
}

int DataNode::minMax()
{
	return keys[0];
}

int DataNode::delKey(int k, Node* leftNeighbor, Node* rightNeighbor, Node* lPivot, Node* rPivot, int level)
{
	int keyIdx = 0;
	while (k > keys[keyIdx] && keyIdx < keyCount)// get to key position
		++keyIdx;
	if (keyIdx == keyCount || k != keys[keyIdx]) //key not found
		return -1;

	removeKey(keyIdx); //remove key and decrement

	if (keyCount< minKey)
	{
			bool leftHeight = leftNeighbor != nullptr && rightNeighbor != nullptr && lPivot->level >= rPivot->level;
			bool leftOnly = rightNeighbor == nullptr && leftNeighbor != nullptr;
			bool rightOnly = leftNeighbor == nullptr && rightNeighbor != nullptr;
			bool leftKeys = leftNeighbor != nullptr && ((rightNeighbor != nullptr && rightNeighbor->keyCount == minKey) && leftNeighbor->keyCount>minKey);
			bool rightAboveMinkeys = rightNeighbor != nullptr && rightNeighbor->keyCount > minKey;

			if ((leftHeight || leftOnly)&& (leftNeighbor->keyCount>minKey || leftKeys))
			{
				leftNeighbor->keyCount--;
				for (int i = minKey; i > 0; --i)
				{
					keys[i] = keys[i - 1];
					tuples[i] = tuples[i - 1];
				}
				keys[0] = leftNeighbor->keys[leftNeighbor->keyCount];
				tuples[0] = leftNeighbor->getTuple(leftNeighbor->keyCount);
				lPivot->swapKey(keys[0]);
				++keyCount;
			}
			//check shift rl  option#2
			else if ((!leftHeight || rightOnly || leftNeighbor->keyCount <= minKey) && rightAboveMinkeys)
			{
				rightNeighbor->keyCount--;

				tuples[keyCount] = rightNeighbor->getTuple(0);
				//change min max, shift to new slot
				keys[keyCount] =rightNeighbor->keys[0];
				rPivot->swapKey(rightNeighbor->keys[1], true);  //swapout key for new minmax
				for (int i = 0; i < rightNeighbor->keyCount; ++i)
				{
					rightNeighbor->keys[i] = rightNeighbor->keys[i + 1];
					rightNeighbor->setTuple(rightNeighbor->getTuple(i + 1), i);
				}
				++keyCount;
			}
			//checkMerges
			else if (leftHeight || leftOnly)
			{
				int i = 0;
				while (i < keyCount)
				{
					leftNeighbor->insert(keys[i], tuples[i]);
					++i;
				}
				lPivot->removeChildNode(this);
				//leftNeighbor->next = this->next;
				leftNeighbor->nextRid = this->nextRid;
				commit(rightNeighbor, leftNeighbor, rPivot, lPivot);
				return this->rid;
			}
			else if ((rightNeighbor != nullptr && !leftHeight) || rightOnly) 
			{
				int i = 0;
				while (i < rightNeighbor->keyCount)
				{
					this->insert(rightNeighbor->keys[i], rightNeighbor->getTuple(i));
					++i;
				}
				rPivot->removeChildNode(rightNeighbor);
				//this->next = rightNeighbor->next;
				this->nextRid = rightNeighbor->nextRid;
				int ret = rightNeighbor->rid;
				commit(rightNeighbor, leftNeighbor, rPivot, lPivot);
				return ret;
			}
			//else tree is root or out of whack
	}
	commit(rightNeighbor, leftNeighbor, rPivot, lPivot);
	return -1;
}

void DataNode::removeKey(int keyIdx)
{
	--keyCount;
	//move to back
	while (keyIdx < keyCount)
	{
		keys[keyIdx] = keys[keyIdx + 1];
		tuples[keyIdx] = tuples[(++keyIdx)];
	}
}

int DataNode::swapKey(int k)
{
	return std::numeric_limits<int>::min(); //easy to see error, this should never get invoked
}

char* DataNode::genBytes(int rid)
{
	FactoryNode xfer = FactoryNode();
	xfer.type = 'L';
	xfer.nxt = nextRid;
	xfer.RID = rid;
	xfer.kCount = keyCount;
	xfer.hgt = height;
	for (int i = 0; i < keyCount; ++i)
	{
		xfer.kVal[i] = keys[i];
	}
	//char* buff = reinterpret_cast<char*>(&xfer);
	//DBMS::Factory::write(buff, rid);
	/*for (int i = 0; i < keyCount; ++i)
	{
		tuples[i].genTupBytes();
	}*/
	return *reinterpret_cast<char**>(&xfer);
}

FactoryNode DataNode::genBytes()
{
	FactoryNode xfer = FactoryNode();
	xfer.type = 'L';
	xfer.nxt = nextRid;
	xfer.RID = rid;
	xfer.kCount = keyCount;
	xfer.hgt = height;
	for (int i = 0; i < keyCount; ++i)
	{
		xfer.kVal[i] = keys[i];
	}

	//memcpy(buffer, reinterpret_cast<char*>(&xfer), sizeof(FactoryNode));
    size_t nodeSize = sizeof(FactoryNode);
    if(xfer.RID == 0)
        nodeSize = 2;
    
	for (int i = 0; i < keyCount;++i)
    {
		//memcpy(buffer + nodeSize + (i*TUPLE_SIZE),ts.str ,TUPLE_SIZE);
        memcpy(xfer.tuple[i],tuples[i].data ,TUPLE_SIZE);
    }
	return xfer;
}


void DataNode::commit(Node* a, Node* b, Node* c, Node* d)
{
	if (a != nullptr)
		cache->WriteToCacheLRU(a->rid,a->genBytes());
	if (b != nullptr)
		cache->WriteToCacheLRU(b->rid,b->genBytes());
	if (c != nullptr)
		cache->WriteToCacheLRU(c->rid,c->genBytes(),false);
	if (d != nullptr)
		cache->WriteToCacheLRU(d->rid , d->genBytes(),false);
	cache->WriteToCacheLRU(this->rid,this->genBytes(),false);
}

bool DataNode::search(int key,DataTuple& dt)
{
	int i;
	for (i = 0; keys[i] != key && i < keyCount; ++i); //see if key exists
	if (i == keyCount)
		return false;
    dt = tuples[i];
	return true;
}

Node* DataNode::AssembleNode(FactoryNode n)
{
    return new DataNode(n,cache);
}

