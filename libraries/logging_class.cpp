#include <string>
#include <iostream>
#include <fstream>
#include <string.h>

#include "logging_class.h"
#include "time_class.h"
#include "string_ops.h"

const char* OUTFILE = "output.txt";

using namespace std; // REQUIRED FOR IOSTREAM TO WORK PROPERLY

Log::Log() {}

Log::~Log() {}

int Log::wr( std::string currFunc, const char* toLog )
{

    std::string outStr = toLog;
    wr( currFunc, outStr );
    return 0;
}

int Log::wr( std::string currFunc, std::string toLog )
{

    ofstream myfile;
    timeOps timeObj;
    char timestr[TIMESIZE];

    timeObj.getFullTime( timestr );

    myfile.open( OUTFILE, ios::out | ios::app );

    myfile << timestr << " - " << currFunc << " - " << toLog << "\n";

    myfile.close();

    return 0;
}

int Log::wr( std::string currFunc, const char* descStr, char* valStr)
{

    std::string concat;
    concat.append( descStr );
    concat.append( valStr );

    wr( currFunc, concat );

    return 0;

}

int Log::wr( std::string currFunc, const char* descStr, std::string valStr )
{

    std::string concat;
    concat.append( descStr );
    concat.append( valStr );

    wr( currFunc, concat );

    return 0;

}

int Log::wr( std::string currFunc, const char* descStr, unsigned char* byteArr )
{

    std::string concat = byteArrToStr( byteArr );
    std::string tgtStr( descStr );
    tgtStr.append( concat );
    wr( currFunc, tgtStr );

    return 0;

}

void Log::wrln()
{

    std::string currFunc = "";
    ofstream myfile;
    char breakLine[81];
    memset( breakLine, '-', 80 );
    breakLine[80] = '\0';

    wr( currFunc, breakLine );
}
