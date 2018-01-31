#ifndef LOGGING_CLASS_H
#define LOGGING_CLASS_H

#include <string>

class Log
{

public:

    Log();

    ~Log();

    /*
     * Writes a const char string to the log file
     */
    int wr( std::string, const char* );

    int wr( std::string, std::string );
    int wr( std::string, const char*, char* );
    int wr( std::string, const char*, std::string );
    int wr( std::string, const char*, unsigned char* );

    /*
     * Writes out a line delimiter to make logs easier to read
     */
    void wrln();

};

#endif // LOGGING_CLASS_H
