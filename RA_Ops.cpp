//
//  RA_Ops.cpp
//  RAEngine
//
//  Created by GregMac on 7/31/14.
//  Copyright (c) 2014 Mac. All rights reserved.
//

#include "RA_Ops.h"
int TSIZE = sizeof(DataTuple);

IntersectOp::IntersectOp(RAOperation* op, RAOperation* tName)
{
    tree = tName;
    buffer = op;
    bufferOffset = 0;
    fetchOffset = 0;
    schema = op->schema;
}

void IntersectOp::Operate()
{
    buffer->Operate();
    DataTuple buff;
    DataTuple dt;
    bool hasNext = getNext(buff);
    
    while(hasNext)
    {
        //verify primary key
        if(tree->GetAttributes()[0].pos == 1)
        {
            if(tree->search(buff.dkey, dt))
            {
                memcpy(outBuffer + (bufferOffset*TSIZE), &buff, TSIZE);
                ++bufferOffset;
            }
        }
        
        hasNext = getNext(buff);
    }
}

bool IntersectOp::getNext(DataTuple& dt)
{
    if(fetchOffset < buffer->bufferOffset)
    {
        char data [TSIZE];
        memcpy(data, buffer->outBuffer+(fetchOffset*TSIZE),TSIZE);
        dt = *reinterpret_cast<DataTuple*>(data);
        ++fetchOffset;
        return true;
    }
    dt = NULL;
    return false;
}

DifferenceOp::DifferenceOp(RAOperation* op, RAOperation* tName)
{
    tree =  tName;
    buffer = op;
    bufferOffset = 0;
    fetchOffset = 0;
    schema = op->schema;
}

void DifferenceOp::Operate()
{
    buffer->Operate();
    DataTuple buff;
    bool hasNext = getNext(buff);
    
    DataTuple dt;
    while(hasNext)
    {
        //verify primary key
        if(tree->GetAttributes()[0].pos == 1)
        {
            if(!tree->search(buff.dkey, dt))
            {
                memcpy(outBuffer + (bufferOffset*TSIZE), &buff, TSIZE);
                ++bufferOffset;
            }
        }
        
        hasNext = getNext(buff);
    }
}

bool DifferenceOp::getNext(DataTuple& dt)
{
    if(fetchOffset < buffer->bufferOffset)
    {
        char data [TSIZE];
        memcpy(data, buffer->outBuffer+(fetchOffset*TSIZE),TSIZE);
        dt = *reinterpret_cast<DataTuple*>(data);
        ++fetchOffset;
        return true;
    }
    return false;
}

UnionOp::UnionOp(RAOperation* op, RAOperation* tName)
{
    tree = tName;
    buffer = op;
    bufferOffset = 0;
    fetchOffset = 0;
    fetchOffset2 =0;
    schema = op->schema;
}

void UnionOp::Operate()
{
    buffer->Operate();
    tree->Operate();
    DataTuple buff;
    bool hasNext = getNext(buff);
    
    while(hasNext)
    {
        //verify primary key
        if(tree->GetAttributes()[0].pos == 1)
        {
            memcpy(outBuffer + (bufferOffset*TSIZE), &buff, TSIZE);
            ++bufferOffset;
        }
        
        hasNext = getNext(buff);
    }
}

bool UnionOp::getNext(DataTuple& dt)
{
    DataTuple tup;
    int sz = buffer->bufferOffset;
    while(fetchOffset < sz)
    {
        char data [TSIZE];
        memcpy(data, buffer->outBuffer+(fetchOffset*TSIZE),TSIZE);
        dt = *reinterpret_cast<DataTuple*>(data);
        ++fetchOffset;
        if(!tree->search(dt.dkey,tup))
           return true;
    }

    sz = tree->bufferOffset;
    
    if(fetchOffset2 < sz)
    {
        char data [TSIZE];
        memcpy(data, tree->outBuffer+(fetchOffset2*TSIZE),TSIZE);
        dt = *reinterpret_cast<DataTuple*>(data);
        ++fetchOffset2;
        return true;
    }
    return false;
}

BPlustreeOp::BPlustreeOp(string tName)
{
    tree = new BPlustree(tName.c_str());
    bufferOffset = 0;
    fetchOffset = 0;
    schema = tree->GetSchemaAttributes();
}

