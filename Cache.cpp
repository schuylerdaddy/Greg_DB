//
//  Cache.cpp
//  RAEngine
//
//  Created by GregMac on 7/6/14.
//  Copyright (c) 2014 Mac. All rights reserved.
//

#include "Cache.h"

Cache::Cache()
{
    usedBlocks = 0;
    cahceBit = false;
    toggleBit = false;
    toggleSecondaryMethod = false;
    fileHandle = FileIO::Connect("temp",10000);
    ToggleCaching(false);
    LRU = nullptr;
    Special = nullptr;
    memoryMap = nullptr;
}

Cache::~Cache()
{
    /*if(LRU != nullptr)
        delete LRU;
    if(Special != nullptr)
        delete Special;
    if(buffer != nullptr)
        delete buffer;
    if(memoryMap!= nullptr)
        delete memoryMap;*/
}

Cache::Cache(string tableName)
{
    string s = FileIO::LookUpCatalog(tableName.c_str(),BlockMax);
    attributes = FileIO::DefineData(s);
    
    usedBlocks = 0;
    cahceBit = false;
    toggleBit = false;
    toggleSecondaryMethod = false;
    fileHandle = FileIO::Connect(tableName.c_str(),BlockMax);
    newFile =FileIO::IsNewDB(fileHandle);
    if(!newFile)
        Set(BlockMax);
    ToggleCaching(false);
    //FileIO::cacheBit = Cache::cahceBit;
}

int Cache::GetAvailableRID()
{
	char * c = FileIO::readActiveBits(fileHandle,BlockMax);
	int i = 1;
	while (i < (BlockMax - 1) && c[i] != '0') //allow last block to keep from crash
		++i;
	c[i] = '1';
	++usedBlocks;
	FileIO::writeActiveBits(fileHandle,c, BlockMax);
	delete c;
	return i;
}

void Cache::DeactivateRID(int rid)
{
	char * c = FileIO::readActiveBits(fileHandle,BlockMax);
	c[rid] = '0';
	--usedBlocks;
	FileIO::writeActiveBits(fileHandle,c, BlockMax);
	delete c;
}

int Cache::GetAvailableRIDLRU(FactoryNode fn)
{
	if (!cahceBit)
		return GetAvailableRID();
    
	int val = -1;
	int num = (BlockMax % 8 == 0) ? BlockMax / 8 : BlockMax / 8 + 1;
	char* bits = buffer +((BlockMax/4)*512);
	int bitCtr = 0;
	char byte = ' ';
	for (int i = 0; i<num && bitCtr<BlockMax && val ==-1; ++i)
	{
		byte = bits[i];
		for (int j = 0; j<8 && bitCtr<BlockMax && val ==-1; ++j)
		{
			if (((1 << j) & byte) == 0)
			{
				bits[i] = byte | (1 << j);
				val = bitCtr;
				fn.RID = val;
				FileIO::write(fileHandle,reinterpret_cast<char*>(&fn), val);
				++usedBlocks;
				return val;
			}
			bitCtr++;
		}
	}
	if (val<1 || val >=BlockMax)
        return val;
	return val;
}

void Cache::DeactivateRIDLRU(int rid)
{
	if (!cahceBit)
		return DeactivateRID(rid);
	int charNum = rid / 8;
	int offset = rid % 8;
	char byte = (buffer + ((BlockMax / 4)*512))[charNum];
	byte &= ~(1 << offset);
	(buffer + (BlockMax / 4*512))[charNum] = byte;
    
	for (int i = 0; i < BlockMax / 4;++i)
        if (LRU[i] == rid)
            LRU[i] = -1;
	--usedBlocks;
}


void Cache::ResetBlocks()
{
	if (cahceBit)
	{
		buffer[(BlockMax / 4) * 512] = char(1u);
		for (int i = 1; i <= BlockMax / 8 +1; ++i)
			buffer[(BlockMax/4)*512 + i] = ('\0' &0);
	}
	char * c = new char[BlockMax];
	c[0] = '1';
	c[1] = '1';
	for (int i = 2; i < BlockMax; ++i)
	{
		c[i] = '0';
	}
	usedBlocks = 1;
	FileIO::writeActiveBits(fileHandle,c, BlockMax);
	delete c;
}

