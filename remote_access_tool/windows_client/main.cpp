#define _WIN32_IE 0x301

#include <windows.h>
#include <string>
#include <string.h>
#include <winver.h>
#include <commctrl.h>
#include "constants.h"
#include "../../libraries/protocol.h"
#include "../../libraries/logging_class.h"
#include "../../libraries/doWinSock.h"
#include "../../libraries/string_ops.h"
#include "child_controls.h"

/* Global variables */
HINSTANCE gInstance;
HWND gWnd;
doWinSock winSockObj;

/*  Function prototypes  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
int SetStatus( std::string );
int Combo_Action_Selected( int );
int EnableSendFile();
int EnableGetRemoteProcs();
int DisableActionControls();
int Bttn_Get_Procs_Click();
std::string encryptBuffer( std::string );
std::string decryptBuffer( std::string );
int populateProcessListView( std::string );
int DisplayProcs( std::string );
int Bttn_Clear_Listview_Click();
int Bttn_Kill_Selected_Click();

/*  Make the class name into a global variable  */
char szClassName[ ] = "WindowsClient";

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_VREDRAW | CS_HREDRAW;
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon(wincl.hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
    wincl.hIconSm = LoadIcon(wincl.hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_WINDOW;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
               0,                   /* Extended possibilites for variation */
               szClassName,         /* Classname */
               "Client",       /* Title Text */
               WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, /* not resizable, not maximizable, can only minimize or close   */
               CW_USEDEFAULT,       /* Windows decides the position */
               CW_USEDEFAULT,       /* where the window ends up on the screen */
               HWND_WIDTH,                 /* The programs width */
               HWND_HEIGHT,                 /* and height in pixels */
               HWND_DESKTOP,        /* The window is a child-window to desktop */
               NULL,                /* No menu */
               hThisInstance,       /* Program Instance handler */
               NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

/*#########################################################################################################################

FUNCTION NAME: int BuildWindowControls()
AUTHOR: SCOTT MORRIS
CREATION DATE: 4/13/2017 10:17:49 PM
DESCRIPTION: Creates all the controls that will be used throughout the application.
RECEIVES: NOTHING
RETURNS: Integer indicating success
PRECONDITIONS: Application and primary window must have been created first.
POSTCONDITIONS: Form controls have been created.
CODE EXAMPLE: BuildWindowControls();
NOTES:

#########################################################################################################################*/
int BuildWindowControls()
{
    /* We're going to use the system font for all of our controls */
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    LABEL_IP_ADDR = CreateWindowEx(0,
                                   "STATIC",
                                   "IP:",
                                   WS_VISIBLE | WS_CHILD,
                                   5, 38,
                                   20, 18,
                                   gWnd,
                                   (HMENU)ID_IP_ADDR_LABEL,
                                   gInstance,
                                   NULL);

    // Tell this control to use system font settings.
    SendMessage(LABEL_IP_ADDR, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    TXT_IP_ADDR = CreateWindowEx(0,
                                 "EDIT",
                                 "10.0.10.32",
                                 WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                 30, 36,
                                 100, 18,
                                 gWnd,
                                 (HMENU)ID_IP_TXT,
                                 gInstance,
                                 NULL);

    // Tell this control to use system font settings.
    SendMessage(TXT_IP_ADDR, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    BTTN_CONNECT = CreateWindowEx(0,
                                  "BUTTON",
                                  "Connect",
                                  WS_VISIBLE | WS_CHILD,
                                  135, 30,
                                  127, 30,
                                  gWnd,
                                  (HMENU)ID_BTTN_CONNECT,
                                  gInstance,
                                  NULL);

    // Tell this control to use system font settings.
    SendMessage(BTTN_CONNECT, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    BTTN_DISCONNECT = CreateWindowEx(0,
                                     "BUTTON",
                                     "Disconnect",
                                     WS_VISIBLE | WS_CHILD | WS_DISABLED,
                                     265, 30,
                                     127, 30,
                                     gWnd,
                                     (HMENU)ID_BTTN_DISCONNECT,
                                     gInstance,
                                     NULL);

    // Tell this control to use system font settings.
    SendMessage(BTTN_DISCONNECT, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    LABEL_ACTION = CreateWindowEx(0,
                                  "STATIC",
                                  "Action:",
                                  WS_VISIBLE | WS_CHILD | WS_DISABLED,
                                  400, 38,
                                  40, 17,
                                  gWnd,
                                  (HMENU)ID_ACTION_LABEL,
                                  gInstance,
                                  NULL);
    // Tell this control to use system font settings.
    SendMessage(LABEL_ACTION, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    COMBOBOX_ACTION = CreateWindowEx(WS_EX_STATICEDGE,
                                     "COMBOBOX",
                                     "",
                                     CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VISIBLE | WS_CHILD | WS_DISABLED,
                                     445, 35,
                                     140, 18,
                                     gWnd,
                                     (HMENU)ID_ACTION_COMBOBOX,
                                     gInstance,
                                     NULL);

    SendMessage( COMBOBOX_ACTION, CB_ADDSTRING, 0, (LPARAM)"Remote Processes");
    SendMessage( COMBOBOX_ACTION, CB_ADDSTRING, 0, (LPARAM)"Remote System Info");
    SendMessage( COMBOBOX_ACTION, CB_ADDSTRING, 0, (LPARAM)"Execute Remote Command");
    SendMessage( COMBOBOX_ACTION, CB_ADDSTRING, 0, (LPARAM)"Send File");
    SendMessage( COMBOBOX_ACTION, CB_ADDSTRING, 0, (LPARAM)"Retrieve File");
    SendMessage( COMBOBOX_ACTION, CB_ADDSTRING, 0, (LPARAM)"Pranks");

    // Tell this control to use system font settings.
    SendMessage(COMBOBOX_ACTION, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    LABEL_PROCLIST = CreateWindowEx(0,
                                    "STATIC",
                                    "Process List:",
                                    WS_CHILD,
                                    5, 70,
                                    65, 17,
                                    gWnd,
                                    (HMENU)ID_PROCLIST_LABEL,
                                    gInstance,
                                    NULL);

    // Tell this control to use system font settings.
    SendMessage(LABEL_PROCLIST, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
    SendMessage(LABEL_PROCLIST,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);

    /* Here's where we initialize the process list listview control */
    InitCommonControls();

    LISTVIEW_PROCLIST = CreateWindow(WC_LISTVIEW,
                                     "",
                                     WS_CHILD | LVS_REPORT,
                                     5, 90,
                                     1019, 410,
                                     gWnd,
                                     (HMENU)ID_PROCLIST_LISTVIEW,
                                     gInstance,
                                     NULL);

    SendMessage(LISTVIEW_PROCLIST,LVM_SETEXTENDEDLISTVIEWSTYLE,
                0,LVS_EX_FULLROWSELECT);

    // Set up the column properties
    memset(&LvCol,0,sizeof(LvCol));                  // Zero Members
    LvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;  // Type of mask
    LvCol.cx = 0x28;                                 // width between each column
    LvCol.cx = COL_WIDTH;                            // width of column

    char item[BUFF_SIZE] = {'\0'};

    // Add the columns
    strcpy( item, "Process ID");
    LvCol.pszText = item;
    ListView_InsertColumn( LISTVIEW_PROCLIST, 0, &LvCol );

    strcpy( item, "EXE Name");
    LvCol.pszText = item;
    ListView_InsertColumn( LISTVIEW_PROCLIST, 1, &LvCol );

    // Tell this control to use system font settings.
    SendMessage(LISTVIEW_PROCLIST, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
    /* Process list listview control creation complete */

    BTTN_GET_PROCS = CreateWindowEx(0,
                                    "BUTTON",
                                    "Get Processes",
                                    WS_CHILD,
                                    5, 510,
                                    127, 30,
                                    gWnd,
                                    (HMENU)ID_BTTN_GET_PROCS,
                                    gInstance,
                                    NULL);

    // Tell this control to use system font settings.
    SendMessage(BTTN_GET_PROCS, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    BTTN_CLEAR_LISTVIEW = CreateWindowEx(0,
                                         "BUTTON",
                                         "Clear",
                                         WS_CHILD,
                                         135, 510,
                                         50, 30,
                                         gWnd,
                                         (HMENU)ID_BTTN_CLEAR_LISTVIEW,
                                         gInstance,
                                         NULL);

    // Tell this control to use system font settings.
    SendMessage(BTTN_CLEAR_LISTVIEW, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    STATUS_BAR = CreateWindowEx(0,
                                "msctls_statusbar32",
                                NULL,
                                WS_CHILD | WS_VISIBLE,
                                0,
                                0,
                                0,
                                0,
                                gWnd,
                                (HMENU)ID_STATUS_BAR,
                                gInstance,
                                NULL);
    int statwidths[] = {110, -1};
    SendMessage(STATUS_BAR, SB_SETPARTS, sizeof(statwidths)/sizeof(int), (LPARAM)statwidths);
    SendMessage(STATUS_BAR, SB_SETTEXT, 0, (LPARAM)"Connection Status:");
    SendDlgItemMessage(gWnd, ID_STATUS_BAR, SB_SETTEXT, 1, (LPARAM)"Not Connected");

    // Tell this control to use system font settings.
    SendMessage(BTTN_CLEAR_LISTVIEW, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    BTTN_KILL_SELECTED = CreateWindowEx(0,
                                         "BUTTON",
                                         "Kill Selected",
                                         WS_CHILD,
                                         188, 510,
                                         150, 30,
                                         gWnd,
                                         (HMENU)ID_KILL_SELECTED,
                                         gInstance,
                                         NULL);

    // Tell this control to use system font settings.
    SendMessage(BTTN_KILL_SELECTED, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    return 0;
}

/*#########################################################################################################################

FUNCTION NAME: int DisplayProcs(std::string rawProcList )
AUTHOR: SCOTT MORRIS
CREATION DATE: 4/13/2017 8:34:40 PM
DESCRIPTION: Populates a listview with a list of processes
RECEIVES: A delimited std::string containing the list of processes.
RETURNS: Integer - tells the status of the process
PRECONDITIONS:
	A global listview handle must exist with the variable name of LISTVIEW_PROCLIST.
	The constant FIELD_DELIM must be declared as a constant specifying what character is used to delimit the field values.
	The constant REC_DELIM must be declared with the character to use to delimit rows.
POSTCONDITIONS: The listview will ideally be populated with the processes.
CODE EXAMPLE:
	std::string processBuffer;
	processBuffer = functionThatBuildsTheProcessBuffer();
	DisplayProcs( processBuffer );
NOTES:

#########################################################################################################################*/
int DisplayProcs(std::string rawProcList )
{

    std::string currFunc = "int DisplayProcs(std::string rawProcList )";
    Log l;
    l.wr( currFunc, "Received buffer : ", rawProcList );

    if ( rawProcList == SOCK_FAIL )
    {
        return -2;
    }

    if( rawProcList.length() == 0 )
    {
        return -1;
    }

    std::string currRec = "";
    std::string currField = "";

    int xposprev = 0;
    int xlength = 0;
    std::string::size_type xpos = 0;
    int yposprev = 0;
    int ylength = 0;
    std::string::size_type ypos = 0;

    int itemPos = 0;
    int subItemPos = 0;

    char tInt[10] = { '\0' };

    char fieldVal[BUFF_SIZE] = { '\0' };

    // Set up the item properties
    memset(&LvItem,0,sizeof(LvItem)); // Zero struct's Members
    LvItem.mask=LVIF_TEXT;   // Text Style
    LvItem.cchTextMax = 256; // Max size of text
    LvItem.iItem=0;          // choose item

    do
    {
        currRec = "";
        xpos = rawProcList.find( REC_DELIM, xposprev );

        if( xpos != std::string::npos )
        {
            itoa( xpos, tInt, 10 );
            l.wr( currFunc, "Record delimiter found at position : ", tInt );
            xlength = xpos - xposprev;
        }
        else
        {
            l.wr( currFunc, "Record delimiter not found - grabbing last record" );
            xlength = rawProcList.length() - xposprev;
        }

        itoa( xlength, tInt, 10 );
        l.wr( currFunc, "Length of record is : ", tInt );

        currRec = rawProcList.substr( xposprev, xlength );
        l.wr( currFunc, "The current record is : ", currRec );
        xposprev = xpos + 1;

        do
        {

            currField = "";
            ypos = currRec.find( FIELD_DELIM, yposprev );

            if( ypos != std::string::npos )
            {
                itoa( ypos, tInt, 10 );
                l.wr( currFunc, "Field delimiter found at position : ", tInt );
                ylength = ypos - yposprev;
            }
            else
            {
                l.wr( currFunc, "Field delimiter not found - grabbing last field" );
                ylength = currRec.length() - yposprev;
            }

            itoa( ylength, tInt, 10 );
            l.wr( currFunc, "Length of field is : ", tInt );

            currField = currRec.substr( yposprev, ylength );
            l.wr( currFunc, "The current field is : ", currField );

            strcpy( fieldVal, currField.c_str() );

            itoa( subItemPos, tInt, 10 );
            l.wr( currFunc, "Sub item value is : ", tInt );
            itoa( itemPos, tInt, 10 );
            l.wr( currFunc, "Item value is : ", tInt );

            LvItem.iItem = itemPos;
            LvItem.iSubItem = subItemPos;       // Put in first column
            LvItem.pszText = fieldVal; // Text to display (can be from a char variable) (Items)

            if( currField.length() > 0 )
            {
                if ( subItemPos == 0 )
                {
                    ListView_InsertItem( LISTVIEW_PROCLIST, &LvItem );
                }
                else
                {
                    SendMessage( LISTVIEW_PROCLIST, LVM_SETITEM, 0, (LPARAM)&LvItem );
                }
            }

            subItemPos++;

            yposprev = ypos + 1;

        }
        while( ypos != std::string::npos );

        subItemPos = 0; // reset the subitem position to zero
        itemPos++; // go to the next row of the listview
    }
    while( xpos != std::string::npos );

    return 0;
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

/*
    function: SetStatus()
    receives: std::string containing the status to be placed into the text box
    requires: Text box with ID of ID_STATUS_TXT created
    preconditions: Text box says one thing
    postconditions: Text box says something else
    returns: integer
*/
int SetStatus( std::string szStatus )
{

    std::string currFunc = "SetStatus()";
    Log l;

    SendDlgItemMessage(gWnd, ID_STATUS_BAR, SB_SETTEXT, 1, (LPARAM)szStatus.c_str());
//    SetDlgItemText( gWnd, ID_STATUS_TXT, szStatus.c_str() );

    return 0;
}

/*
    function: Socket_Connect()
    requires: Log class, winSockObj object
    preconditions: instantiated winSockObj object
    postconditions: status on whether the socket connected or not
    returns: integer indicating status on whether the socket connected or not
*/
int SocketConnect( char* IPAddress )
{
    std::string currFunc = "SocketConnect()";
    Log l;

    int socketStatus = 0;
    std::string recvBuffer = "";
    std::string rcvMsg = "Received data: ";
    std::string outMsg = "";
    char tPortNum[20];

    l.wr( currFunc, "Attempting to initialize socket." );

    socketStatus = 0;
    socketStatus = winSockObj.initSocketSetup();
    if ( socketStatus != 0 )
    {
        l.wr( currFunc, "Unable to set up socket" );
        SetStatus( "Error setting up socket" );
        return socketStatus;
    }

    l.wr( currFunc, "Connection socket initialized" );

    l.wr( currFunc, "Attempting to connect" );

    socketStatus = winSockObj.initSocketConnect( IPAddress );

    if( socketStatus != 0 )
    {
        l.wr( currFunc, "Unable to create connection" );
        SetStatus( "Error creating connection" );
        return socketStatus;
    }

    itoa( CONN_PORT, tPortNum, 10 );
    outMsg.append( "Connected to " );
    outMsg.append( IPAddress );
    outMsg.append( ":" );
    outMsg.append( tPortNum );

    l.wr( currFunc, "Socket connected" );
    SetStatus( outMsg );

    /*
        If we got here, we can update the window with a visual status indicating
        a successful connection.
    */

    return 0;
}

/*
    function: Connect_Enable()
    requires: Log class
    receives: nothing
    preconditions: instantiated winSockObj object
    postconditions: buttons active/inactive based on socket status
    returns: nothing
*/
void Connect_Enable()
{

    std::string currFunc = "Connect_Enable";

    Log l;

    l.wr( currFunc, "Checking whether socket is connected" );


    if( winSockObj.isConnected == true )
    {
        // Disable any child controls that are controlled by the action combobox
        DisableActionControls();

        l.wr( currFunc, "Socket is connected" );
        EnableWindow( GetDlgItem( gWnd, ID_BTTN_DISCONNECT ), 1 );
        EnableWindow( GetDlgItem( gWnd, ID_BTTN_CONNECT ), 0 );
        EnableWindow( GetDlgItem( gWnd, ID_ACTION_LABEL ), 1 );
        EnableWindow( GetDlgItem( gWnd, ID_ACTION_COMBOBOX ), 1 );
        UpdateWindow( GetDlgItem( gWnd, ID_ACTION_COMBOBOX ) );

    }
    else
    {
        l.wr( currFunc, "Socket is not connected" );
    }

}

/*
    function: Disconnect_Enable()
    requires: Log class
    receives: nothing
    preconditions: instantiated winSockObj object
    postconditions: buttons active/inactive based on socket status
    returns: nothing
*/
void Disconnect_Enable()
{

    std::string currFunc = "Disconnect_Enable";

    Log l;

    l.wr( currFunc, "Checking whether socket is connected" );
    if( winSockObj.isConnected == false )
    {
        // Disable any child controls that are controlled by the action combobox
        DisableActionControls();

        l.wr( currFunc, "Socket is not connected" );
        EnableWindow( GetDlgItem( gWnd, ID_BTTN_DISCONNECT ), 0 );
        EnableWindow( GetDlgItem( gWnd, ID_BTTN_CONNECT ), 1 );
        EnableWindow( GetDlgItem( gWnd, ID_ACTION_LABEL ), 0 );
        EnableWindow( GetDlgItem( gWnd, ID_ACTION_COMBOBOX ), 0 );

    }
    else
    {
        l.wr( currFunc, "Socket is connected" );
    }

}

int Bttn_Kill_Selected_Click(){
    std::string currFunc = "Bttn_Kill_Selected_Click()";
    std::string preBuffer = "";
    std::string recvBuffer = "";
    Log l;

    l.wr( currFunc, "Clicked the button to kill a remote process" );

    preBuffer.append( KPROC );
    preBuffer.append( FIELD_DELIM );
    preBuffer.append( "Process id goes here" );
    l.wr( currFunc, "Sending data: ", preBuffer );
    winSockObj.sendData( encryptBuffer( preBuffer ) );
    l.wr( currFunc, "Data sent" );

    recvBuffer = winSockObj.recvData();
    l.wr( currFunc, "Received data - decrypting " );
    return 0;
}

/*#########################################################################################################################

FUNCTION NAME: int Bttn_Clear_Listview_Click()
AUTHOR: SCOTT MORRIS
CREATION DATE: 4/13/2017 10:14:55 PM
DESCRIPTION: Clears all items in the listview control identified by LISTVIEW_PROCLIST
RECEIVES: NOTHING
RETURNS: Integer indicating success
PRECONDITIONS: LISTVIEW_PROCLIST must be a valid handle to the listview control that has already been created.
POSTCONDITIONS: The items in the listview control have been cleared.
CODE EXAMPLE: Bttn_Clear_Listview_Click();
NOTES:

#########################################################################################################################*/
int Bttn_Clear_Listview_Click()
{

    std::string currFunc = "Bttn_Get_Procs_Click()";
    Log l;

    l.wr( currFunc, "Clicked the button to clear processes" );

    ListView_DeleteAllItems( LISTVIEW_PROCLIST );

    return 0;

}

/*#########################################################################################################################

FUNCTION NAME: int Bttn_Get_Procs_Click()
AUTHOR: SCOTT MORRIS
CREATION DATE: 4/14/2017 10:26:41 AM
DESCRIPTION: Function that executes when the "Get Processes" button is clicked.
RECEIVES: NOTHING
RETURNS: Integer indicating the success of the operation.
PRECONDITIONS:
	LPROC string constant defined as a command to send to the remote server.
	Log class defined.
	Winsock object created.
	Connection established to remote system.
	Encryption function defined.
	DisplayProcs function defined.
POSTCONDITIONS:
	A 'list processes' command is sent to the remote system.
	The list of processes, returned as a string is passed into the "DisplayProcs" function.
CODE EXAMPLE: Bttn_Get_Procs_Click();
NOTES:

#########################################################################################################################*/
int Bttn_Get_Procs_Click()
{

    std::string currFunc = "Bttn_Get_Procs_Click()";
    std::string preBuffer = "";
    std::string recvBuffer = "";
    Log l;

    l.wr( currFunc, "Clicked the button to get remote processes" );

    preBuffer.append( LPROC );

    l.wr( currFunc, "Sending data" );
    winSockObj.sendData( encryptBuffer( preBuffer ) );
    l.wr( currFunc, "Data sent" );

    recvBuffer = winSockObj.recvData();
    l.wr( currFunc, "Received data - decrypting" );

    DisplayProcs( decryptBuffer( recvBuffer ) );
    // need to set the socket to listen state

    return 0;
}

/*#########################################################################################################################

FUNCTION NAME: Bttn_Connect_Click()
AUTHOR: SCOTT MORRIS
CREATION DATE: 4/14/2017 10:29:39 AM
DESCRIPTION: A function that gets called when the "Connect" button is clicked.
RECEIVES: NOTHING
RETURNS: An integer indicating the success of the operation.
PRECONDITIONS:
	BUFF_SIZE integer constant defined.
	Log class defined.
	ID_IP_TXT initialized as a handle to the text box containing the IP address to connect to.
	Winsock object defined and initialized.
	'SocketConnect' function defined.
POSTCONDITIONS: A connection to the remote system has been established.
CODE EXAMPLE: Bttn_Connect_Click();
NOTES:

#########################################################################################################################*/
int Bttn_Connect_Click()
{
    std::string currFunc = "Bttn_Connect_Click()";
    char IPAddr[BUFF_SIZE];
    Log l;

    l.wr( currFunc, "'Connect' button clicked");

    /*
     * Grab the IP address out of the text box.
     */
    GetWindowText( GetDlgItem( gWnd, ID_IP_TXT ), IPAddr, BUFF_SIZE);

    return SocketConnect( IPAddr );
}

/*#########################################################################################################################

FUNCTION NAME: int Bttn_Disconnect_Click()
AUTHOR: SCOTT MORRIS
CREATION DATE: 4/14/2017 10:31:54 AM
DESCRIPTION: A function that gets called when the 'disconnect' button is clicked.
RECEIVES: NOTHING
RETURNS: An integer indicating the success of the operation.
PRECONDITIONS:
	Log class is defined.
	SOCK_RESET string constant defined as a command to send to the remote system.
	Winsock object initialized.
	Connection to the remote system is established.
POSTCONDITIONS: Connection to the remote system is closed.
CODE EXAMPLE: Bttn_Disconnect_Click();
NOTES:

#########################################################################################################################*/
int Bttn_Disconnect_Click()
{
    std::string currFunc = "Bttn_Disconnect_Click()";
    std::string preBuffer;
    Log l;
//    int socketStatus = 0;
    bool shutDownStatus = true;

    l.wr( currFunc, "'Disconnect' button clicked");

    // encrypt the buffer first

    preBuffer.append( SOCK_RESET );
    winSockObj.sendData( encryptBuffer( preBuffer ) );

    shutDownStatus = winSockObj.ShutdownConnection();

    SetStatus( "Not connected" );

    if( shutDownStatus )
    {
        l.wr( currFunc, "Socket shut down successfully" );
        return 0;
    }
    else
    {
        l.wr( currFunc, "Unable to shut down socket" );
        return -1;
    }

}

/*#########################################################################################################################

FUNCTION NAME: int DisableActionControls()
AUTHOR: SCOTT MORRIS
CREATION DATE: 4/14/2017 10:35:01 AM
DESCRIPTION:
	Hides all of the controls that are managed by the 'actions' drop-down box.Clears out the contents of the 'processes'
	listview box.
RECEIVES: NOTHING
RETURNS: An integer indicating the success of the operation.
PRECONDITIONS:
	Log class defined.
	Handles to all of the initialized controls that need to be de-activated.
POSTCONDITIONS: The controls have been hidden and cleared out as necessary.
CODE EXAMPLE: DisableActionControls();
NOTES:

#########################################################################################################################*/
int DisableActionControls()
{

    Log l;
    std::string currFunc = "DisableActionControls()";

    ShowWindow( GetDlgItem( gWnd, ID_BTTN_GET_PROCS ), 0 );
    ShowWindow( GetDlgItem( gWnd, ID_PROCLIST_LABEL ), 0 );
    ShowWindow( GetDlgItem( gWnd, ID_PROCLIST_LISTVIEW ), 0 );
    ShowWindow( GetDlgItem( gWnd, ID_BTTN_CLEAR_LISTVIEW ), 0 );
    ShowWindow( GetDlgItem( gWnd, ID_KILL_SELECTED ), 0 );
    Bttn_Clear_Listview_Click();

    return 0;

}

int EnableExecuteRemoteCommand()
{
    Log l;
    std::string currFunc = "EnableExecuteRemoteCommand()";

    DisableActionControls();

    return 0;
}

int EnableSendFile()
{
    Log l;
    std::string currFunc = "EnableSendFile()";

    DisableActionControls();

    return 0;
}

/*#########################################################################################################################

FUNCTION NAME: int EnableGetRemoteProcs()
AUTHOR: SCOTT MORRIS
CREATION DATE: 4/14/2017 10:38:36 AM
DESCRIPTION:
	Disables, hides, and clears out all of the controls in the form.  Re-enables only the ones related to operations de
	aling with remote processes.
RECEIVES: NOTHING
RETURNS: An integer indicating the success of the operation.
PRECONDITIONS:
	Log class defined.
	Handles to all related controls initialized.
POSTCONDITIONS: Only the controls related to operations dealing with remote processes are visible.
CODE EXAMPLE: EnableGetRemoteProcs();
NOTES:

#########################################################################################################################*/
int EnableGetRemoteProcs()
{
    Log l;
    std::string currFunc = "EnableGetRemoteProcs()";

    DisableActionControls();

    ShowWindow( GetDlgItem( gWnd, ID_BTTN_GET_PROCS ), 1 );
    ShowWindow( GetDlgItem( gWnd, ID_PROCLIST_LABEL ), 1 );
    ShowWindow( GetDlgItem( gWnd, ID_PROCLIST_LISTVIEW ), 1 );
    ShowWindow( GetDlgItem( gWnd, ID_BTTN_CLEAR_LISTVIEW ), 1 );
    ShowWindow( GetDlgItem( gWnd, ID_KILL_SELECTED ), 1 );

    return 0;
}

int Combo_Action_Selected( int selIndex )
{

    Log l;
    std::string currFunc = "Combo_Action_Selected";
    char tIdx[20];
    std::string msg = "Selected item: ";

    itoa( selIndex, tIdx, 10 );
    msg.append( tIdx );
    l.wr( currFunc, msg );

    switch( selIndex )
    {
    case SEL_IDX_REMOTE_PROCS:
        l.wr( currFunc, "User wants to see remote processes" );
        EnableGetRemoteProcs();
        break;
    case SEL_IDX_REMOTE_INFO:
        l.wr( currFunc, "User wants to gather remote system info" );
        break;
    case SEL_IDX_REMOTE_CMD:
        l.wr( currFunc, "User wants to execute remote command" );
        EnableExecuteRemoteCommand();
        break;
    case SEL_IDX_SEND_FILE:
        l.wr( currFunc, "User wants to send a file" );
        EnableSendFile();
        break;
    case SEL_IDX_RETRIEVE_FILE:
        l.wr( currFunc, "User wants to retrieve a file" );
        break;
    case SEL_IDX_PRANKS:
        l.wr( currFunc, "User wants to prank remote user" );
        break;
    }

    return 0;
}

/*
    function: CenterWindow()
    requires: Application window created
    preconditions: Application window created, not necessarily centered
    postconditions: Application window centered in the desktop
    returns: nothing
*/
void CenterWindow(HWND hWnd)
{
    RECT rect, rect2;
    GetWindowRect(GetDesktopWindow(), &rect);
    GetClientRect(hWnd, &rect2);
    int left, right, top, bottom;
    left = rect.right / 2 - rect2.right / 2;
    top  = rect.bottom / 2 - rect2.bottom / 2;
    right = rect2.right + (GetSystemMetrics(SM_CXFRAME) * 2);
    bottom = rect2.bottom + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXFRAME);
    SetWindowPos(hWnd, NULL, left, top, right, bottom, SWP_NOZORDER | SWP_NOACTIVATE);
}

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    gWnd = hwnd;
    Log l;

    std::string currFunc = "WindowProcedure";

    switch (message)                  /* handle the messages */
    {
    case WM_CREATE:
        CenterWindow(hwnd);
        BuildWindowControls();
        break;
    case WM_COMMAND:

        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);

        switch (wmId)
        {
        case ID_BTTN_CONNECT:
            if( Bttn_Connect_Click() == 0 )
            {
                Connect_Enable();
            }
            break;
        case ID_BTTN_DISCONNECT:
            Bttn_Disconnect_Click();
            Disconnect_Enable();
            break;
        case ID_ACTION_COMBOBOX:
            if( wmEvent == CBN_SELCHANGE )
            {
                l.wr( currFunc, "The selection changed in the action combobox" );

                Combo_Action_Selected( SendMessage( GetDlgItem( gWnd, ID_ACTION_COMBOBOX ), CB_GETCURSEL, 0, 0) );
            }
            break;
        case ID_BTTN_GET_PROCS:
            Bttn_Get_Procs_Click();
            break;
        case ID_BTTN_CLEAR_LISTVIEW:
            Bttn_Clear_Listview_Click();
            break;
        case ID_KILL_SELECTED:
            break;
        }

        break;
    case WM_DESTROY:
        PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
        break;
    default:                      /* for messages that we don't deal with */
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