void BPlustreeOp::Operate()
{
    DataTuple buff;
    bool hasNext = getNext(buff);

    while(hasNext)
    {
        memcpy(outBuffer + (bufferOffset*TSIZE), &buff, TSIZE);
        ++bufferOffset;
        hasNext = getNext(buff);
    }
}

bool BPlustreeOp::getNext(DataTuple& dt)
{
    if(!tree->EndofLinkList())
    {
        return tree->GetNextLinkedTuple(dt);
    }
    else
        return false;
}

PrintOp::PrintOp(RAOperation* op)
{
    fetchOffset = 0;
    bufferOffset = 0;
    buffer = op;
    schema = op->schema;
}

void PrintOp::Operate()
{
    buffer->Operate();
    DataTuple buff;
    DisplaySchema();
    bool hasNext = getNext(buff);
    
    while(hasNext)
    {
        DisplayRow(buff);
        memcpy(outBuffer + (bufferOffset*TSIZE), &buff, TSIZE);
        ++bufferOffset;
        hasNext = getNext(buff);
    }
}

void PrintOp::DisplaySchema()
{
    int i =0;
    cout<<schema[i].field;
    for(i=1; i < schema.size();++i)
        cout<<"\t"<<schema[i].field;
    cout<<endl;
}

void PrintOp::DisplayRow(DataTuple dt)
{
    int itr = 0;
    //first is int
    for (int i = 0; i < schema.size(); ++i)
    {
        if(i>0)
            cout<<"\t";
        string type = schema[i].type;
        if (type == "int")
        {
            char temp[4];
            memcpy(temp, dt.data + itr, 4);
            cout << *reinterpret_cast<int*>(&temp);
            itr += 4;
        }
        else if (type == "V20")
        {
            char temp[21];
            temp[20] = '\0';
            memcpy(temp, dt.data + itr, 20);
            string t = temp;
            cout << t;
            itr += 21;
        }
    }
    cout<<endl;
}

BPlustreeSelectionOp::BPlustreeSelectionOp(string tName, string exp)
{
    tree = new BPlustree(tName.c_str());
    bufferOffset = 0;
    fetchOffset = 0;
    schema = tree->GetSchemaAttributes();
    value=exp;
    schema = tree->GetSchemaAttributes();
}

bool BPlustreeSelectionOp::getNext(DataTuple& dt)
{
    if(!tree->EndofLinkList())
    {
        return tree->GetNextLinkedTuple(dt);
    }
    else
        return NULL;
}

void BPlustreeSelectionOp::Operate()
{
    string name;
    char opr= ' ';
    string val;
    
    size_t idx = value.find_first_of("<=>");
    opr = value[idx];
    name= value.substr(idx);
    val = value.substr(idx,value.length()-idx);
    
    int keyval = stoi(value);
    
    DataTuple buff;
    bool hasNext = getNext(buff);
    switch (opr) {
        case '<':
            while(hasNext && buff.dkey < keyval)
            {
                memcpy(outBuffer + (bufferOffset*TSIZE), &buff, TSIZE);
                ++bufferOffset;
                hasNext = getNext(buff);
            }
            break;
        case '=':
            while(hasNext && buff.dkey < keyval)
            {
                hasNext = getNext(buff);
            }
            memcpy(outBuffer + (bufferOffset*TSIZE), buff.data, TSIZE);
            ++bufferOffset;
            break;
        case '>':
            while(buff != NULL && buff.dkey < keyval)
            {
                hasNext = getNext(buff);
            }
            hasNext = getNext(buff);
            memcpy(outBuffer + (bufferOffset*TSIZE), buff.data, TSIZE);
            ++bufferOffset;
            break;
            
        default:
            break;
    }
}

bool PrintOp::getNext(DataTuple& dt)
{
    if(fetchOffset < buffer->bufferOffset)
    {
        char data [TSIZE];
        memcpy(data, buffer->outBuffer+(fetchOffset*TSIZE),TSIZE);
        dt=  *reinterpret_cast<DataTuple*>(data);
        ++fetchOffset;
        return true;
    }
    return false;
}

class AttCompare
{
    bool operator()(Attribute x,string y) const
    {
        return x.field == y;
    }
};

