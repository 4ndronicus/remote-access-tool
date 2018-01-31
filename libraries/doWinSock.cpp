#ifndef WINVER
#define WINVER 0x0600
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06000000
#endif

#include <iostream>
#include <winsock2.h>
#include "protocol.h"
#include "doWinSock.h"
#include "logging_class.h"
#include <ws2tcpip.h>
#include <string>
#include <windows.h>

//ctor
doWinSock::doWinSock()
{
    isConnected = false;
//    mPort = lPort;
}

//dtor
doWinSock::~doWinSock()
{
    std::string currFunc = "doWinsock::~doWinsock()";
    l.wr( currFunc, "Destroying windows socket object" );
    WSACleanup();
}

int doWinSock::initSocketSetup()
{
//    WSADATA wsaData;
    int nCode;
    std::string currFunc = "doWinSock::initSocketSetup()";

    if ((nCode = WSAStartup(MAKEWORD(2, 0), &wsaData)) != 0)
    {
        logWSError( WSAGetLastError() );
        l.wr( currFunc, "Socket creation failed" );
        return -1;
    }

    l.wr( currFunc, "Socket initialized" );

    return 0;
}

int doWinSock::initSocketListen()
{

    std::string currFunc = "doWinSock::initSocketListen()";

    l.wr( currFunc, "Initializing the socket - setting up listener" );

    SOCKET listeningSocket = setUpListener();

    if ( listeningSocket == INVALID_SOCKET )
    {
        logWSError( WSAGetLastError() );
        l.wr( currFunc, "Attempt to create the socket and listener failed" );
        WSACleanup();
        isConnected = false;
        return -1;
    }

    l.wr( currFunc, "Listening for a connection" );

    sockaddr_in sinRemote;

    mWinSock = acceptConnection( listeningSocket, sinRemote );

    if ( mWinSock == INVALID_SOCKET )
    {
        logWSError( WSAGetLastError() );
        l.wr( currFunc, "Attempt to accept incoming connection failed" );
        WSACleanup();
        isConnected = false;
        return -2;
    }

    char outMsg[100] = "Accepted connection from ";
    strcat( outMsg, inet_ntoa( sinRemote.sin_addr ) );
    l.wr( currFunc, outMsg );

    isConnected = true;

    return 0;
}


int doWinSock::initSocketConnect(char* IPAddress)
{
    std::string currFunc = "doWinSock::initSocketConnect()";

    std::string connResponse;

    char tPortNum[20]; // The target port number

    std::string outMsg = "Attempting to connect to ";

    l.wr( currFunc, "Attempting to set up a connection" );

    l.wr( currFunc, "Attempting to create the socket" );

    int connStatus = 0;

    mWinSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if ( mWinSock == INVALID_SOCKET )
    {
        logWSError( WSAGetLastError() );
        l.wr( currFunc, "Unable to create socket" );
        WSACleanup();
        isConnected = false;
        return INVALID_SOCKET;
    }

    l.wr( currFunc, "Successfully created socket" );

    l.wr( currFunc, "Configuring socket" );

    sockaddr_in sinInterface;
    sinInterface.sin_family = AF_INET;
    sinInterface.sin_addr.s_addr = inet_addr( IPAddress );
    sinInterface.sin_port = htons( CONN_PORT );

    outMsg.append( IPAddress );
    outMsg.append( ":" );

    itoa( CONN_PORT, tPortNum, 10 );
    outMsg.append( tPortNum );

    l.wr( currFunc, outMsg );

    connStatus = connect( mWinSock, (SOCKADDR*) &sinInterface, sizeof( sinInterface ) );

    if( connStatus != 0 )
    {
        logWSError( WSAGetLastError() );
        l.wr( currFunc, "There was an error connecting" );
        WSACleanup();
        isConnected = false;
        return connStatus;
    }

    l.wr( currFunc, "Connected successfully ");

    isConnected = true;

    return 0;
}

/*
 * Used to send a message to the client - this will
 * usually be a menu, but could be a status or some
 * other message
 */