void Cache::showBlocks()
{
	if (!cahceBit){
        for (int i = 0; i < BlockMax; ++i)
            cout << (i % 10);
        cout << endl << 'X';  //first block is system catalog
        char * bits = FileIO::readActiveBits(fileHandle,BlockMax);
        for (int i = 1; i < BlockMax; ++i)
            cout << (bits[i] != '0' ? '1' : '0');
        cout << endl;
        delete bits;
        return;
	}
    
	char byte = ' ';
	int num = (BlockMax % 8 == 0) ? BlockMax / 8 : BlockMax / 8 + 1;
	int bitCtr = 0;
	for (int i = 0; i<num && bitCtr<BlockMax; ++i)
	{
		byte = buffer[(BlockMax / 4) * 512 + (i)];
		for (int j = 0; j<8 && bitCtr<BlockMax; ++j)
		{
			if (bitCtr == 0)
				cout << 'X';
			else
				cout << ((1 << j & byte) ? '1' : '0');
			bitCtr++;
		}
	}
	cout << endl;
}

void Cache::Set(int num)
{
	BlockMax = num;
	if (!cahceBit)
		return;
	int portion = num / 4;
	LRU = new int[portion];
	Special = new int[portion];
	buffer = new char[(portion+1)*512];
	char b[512];
	memcpy(b,FileIO::readActiveBits(fileHandle,BlockMax), BlockMax);
    
	unsigned int bits = 1;//first block info
	//compress bits
	if (newFile)
	{
		int i = 0;
		for (; i < BlockMax / 8 + 2; ++i)
		{
			buffer[((BlockMax / 4) * 512) + i] = *reinterpret_cast<char*>(&bits);
			bits = 0;
		}
        
	}
	else
	{
		for (int i = 0; i < BlockMax / 8 + 1; ++i)
			buffer[(BlockMax / 4) * 512 + i] = *reinterpret_cast<char*>(b+i);
        
	}
	memoryMap = new int[portion];
	for (int i = 0; i < portion; ++i)
	{
		memoryMap[i] = i * 512;
		LRU[i] = -1;
		Special[i] = -1;
	}
}

FactoryNode Cache::GetNodeLRU(int rid)
{
	if (toggleSecondaryMethod)
		return GetNodeSpecial(rid);
	if (!cahceBit)
		return FileIO::read(fileHandle,rid);
	++cacheRequests;
	int idx = -1;
	for (int i = 0;idx == -1 && i < (BlockMax / 4);++i)
        if (LRU[i] == rid)
            idx = i;
	if (idx != -1)
	{
		int sel = LRU[idx];
		int mSel = memoryMap[idx];
		char data[512];
		memcpy(data, buffer + (memoryMap[idx]), 512);
		if (idx != 0) //move over earlier records
		{
			for (int i = idx; i > 0; --i)
			{
				LRU[i] = LRU[i - 1];
				memoryMap[i] = memoryMap[i - 1];
			}
			LRU[0] = sel;   //promote found record
			memoryMap[0] = mSel;
			memcpy(buffer + (mSel), data, 512);
		}
        return FileIO::AssembleNode(data);
	}
	else if (toggleBit)
	{
		++pageFaults;
		return FileIO::read(fileHandle,rid);
	}
	else
	{
		++pageFaults;
		char * data;
		int idx2 = ((BlockMax / 4)) - 1;
		while (LRU[idx2] == -1 && idx2 != 0)
			--idx2;
		int loc = memoryMap[BlockMax / 4 - 1];
        
		if (LRU[(BlockMax / 4) - 1]  != -1)//write if data exists
		{
			data = buffer + (loc);  //leaving cache
			FileIO::write(fileHandle,buffer + loc, LRU[(BlockMax / 4) - 1]);
		}
		data =buffer + (loc);  //leaving cache
		if (idx2 != (BlockMax/4)-1)
		{
			LRU[idx2+1] = LRU[idx2];
			loc = memoryMap[idx2 + 1];
			memoryMap[idx2 + 1] = memoryMap[idx2];
		}
		else
		{
			loc = memoryMap[idx2];
		}
		for (int i = idx2; i > 0; --i)  //move all  over one
		{
			LRU[i] = LRU[i - 1];
			memoryMap[i] = memoryMap[i - 1];
		}
        
		LRU[0] = rid;    //new entry
		FactoryNode n = FileIO::read(fileHandle,rid);
		memcpy(&buffer[loc], &n, sizeof(512));
		memoryMap[0] = loc;
		return n;
	}
	return FactoryNode();
}

