//
//  RAEngine.cpp
//  RAEngine
//
//  Created by GregMac on 6/1/14.
//  Copyright (c) 2014 Mac. All rights reserved.
//

#include "RAEngine.h"

list<Attribute> RAEngine::schema = list<Attribute>();;
list<DataTuple> RAEngine::nodeList = list<DataTuple> ();

RALexer* RALexer::lexer;
RALexer RAEngine::lex = RALexer::GetInstance("");


bool IsOperator(char c)
{
    return c == '(' || c == ')' || c == '[' || c == ']' || c=='\'' || c=='"' ||
           c == '&' || c == '|' || c == '-' || c == '=' || c == '!';
}

void RAToken::AssignType()
{
    if(str =="")
    {
        type = None;
        return;
    }
    if(str.length()>1)
    {
        type = Name;
        return;
    }
    
        switch(str[0])
        {
            case '(': type = Paren & Operator & Contextual&Selector;
                break;
            case ')': type = Paren & Operator& Contextual&Selector;
                break;
            case '|': type = SetOperator & Operator;
                break;
            case '&': type = SetOperator & Operator;
                break;
            case '-': type = SetOperator & Operator;
                break;
            case '[': type = Project & Operator & Contextual;
                break;
            case ']': type = Project & Operator & Contextual;
                break;
            case '\'': type = Tick & Operator & Contextual;
                break;
            case '"': type = Quote & Operator & Contextual;
                break;
            case '=': type = Comparator & Operator;
                break;
            case '!': type = Comparator & Operator;
                break;
            default: type = Invalid;
                break;
                
        }
}
RAToken::RAToken(const char c)
{
    str = char{c};
    AssignType();
}

RAToken::RAToken(const string s)
{
    str = s;
    AssignType();
}

string RAToken::GetString()
{
    return str;
}

int RAToken::GetTokenType()
{
    return type;
}

bool RAToken::operator ==(const string& s)
{
    return str == s;
}

RALexer::RALexer()
{
    tokens = vector<RAToken>();
}

//singleton pattern returns only instance allowed
RALexer RALexer::GetInstance(string s)
{
    if(lexer == nullptr)
        lexer = new RALexer();
    lexer->ProcessTokens(s);
    return *lexer;
}

///simulates popping a token from front
RAToken RALexer::PopToken()
{
    RAToken t = tokens[0];
    tokens.erase(tokens.begin());
    return t;
}

//Does it have a token?
bool RALexer::HasToken()
{
    return tokens.size() != 0;
}

void RALexer::ProcessTokens(string s)
{
    query = s;
    tokens = vector<RAToken> ();
    
    RAToken tok = GetNextToken(s);
    int type = tok.GetTokenType();
    string lexum = tok.GetString();
    
    while(type != None)
    {
        tokens.push_back(tok);
        tok = GetNextToken(query);
        type = tok.GetTokenType();
    }
}

RAToken RALexer::GetNextToken(string s)
{
    if(s == "")
        return RAToken(s);
    if(IsOperator(s[0]))
    {
        query = s.substr(1);
        return RAToken(s[0]);
    }
    int ctr = 0;
    string buffer = "";
    
    while(!IsOperator(s[ctr]) && ctr < s.size())
    {
        buffer += s[ctr];
        ++ctr;
    }
    query = s.substr(ctr);
    return RAToken(buffer);
}