int doWinSock::sendData( std::string sendBuffer )
{

    std::string currFunc = "doWinsock::sendData()";

    std::string chkBuffer = "";

    l.wr( currFunc, "Entering function" );

    int sSendBytes = 0;

    l.wr( currFunc, "Copying buffer : ", sendBuffer );
    char* sendBufferchar = new char[sendBuffer.length() + 1];
    strcpy( sendBufferchar, sendBuffer.c_str() );

    chkBuffer.append( sendBufferchar );

    l.wr( currFunc, "Buffer copied : ", chkBuffer );

    sendBuffer[sendBuffer.length() + 1] = '\0';

    l.wr( currFunc, "Attempting to send data" );

    sSendBytes = send( mWinSock, sendBufferchar, sendBuffer.length(), 0 );

    if ( sSendBytes == SOCKET_ERROR )
    {
        logWSError( WSAGetLastError() );
        l.wr( currFunc, "There was an error sending the data" );
        WSACleanup();
        isConnected = false;
        return -1;
    }

    l.wr( currFunc, "Data sent successfully" );

    return 0;
}

std::string doWinSock::recvData()
{

    std::string currFunc = "doWinSock::recvCmd()";

    l.wr( currFunc, "Entering function" );

    std::string cmdRcvBuffer = "";
    char acReadBuffer[kBufferSize] = { '\0' };
//    char tNumBytes[20];
//    char cRecvBuffer[kBufferSize] = { '\0' };
    int nReadBytes = 0;
//    int buffSize = 0;

//    char tBuffSize[20] = { '\0' };

    l.wr( currFunc, "Receiving data");
    do
    {
        for( int i = 0; i < kBufferSize; i++ )
        {
            acReadBuffer[i] = '\0';
        }

        nReadBytes = recv(mWinSock, acReadBuffer, kBufferSize, 0);
        cmdRcvBuffer.append( acReadBuffer );

    }
    while ( nReadBytes == kBufferSize );

    if ( nReadBytes == SOCKET_ERROR )
    {
        logWSError( WSAGetLastError() );
        l.wr( currFunc, "There was an error receiving the data" );
        return SOCK_FAIL;
    }

    l.wr( currFunc, "Successfully received buffer" );

    return cmdRcvBuffer;
}



//// SetUpListener /////////////////////////////////////////////////////
// Sets up a listener on the given interface and port, returning the
// listening socket if successful; if not, returns INVALID_SOCKET.

SOCKET doWinSock::setUpListener()
{
    std::string currFunc = "doWinSock::SetUpListener()";
    l.wr( currFunc, "Attempting to set up a listener" );

    l.wr( currFunc, "Attempting to create the socket" );
    SOCKET sd = socket(AF_INET, SOCK_STREAM, 0);
    if ( sd == INVALID_SOCKET )
    {
        logWSError( WSAGetLastError() );
        l.wr( currFunc, "Unable to create socket" );
        WSACleanup();
        return INVALID_SOCKET;
    }
    l.wr( currFunc, "Successfully created socket" );

    l.wr( currFunc, "Attempting to set up the listener" );
    sockaddr_in sinInterface;
    sinInterface.sin_family = AF_INET;
    sinInterface.sin_addr.s_addr = htonl( INADDR_ANY );
    sinInterface.sin_port = htons( CONN_PORT );
    if ( bind( sd, (sockaddr*)&sinInterface, sizeof(sockaddr_in) ) != SOCKET_ERROR )
    {
        listen( sd, 1 );
        l.wr( currFunc, "Attempt to set up listener successful" );
        return sd;
    }

    logWSError( WSAGetLastError() );
    l.wr( currFunc, "Attempt to set up listener failed" );
    return INVALID_SOCKET;
}

//// AcceptConnection //////////////////////////////////////////////////
// Waits for a connection on the given socket.  When one comes in, we
// return a socket for it.  If an error occurs, we return
// INVALID_SOCKET.

