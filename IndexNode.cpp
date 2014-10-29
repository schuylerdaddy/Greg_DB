//modifiers for node position relation to Key
#include "IndexNode.h"

const int LEFT = 0;
const int RIGHT = 1;


IndexNode::IndexNode(Cache* c)
{
    rid = -1;
     keyCount=0;
	 minKey = MAX_KEY/2;
	 maxKey = MAX_KEY;
	 height = 0;
	 level = 0;
	 nextRid = -1;
    cache = c;
}

IndexNode::IndexNode(FactoryNode fn,Cache* c)
{
    rid = fn.RID;
    minKey = MAX_KEY/2;
    maxKey = MAX_KEY;
    keyCount = fn.kCount;
    height = fn.hgt;
	for (int i = 0; i < keyCount; ++i)
	{
		keys[i] = fn.kVal[i];
        childRid[i] = fn.children[i];
	}
    cache = c;
	childRid[keyCount]=fn.children[keyCount];
}
/*
int IndexNode::insert(int key, DataTuple tup)
{
	//int keyPosition = -1;
	int idxPosition = -1;
	int iNode = -1;
	Node * extraNode = nullptr;

	for (int idx = 0; idx < keyCount; idx++)
	{
		Node * temp = nullptr;
		if (key == keys[idx])
		{
			idxPosition = idx;  
			//iNode = childNodes[idx+RIGHT]->insert(key, tup);
			temp = DBMS::Factory::read(childRid[idx + RIGHT]);
			iNode = temp->insert(key, tup);
			idx = keyCount;
		}
		else if (key < keys[idx])
		{
			idxPosition = idx;
			//iNode = childNodes[idx+LEFT]->insert(key, tup);
			temp = DBMS::Factory::read(childRid[idx + LEFT]);
			iNode = temp->insert(key, tup);
			idx = keyCount;
		}
		else if (key > keys[keyCount-1])
		{
			idx = keyCount;
			idxPosition = idx;
			//iNode = childNodes[keyCount]->insert(key, tup);
			temp = DBMS::Factory::read(childRid[keyCount]);
			iNode = temp->insert(key, tup);
		}
		if (temp != nullptr)
			delete temp;

		//rearrange
		if (iNode != -1)  //child split
		{
			if (keyCount < maxKey) //don't need split on this node
			{
				Node* iNodeAdd = DBMS::Factory::read(iNode);
				insertKeyAndNodeInPlace(idxPosition, RIGHT, iNodeAdd, true);
				iNodeAdd->genBytes(iNode);
				delete iNodeAdd;
				keyCount++;
			}
			else  //need to split this node
			{
				int idx = 0;
				while (keys[idx] < key && idx < keyCount)
					idx++;
				bool evenSplit = idx >= minKey;
				Node * extraNode = this->split(evenSplit);
				if (evenSplit)
				{
					extraNode->keyCount++;
					//insertKeyAndNodeInPlace(idx - minKey, (this->childNodes[keyCount]->keys[0]) < key, extraNode, iNode,false);
					for (int i = extraNode->keyCount-1; i>idx - minKey; --i)//copy keys right
					{
						extraNode->keys[i] = extraNode->keys[i - 1];
					}
					for (int i = 0; i < idx - minKey; ++i) //shift child nodes to left of insert position
					{
						//extraNode->childNodes[i] = extraNode->childNodes[i +1];
						extraNode->childRid[i] = extraNode->childRid[i + 1];
					}
					Node* INodeAdd = DBMS::Factory::read(iNode);
					extraNode->keys[idx - minKey] = INodeAdd->minMax();
					//extraNode->childNodes[idx - minKey] = INodeAdd;
					extraNode->childRid[idx - minKey] = iNode;
					//extraNode->rid = Registry::AddToCatalog(extraNode);
					INodeAdd->genBytes(INodeAdd->rid);
					delete INodeAdd;
				    extraNode->genBytes(extraNode->rid);
					genBytes(rid);
					int r = extraNode->rid;
					delete extraNode;
					return r;
				}
				else
				{
					this->keyCount++;
					Node* INodeAdd = DBMS::Factory::read(iNode);
					insertKeyAndNodeInPlace(idx,true , INodeAdd,true);
					INodeAdd->genBytes(iNode);
					delete INodeAdd;
					//extraNode->rid = Registry::AddToCatalog(extraNode);
					extraNode->genBytes(extraNode->rid);
					genBytes(rid);
					int r = extraNode->rid;
					delete extraNode;
					return r;
				}
			}
		}
	}
	int r = 0;
	if (extraNode != nullptr)
	{
		extraNode->rid = Registry::GetAvailableRID();
		//extraNode->rid = Registry::AddToCatalog(extraNode);
		extraNode->genBytes(extraNode->rid);
		r = extraNode->rid;
		delete extraNode;
	}
	genBytes(rid);
	return extraNode==nullptr?-1:r;
}
IndexNode* IndexNode::split(bool even)
{
IndexNode* iNode = new IndexNode();
iNode->rid = Registry::GetAvailableRID();
//iNode->rid = Registry::AddToCatalog(iNode);
iNode->height = height+1;
if (!even)
{
this->keyCount = minKey-1;
iNode->keyCount = minKey + 1;

int offset = minKey - 1;
//copy data with nodes to right
int idx = 0;
for (idx = 0; idx < minKey+1; idx++)
{
iNode->keys[idx] = this->keys[idx + offset];
//iNode->childNodes[idx] = this->childNodes[idx + offset+1];
iNode->childRid[idx] = this->childRid[idx + offset + 1];
}
}
else
{
this->keyCount = minKey;
iNode->keyCount = minKey;

int offset = minKey;
//copy data with nodes to right, insert dummy key
int idx = 0;
for (idx = 0; idx < minKey; idx++)
{
iNode->keys[idx] = this->keys[idx + offset];
//iNode->childNodes[idx+1] = this->childNodes[idx + offset+1];
iNode->childRid[idx + 1] = childRid[idx + offset + 1];
}
iNode->childRid[0] = this->childRid[offset];
}
return iNode;
}

void IndexNode::commit(Node* a, Node* b, Node* c, Node* d)
{
if (a != nullptr)
{
a->genBytes(a->rid);
delete a;
}
if (b != nullptr)
{
b->genBytes(b->rid);
delete b;
}
if (c != nullptr)
{
c->genBytes(c->rid);
}
if (d != nullptr)
{
d->genBytes(d->rid);
}
this->genBytes(rid);
}

int IndexNode::search(int key)
{
int keyPos = 0;
while (keyPos < keyCount && key >= keys[keyPos])
++keyPos;
Node* temp = DBMS::Factory::read(childRid[keyPos]);
int ret = temp->search(key);
delete temp;
return ret;
}
*/

