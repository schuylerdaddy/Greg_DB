//
//  Cache.h
//  RAEngine
//
//  Created by GregMac on 7/6/14.
//  Copyright (c) 2014 Mac. All rights reserved.
//

#ifndef __RAEngine__Cache__
#define __RAEngine__Cache__

#include <algorithm>
#include "FileIO.h"

/*struct RID
{
	RID(){ address = nullptr; id = -1; }
	RID(Node* a){ address = a; id = -1; }
	RID(int i){ id = i; address = nullptr; }
	RID(Node* a, int i){ address = a; id = i; }
	Node* address;
	int id;
	bool active;
};*/

class Cache
{
private:

public:
    vector<Attribute> attributes;
    
    Cache();
    ~Cache();
    Cache(string s);
    int fileHandle;
	int usedBlocks;
	int BlockMax;
    bool cahceBit;
	int * LRU;
	int * Special;
	char * buffer;
	int * memoryMap;
	int cacheRequests;
	int pageFaults;
	bool toggleBit;
	bool toggleSecondaryMethod;
	bool bufferFull;
    bool newFile;
    
	void Set(int num);
	int GetAvailableRID();
	void DeactivateRID(int rid);
	int GetAvailableRIDLRU(FactoryNode);
	void DeactivateRIDLRU(int rid);
	FactoryNode GetNodeLRU(int);
	FactoryNode GetNodeSpecial(int);
	void showBlocks();
	void ResetBlocks();
	void WriteToCacheLRU(int, FactoryNode);
	void WriteToCacheLRU(int, FactoryNode,bool);
	void WriteToCacheLRU(int, FactoryNode, int);
	void UploadCache(int,int);
	void ResetStats();
    void SetCacheOn(){cahceBit = true;}
    void SetCacheOff(){cahceBit = false;}
    void saveSettings(int rootId, int dataLinkRid){FileIO::writeCatalog(fileHandle,rootId, dataLinkRid, usedBlocks, BlockMax);}
    SystemCat readTreeSettings();
    void ToggleCaching(bool on);
};

#endif /* defined(__RAEngine__Cache__) */
