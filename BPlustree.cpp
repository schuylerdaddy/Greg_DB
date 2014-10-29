
#include "BPlustree.h"


BPlustree::BPlustree()
{
    /*
    Node * root = new DataNode(cache);
	root->rid = rootId;
    
    try{
        cache = new Cache();
    }
    catch(std::exception e)
    {
        throw e;
    }
	root->genBytes(rootId);
	dataLinkRid = rootId;
    tupleIdx =0;
	delete root;*/
}

BPlustree::BPlustree(const char* fName)
{
    std::string s = fName;
    try{
        cache = new Cache(fName);
    }
    catch(std::exception e)
    {
        throw e;
    }
    
    treeIdx = cache->fileHandle;
    
    //start tree with new rootnode if new
    if(cache->newFile)
    {
        Node * root = new DataNode(cache);
        rootId = cache->GetAvailableRIDLRU(root->genBytes());
        root->rid = rootId;
        cache->WriteToCacheLRU(root->rid, root->genBytes());
        dataLinkRid = rootId;
        linkedProgressionId = rootId;
        tupleIdx = 0;
    }
    else //load tree stats
    {
        loadTree();
    }
}

BPlustree::~BPlustree()
{
    close();
}

/*
void BPlustree::insert(const int key, DataTuple tup)
{
	Node * root = Factory::read(rootId);
	if (root->height + Registry::usedBlocks + 1 >= Registry::BlockMax - 1)
	{
		cout << "Database is full. Did not insert key " << key << endl;
		return;
	}

	int splitNode = root->insert(key, tup);

	if (splitNode != -1)
	{
		IndexNode* iNode = new IndexNode();

		iNode->keyCount++;
		DataNode* temp = static_cast<DataNode*>(Factory::read(splitNode));
		iNode->keys[0] = temp->minMax();
		temp->genBytes(splitNode);
		delete temp;

		iNode->height = root->height + 1;
		//iNode->childNodes[0] = root;
		iNode->childRid[0] = root->rid;
		iNode->childRid[iNode->keyCount] = splitNode;

		iNode->rid = Registry::GetAvailableRID();
		//iNode->rid = Registry::AddToCatalog(iNode);
		rootId = iNode->rid;
		iNode->genBytes(iNode->rid);
		delete iNode;
	}
	root->genBytes(root->rid);
	delete root;
	updateSystemCat();
}*/

//LRU
void BPlustree::insert(const int key, DataTuple tup)
{
	Node * root = AssembleNode(cache->GetNodeLRU(rootId));
	if (root->height + cache->usedBlocks + 1 >= cache->BlockMax-1)
	{
		cout << "Database is full. Did not insert key "<< key<<endl;
		return;
	}

	int splitNode = root->insert(key, tup);

	if(splitNode != -1)
	{
			IndexNode* iNode = new IndexNode(cache);

			iNode->keyCount++;
			DataNode* temp =static_cast<DataNode*>(AssembleNode((cache->GetNodeLRU(splitNode))));
			iNode->keys[0] = temp->minMax();
            cache->WriteToCacheLRU(temp->rid,temp->genBytes());

			iNode->height = root->height + 1;
			iNode->childRid[0] = root->rid;
			iNode->childRid[iNode->keyCount] = splitNode;
		
			iNode->rid = cache->GetAvailableRIDLRU(iNode->genBytes());
			rootId = iNode->rid;
			cache->WriteToCacheLRU(iNode->rid,iNode->genBytes());
	}
	cache->WriteToCacheLRU(root->rid,root->genBytes(),dataLinkRid);
	//updateSystemCat();
}

void BPlustree::PrintLinkedList()
{
	//if (dataLinkHead == nullptr)
	if (dataLinkRid < 1)
		cout << "No data in tree";
	else
	{
		cout << "Leaf nodes: "<<endl;
		Node* temp = AssembleNode(cache->GetNodeLRU(dataLinkRid));
		temp->PrintList();
		delete temp;
		//dataLinkHead->PrintList();
	}
	cout << "\n" << endl;
}