int IndexNode::insert(int key, DataTuple tup)
{
	int idxPosition = -1;
	int iNode = -1;
	Node * extraNode = nullptr;

	for (int idx = 0; idx < keyCount; idx++)
	{
		Node * temp = nullptr;
		if (key == keys[idx])
		{
			idxPosition = idx;
			temp = AssembleNode(cache->GetNodeLRU(childRid[idx + RIGHT]));
			iNode = temp->insert(key, tup);
			idx = keyCount;
		}
		else if (key < keys[idx])
		{
			idxPosition = idx;
			temp = AssembleNode(cache->GetNodeLRU(childRid[idx + LEFT]));
			iNode = temp->insert(key, tup);
			idx = keyCount;
		}
		else if (key > keys[keyCount - 1])
		{
			idx = keyCount;
			idxPosition = idx;
			temp = AssembleNode(cache->GetNodeLRU(childRid[keyCount]));
			iNode = temp->insert(key, tup);
		}
		if (temp != nullptr)
			delete temp;

		//rearrange
		if (iNode != -1)  //child split
		{
			if (keyCount < maxKey) //don't need split on this node
			{
				Node* iNodeAdd = AssembleNode(cache->GetNodeLRU(iNode));
				insertKeyAndNodeInPlace(idxPosition, RIGHT, iNodeAdd, true);
				cache->WriteToCacheLRU(iNodeAdd->rid, iNodeAdd->genBytes());
				keyCount++;
			}
			else  //need to split this node
			{
				int idx = 0;
				while (keys[idx] < key && idx < keyCount)
					idx++;
				bool evenSplit = idx >= minKey;
				Node * extraNode = this->split(evenSplit);
				if (evenSplit)
				{
					extraNode->keyCount++;
					for (int i = extraNode->keyCount - 1; i>idx - minKey; --i)//copy keys right
					{
						extraNode->keys[i] = extraNode->keys[i - 1];
					}
					for (int i = 0; i < idx - minKey; ++i) //shift child nodes to left of insert position
					{
						extraNode->childRid[i] = extraNode->childRid[i + 1];
					}
					Node* INodeAdd = AssembleNode(cache->GetNodeLRU(iNode));
					extraNode->keys[idx - minKey] = INodeAdd->minMax();
					//extraNode->childNodes[idx - minKey] = INodeAdd;
					extraNode->childRid[idx - minKey] = iNode;
					//extraNode->rid = Registry::AddToCatalog(extraNode);
					cache->WriteToCacheLRU(INodeAdd->rid,INodeAdd->genBytes());
					int r = extraNode->rid;
					cache->WriteToCacheLRU(extraNode->rid,extraNode->genBytes());
					cache->WriteToCacheLRU(this->rid,this->genBytes(), false);
					return r;
				}
				else
				{
					this->keyCount++;
					Node* INodeAdd = AssembleNode(cache->GetNodeLRU(iNode));
					insertKeyAndNodeInPlace(idx, true, INodeAdd, true);
					cache->WriteToCacheLRU(INodeAdd->rid,INodeAdd->genBytes());
					int r = extraNode->rid;
					cache->WriteToCacheLRU(extraNode->rid,extraNode->genBytes());
					cache->WriteToCacheLRU(this->rid,this->genBytes(), false);
					return r;
				}
			}
		}
	}
	int r = 0;
	if (extraNode != nullptr)
	{
		extraNode->rid = cache->GetAvailableRIDLRU(extraNode->genBytes());
		r = extraNode->rid;
		cache->WriteToCacheLRU(extraNode->rid,extraNode->genBytes());
	}
	cache->WriteToCacheLRU(this->rid,this->genBytes(), false);
	return extraNode == nullptr ? -1 : r;
}