void RAEngine::RunQueryLeftToRight(string s)
{
    lex = RALexer::GetInstance(s);
    
    //RunTokensLeftToRight();
    //StartOperations();
    
    std::stringstream stream = std::stringstream(s);
    string tok = "";
    
    
    char c = ' ';
    queue<string> toks = queue<string>();
    string table ="";
    
    while(stream>>c)
    {
        table="";
        switch(c)
        {
            case ' ':
                break;
            case '|':
                toks.push("|");
                break;
            case '&':
                toks.push("&");
                break;
            case '*':
                toks.push("*");
                break;
            case '-':
                toks.push("-");
                break;
            case '(':
                toks.push(to_string(c));
                stream>>c;
                if(!IsSelectExpression(c))
                    throw runtime_error(to_string(c)+ "is not a recognizable token");
                table+=c;
                while(stream>>c && (IsSelectExpression(c)) && c != ']' )
                    if(!isspace(c))
                        table+=c;
                if(stream.eof())
                    throw runtime_error(to_string(' ')+"Did not find end of select expression");
                if(!IsSelectExpression(c))
                    throw runtime_error(to_string(c)+ "is not a recognizable token");
                toks.push(")");
                break;
            case '[':
                toks.push(to_string(c));
                stream>>c;
                if(!IsProjectExpression(c))
                    throw runtime_error(to_string(c)+ "is not a recognizable token");
                table+=c;
                while(stream>>c && (IsProjectExpression(c)) && c != ']' )
                    if(!isspace(c))
                        table+=c;
                if(stream.eof())
                    throw runtime_error(to_string(' ')+"Did not find end of select expression");
                if(!IsProjectExpression(c))
                    throw runtime_error(to_string(c)+ "is not a recognizable token");
                break;
                toks.push("]");
            default:
                if(!isalnum(c))
                    throw runtime_error(to_string(c)+ "is not a recognizable token");
                table+=c;
                while(stream>>c && (isalnum(c)) )
                    table+=c;
                size_t lng = table.length();

                if(lng < 4 || (table.substr(lng-3)) != ".db")
                    table+=".db";
                toks.push(table);
                stream.unget();
                break;
        }
    }
    
        if(!IsTable(tok))
        {
            throw runtime_error("cannot start without table");
        }
        string checkTok = "";
        
        RAOperation* llink = nullptr;
        RAOperation * op = nullptr;
        RAOperation * rlink = nullptr;
        
        if(toks.size()!=0)
        {
            //expect table
            checkTok = toks.front();
            if(!IsTable(checkTok))
                throw runtime_error("cannot start without table");
            llink = GetTable(&toks);
        }
                    size_t t = toks.size();
        while(toks.size() != 0)
        {
            checkTok = toks.front();
            if(!IsOperator(checkTok))
                throw runtime_error("Error:expected operand after table expression");
            
            char optype = checkTok[0];
            
            toks.pop();
            checkTok = toks.front();
            
            if(!IsTable(checkTok))
                throw runtime_error("Error:expected table expression after operand");
            
            rlink = GetTable(&toks);
            

            switch (optype)
            {
            case '|':
                op = new UnionOp(llink,rlink);
                break;
            case '&':
                op = new IntersectOp(llink,rlink);
                break;
            case '-':
                op = new DifferenceOp(llink,rlink);
                break;
            default:
                break;
            }
        
         llink= op;
        }
    
        PrintOp final = PrintOp(llink);
        final.Operate();
}

void RAEngine::RunTokensLeftToRight()
{
   /* if(lex.HasToken())
        return;
    RAToken tok = lex.PopToken();
    if((tok.GetTokenType() & Operator)&& !(tok.GetTokenType() & Paren))
    {
        cout<<"Invalid query.  Cannot start with non-paren operator."<<endl;
        return;
    }
    
    int type = tok.GetTokenType();
    
    while(lex.HasToken())
    {
        RAToken nextTok = lex.PopToken();
        int nextType = nextTok.GetTokenType();
        if(type & nextType & SetOperator)  //two set operators
            throw runtime_error("Cannot cascade set operators");
        if(!((type | nextType) & Operator)) //two tokens without operator
            throw runtime_error("Cannot proccess tokens " + tok.GetString()+" and " + nextTok.GetString() + " without operator" );
        if (!DBMS::Factory::LookUpCatalog(tok.GetString().c_str()))
        {
            throw runtime_error("Table: "+tok.GetString()+" not found in catalog");
        }
        if(nextType & SetOperator) // expecting Name token
        {
            RAToken operand = lex.PopToken();
            int opType = operand.GetTokenType();
            if(!(opType & (Name|Paren)))
                throw runtime_error("Cannot perform set eval on " + operand.GetString());
            
        }
    }*/
    
}



void EvaluateTokenPrecedence(RAToken tok, RAToken nextTok)
{

}

static list<Attribute> ProjectSchema(list<string>)
{
    return list<Attribute> ();
}

