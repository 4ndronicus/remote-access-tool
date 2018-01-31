#ifndef TIME_CLASS_H
#define TIME_CLASS_H

const int TIMESIZE = 80;

class timeOps
{

public:

    timeOps();

    ~timeOps();

    int getFullTime( char* );
    int getTimeMin( char* );
    int getDayInt( );

};

#endif // TIME_CLASS_H
