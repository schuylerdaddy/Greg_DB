//
//  RAEngine.h
//  RAEngine
//
//  Created by GregMac on 6/1/14.
//  Copyright (c) 2014 Mac. All rights reserved.
//

#ifndef __RAEngine__RAEngine__
#define __RAEngine__RAEngine__
#include "BPlustree.h"
#include "RA_Ops.h"
#include <stdexcept>
#include <list>
#include <queue>
#include <sstream>

enum RATokenTypes{
    Invalid,
    Name,
    TableName,
    FieldName,
    Contextual,
    Operator,
    SetOperator,
    Project,
    Value,
    Paren,
    Tick,
    Quote,
    Comparator,
    Selector,
    None
};

class RAToken{
    string str;
    int type;
    void AssignType();
public:
    RAToken(const char c);
    RAToken(const string s);
    string GetString();
    int GetTokenType();
    bool operator ==(const string&);
};

class RALexer{
private:
    string query;
    RAToken GetNextToken(string);
    RALexer();
    static RALexer* lexer;
public:
    vector<RAToken> tokens;
    static RALexer GetInstance(string);
    void ProcessTokens(string);
    RAToken PopToken();
    bool HasToken();
};



class RAEngine{
private:
    static RALexer lex;
    static list<Attribute> schema;
    static list<DataTuple> nodeList;
    static mutex processLocks[1000];
    static RAToken PopToken();
    static RAToken PeekToken();
public:
    static void RunQueryLeftToRight(string);
    static void RunTokensLeftToRight();
    static void EvaluateTokenPrecedence(RAToken tok, RAToken nextTok);
    static list<Attribute> GetTableSchema(string);
    static list<Attribute> ProjectSchema(queue<string>);
    static list<DataTuple> SetOperation(RAToken,RAToken,RAToken, int);
    static void StartOperations();
    static void ChainOperations(queue<DataTuple>&);
    static RAOperation* GetTable(queue<string>*);
};

bool IsSelectExpression(char);
bool IsProjectExpression(char);
bool IsTable(string);
bool IsExpression(string);
bool IsOperator(string);

#endif /* defined(__RAEngine__RAEngine__) */