void Cache::WriteToCacheLRU(int rid, FactoryNode f_str)
{
	WriteToCacheLRU(rid, f_str, true);
}

void Cache::WriteToCacheLRU(int rid, FactoryNode f_str, int id)
{
	WriteToCacheLRU(rid, f_str, true);
	if (!cahceBit )
		FileIO::writeCatalog(fileHandle,rid, id, usedBlocks, BlockMax);
}

void Cache::WriteToCacheLRU(int rid, FactoryNode fn, bool delFlag)
{
    char bytes[BLOCK_SIZE];
    memcpy(bytes,reinterpret_cast<char*>(&fn),BLOCK_SIZE);
	if (!cahceBit)
	{
		FileIO::write(fileHandle,bytes, rid);
		return;
	}
	bool found = false;
	for (int i = 0; i < (BlockMax / 4) && !found; i++)   //replace cache data
	{
		if (LRU[i] == rid)
		{
			memcpy(buffer + (memoryMap[i]),bytes, 512);
			found = true;
		}
		else if (LRU[i] == -1 && !toggleBit)
		{
			LRU[i] = rid;
			memcpy(buffer + (memoryMap[i]), bytes, 512);
			found = true;
		}
	}
	if (!found)  //write to file
	{
		FileIO::write(fileHandle,bytes, rid);
	}
}
void Cache::UploadCache(int root, int link)
{
	if (!cahceBit)
		return;
	char data[512];
	for (int i = 0; i < BlockMax / 4; ++i)
	{
		memcpy(data,buffer + memoryMap[i],512);
		FileIO::write(fileHandle,data, LRU[i]);
	}
	memcpy(data, &buffer[BlockMax / 4 *512],512);
	FileIO::writeCatalog(fileHandle,root,link,usedBlocks,BlockMax);
	FileIO::writeActiveBits(fileHandle,data,BlockMax);
}

void Cache::ResetStats()
{
	pageFaults = 0;
	cacheRequests = 0;
}

FactoryNode Cache::GetNodeSpecial(int rid)
{
	//srand(time(NULL));
    
	if (!cahceBit)
		return FileIO::read(fileHandle,rid);
	++cacheRequests;
	int idx = -1;
	int i = 0;
	for (; idx == -1 && i < (BlockMax / 4); ++i)
	{
		if (LRU[i] == rid)
			idx = i;
	}
	if (i == BlockMax / 4)
		bufferFull = true;
	if (idx != -1)
	{
		char data[512];
		memcpy(data, buffer + (memoryMap[idx]), 512);
		FactoryNode n = FileIO::AssembleNode(data);
		return n;
	}
	else if (toggleBit)
	{
		++pageFaults;
		return FileIO::read(fileHandle,rid);
	}
	else
	{
		++pageFaults;
		//srand(time(NULL));
		int idx2 = -1;
		if (bufferFull)
			idx2 = rand() % (BlockMax / 4);
		else
		{
			while (LRU[idx2] != -1 && !bufferFull)
				idx2 = rand() % (BlockMax / 4);
		}
		int loc = memoryMap[idx2];
		FileIO::write(fileHandle,buffer + loc, LRU[idx2]);
        
		LRU[idx2] = rid;    //new entry
		FactoryNode  n = FileIO::read(fileHandle,rid);
        
		memcpy(&buffer[loc], &n, sizeof(FactoryNode));
		memoryMap[idx2] = loc;
		return n;
	}
	return FactoryNode();
}

SystemCat Cache::readTreeSettings()
{
    return FileIO::readCatalog(fileHandle);
}

void Cache::ToggleCaching(bool on)
{
    cahceBit = on;
    FileIO::ToggleCaching(fileHandle, on);
}

