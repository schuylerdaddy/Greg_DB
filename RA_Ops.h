//
//  RA_Ops.h
//  RAEngine
//
//  Created by GregMac on 7/31/14.
//  Copyright (c) 2014 Mac. All rights reserved.
//

#ifndef __RAEngine__RA_Ops__
#define __RAEngine__RA_Ops__

#include "BPlustree.h"
#include <list>
#include <algorithm>
#include <iterator>
#include <regex>
#include <atomic>

struct SelectedAttribute
{
    int type;
    int offset;
    int length;
};

SelectedAttribute GetSelectedAttribute(string value, vector<Attribute> atts);

class RAOperation
{
public:
    //condition_variable q_cond;
    //mutex q_sync;
    char outBuffer [sizeof(DataTuple) * 10000];
    vector<Attribute> schema;
    int bufferOffset;
    int fetchOffset;
    size_t tsize;
    virtual bool getNext(DataTuple&) =0;
    virtual void Operate()=0;
    virtual bool search(int,DataTuple&)=0;
    virtual vector<Attribute> GetAttributes()=0;
    virtual ~RAOperation(){}
};

class IntersectOp:public RAOperation
{
private:
    RAOperation* tree;
    RAOperation* buffer;
public:
    IntersectOp(RAOperation*,RAOperation*);
    ~IntersectOp(){delete tree;delete buffer;};
    bool getNext(DataTuple&);
    void Operate();
    bool search(int,DataTuple&);
    vector<Attribute> GetAttributes();
};

class DifferenceOp:public RAOperation
{
private:
    RAOperation* tree;
    RAOperation* buffer;
public:
    DifferenceOp(RAOperation*,RAOperation*);
    ~DifferenceOp(){delete tree;delete buffer;}
    bool getNext(DataTuple&);
    void Operate();
    bool search(int,DataTuple&);
    vector<Attribute> GetAttributes();
};

class UnionOp:public RAOperation
{
private:
    RAOperation* tree;
    RAOperation* buffer;
    int fetchOffset2;
public:
    UnionOp(RAOperation*,RAOperation*);
    ~UnionOp(){delete tree;delete buffer;}
    bool getNext(DataTuple&);
    void Operate();
    bool search(int,DataTuple&);
    vector<Attribute> GetAttributes();
};

class SelectionOp:public RAOperation
{
private:
    int evalType;
    string value;
    RAOperation* buffer;
public:
    SelectionOp(RAOperation*,string);
    ~SelectionOp(){delete buffer;}
    bool getNext(DataTuple&);
    void Operate();
    bool search(int,DataTuple&);
    vector<Attribute> GetAttributes();
};

class BPlustreeSelectionOp:public RAOperation
{
private:
    int evalType;
    string value;
    BPlustree* tree;
public:
    BPlustreeSelectionOp(string,string);
    ~BPlustreeSelectionOp(){delete tree;}
    bool search(int,DataTuple&);
    bool getNext(DataTuple&);
    void Operate();
    vector<Attribute> GetAttributes();
};

class PrintOp:public RAOperation
{
private:
    RAOperation* buffer;
public:
    PrintOp(RAOperation*);
    ~PrintOp(){delete buffer;}
    bool getNext(DataTuple&);
    void Operate();
    bool search(int,DataTuple&);
    vector<Attribute> GetAttributes();
    void DisplaySchema();
    void DisplayRow(DataTuple);
};

class BPlustreeOp:public RAOperation
{
private:
    BPlustree* tree;
public:
    BPlustreeOp(string);
    ~BPlustreeOp(){delete tree;}
    bool getNext(DataTuple&);
    void Operate();
    bool search(int,DataTuple&);
    vector<Attribute> GetAttributes();
};

class ProjectionOP:public RAOperation
{
public:
    RAOperation * buffer;
    string atts;
    vector<Attribute> oldSchema;
private:
    ProjectionOP(RAOperation*,string);
    ~ProjectionOP(){delete buffer;}
    bool getNext(DataTuple&);
    void Operate();
    bool search(int,DataTuple&);
    vector<Attribute> GetAttributes();
    vector<Attribute> SelectAttributes(string);
};

#endif /* defined(__RAEngine__RA_Ops__) */