int IndexNode::delKey(int k, Node* leftNeighbor, Node* rightNeighbor, Node* lPivot, Node* rPivot, int lvl)
{
	level = lvl;
	int keyPos = 0;
	while (keyPos < keyCount && k >= keys[keyPos])
		++keyPos;
	Node * nextRight;
	Node * nextLeft;
	Node * lAnchor;
	Node * rAnchor;

	anchorKeyIdx = keyPos != 0 ? keyPos - 1 : keyPos;

	if (keyCount == keyPos)
	{
		nextRight = (rightNeighbor == nullptr) ? nullptr : AssembleNode(cache->GetNodeLRU(rightNeighbor->childRid[0]));
		rAnchor = (rightNeighbor == nullptr) ? nullptr : rPivot;
	}
	else
	{
		nextRight = AssembleNode(cache->GetNodeLRU(childRid[keyPos + 1]));
		rAnchor = this;
	}

	if (keyPos == 0)
	{
		nextLeft = (leftNeighbor == nullptr) ? nullptr : AssembleNode(cache->GetNodeLRU(leftNeighbor->childRid[leftNeighbor->keyCount]));
		lAnchor = (leftNeighbor == nullptr) ? nullptr : lPivot;
	}
	else
	{
		nextLeft = AssembleNode(cache->GetNodeLRU(childRid[keyPos - 1]));
		lAnchor = this;
	}

	Node * temp = AssembleNode(cache->GetNodeLRU(childRid[keyPos]));
	int current = temp->delKey(k, nextLeft, nextRight, lAnchor, rAnchor, level + 1);

	if (current != -1)  // node deleted below
	{
		cache->DeactivateRIDLRU(current);
	}
	cache->WriteToCacheLRU(temp->rid,temp->genBytes());
	//check for rebalance
	if (keyCount < minKey && (rightNeighbor != nullptr || leftNeighbor != nullptr))  //roots have no neighbors
	{

		bool leftHeight = leftNeighbor != nullptr && rightNeighbor != nullptr&& lPivot->level >= rPivot->level;
		bool leftOnly = rightNeighbor == nullptr && leftNeighbor != nullptr;
		bool rightOnly = leftNeighbor == nullptr && rightNeighbor != nullptr;
		bool leftKeys = leftNeighbor != nullptr && rightNeighbor != nullptr && rightNeighbor->keyCount == minKey && leftNeighbor->keyCount>minKey;
		bool rightAboveMinkeys = rightNeighbor != nullptr && rightNeighbor->keyCount > minKey;

		//check shift lr
		if ((leftHeight || leftOnly) && (leftNeighbor->keyCount>minKey || leftKeys))
		{
			for (int i = minKey; i > 0; --i)
			{
				keys[i] = keys[i - 1];
				childRid[i] = childRid[i - 1];
			}
			keys[0] = lPivot->swapKey(leftNeighbor->keys[leftNeighbor->keyCount - 1]);
			childRid[0] = leftNeighbor->childRid[leftNeighbor->keyCount];
			--(leftNeighbor->keyCount);
			++keyCount;
		}
		//check shift rl  option#2
		else if ((!leftHeight || rightOnly || leftNeighbor->keyCount <= minKey) && rightAboveMinkeys)
		{
			rightNeighbor->keyCount--;
			int movedKey = rPivot->swapKey(rightNeighbor->keys[0], true);
			keys[keyCount] = movedKey;
			childRid[keyCount + 1] = rightNeighbor->childRid[0];
			int minMax = rightNeighbor->keys[0];
			for (int i = 0; i < rightNeighbor->keyCount; ++i)  //copy left
			{
				rightNeighbor->keys[i] = rightNeighbor->keys[i + 1];
				//rightNeighbor->childNodes[i] = rightNeighbor->childNodes[i + 1];
				rightNeighbor->childRid[i] = rightNeighbor->childRid[i + 1];
			}
			//rightNeighbor->childNodes[rightNeighbor->keyCount] = rightNeighbor->childNodes[rightNeighbor->keyCount + 1];
			rightNeighbor->childRid[rightNeighbor->keyCount] = rightNeighbor->childRid[rightNeighbor->keyCount + 1];
			rPivot->swapKey(minMax, true);
			++keyCount;
		}
		//merges
		else if (leftHeight || leftOnly)
		{
			int l = leftNeighbor->keyCount;
			for (int idx = 0; idx < keyCount; idx++) //copy keys and nodes
			{
				leftNeighbor->keys[l + idx + 1] = keys[idx];
				leftNeighbor->childRid[l + idx + 1] = childRid[idx];
			}
			//leftNeighbor->childNodes[l + keyCount + 1] = childNodes[keyCount];
			leftNeighbor->childRid[l + keyCount + 1] = childRid[keyCount];
			leftNeighbor->keyCount += keyCount + 1;
			leftNeighbor->keys[l] = lPivot->removeChildNode(this);
			commit(rightNeighbor, leftNeighbor, lPivot, rPivot);
			return this->rid;
		}
		else if (!leftHeight || rightOnly)
		{

			int l = keyCount;
			for (int idx = 0; idx < rightNeighbor->keyCount; idx++) //copy keys and nodes
			{
				keys[l + idx + 1] = rightNeighbor->keys[idx];
				childRid[l + idx + 1] = rightNeighbor->childRid[idx];
			}
			childRid[l + rightNeighbor->keyCount + 1] = rightNeighbor->childRid[rightNeighbor->keyCount];
			keyCount += rightNeighbor->keyCount + 1;
			keys[l] = rPivot->removeChildNode(rightNeighbor);
			int ret = rightNeighbor->rid;
			commit(rightNeighbor, leftNeighbor, lPivot, rPivot);
			return ret;
		}
		//else tree is root or out of whack
	}
	commit(rightNeighbor, leftNeighbor, lPivot, rPivot);
	return -1;
}

