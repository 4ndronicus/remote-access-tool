#include <iostream>
#include <string.h>
#include <string>
#include <locale>
#include "string_ops.h"
#include "logging_class.h"

std::string strtoupper( std::string inStr )
{
    std::string newBuffer;
    std::use_facet<std::ctype<char> >(std::locale()).toupper(&inStr[0], &inStr[0] + inStr.size());
    newBuffer.assign( inStr );
    return newBuffer;
}

int strToCharArr( std::string inStr, char* tgtBuffer )
{

    Log l;

    std::string currFunc = "strToCharArr()";

    l.wr( currFunc, "Entering function" );

    strcpy( tgtBuffer, inStr.c_str() );

    l.wr( currFunc, "Leaving function" );

    return 0;
}

int strToByteArr( std::string inStr, unsigned char* tgtArr, int strLen )
{

    Log l;

//    int strLen = inStr.length();

//    char tgtBuffer[ strLen ];

    std::string currFunc = "strToCharArr()";

    l.wr( currFunc, "Entering function" );

//    strcpy( tgtBuffer, inStr.c_str() );

    memcpy( tgtArr, inStr.c_str(), strLen );

    tgtArr[strLen] = '\0';

    l.wr( currFunc, "Leaving function" );

    return 0;
}

std::string charArrToStr( char* srcBuffer )
{

    Log l;

    std::string currFunc = "charArrToStr()";

    l.wr( currFunc, "Entering function" );

    std::string tgtBuffer( srcBuffer );

    l.wr( currFunc, "Target string: ", tgtBuffer );

    l.wr( currFunc, "Leaving function" );

    return tgtBuffer;
}

std::string byteArrToStr( unsigned char* tgtArr )
{

    Log l;

    std::string currFunc = "byteArrToStr()";

    l.wr( currFunc, "Entering function" );

    std::string tgtString( ( const char* ) tgtArr );

//    memcpy( tgtArr, inStr.c_str(), strLen );

    l.wr( currFunc, "Target string: ", tgtString );

    l.wr( currFunc, "Leaving function" );

    return tgtString;
}
