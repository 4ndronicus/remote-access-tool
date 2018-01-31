#include "time_class.h"
//#include "time_constants.h"
#include <stdlib.h>
#include <time.h>

using namespace std;

timeOps::timeOps() {}

timeOps::~timeOps() {}

int timeOps::getFullTime( char* timeBuffer )
{

    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    strftime ( timeBuffer, TIMESIZE, "%Y-%m-%d %H:%M:%S", timeinfo );

    return 0;
}

int timeOps::getTimeMin( char* timeBuffer )
{

    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    strftime ( timeBuffer, TIMESIZE, "%Y-%m-%d %H:%M", timeinfo );

    return 0;
}

/*
    Returns the day of the year as a decimal number (range 1 to 366)
*/
int timeOps::getDayInt( )
{

    time_t rawtime;
    struct tm * timeinfo;
    char timeBuffer[TIMESIZE];

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    strftime ( timeBuffer, TIMESIZE, "%j", timeinfo );

    return atoi( timeBuffer );
}