SOCKET doWinSock::acceptConnection(SOCKET ListeningSocket, sockaddr_in& sinRemote)
{
    std::string currFunc = "doWinSock::acceptConnection()";
    l.wr( currFunc, "Accepting connections on the listening socket" );
    int nAddrSize = sizeof(sinRemote);
    return accept( ListeningSocket, (sockaddr*)&sinRemote, &nAddrSize );
}

//// ShutdownConnection ////////////////////////////////////////////////
// Gracefully shuts the connection sd down.  Returns true if we're
// successful, false otherwise.

bool doWinSock::ShutdownConnection()
{
    // Disallow any further data sends.  This will tell the other side
    // that we want to go away now.  If we skip this step, we don't
    // shut the connection down nicely.

    std::string currFunc = "doWinSock::ShutdownConnection()";

    l.wr( currFunc, "Shutting down the connection" );
    if ( shutdown(mWinSock, SD_SEND) == SOCKET_ERROR )
    {
        l.wr( currFunc, "Failed to shut down the connection" );
        logWSError( WSAGetLastError() );
        WSACleanup();
        isConnected = false;
        return false;
    }
    l.wr( currFunc, "Successfully shut down the connection" );

    // Receive any extra data still sitting on the socket.  After all
    // data is received, this call will block until the remote host
    // acknowledges the TCP control packet sent by the shutdown above.
    // Then we'll get a 0 back from recv, signaling that the remote
    // host has closed its side of the connection.
    char acReadBuffer[kBufferSize];
    while ( true )
    {
        int nNewBytes = recv( mWinSock, acReadBuffer, kBufferSize, 0 );
        if ( nNewBytes == SOCKET_ERROR )
        {
            logWSError( WSAGetLastError() );
            WSACleanup();
            isConnected = false;
            return false;
        }
        else if ( nNewBytes != 0 ) {}
        else
        {
            break;
        }
    }

    // Close the socket.
    l.wr( currFunc, "Attempting to close the socket" );
    if ( closesocket( mWinSock ) == SOCKET_ERROR )
    {
        l.wr( currFunc, "Failed to close the socket" );
        logWSError( WSAGetLastError() );
        WSACleanup();
        isConnected = false;
        return false;
    }
    l.wr( currFunc, "Successfully closed the socket" );

    WSACleanup();

    isConnected = false;

    return true;
}

void doWinSock::resetConnection()
{

    std::string currFunc = "doWinSock::ShutdownConnection()";
    /*std::string statusMsg = "\r\nClosing connection.\r\n";

    l.wr( currFunc, "Sending close notification" );
    if ( sendData( statusMsg ) == 0 )
    {
        l.wr( currFunc, "Close notification successfully sent" );
    }
    else
    {
        l.wr( currFunc, "Failed to send close notification" );
    }*/

    l.wr( currFunc, "Attempting to close connection" );
    if ( ShutdownConnection() )
    {
        l.wr( currFunc, "Successfully closed connection" );
    }
    else
    {
        l.wr( currFunc, "Failed to close connection" );
    }

    l.wr( currFunc, "Attempting to initialize socket" );
    if ( initSocketSetup() == 0 )
    {
        l.wr( currFunc, "Socket initialized successfully" );
    }
    else
    {
        l.wr( currFunc, "Failed to initialize socket" );
    }

    l.wr( currFunc, "Attempting to set socket to listen" );
    if ( initSocketListen() == 0 )
    {
        l.wr( currFunc, "Socket successfully set to listen" );
    }
    else
    {
        l.wr( currFunc, "Failed to set socket to listen" );
    }
}

/*
 * Process the command that the user sent the program
 */
//int doWinSock::processCommand( std::string cmdRcvBuffer )
//{
//
//    std::string currFunc = "doWinSock::processCommand()";
////    int socketStatus = 0;
//
//    l.wr( currFunc, "Entering function" );
//
//    /* If the user wants to close the connection */
//    if ( cmdRcvBuffer == SOCK_RESET )
//    {
//        resetConnection();
//
//        return 0;
//
//    }
//    else if ( cmdRcvBuffer == LPROC )
//    {
//
//        l.wr( currFunc, "Remote user wants the list of processes running here" );
//
//        return 0;
//
//    }
//
//    return 0;
//}