list<DataTuple> RAEngine::SetOperation(RAToken lOp,RAToken op,RAToken rOp, int pId)
{
    if(!(op.GetTokenType() & SetOperator))
        throw runtime_error("Programming error: allowed non set operator in set operation");
    
    BPlustree lTable = BPlustree(lOp.GetString().c_str());
    BPlustree rTable = BPlustree(rOp.GetString().c_str());
    
    DataTuple lt;
    DataTuple rt;
    
    /*
    switch(op.GetString()[0])
    {
        case '&' :
            
            lt = rTable.GetNextLinkedTuple();
            while(!lTable.EndofLinkList())
            {
                rt = rTable.GetNextLinkedTuple();
                while(!rTable.EndofLinkList())
                {
                    if(rt == lt)
                    {
                    }
                }
            }
            break;
        case '|' :
            lt = rTable.GetNextLinkedTuple();
            while(!lTable.EndofLinkList())
            {
                DataTuple rt = rTable.GetNextLinkedTuple();
                while(!rTable.EndofLinkList())
                {
                }
            }
            break;
        case '-' :
            lt = rTable.GetNextLinkedTuple();
            while(!lTable.EndofLinkList())
            {
                rt = rTable.GetNextLinkedTuple();
                while(!rTable.EndofLinkList())
                {
                    if(rt != lt)
                    {
                    }
                }
            }
            break;
        default: throw runtime_error("unrecognized set operation: " +op.GetString());
    }
     */
    return list<DataTuple>();
}

RAToken RAEngine::PopToken()
{
    if(lex.tokens.size() == 0)
        throw runtime_error("Cannot pop on an empty list");
    RAToken tok = lex.tokens[0];
    lex.tokens.erase(lex.tokens.begin());
    return tok;
}

RAToken RAEngine::PeekToken()
{
    if(lex.tokens.size() == 0)
        throw runtime_error("Cannot pop on an empty list");
    return lex.tokens[0];
    lex.tokens.erase(lex.tokens.begin());

}

void RAEngine::StartOperations()
{
    RAToken tok = PopToken();
    if(tok.GetTokenType() & Name)
    {
        RAToken tok2 = PeekToken();
        if(tok2.GetTokenType() & SetOperator)
        {
            tok2 = PopToken();
            SetOperation(tok, tok2, PopToken(), 0);
        }
        else if(tok2.GetTokenType() & Project)
        {
            
        }
        else if(tok2.GetTokenType() & Selector)
        {
            
        }
    }
    else
        throw runtime_error("Error. Must start query with a table name");
}

void RAEngine::ChainOperations(queue<DataTuple>& previous)
{
    RAToken x = PopToken();
    int type = x.GetTokenType();
    
    //Set Operations
    if(type & SetOperator)
    {
        RAToken tok = PopToken();
        BPlustree tree = BPlustree(tok.GetString().c_str());
        /*switch(x.GetString()[0])
        {
            //union
            case '&' :
                while(!tree.EndofLinkList())
                    previous.push(tree.GetNextLinkedTuple());
                
        }*/
    }
}

RAOperation* RAEngine::GetTable(queue<string>* toks)
{
    if(toks->size() == 0)
        return nullptr;
    
    string tok = toks->front();
    toks->pop();
    
    if(!IsTable(tok))
        return nullptr;
    
    if(toks->size() == 0)
        return new BPlustreeOp(tok);
    
    string next = toks->front();
    
    
    if(IsExpression(next))
    {
        toks->pop();
        if(next[0]=='(')
        {
            string exp = next.substr(1,next.length()-2);
            return new BPlustreeSelectionOp(tok,exp);
        }
        else if(next[0] == '[')
        {
            
        }
    }
    return new BPlustreeOp(tok);
}

bool IsSelectExpression(char c)
{
    return (c>47 && c<58) ||
            (c>64 && c<91) ||
            (c>96 && c<123) ||
            c=='_' || c==')'|| c== 96 || c== 39 || c== 34 ||
            isspace(c);
}

bool IsProjectExpression(char c)
{
    return (c>47 && c<58) ||
    (c>64 && c<91) ||
    (c>96 && c<123) ||
    c=='_' || c==']'|| c== 96 || c== 39 || c== 34 ||
    isspace(c);
}

bool IsTable(string s)
{
    for(auto c : s)
    {
        if(c!='.' && !isalnum(c) && c!= '_')
            return false;
    }
    return true;
}

bool IsExpression(string s)
{
    for(auto c : s)
    {
        if(!IsSelectExpression(c) && !IsProjectExpression(c))
            return false;
    }
    return true;
}

bool IsOperator(string s)
{
    if(s.length() > 1 || s.length() == 0)
        return false;
    if(s[0] == '|' ||s[0] == '&' ||s[0] == '-' || s[0] == '*')
        return true;
    return false;
}