bool IndexNode::search(int key,DataTuple& dt)
{
	int keyPos = 0;
	while (keyPos < keyCount && key >= keys[keyPos])
		++keyPos;
	Node* temp = AssembleNode(cache->GetNodeLRU(childRid[keyPos]));
	bool ret = temp->search(key,dt);
	delete temp;
	return ret;
}

IndexNode* IndexNode::split(bool even)
{
	IndexNode* iNode = new IndexNode(cache);
	iNode->rid = cache->GetAvailableRIDLRU(iNode->genBytes());
	iNode->height = height+1;
	if (!even)
	{
		this->keyCount = minKey-1;
		iNode->keyCount = minKey + 1;

		int offset = minKey - 1;
		//copy data with nodes to right
		int idx = 0;
		for (idx = 0; idx < minKey+1; idx++)
		{
			iNode->keys[idx] = this->keys[idx + offset];
			iNode->childRid[idx] = this->childRid[idx + offset + 1];
		}
	}
	else
	{
		this->keyCount = minKey;
		iNode->keyCount = minKey;

		int offset = minKey;
		//copy data with nodes to right, insert dummy key
		int idx = 0;
		for (idx = 0; idx < minKey; idx++)
		{
			iNode->keys[idx] = this->keys[idx + offset];
			//iNode->childNodes[idx+1] = this->childNodes[idx + offset+1];
			iNode->childRid[idx + 1] = childRid[idx + offset + 1];
		}
		iNode->childRid[0] = this->childRid[offset];
	}
	return iNode;
}