void BPlustree::TreeDump()
{

	cout << "\n" << endl;
	Node * root = AssembleNode(cache->GetNodeLRU(rootId));
	root->TreeDump(0);
	delete root;
	cout << "\n" << endl;
}

void BPlustree::delKey(const int k)
{
	/*
	Node * root = Factory::read(rootId);
	root->delKey(k, nullptr, nullptr, nullptr, nullptr, 0);

	if (root->keyCount == 0 && root->height != 0)
	{
		Node * temp = root;
		root = Factory::read(root->childRid[0]);
		rootId = root->rid;
		temp->keyCount = 0;
		temp->nextRid = -1;
		temp->genBytes(temp->rid);
		Registry::DeactivateRID(temp->rid);

		delete temp;
	}
	root->genBytes(root->rid);

	delete root;
	updateSystemCat();*/
	Node * root = AssembleNode(cache->GetNodeLRU(rootId));
	root->delKey(k, nullptr, nullptr, nullptr, nullptr, 0);

	if (root->keyCount == 0 && root->height != 0)
	{
		Node * temp = root;
		root = AssembleNode(cache->GetNodeLRU(root->childRid[0]));
		rootId = root->rid;
		temp->keyCount = -1;
		temp->nextRid = -1;
		cache->DeactivateRIDLRU(temp->rid);
		cache->WriteToCacheLRU(temp->rid,temp->genBytes());
	}
	cache->WriteToCacheLRU(root->rid,root->genBytes(),dataLinkRid);
}

void BPlustree::updateSystemCat()
{
	if (!cache->cahceBit)
        cache->saveSettings(rootId,dataLinkRid);
	else
		cache->UploadCache(rootId,dataLinkRid);
}

void BPlustree::loadTree()
{
	SystemCat cat = cache->readTreeSettings();

	//Registry::SetCatalog(cat.blockCount);

	rootId = cat.rootLoc;
	dataLinkRid = cat.HeadLoc;
    linkedProgressionId = dataLinkRid;

	cache->usedBlocks = cat.usedBlocks;
	cache->Set(cat.blockCount);

	/*char* activeBits = Factory::readActiveBits(cat.blockCount);
	for (int i = 0; i < cat.blockCount;++i)
		Registry::catalog[i+1].active = (activeBits[i] == '1');

	cout << "\n" << endl;

	delete activeBits;*/
}

void BPlustree::clear()
{
	Node * root = new DataNode(cache);
	cache->ResetBlocks();
	root->rid = 1;
	rootId = 1;
	dataLinkRid = 1;
	root->nextRid = -1;
	updateSystemCat();
	root->genBytes(rootId);

	rootId = cache->GetAvailableRIDLRU(root->genBytes()); //update for cache
	delete root;

}

bool BPlustree::search(int key,DataTuple& dt)
{
	Node* root = AssembleNode(cache->GetNodeLRU(rootId));
	bool ret = root->search(key,dt);
	cache->WriteToCacheLRU(root->rid,root->genBytes());
	return ret;
}

void BPlustree::close()
{
	updateSystemCat();
	cache->UploadCache(rootId,dataLinkRid);
    delete cache;
}
    
bool BPlustree::EndofLinkList()
    {
        return linkedProgressionId == -1;
    }
    
bool BPlustree::GetNextLinkedTuple(DataTuple& tup)
    {
        if(linkedProgressionId == -1)
        {
            return NULL;
            return false;
        }
        
        Node* n = AssembleNode(cache->GetNodeLRU(linkedProgressionId));
        tup = NULL;
        if(tupleIdx == n->keyCount)
        {
            tupleIdx =0;
            linkedProgressionId = n->nextRid;
        }
        
        if(linkedProgressionId != -1)
        {
            tup = n->getTuple(tupleIdx);
            ++tupleIdx;
            delete n;
            return true;
        }
        delete n;
        return false;
    }

vector<Attribute> BPlustree::GetSchemaAttributes()
{
    return cache->attributes;
}

void BPlustree::ShowBlocks()
{
    cache->showBlocks();
}

Node * BPlustree::AssembleNode(FactoryNode n)
{
    if(n.type == 'L')
        return  new DataNode(n, cache);
    else
        return new IndexNode(n, cache);
}