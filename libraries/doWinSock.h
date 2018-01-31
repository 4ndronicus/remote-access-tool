#ifndef DOWINSOCK_H
#define DOWINSOCK_H

#include "logging_class.h"

const int kBufferSize = 4096;

class doWinSock
{

public:

    bool isConnected;

    doWinSock();
    virtual ~doWinSock();

    SOCKET GetmWinSock()
    {
        return mWinSock;
    }
    int initSocketListen();
    int initSocketConnect( char* );
    int initSocketSetup();
    int sendData( std::string );
    std::string recvData();
//    int processCommand( std::string );
    void resetConnection();
    bool ShutdownConnection();

protected:

private:
    Log l;
    WSADATA wsaData;
    SOCKET mWinSock;
    int mPort;
    SOCKET setUpListener();
    SOCKET acceptConnection( SOCKET, sockaddr_in& );
    void logWSError( int );
};

#endif // DOWINSOCK_H
