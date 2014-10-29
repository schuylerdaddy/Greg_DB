//
//  main.cpp
//  RAEngine
//
//  Created by GregMac on 6/1/14.
//  Copyright (c) 2014 Mac. All rights reserved.
//

#include <iostream>
#include "RAEngine.h"

int main(int argc, const char * argv[])
{
    string query = "(parts('pno'=7)|(parts('color'=\"red\")&parts())";
    //RALexer::ProcessTokens(query);
    query = "PHXParts|SLCParts";
    //RALexer::ProcessTokens(query);
    RAEngine eng = RAEngine();
    eng.RunQueryLeftToRight(query);
    return 0;
}