void doWinSock::logWSError( int errNum )
{

    std::string currFunc = "doWinSock::logWSError()";

//    char outMsg[] = "Error Number: ";

    std::string outMsg = "Error Number: ";
    char errNo[20];
    itoa( errNum, errNo, 10 );
    outMsg.append( errNo );
    outMsg.append( " - " );
//    strcat( outMsg, errNo );
//    strcat( outMsg, " - " );

    switch ( errNum )
    {
    case WSA_INVALID_HANDLE:
        outMsg.append( "The event object handle that we tried to use was not valid." );
        break;
    case WSA_NOT_ENOUGH_MEMORY:
        outMsg.append( "There was not enough memory left to perform the requested operation." );
        break;
    case WSA_INVALID_PARAMETER:
        outMsg.append( "There was a problem with one or more of the parameters." );
        break;
    case WSA_OPERATION_ABORTED:
        outMsg.append( "An overlapped operation was canceled for some reason." );
        break;
    case WSA_IO_INCOMPLETE:
        outMsg.append( "The application has tried to determine the status of an overlapped operation which is not yet completed." );
        break;
    case WSA_IO_PENDING:
        outMsg.append( "The application has initiated an overlapped operation that cannot be completed immediately." );
        break;
    case WSAEINTR:
        outMsg.append( "A blocking operation was interrupted by a call to WSACancelBlockingCall." );
        break;
    case WSAEBADF:
        outMsg.append( "The file handle that was provided is not valid." );
        break;
    case WSAEACCES:
        outMsg.append( "An attempt was made to access a socket in a way forbidden by its access permissions." );
        break;
    case WSAEFAULT:
        outMsg.append( "The system detected an invalid pointer address in attempting to use a pointer argument of a call.  " );
        outMsg.append( "This error occurs if an application passes an invalid pointer value, or if the length of the buffer is too small.  " );
        outMsg.append( "For instance, if the length of an argument, which is a sockaddr structure, is smaller than the sizeof(sockaddr)." );
        break;
    case WSAEINVAL:
        outMsg.append( "Some invalid argument was supplied (for example, specifying an invalid level to the setsockopt function).  " );
        outMsg.append( "In some instances, it also refers to the current state of the socket - for instance, calling accept on a socket that is not listening." );
        break;
    case WSAEMFILE:
        outMsg.append( "Too many open sockets. Each implementation may have a maximum number of socket handles available, either globally, per process, or per thread." );
        break;
    case WSAEWOULDBLOCK:
        outMsg.append( "This error is returned from operations on nonblocking sockets that cannot be completed immediately, for example recv when no data is queued to " );
        outMsg.append( "be read from the socket. It is a nonfatal error, and the operation should be retried later. It is normal for WSAEWOULDBLOCK to be reported as the " );
        outMsg.append( "result from calling connect on a nonblocking SOCK_STREAM socket, since some time must elapse for the connection to be established." );
        break;
    case WSAEINPROGRESS:
        outMsg.append( "A blocking operation is currently executing. Windows Sockets only allows a single blocking operation -- per task or thread -- to be outstanding, and if " );
        outMsg.append( "any other function call is made (whether or not it references that or any other socket) the function fails with the WSAEINPROGRESS error." );
        break;
    case WSAEALREADY:
        outMsg.append( "An operation was attempted on a nonblocking socket with an operation already in progress -- that is, calling connect a second time on a nonblocking socket " );
        outMsg.append( "that is already connecting, or canceling an asynchronous request (WSAAsyncGetXbyY) that has already been canceled or completed." );
        break;
    case WSAENOTSOCK:
        outMsg.append( "An operation was attempted on something that is not a socket. Either the socket handle parameter did not reference a valid socket, or for select, " );
        outMsg.append( "a member of an fd_set was not valid." );
        break;
    case WSAEDESTADDRREQ:
        outMsg.append( "A required address was omitted from an operation on a socket. For example, this error is returned if sendto is called with the remote address of ADDR_ANY." );
        break;
    case WSAEMSGSIZE:
        outMsg.append( "A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram was " );
        outMsg.append( "smaller than the datagram itself." );
        break;
    case WSAEPROTOTYPE:
        outMsg.append( "A protocol was specified in the socket function call that does not support the semantics of the socket type requested. For example, the ARPA Internet " );
        outMsg.append( "UDP protocol cannot be specified with a socket type of SOCK_STREAM." );
        break;
    case WSAENOPROTOOPT:
        outMsg.append( "An unknown, invalid or unsupported option or level was specified in a getsockopt or setsockopt call." );
        break;
    case WSAEPROTONOSUPPORT:
        outMsg.append( "The requested protocol has not been configured into the system, or no implementation for it exists. For example, a socket call requests a SOCK_DGRAM " );
        outMsg.append( "socket, but specifies a stream protocol." );
        break;
    case WSAESOCKTNOSUPPORT:
        outMsg.append( "The support for the specified socket type does not exist in this address family. For example, the optional type SOCK_RAW might be selected in a socket " );
        outMsg.append( "call, and the implementation does not support SOCK_RAW sockets at all." );
        break;
    case WSAEOPNOTSUPP:
        outMsg.append( "The attempted operation is not supported for the type of object referenced. Usually this occurs when a socket descriptor to a socket that cannot support " );
        outMsg.append( "this operation is trying to accept a connection on a datagram socket." );
        break;
    case WSAEPFNOSUPPORT:
        outMsg.append( "The protocol family has not been configured into the system or no implementation for it exists. This message has a slightly different meaning from " );
        outMsg.append( "WSAEAFNOSUPPORT. However, it is interchangeable in most cases, and all Windows Sockets functions that return one of these messages also specify WSAEAFNOSUPPORT." );
        break;
    case WSAEAFNOSUPPORT:
        outMsg.append( "An address incompatible with the requested protocol was used. All sockets are created with an associated address family (that is, AF_INET for Internet Protocols) " );
        outMsg.append( "and a generic protocol type (that is, SOCK_STREAM). This error is returned if an incorrect protocol is explicitly requested in the socket call, or if an address " );
        outMsg.append( "of the wrong family is used for a socket, for example, in sendto." );
        break;
    case WSAEADDRINUSE:
        outMsg.append( "The address and port combination that we attempted to use were already in use by some other process or application.  Typically, only one usage of each socket " );
        outMsg.append( "address (protocol/IP address/port) is permitted. This error occurs if an application attempts to bind a socket to an IP address/port that has already been " );
        outMsg.append( "used for an existing socket, or a socket that was not closed properly, or one that is still in the process of closing.");
        break;
    case WSAEADDRNOTAVAIL:
        outMsg.append( "The requested address is not valid in its context. This normally results from an attempt to bind to an address that is not valid for the local computer. " );
        outMsg.append( "This can also result from connect, sendto, WSAConnect, WSAJoinLeaf, or WSASendTo when the remote address or port is not valid for a remote computer " );
        outMsg.append( "(for example, address or port 0)." );
        break;
    case WSAENETDOWN:
        outMsg.append( "A socket operation encountered a dead network. This could indicate a serious failure of the network system (that is, the protocol stack that the Windows " );
        outMsg.append( "Sockets DLL runs over), the network interface, or the local network itself." );
        break;
    case WSAENETUNREACH:
        outMsg.append( "A socket operation was attempted to an unreachable network. This usually means the local software knows no route to reach the remote host." );
        break;
    case WSAENETRESET:
        outMsg.append( "The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress. It can also be returned " );
        outMsg.append( "by setsockopt if an attempt is made to set SO_KEEPALIVE on a connection that has already failed." );
        break;
    case WSAECONNABORTED:
        outMsg.append( "An established connection was aborted by the software in your host computer, possibly due to a data transmission time-out or protocol error." );
        break;
    case WSAECONNRESET:
        outMsg.append( "An existing connection was forcibly closed by the remote host. This normally results if the peer application on the remote host is suddenly " );
        outMsg.append( "stopped, the host is rebooted, the host or remote network interface is disabled, or the remote host uses a hard close (see setsockopt for more " );
        outMsg.append( "information on the SO_LINGER option on the remote socket). This error may also result if a connection was broken due to keep-alive activity " );
        outMsg.append( "detecting a failure while one or more operations are in progress. Operations that were in progress fail with WSAENETRESET. Subsequent operations " );
        outMsg.append( "fail with WSAECONNRESET." );
        break;
    case WSAENOBUFS:
        outMsg.append( "An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full." );
        break;
    case WSAEISCONN:
        outMsg.append( "A connect request was made on an already-connected socket. Some implementations also return this error if sendto is called on a connected SOCK_DGRAM " );
        outMsg.append( "socket (for SOCK_STREAM sockets, the to parameter in sendto is ignored) although other implementations treat this as a legal occurrence." );
        break;
    case WSAENOTCONN:
        outMsg.append( "A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using sendto) " );
        outMsg.append( "no address was supplied. Any other type of operation might also return this error -- for example, setsockopt setting SO_KEEPALIVE if the " );
        outMsg.append( "connection has been reset." );
        break;
    case WSAESHUTDOWN:
        outMsg.append( "A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown " );
        outMsg.append( "call. By calling shutdown a partial close of a socket is requested, which is a signal that sending or receiving, or both have been discontinued." );
        break;
    case WSAETOOMANYREFS:
        outMsg.append( "Too many references to some kernel object." );
        break;
    case WSAETIMEDOUT:
        outMsg.append( "A connection attempt failed because the connected party did not properly respond after a period of time, or the established connection failed " );
        outMsg.append( "because the connected host has failed to respond." );
        break;
    case WSAECONNREFUSED:
        outMsg.append( "No connection could be made because the target computer actively refused it. This usually results from trying to connect to a service that is " );
        outMsg.append( "inactive on the foreign host -- that is, one with no server application running." );
        break;
    case WSAELOOP:
        outMsg.append( "Cannot translate a name." );
        break;
    case WSAENAMETOOLONG:
        outMsg.append( "A name component or a name was too long." );
        break;
    case WSAEHOSTDOWN:
        outMsg.append( "A socket operation failed because the destination host is down. A socket operation encountered a dead host. Networking activity on the local " );
        outMsg.append( "host has not been initiated. These conditions are more likely to be indicated by the error WSAETIMEDOUT." );
        break;
    case WSAEHOSTUNREACH:
        outMsg.append( "A socket operation was attempted to an unreachable host. See WSAENETUNREACH." );
        break;
    case WSAENOTEMPTY:
        outMsg.append( "Cannot remove a directory that is not empty." );
        break;
    case WSAEPROCLIM:
        outMsg.append( "A Windows Sockets implementation may have a limit on the number of applications that can use it simultaneously. WSAStartup may fail with this " );
        outMsg.append( "error if the limit has been reached." );
        break;
    case WSAEUSERS:
        outMsg.append( "Ran out of user quota." );
        break;
    case WSAEDQUOT:
        outMsg.append( "Ran out of disk quota." );
        break;
    case WSAESTALE:
        outMsg.append( "The file handle reference is no longer available." );
        break;
    case WSAEREMOTE:
        outMsg.append( "The item is not available locally." );
        break;
    case WSASYSNOTREADY:
        outMsg.append( "This error is returned by WSAStartup if the Windows Sockets implementation cannot function at this time because the underlying system it uses " );
        outMsg.append( "to provide network services (the winsock dll, etc) is currently unavailable, maybe because the wrong path to it was specified." );
        break;
    case WSAVERNOTSUPPORTED:
        outMsg.append( "The current Windows Sockets implementation does not support the Windows Sockets specification version requested by the application. " );
        outMsg.append( "Check that no old Windows Sockets DLL files are being accessed." );
        break;
    case WSANOTINITIALISED:
        outMsg.append( "Either the application has not called WSAStartup or WSAStartup failed. The application may be accessing a socket that the current " );
        outMsg.append( "active task does not own (that is, trying to share a socket between tasks), or WSACleanup has been called too many times." );
        break;
    case WSAEDISCON:
        outMsg.append( "Returned by WSARecv and WSARecvFrom to indicate that the remote party has initiated a graceful shutdown sequence." );
        break;
    case WSAENOMORE:
        outMsg.append( "No more results can be returned by the WSALookupServiceNext function." );
        break;
    case WSAECANCELLED:
        outMsg.append( "A call to the WSALookupServiceEnd function was made while this call was still processing. The call has been canceled." );
        break;
    case WSAEINVALIDPROCTABLE:
        outMsg.append( "The service provider procedure call table is invalid. A service provider returned a bogus procedure table to Ws2_32.dll. " );
        outMsg.append( "This is usually caused by one or more of the function pointers being NULL." );
        break;
    case WSAEINVALIDPROVIDER:
        outMsg.append( "The requested service provider is invalid. This error is returned by the WSCGetProviderInfo and WSCGetProviderInfo32 " );
        outMsg.append( "functions if the protocol entry specified could not be found. This error is also returned if the service provider " );
        outMsg.append( "returned a version number other than 2.0." );
        break;
    case WSAEPROVIDERFAILEDINIT:
        outMsg.append( "The requested service provider could not be loaded or initialized. This error is returned if either a service " );
        outMsg.append( "provider's DLL could not be loaded (LoadLibrary failed) or the provider's WSPStartup or NSPStartup function failed." );
        break;
    case WSASYSCALLFAILURE:
        outMsg.append( "A system call that should never fail has failed. This is a generic error code, returned under various conditions.  " );
        outMsg.append( "Returned when a system call that should never fail does fail.  Returned when a provider does not return SUCCESS and " );
        outMsg.append( "does not provide an extended error code. Can indicate a service provider implementation error." );
        break;
    case WSASERVICE_NOT_FOUND:
        outMsg.append( "No such service is known. The service cannot be found in the specified name space." );
        break;
    case WSATYPE_NOT_FOUND:
        outMsg.append( "The specified class was not found." );
        break;
    case WSA_E_NO_MORE:
        outMsg.append( "No more results can be returned by the WSALookupServiceNext function." );
        break;
    case WSA_E_CANCELLED:
        outMsg.append( "A call to the WSALookupServiceEnd function was made while this call was still processing. The call has been canceled." );
        break;
    case WSAEREFUSED:
        outMsg.append( "A database query failed because it was actively refused." );
        break;
    case WSAHOST_NOT_FOUND:
        outMsg.append( "No such host is known. The name is not an official host name or alias, or it cannot be found in the database(s) being " );
        outMsg.append( "queried. This error may also be returned for protocol and service queries, and means that the specified name could not " );
        outMsg.append( "be found in the relevant database." );
        break;
    case WSATRY_AGAIN:
        outMsg.append( "This is usually a temporary error during host name resolution and means that the local server did not receive a response " );
        outMsg.append( "from an authoritative server. A retry at some time later may be successful." );
        break;
    case WSANO_RECOVERY:
        outMsg.append( "This indicates that some sort of nonrecoverable error occurred during a database lookup. This may be because the database " );
        outMsg.append( "files (for example, BSD-compatible HOSTS, SERVICES, or PROTOCOLS files) could not be found, or a DNS request was returned " );
        outMsg.append( "by the server with a severe error." );
        break;
    case WSANO_DATA:
        outMsg.append( "The requested name is valid and was found in the database, but it does not have the correct associated data being resolved " );
        outMsg.append( "for. The usual example for this is a host name-to-address translation attempt (using gethostbyname or WSAAsyncGetHostByName) " );
        outMsg.append( "which uses the DNS (Domain Name Server). An MX record is returned but no A record -- indicating the host itself exists, but " );
        outMsg.append( "is not directly reachable." );
        break;
    case WSA_QOS_RECEIVERS:
        outMsg.append( "At least one QoS reserve has arrived." );
        break;
    case WSA_QOS_SENDERS:
        outMsg.append( "At least one QoS send path has arrived." );
        break;
    case WSA_QOS_NO_SENDERS:
        outMsg.append( "There are no QoS senders." );
        break;
    case WSA_QOS_NO_RECEIVERS:
        outMsg.append( "There are no QoS receivers." );
        break;
    case WSA_QOS_REQUEST_CONFIRMED:
        outMsg.append( "The QoS reserve request has been confirmed." );
        break;
    case WSA_QOS_ADMISSION_FAILURE:
        outMsg.append( "A QoS error occurred due to lack of resources." );
        break;
    case WSA_QOS_POLICY_FAILURE:
        outMsg.append( "The QoS request was rejected because the policy system couldn't allocate the requested resource within the existing policy." );
        break;
    case WSA_QOS_BAD_STYLE:
        outMsg.append( "An unknown or conflicting QoS style was encountered." );
        break;
    case WSA_QOS_BAD_OBJECT:
        outMsg.append( "A problem was encountered with some part of the filterspec or the provider-specific buffer in general." );
        break;
    case WSA_QOS_TRAFFIC_CTRL_ERROR:
        outMsg.append( "An error with the underlying traffic control (TC) API as the generic QoS request was converted for local " );
        outMsg.append( "enforcement by the TC API. This could be due to an out of memory error or to an internal QoS provider error." );
        break;
    case WSA_QOS_GENERIC_ERROR:
        outMsg.append( "A general QoS error." );
        break;
    case WSA_QOS_ESERVICETYPE:
        outMsg.append( "An invalid or unrecognized service type was found in the QoS flowspec." );
        break;
    case WSA_QOS_EFLOWSPEC:
        outMsg.append( "An invalid or inconsistent flowspec was found in the QOS structure." );
        break;
    case WSA_QOS_EPROVSPECBUF:
        outMsg.append( "An invalid QoS provider-specific buffer." );
        break;
    case WSA_QOS_EFILTERSTYLE:
        outMsg.append( "An invalid QoS filter style was used." );
        break;
    case WSA_QOS_EFILTERTYPE:
        outMsg.append( "An invalid QoS filter type was used." );
        break;
    case WSA_QOS_EFILTERCOUNT:
        outMsg.append( "An incorrect number of QoS FILTERSPECs were specified in the FLOWDESCRIPTOR." );
        break;
    case WSA_QOS_EOBJLENGTH:
        outMsg.append( "An object with an invalid ObjectLength field was specified in the QoS provider-specific buffer." );
        break;
    case WSA_QOS_EFLOWCOUNT:
        outMsg.append( "An incorrect number of flow descriptors was specified in the QoS structure." );
        break;
    case WSA_QOS_EUNKOWNPSOBJ:
        outMsg.append( "An unrecognized object was found in the QoS provider-specific buffer." );
        break;
    case WSA_QOS_EPOLICYOBJ:
        outMsg.append( "An invalid policy object was found in the QoS provider-specific buffer." );
        break;
    case WSA_QOS_EFLOWDESC:
        outMsg.append( "An invalid QoS flow descriptor was found in the flow descriptor list." );
        break;
    case WSA_QOS_EPSFLOWSPEC:
        outMsg.append( "An invalid or inconsistent flowspec was found in the QoS provider-specific buffer." );
        break;
    case WSA_QOS_EPSFILTERSPEC:
        outMsg.append( "An invalid FILTERSPEC was found in the QoS provider-specific buffer." );
        break;
    case WSA_QOS_ESDMODEOBJ:
        outMsg.append( "An invalid shape discard mode object was found in the QoS provider-specific buffer." );
        break;
    case WSA_QOS_ESHAPERATEOBJ:
        outMsg.append( "An invalid shaping rate object was found in the QoS provider-specific buffer." );
        break;
    case WSA_QOS_RESERVED_PETYPE:
        outMsg.append( "A reserved policy element was found in the QoS provider-specific buffer." );
        break;
    default:
        outMsg.append( "We were unable to tell what the exact error was." );
    }

    l.wr( currFunc, outMsg );
}