void IndexNode::commit(Node* a, Node* b, Node* c, Node* d)
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

void IndexNode::insertKeyAndNodeInPlace(int pos, int nodeOffset, Node* child, bool oddSplit)
{
	insertKeyAndNodeInPlace(pos, nodeOffset, this, child, oddSplit);
}

void IndexNode::insertKeyAndNodeInPlace(int pos, int nodeOffset, Node* target, Node* child, bool oddSplit)
{
	//shift data right
	int idx = maxKey - 1;
	for (; idx>pos; idx--)
	{
		target->keys[idx] = target->keys[idx - 1];
		//target->childNodes[idx + nodeOffset+1] = target->childNodes[idx + nodeOffset];
		target->childRid[idx + nodeOffset + 1] = target->childRid[idx + nodeOffset];
	}
	//target->childNodes[idx + nodeOffset + 1] = target->childNodes[idx + nodeOffset];
	target->childRid[idx + nodeOffset + 1] = target->childRid[idx + nodeOffset];

	target->keys[pos] = child->minMax();
	target->childRid[pos+ 1] = child->rid; //to right of new min max
}

int IndexNode::minMax()
{
	int key = keys[0];
	for (int idx = 0; idx < keyCount; idx++)
	{
		keys[idx] = keys[idx + 1];
	}
	keyCount--;
	return key;
}

void IndexNode::TreeDump(int depth)
{
	cout << std::setw(depth * 2) << "";
	cout << "(";
	if (keyCount != 0)
		cout << "*,";
	for (int idx = 0; idx < keyCount; idx++)
	{
		std::cout << keys[idx] << ",";
	}
	std::cout << "*) " << rid << endl;

	int range = keyCount == 0 ? -1 : keyCount;
	for (int idx = 0; idx <= range; idx++)
	{
		Node * temp = AssembleNode(cache->GetNodeLRU(childRid[idx]));
		temp->TreeDump(depth + 1);
		delete temp;
	}
}

int IndexNode::swapKey(int k)
{
	return swapKey(k, false);
}

int IndexNode::swapKey(int k, bool rightPivot)
{
	int swapOut = 0;
	int idx = 0;
	while (k >= keys[idx] && idx < keyCount)
	{
		++idx;
	}
	swapOut = rightPivot ? keys[--idx] : keys[idx];
	keys[idx] = k;
	return swapOut ;
}

int IndexNode::removeChildNode(Node* zNode)
{
	--keyCount;
	//move to back
	int zIdx = (childRid[anchorKeyIdx + 1] == zNode->rid) ? anchorKeyIdx + 1 : anchorKeyIdx; //left or right node?
	int pop = keys[anchorKeyIdx];
	while (++anchorKeyIdx <= keyCount)
		keys[anchorKeyIdx-1] = keys[anchorKeyIdx];
	while (++zIdx <= keyCount + 1)
	{
		//childNodes[zIdx - 1] = childNodes[zIdx];
		childRid[zIdx - 1] = childRid[zIdx];
	}
	return pop;
}

char* IndexNode::genBytes(int rid)
{
	FactoryNode xfer = FactoryNode();
	xfer.type = 'I';
	xfer.RID = rid;
	xfer.kCount = keyCount;
	xfer.hgt = height;
	for (int i = 0; i < keyCount; ++i)
	{
		xfer.kVal[i] = keys[i];
		xfer.children[i] = childRid[i];
	}
	xfer.children[keyCount] = childRid[keyCount];
	char* buff = reinterpret_cast<char*>(&xfer);
	return buff;
}

FactoryNode IndexNode::genBytes()
{
	FactoryNode xfer = FactoryNode();
	xfer.type = 'I';
	xfer.RID = rid;
	xfer.kCount = keyCount;
	xfer.hgt = height;
	for (int i = 0; i < keyCount; ++i)
	{
		xfer.kVal[i] = keys[i];
		xfer.children[i] = childRid[i];
	}
	
	xfer.children[keyCount] = childRid[keyCount];
	//memcpy(buff, reinterpret_cast<const char*>(&xfer), sizeof(FactoryNode));
	return xfer;
}

int IndexNode::getFirst()
{
	Node* temp = AssembleNode(cache->GetNodeLRU(childRid[0]));
	int ret = temp->getFirst(); 
	delete temp; 
	return ret;
}

Node * IndexNode::AssembleNode(FactoryNode n)
{
    if(n.type == 'L')
        return  new DataNode(n, cache);
    else
        return new IndexNode(n, cache);
}