SelectedAttribute GetSelectedAttribute(string value, vector<Attribute> atts)
{
    SelectedAttribute a = SelectedAttribute();
    Attribute trib;
    int off = 0;
    bool found=false;
    for(auto x : atts)
    {
        if(!found)
        {
        if(x.field == value)
        {
            trib = x;
            found = true;
        }
        else
        {
            if(strcmp(x.type,"int"))
            {
                off += 4;
            }
            else if (strcmp(x.type,"V20"))
            {
                off +=21;
            }
        }
        }
    }
    
    if(strcmp(trib.type, "int") )
    {
        a.type = integer;
        a.length = 4;
    }
    else if(strcmp(trib.type, "V20") )
    {
        a.type = v20;
        a.length = 21;
    }
    
    a.offset = off;
    return a;
}

bool BPlustreeOp::search(int i,DataTuple& dt)
{
    return tree->search(i,dt);
}

bool BPlustreeSelectionOp::search(int i, DataTuple& dt)
{
    return tree->search(i, dt);
}

vector<Attribute> BPlustreeOp::GetAttributes()
{
    return schema;
}

vector<Attribute> BPlustreeSelectionOp::GetAttributes()
{
    return schema;
}

bool IntersectOp::search(int k,DataTuple& dt)
{
    for(int idx=0;idx<bufferOffset;++idx)
    {
        dt = *reinterpret_cast<DataTuple*>(buffer +(TSIZE*idx));
        if(dt.dkey == k)
            return true;
    }
    return false;
}

vector<Attribute> IntersectOp::GetAttributes()
{
    return schema;
}

bool DifferenceOp::search(int k,DataTuple& dt)
{
    for(int idx=0;idx<bufferOffset;++idx)
    {
        dt = *reinterpret_cast<DataTuple*>(buffer +(TSIZE*idx));
        if(dt.dkey == k)
            return true;
    }
    return false;
}

vector<Attribute> DifferenceOp::GetAttributes()
{
    return schema;
}

bool UnionOp::search(int k, DataTuple& dt)
{
    for(int idx=0;idx<bufferOffset;++idx)
    {
       dt = *reinterpret_cast<DataTuple*>(buffer +(TSIZE*idx));
       if(dt.dkey == k)
           return true;
    }
    return false;
}

vector<Attribute> UnionOp::GetAttributes()
{
    return schema ;
}

bool SelectionOp::search(int k,DataTuple& dt)
{
    for(int idx=0;idx<bufferOffset;++idx)
    {
        dt = *reinterpret_cast<DataTuple*>(buffer +(TSIZE*idx));
        if(dt.dkey == k)
            return true;
    }
    return false;
}

vector<Attribute> SelectionOp::GetAttributes()
{
    return schema;
}

bool PrintOp::search(int k,DataTuple& dt)
{
    for(int idx=0;idx<bufferOffset;++idx)
    {
        DataTuple t = *reinterpret_cast<DataTuple*>(buffer +(TSIZE*idx));
        if(t.dkey == k)
            return true;
    }
    return false;
}

vector<Attribute> PrintOp::GetAttributes()
{
    return schema;
}

ProjectionOP::ProjectionOP(RAOperation* op, string sch)
{
    buffer = op;
    bufferOffset = 0;
    atts = sch;
    fetchOffset = 0;
    oldSchema = op->schema;
    schema = SelectAttributes(sch);
    tsize = TSIZE;
}

vector<Attribute> ProjectionOP::SelectAttributes(string s)
{
    vector<Attribute> filtered = vector<Attribute>();
    sregex_iterator toks = sregex_iterator(s.begin(),s.end(),regex("[^,]+"));
    int pos =1;
    int off =0;
    
    for (; toks != sregex_iterator(); ++toks) {
        const char* m = (*toks).str().c_str();
        Attribute sel = *find_if(oldSchema.begin(), oldSchema.end(),[m](Attribute x)->bool{
            return strcmp(x.field, m) >0;});
        sel.pos = pos++;
        sel.offset = off;
        off += AttUtil::AttributeSizes[sel.type];
        filtered.push_back(sel);
    }
    
    return filtered;
}

void ProjectionOP::Operate()
{
    
}

bool ProjectionOP::search(int,DataTuple&)
{
    return true;
}

vector<Attribute> ProjectionOP::GetAttributes()
{
    return schema;
}

bool ProjectionOP::getNext(DataTuple& dt)
{
    return true;
}