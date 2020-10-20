// CArcAPIClient.cpp : Defines the exported functions for the DLL application.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cmath>

#include <CArcAPIClient.h>
#include <ArcCltSrvStr.h>


#ifdef WIN32
	#include <windows.h>
	#include <strsafe.h>
	#include <ws2tcpip.h>
#else
	#include <cstdlib>
	#include <sys/socket.h>
	#include <netdb.h>
#endif

using namespace std;
using namespace arc;


// +-------------------------------------------------------------------------+
// | Macros                                                                  |
// +-------------------------------------------------------------------------+
#define CHECK_EXCEPTION_OCCURRED( s )	\
				if ( string( s ).find( ERROR_STRING ) != string::npos )	\
				{ throw runtime_error( string( s ).c_str() ); }


//// +---------------------------------------------------------------------------+
//// | ArrayDeleter                                                              |
//// +---------------------------------------------------------------------------+
//// | Called by std::shared_ptr to delete the temporary image buffer.           |
//// | This method should NEVER be called directly by the user.                  |
//// +---------------------------------------------------------------------------+
//void CArcAPIClient::ArrayDeleter( void* p )
//{
//	if ( p != NULL )
//	{
//		delete [] p;
//	}
//}


// +-------------------------------------------------------------------------+
// | Class Constructor                                                       |
// +-------------------------------------------------------------------------+
CArcAPIClient::CArcAPIClient()
{
#ifdef WIN32

	//  Initialize Winsock
	// +--------------------------------------------------------------+
	int dError = WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );

	if ( dError )
	{
		string msg =
			"( CArcAPIClient::CArcAPIClient ): WSAStartup failed!\n" +
			GetSystemMessage( GetError() );

		::MessageBoxA( NULL, msg.c_str(), "WSAStartup Error!", MB_OK );
	}

#endif

	m_hStdOutConsole	= INVALID_HANDLE_VALUE;
	m_tSocket			= INVALID_SOCKET;
	m_pHist				= NULL;
	m_bEndOfLine		= false;
	m_pDevList			= NULL;

	return;
}


// +-------------------------------------------------------------------------+
// | Class Destructor                                                        |
// +-------------------------------------------------------------------------+
CArcAPIClient::~CArcAPIClient()
{
//	FreeDeviceList();
//	FreeDebugConsole();
//	FreeHistogram();
	CloseConnections();

#ifdef WIN32
	WSACleanup();
#endif
}


//
// /////////////////////////////////////////////////////////////////////////////
// /  Server Connect Methods                                                   /
// /////////////////////////////////////////////////////////////////////////////
//

// +-------------------------------------------------------------------------+
// | Connect                                                                 |
// +-------------------------------------------------------------------------+
// | Connects to an ArcAPIService application.                               |
// |                                                                         |
// | Throws std::runtime_error                                               |
// |                                                                         |
// | <IN> -> pszIPAddr : The IP address of the computer running the          |
// |                     ArcAPIServer application. Of the form: xxx.xxx.xxx  |
// | <IN> -> u16Port   : The port to connect to. Default: ::DEFAULT_PORT     |
// +-------------------------------------------------------------------------+
void CArcAPIClient::Connect( const char* pszIPAddr, unsigned short u16Port )
{
	if ( m_tSocket != INVALID_SOCKET )
	{
		CloseConnections();
	}

	//  Create the socket
	// +---------------------------------------------------------------------+
	m_tSocket = socket( AF_INET,			// Go over TCP/IP
						SOCK_STREAM,		// This is a stream-oriented socket
						IPPROTO_TCP );		// Use TCP rather than UDP

	if ( m_tSocket == INVALID_SOCKET )
	{
		//  Save error code now because CloseConnections will change it!
		// +-----------------------------------------------------------------+
		int dError = GetError();

//		CloseConnections();

		ThrowException( "Connect",
						"socket() returned error: \n" +
						 GetSystemMessage( dError ) );
	}

	//  Fill a SOCKADDR_IN struct with address information
	// +---------------------------------------------------------------------+
	SOCKADDR_IN tServerInfo;

	tServerInfo.sin_family = AF_INET;
	tServerInfo.sin_port = htons( u16Port );		// Change to network-byte order and insert into port field

	auto retVal = inet_pton( tServerInfo.sin_family, pszIPAddr, &tServerInfo.sin_addr.s_addr );

	if ( retVal == 0 )
	{
		ThrowException( "Connect", "Invalid address format" );
	}

	else if ( retVal < 0 )
	{
		ThrowException( "Connect", "Failed to convert send address." );
	}

	//  Connect to the server
	// +---------------------------------------------------------------------+
	int nret = connect( m_tSocket,
					   ( LPSOCKADDR )&tServerInfo,
					   sizeof( struct sockaddr ) );

	if ( nret == SOCKET_ERROR )
	{
		//  Save error code now because CloseConnections will change it!
		// +-----------------------------------------------------------------+
		int dError = GetError();

		CloseConnections();

		ThrowException( "Connect",
						"connect() returned error: \n" +
						 GetSystemMessage( dError ) );
	}

//	cout << "Connected to: " << inet_ntoa( tServerInfo.sin_addr ) << std::endl;

	//  Call getnameinfo
	// +---------------------------------------------------------------------+
    char szHostName[ NI_MAXHOST ];
    char szServInfo[ NI_MAXSERV ];

	nret = getnameinfo( ( struct sockaddr * )&tServerInfo,
                        sizeof( struct sockaddr ),
                        szHostName,
                        NI_MAXHOST,
						szServInfo,
                        NI_MAXSERV,
						NI_NUMERICSERV );

	if ( nret == 0 )
	{
		cout << "HostName: " << szHostName
			 << " - Port Name: " << szServInfo
			 << std::endl; 
	}

	if ( m_tSocket == INVALID_SOCKET )
	{
		ostringstream oss;

		oss << "Failed to connect to server: " << pszIPAddr;

		ThrowException( "Connect", oss.str() );
	}
}


// +-------------------------------------------------------------------------+
// | CloseConnections                                                        |
// +-------------------------------------------------------------------------+
// | Closes all connections to the ArcAPIService application.                |
// |                                                                         |
// | Throws std::runtime_error                                               |
// +-------------------------------------------------------------------------+
void CArcAPIClient::CloseConnections()
{
	if ( m_tSocket == INVALID_SOCKET )
	{
		return;
	}

#ifdef WIN32

	WSAEVENT hEvent = WSACreateEvent();

	if ( hEvent != WSA_INVALID_EVENT )
	{
		int dRetVal = WSAEventSelect( m_tSocket,
									  hEvent,
									  FD_CLOSE );

		if ( dRetVal != SOCKET_ERROR )
		{
			WSANETWORKEVENTS ne;

			shutdown( m_tSocket, SD_SEND );

			DWORD dwSuccess = WSAWaitForMultipleEvents( 1,
														&hEvent,
														FALSE,
														1000, //WSA_INFINITE,
														FALSE );
			if ( dwSuccess != WSA_WAIT_FAILED )
			{
				WSAEnumNetworkEvents( m_tSocket, hEvent, &ne );

				if ( ne.lNetworkEvents & FD_CLOSE )
				{
					char szDumpBuf[ 256 ];
					int nret = 0;

					do {
						nret = recv( m_tSocket, szDumpBuf, 256, 0 );
					} while ( nret != 0 && nret != SOCKET_ERROR );
				}
			}
		}
	}

	closesocket( m_tSocket );

#else

	close( m_tSocket );

#endif

	m_tSocket = INVALID_SOCKET;
}


// +-------------------------------------------------------------------------+
// | SendInvalidCommand                                                      |
// +-------------------------------------------------------------------------+
// | Sends an invalid command to the server. For DEBUG ONLY.                 |
// |                                                                         |
// | Throws std::runtime_error                                               |
// |                                                                         |
// | <IN> -> pszCmd : The invalid command to send.                           |
// +-------------------------------------------------------------------------+
void CArcAPIClient::SendInvalidCommand( const char* pszCmd )
{
	Send( pszCmd );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}


// +-------------------------------------------------------------------------+
// | SetEndOfLine                                                            |
// +-------------------------------------------------------------------------+
// | Toggles the use of end-of-line characters.                              |
// |                                                                         |
// | <IN> -> bOnOff : 'true' to use EOL characters; 'false' otherwise.       |
// +-------------------------------------------------------------------------+
void CArcAPIClient::SetEndOfLine( bool bOnOff )
{
	m_bEndOfLine = bOnOff;
}


//
// /////////////////////////////////////////////////////////////////////////////
// /  General Server Methods                                                   /
// /////////////////////////////////////////////////////////////////////////////
//

// +-------------------------------------------------------------------------+
// | GetDirListing                                                           |
// +-------------------------------------------------------------------------+
// | Returns a directory listing from the server.                            |
// |                                                                         |
// | Throws std::runtime_error                                               |
// |                                                                         |
// | <IN>  -> sTargetDir    : The directory to retrieve the listing for.     |
// | <OUT> -> vecDirList    : The directory listing.                         |
// | <IN>  -> bSearchSubDir : 'true' to list sub-directories.                |
// +-------------------------------------------------------------------------+
void CArcAPIClient::GetDirListing( const std::string sTargetDir, vector<std::string>&
								   vecDirList, bool bSearchSubDir )
{
	Send( "%s::GetDirListing %s", CLASS_CArcAPIServer, sTargetDir.c_str() );

	CHECK_EXCEPTION_OCCURRED( Recv() );

	istringstream iss( Recv() );
	char szTemp[ ARC_MAX_PATH ];

	while ( iss.good() )
	{
		iss.getline( szTemp, ARC_MAX_PATH, '|' );

		if ( !string( szTemp ).empty() )
		{
			vecDirList.push_back( szTemp );
		}
	}
}


// +-------------------------------------------------------------------------+
// | LogMsgOnServer                                                          |
// +-------------------------------------------------------------------------+
// | Logs a message to std out on the server.                                |
// |                                                                         |
// | Throws std::runtime_error                                               |
// |                                                                         |
// | <IN> -> pszMsg : The message to output.                                 |
// +-------------------------------------------------------------------------+
void CArcAPIClient::LogMsgOnServer( const char* pszMsg )
{
	Send( "%s::%s %s",
		   CLASS_CArcAPIServer,
		   METHOD_LogMsgOnServer,
		   pszMsg );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}


// +-------------------------------------------------------------------------+
// | EnableServerLog                                                         |
// +-------------------------------------------------------------------------+
// | Enables logging on the server.                                          |
// |                                                                         |
// | Throws std::runtime_error                                               |
// |                                                                         |
// | <IN> -> bEnable : 'true' to enable logging; 'false' otherwise.          |
// +-------------------------------------------------------------------------+
void CArcAPIClient::EnableServerLog( bool bEnable )
{
	Send( "%s::%s %d",
		   CLASS_CArcAPIServer,
		   METHOD_EnableServerLog,
		   static_cast<int>( bEnable ) );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}


// +-------------------------------------------------------------------------+
// | IsServerLogging                                                         |
// +-------------------------------------------------------------------------+
// | Returns whether or not the server has logging enabled.  Returns 'true'  |
// | if server logging is enabled; 'false' otherwise.                        |
// |                                                                         |
// | Throws std::runtime_error                                               |
// +-------------------------------------------------------------------------+
bool CArcAPIClient::IsServerLogging()
{
	Send( "%s::%s",
		   CLASS_CArcAPIServer,
		   METHOD_IsServerLogging );

	return ( atoi( Recv().c_str() ) != 0 );
}


// +-------------------------------------------------------------------------+
// | GetServerVersion                                                        |
// +-------------------------------------------------------------------------+
// | Returns the server version number.                                      |
// |                                                                         |
// | Throws std::runtime_error                                               |
// +-------------------------------------------------------------------------+
double CArcAPIClient::GetServerVersion()
{
	double gVersion = 0.0;

	try
	{
		Send( "%s::%s",
			  CLASS_CArcAPIServer,
			  METHOD_GetServerVersion );

		gVersion = atof( Recv().c_str() );

		if ( fabs( gVersion ) == HUGE_VAL )
		{
			gVersion = 0.0;
		}
	}
	catch ( ... )
	{
		gVersion = 0.0;
	}

	return gVersion;
}


//
// /////////////////////////////////////////////////////////////////////////////
// /  Method Call Methods                                                      /
// /////////////////////////////////////////////////////////////////////////////
//
string CArcAPIClient::CallMethod( const string sClazz, const string sMethod, const string sFormat, ... )
{
	ostringstream oss;

	oss << sClazz << "::" << sMethod << " " << sFormat << ends;

	va_list tArgList;

	va_start( tArgList, sFormat );

	Send2( oss.str().c_str(), tArgList );

	va_end( tArgList );

	return Recv();
}


//
// /////////////////////////////////////////////////////////////////////////////
// /  CArcDevice Methods                                                       /
// /////////////////////////////////////////////////////////////////////////////
//

// +-------------------------------------------------------------------------+
// | ToString                                                                |
// +-------------------------------------------------------------------------+
// | Returns the device class ID string.                                     |
// |                                                                         |
// | Throws std::runtime_error                                               |
// +-------------------------------------------------------------------------+
const string CArcAPIClient::ToString()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_ToString );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return sRecv;
}




/********************
char** CArcAPIClient::GetDeviceList( int& dDeviceCount )
{
	const int dElemSize = 100;

	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetDeviceList );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	dDeviceCount = atoi( sRecv.c_str() );

	if ( dDeviceCount >  0 )
	{
		m_pDevList = new char*[ dDeviceCount ];

		if ( m_pDevList != NULL )
		{
			for ( int i=0; i<dDeviceCount; i++ )
			{
				m_pDevList[ i ] = new char[ dElemSize ];

				SendOK();

				sRecv = Recv();
				CHECK_EXCEPTION_OCCURRED( sRecv );

	#ifdef WIN32
				strcpy_s( m_pDevList[ i ], dElemSize, sRecv.c_str() );
	#else
				strcpy( m_pDevList[ i ], sRecv.c_str() );
	#endif
			}
		}
		else
		{
			ThrowException( METHOD_GetDeviceList,
							"Failed to allocate space for device list!" );
		}
	}

	return m_pDevList;
}

void CArcAPIClient::FreeDeviceList()
{
	if ( m_pDevList != NULL )
	{
		delete [] m_pDevList;

		m_pDevList = NULL;
	}
}

void CArcAPIClient::UseDevices( const char** pszDeviceList, int dListCount )
{
	if ( pszDeviceList != NULL )
	{
		Send( "%s::%s %d", CLASS_CArcDevice, METHOD_UseDevices, dListCount );

		for ( int i=0; i<dListCount; i++ )
		{
			Send( " %d", pszDeviceList[ i ] );		// ?????? %d ???????
		}

		CHECK_EXCEPTION_OCCURRED( Recv() );
	}
}

bool CArcAPIClient::IsOpen()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_IsOpen );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return ( atoi( sRecv.c_str() ) != 0 );
}

void CArcAPIClient::Open( const char* pszDevice )
{
	if ( pszDevice == NULL )
	{
		ThrowException( "Open", "Invalid Device Name: NULL!" );
	}

	string sDevice( pszDevice );

	PrepMultiStringData( sDevice );

	Send( "%s::%s %s", CLASS_CArcDevice, METHOD_Open, sDevice.c_str() );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::Open( const char* pszDevice, int dBytes )
{
	if ( pszDevice == NULL )
	{
		ThrowException( "Open", "Invalid Device Name: NULL!" );
	}

	string sDevice( pszDevice );

	PrepMultiStringData( sDevice );

	Send( "%s::%s %s %d",
		   CLASS_CArcDevice,
		   METHOD_Open,
		   sDevice.c_str(),
		   dBytes );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::Close()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_Close );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::Reset()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_Reset );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::MapCommonBuffer( int dBytes )
{
	Send( "%s::%s %d",
		  CLASS_CArcDevice,
		  METHOD_MapCommonBuffer,
		  dBytes );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::UnMapCommonBuffer()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_UnMapCommonBuffer );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::ReMapCommonBuffer( int dBytes )
{
	Send( "%s::%s %d", CLASS_CArcDevice, METHOD_ReMapCommonBuffer, dBytes );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

bool CArcAPIClient::GetCommonBufferProperties()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetCommonBufferProperties );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return ( atoi( sRecv.c_str() ) != 0 );
}

void CArcAPIClient::FillCommonBuffer( unsigned short u16Value )
{
	Send( "%s::%s %d", CLASS_CArcDevice, METHOD_FillCommonBuffer, u16Value );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void* CArcAPIClient::CommonBufferVA()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_CommonBufferVA );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return ( ( void * )strtoul( sRecv.c_str(), NULL, 16 ) );
}

arc::ulong CArcAPIClient::CommonBufferPA()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_CommonBufferPA );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return strtoul( sRecv.c_str(), NULL, 16 );
}

int CArcAPIClient::CommonBufferSize()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_CommonBufferSize );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

int CArcAPIClient::CommonBufferPixels( void* pBuffer, int dSize )
{
	if ( pBuffer == NULL )
	{
		ThrowException( "CommonBufferPixels",
						"Invalid data buffer ( NULL )!" );
	}

	Send( "%s::%s %d",
		   CLASS_CArcDevice,
		   METHOD_CommonBufferPixels,
		   dSize );

	return Recv( pBuffer, dSize );
}

int CArcAPIClient::GetId()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetId );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

int CArcAPIClient::GetStatus()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetStatus );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

void CArcAPIClient::ClearStatus()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_ClearStatus );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::Set2xFOTransmitter( bool bOnOff )
{
	Send( "%s::%s %d",
		   CLASS_CArcDevice,
		   METHOD_Set2xFOTransmitter,
		   static_cast<int>( bOnOff ) );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::LoadDeviceFile( const char* pszFile )
{
	Send( "%s::%s %d",
		   CLASS_CArcDevice,
		   METHOD_LoadDeviceFile,
		   GetFileLength( pszFile ) );

	CHECK_EXCEPTION_OCCURRED( Recv() );

	SendFile( pszFile );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

// +----------------------------------------------------------------------------
// |  GetCfgSpByte
// +----------------------------------------------------------------------------
// |  Returns the specified BYTE from the specified PCI configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI config space
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
int CArcAPIClient::GetCfgSpByte( int dOffset )
{
	Send( "%s::%s %d",
		   CLASS_CArcDevice,
		   METHOD_GetCfgSpByte,
		   dOffset );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

// +----------------------------------------------------------------------------
// |  GetCfgSpWord
// +----------------------------------------------------------------------------
// |  Returns the specified WORD from the specified PCI configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI config space
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
int CArcAPIClient::GetCfgSpWord( int dOffset )
{
	Send( "%s::%s %d",
		   CLASS_CArcDevice,
		   METHOD_GetCfgSpWord,
		   dOffset );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

// +----------------------------------------------------------------------------
// |  GetCfgSpDWord
// +----------------------------------------------------------------------------
// |  Returns the specified DWORD from the specified PCI configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI config space
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
int CArcAPIClient::GetCfgSpDWord( int dOffset )
{
	Send( "%s::%s %d",
		   CLASS_CArcDevice,
		   METHOD_GetCfgSpDWord,
		   dOffset );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

// +----------------------------------------------------------------------------
// |  SetCfgSpByte
// +----------------------------------------------------------------------------
// |  Writes the specified BYTE to the specified PCI configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI config space
// |  <IN> -> dVal    - The BYTE value to write
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
void CArcAPIClient::SetCfgSpByte( int dOffset, int dVal )
{
	Send( "%s::%s %d %d",
		   CLASS_CArcDevice,
		   METHOD_SetCfgSpByte,
		   dOffset,
		   dVal );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

// +----------------------------------------------------------------------------
// |  SetCfgSpWord
// +----------------------------------------------------------------------------
// |  Writes the specified WORD to the specified PCI configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI config space
// |  <IN> -> dVal    - The WORD value to write
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
void CArcAPIClient::SetCfgSpWord( int dOffset, int dVal )
{
	Send( "%s::%s %d %d",
		   CLASS_CArcDevice,
		   METHOD_SetCfgSpWord,
		   dOffset,
		   dVal );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

// +----------------------------------------------------------------------------
// |  SetCfgSpDWord
// +----------------------------------------------------------------------------
// |  Writes the specified DWORD to the specified PCI configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI config space
// |  <IN> -> dVal    - The DWORD value to write
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
void CArcAPIClient::SetCfgSpDWord( int dOffset, int dVal )
{
	Send( "%s::%s %d %d",
		   CLASS_CArcDevice,
		   METHOD_SetCfgSpDWord,
		   dOffset,
		   dVal );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

int CArcAPIClient::Command( int dBoardId, int dCommand, int dArg1, int dArg2, int dArg3, int dArg4 )
{
	Send( "%s::%s %d %d %d %d %d %d",
		   CLASS_CArcDevice, METHOD_Command, dBoardId,
		   dCommand, dArg1, dArg2, dArg3, dArg4 );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

int CArcAPIClient::GetControllerId()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetControllerId );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

void CArcAPIClient::ResetController()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_ResetController );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

bool CArcAPIClient::IsControllerConnected()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_IsControllerConnected );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return ( atoi( sRecv.c_str() ) != 0 );
}

void CArcAPIClient::SetupController( bool bReset, bool bTdl, bool bPower, int dRows, int dCols,
									 const char* pszTimFile, const char* pszUtilFile,
									 const char* pszPciFile )
{
	Send( "%s::%s %d %d %d %d %d %d %d %d",
		   CLASS_CArcDevice,
		   METHOD_SetupController,
		   bReset,
		   bTdl,
		   bPower,
		   dRows,
		   dCols,
		   ( pszTimFile  != NULL  ? 1 : 0 ),
		   ( pszUtilFile != NULL  ? 1 : 0 ),
		   ( pszPciFile  != NULL  ? 1 : 0 ) );

	string sServerMsg = "";

	while ( ( sServerMsg = Recv() ).find( "DONE" ) == string::npos )
	{
		CHECK_EXCEPTION_OCCURRED( sServerMsg );

		if ( sServerMsg.find( "TIM" ) != string::npos )
		{
			Send( "%d", GetFileLength( pszTimFile ) );

			CHECK_EXCEPTION_OCCURRED( Recv() );

			SendFile( pszTimFile );
		}

		else if ( sServerMsg.find( "UTIL" ) != string::npos )
		{
			Send( "%d", GetFileLength( pszUtilFile ) );

			CHECK_EXCEPTION_OCCURRED( Recv() );

			SendFile( pszUtilFile );
		}

		else if ( sServerMsg.find( "PCI" ) != string::npos )
		{
			Send( "%d", GetFileLength( pszPciFile ) );

			CHECK_EXCEPTION_OCCURRED( Recv() );

			SendFile( pszPciFile );
		}
	}

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::LoadControllerFile( const char* pszFileName, bool bValidate )
{
	Send( "%s::%s %d %d",
		   CLASS_CArcDevice,
		   METHOD_LoadControllerFile,
		   GetFileLength( pszFileName ),
		   static_cast<int>( bValidate ) );

	CHECK_EXCEPTION_OCCURRED( Recv() );

	SendFile( pszFileName );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::SetImageSize( int dRows, int dCols )
{
	Send( "%s::%s %d %d %d",
		   CLASS_CArcDevice,
		   METHOD_SetImageSize,
		   dRows,
		   dCols );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

int CArcAPIClient::GetImageRows()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetImageRows );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

int CArcAPIClient::GetImageCols()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetImageCols );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

int CArcAPIClient::GetCCParams()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetCCParams );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

bool CArcAPIClient::IsCCParamSupported( int dParameter )
{
	Send( "%s::%s %d", CLASS_CArcDevice, METHOD_IsCCParamSupported, dParameter );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return ( atoi( sRecv.c_str() ) != 0 );
}

bool CArcAPIClient::IsCCD()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_IsCCD );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return ( atoi( sRecv.c_str() ) != 0 );
}

bool CArcAPIClient::IsBinningSet()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_IsBinningSet );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return ( atoi( sRecv.c_str() ) != 0 );
}

void CArcAPIClient::SetBinning( int dRows, int dCols, int dRowFactor, int dColFactor, int* dBinRows, int* dBinCols )
{
	Send( "%s::%s %d %d %d %d",
		   CLASS_CArcDevice,
		   METHOD_SetBinning,
		   dRows,
		   dCols,
		   dRowFactor,
		   dColFactor );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	if ( dBinRows != NULL && dBinCols != NULL )
	{
		istringstream iss( sRecv );
		iss >> *dBinRows >> *dBinCols;
	}
}

void CArcAPIClient::UnSetBinning( int dRows, int dCols )
{
	Send( "%s::%s %d %d",
		   CLASS_CArcDevice,
		   METHOD_UnSetBinning,
		   dRows,
		   dCols );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::SetSubArray( int& dOldRows, int& dOldCols, int dRow, int dCol, int dSubRows, int dSubCols, int dBiasOffset, int dBiasWidth )
{
	Send( "%s::%s %d %d %d %d %d %d %d %d",
		   CLASS_CArcDevice,
		   METHOD_SetSubArray,
		   dOldRows,
		   dOldCols,
		   dRow,
		   dCol,
		   dSubRows,
		   dSubCols,
		   dBiasOffset,
		   dBiasWidth );

	string sRecvd = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecvd );

	istringstream iss( sRecvd );
	iss >> dOldRows >> dOldCols;
}

void CArcAPIClient::UnSetSubArray( int dRows, int dCols )
{
	Send( "%s::%s %d %d",
		   CLASS_CArcDevice, METHOD_UnSetSubArray,
		   dRows,
		   dCols );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

bool CArcAPIClient::IsSyntheticImageMode()
{
	Send( "%s::%s",
		   CLASS_CArcDevice,
		   METHOD_IsSyntheticImageMode );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return ( atoi( sRecv.c_str() ) != 0 );
}

void CArcAPIClient::SetSyntheticImageMode( bool bMode )
{
	Send( "%s::%s %d",
		   CLASS_CArcDevice,
		   METHOD_SetSyntheticImageMode,
		   static_cast<int>( bMode ) );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::SetOpenShutter( bool bShouldOpen )
{
	Send( "%s::%s %d",
		   CLASS_CArcDevice,
		   METHOD_SetOpenShutter,
		   static_cast<int>( bShouldOpen ) );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::Expose( float fExpTime, int dRows, int dCols, const bool& bAbort, CExpIFace* pExpIFace, bool bOpenShutter )
{
	string sRecv			= "";
	float  fElapsedTime		= fExpTime;
	bool   bInReadout		= false;
	int    dTimeoutCounter	= 0;
	int    dLastPixelCount	= 0;
	int    dPixelCount		= 0;
	int    dExposeCounter	= 0;

	//
	// Set the shutter position
	//
	Send( "%s::%s %d", CLASS_CArcDevice, METHOD_SetOpenShutter, bOpenShutter );
	CHECK_EXCEPTION_OCCURRED( Recv().c_str() );

	//
	// Set the exposure time
	//
	int dExpTime = ( int )( fExpTime * 1000.0 );

	Send( "%s::%s %d %d %d",
		   CLASS_CArcDevice,
		   METHOD_Command,
		   TIM_ID,
		   SET,
		   dExpTime );

	sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	int dRetVal = atoi( sRecv.c_str() );

	if ( dRetVal != DON )
	{
		ostringstream oss;

		oss << "Set exposure time failed. Reply: 0x"
			<< hex << dRetVal << dec << ends;

		ThrowException( METHOD_Expose, oss.str() );
	}

	//
	// Start the exposure
	//
	Send( "%s::%s %d %d",
		   CLASS_CArcDevice,
		   METHOD_Command,
		   TIM_ID,
		   SEX );

	sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	dRetVal = atoi( sRecv.c_str() );

	if ( dRetVal != DON )
	{
		ostringstream oss;

		oss << "Start exposure command failed. Reply: 0x"
			<< hex << dRetVal << dec << ends;

		ThrowException( METHOD_Expose, oss.str() );
	}

	while ( dPixelCount < ( dRows * dCols ) )
	{
		Send( "%s::%s", CLASS_CArcDevice, METHOD_IsReadout );

		sRecv = Recv();
		CHECK_EXCEPTION_OCCURRED( sRecv );

		bInReadout = ( atoi( sRecv.c_str() ) != 0 );

		// ----------------------------
		// READ ELAPSED EXPOSURE TIME
		// ----------------------------
		// Checking the elapsed time > 1 sec. is to prevent race conditions with
		// sending RET while the PCI board is going into readout. Added check
		// for exposure_time > 1 sec. to prevent RET error.
		if ( !bInReadout && fElapsedTime > 1.1f && dExposeCounter >= 5 && fExpTime > 1.0f )
		{
			// Read the elapsed exposure time.
			Send( "%s::%s %d %d", CLASS_CArcDevice, METHOD_Command, TIM_ID, RET );

			sRecv = Recv();
			CHECK_EXCEPTION_OCCURRED( sRecv );

			dRetVal = atoi( sRecv.c_str() );

			if ( ContainsError( dRetVal ) )
			{
				StopExposure();

				ThrowException( METHOD_Expose,
								"Failed to read elapsed time!" );
			}

			if ( bAbort )
			{
				StopExposure();
				ThrowException( METHOD_Expose, "Expose Aborted!" );
			}

			dExposeCounter  = 0;
			dTimeoutCounter = 0;
			fElapsedTime    = fExpTime - ( float )( dRetVal / 1000 );

			if ( pExpIFace != NULL )
			{
				pExpIFace->ExposeCallback( fElapsedTime );
			}
		}

		dExposeCounter++;

		// ----------------------------
		// READOUT PIXEL COUNT
		// ----------------------------
		if ( bAbort )
		{
			StopExposure();
			ThrowException( METHOD_Expose, "Expose Aborted!" );
		}

		// Save the last pixel count for use by the timeout counter.
		dLastPixelCount = dPixelCount;

		Send( "%s::%s", CLASS_CArcDevice, METHOD_GetPixelCount );

		sRecv = Recv();
		CHECK_EXCEPTION_OCCURRED( sRecv );

		dPixelCount = atoi( sRecv.c_str() );

		if ( ContainsError( dPixelCount ) )
		{
			StopExposure();
			ThrowException( METHOD_Expose,
							"Failed to read pixel count!" );
		}

		if ( bAbort )
		{
			StopExposure();
			ThrowException( METHOD_Expose, "Expose Aborted!" );
		}

		if ( pExpIFace != NULL )
		{
			pExpIFace->ReadCallback( dPixelCount );
		}

		if ( bAbort )
		{
			StopExposure();
			ThrowException( METHOD_Expose, "Expose Aborted!" );
		}

		// If the controller's in READOUT, then increment the timeout
		// counter. Checking for readout prevents timeouts when clearing
		// large and/or slow arrays.
		if ( dPixelCount == dLastPixelCount )
		{
			dTimeoutCounter++;
		}
		else
		{
			dTimeoutCounter = 0;
		}

		if ( bAbort )
		{
			StopExposure();
			ThrowException( METHOD_Expose, "Expose Aborted!" );
		}

		if ( dTimeoutCounter >= 200 ) //READ_TIMEOUT )
		{
			StopExposure();
			ThrowException( METHOD_Expose, "Read Timeout!" );
		}

		#ifdef WIN32
			Sleep( 25 );
		#else
			usleep( 2500 );
		#endif
	}
}

void CArcAPIClient::StopExposure()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_StopExposure );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::Continuous( int dRows, int dCols, int dNumOfFrames, float fExpTime, const bool& bAbort,
								CConIFace* pConIFace, bool bOpenShutter )
{
	int dImageSize			= dRows * dCols * sizeof( unsigned short );
	int dBoundedImageSize	= 0;
	int dFramesPerBuffer	= 0;
	int dPCIFrameCount		= 0;
	int dLastPCIFrameCount	= 0;
	int dFPBCount			= 0;
	int dRetVal				= 0;
	string sRecv			= "";

	void* pMapFd = CommonBufferVA();

	// Check for valid frame count
	if ( dNumOfFrames <= 0 )
	{
		ThrowException( METHOD_Continuous,
						"Number of frames must be > 0" );
	}

	if ( string( ToString() ).find( "PCIe" ) == string::npos && ( dImageSize & 0x3FF ) != 0 )
	{
		dBoundedImageSize = dImageSize - ( dImageSize & 0x3FF ) + 1024;
	}
	else
	{
		dBoundedImageSize = dImageSize;
	}

	if ( bAbort )
	{
		ThrowException( METHOD_Continuous,
						"Readout aborted by user!" );
	}

	int dMapFdBytes = CommonBufferSize();

	dFramesPerBuffer = ( int )floor( ( float )( dMapFdBytes / dBoundedImageSize ) );

	if ( bAbort )
	{
		ThrowException( METHOD_Continuous,
						"Readout aborted by user!" );
	}

	try
	{
		// Set the frames-per-buffer
		Send( "%s::%s %d %d %d",
			   CLASS_CArcDevice,
			   METHOD_Command,
			   TIM_ID,
			   FPB,
			   dFramesPerBuffer );

		sRecv = Recv();
		CHECK_EXCEPTION_OCCURRED( sRecv );

		dRetVal = atoi( sRecv.c_str() );

		if ( dRetVal != DON )
		{
			ostringstream oss;

			oss << "Failed to set the frames per buffer (FPB). Reply: 0x"
				<< hex << dRetVal << dec << ends;

			ThrowException( METHOD_Continuous, oss.str() );
		}

		if ( bAbort )
		{
			ThrowException( METHOD_Continuous,
							"Readout aborted by user!" );
		}

		// Set the number of frames-to-take
		Send( "%s::%s %d %d %d",
			   CLASS_CArcDevice, METHOD_Command,
			   TIM_ID,
			   SNF,
			   dNumOfFrames );

		sRecv = Recv();
		CHECK_EXCEPTION_OCCURRED( sRecv );

		dRetVal = atoi( sRecv.c_str() );

		if ( dRetVal != DON )
		{
			ostringstream oss;

			oss << "Failed to set the number of frames (SNF). Reply: 0x"
				<< hex << dRetVal << dec << ends;

			ThrowException( METHOD_Continuous, oss.str() );
		}

		if ( bAbort )
		{
			ThrowException( METHOD_Continuous,
							"Readout aborted by user!" );
		}

		//
		// Set the shutter position
		//
		Send( "%s::%s %d", CLASS_CArcDevice, METHOD_SetOpenShutter, bOpenShutter );
		CHECK_EXCEPTION_OCCURRED( Recv().c_str() );

		//
		// Set the exposure time
		//
		int dExpTime = ( int )( fExpTime * 1000.0 );

		Send( "%s::%s %d %d %d",
			   CLASS_CArcDevice, METHOD_Command,
			   TIM_ID,
			   SET,
			   dExpTime );

		sRecv = Recv();
		CHECK_EXCEPTION_OCCURRED( sRecv );

		dRetVal = atoi( sRecv.c_str() );

		if ( dRetVal != DON )
		{
			ostringstream oss;

			oss << "Set exposure time failed. Reply: 0x"
				<< hex << dRetVal << dec << ends;

			ThrowException( METHOD_Continuous, oss.str() );
		}

		//
		// Start the exposure
		//
		Send( "%s::%s %d %d",
			   CLASS_CArcDevice, METHOD_Command,
			   TIM_ID,
			   SEX );

		sRecv = Recv();
		CHECK_EXCEPTION_OCCURRED( sRecv );

		dRetVal = atoi( sRecv.c_str() );

		if ( dRetVal != DON )
		{
			ostringstream oss;

			oss << "Start exposure command failed. Reply: 0x"
				<< hex << dRetVal << dec << ends;

			ThrowException( METHOD_Continuous, oss.str() );
		}

		if ( bAbort )
		{
			ThrowException( METHOD_Continuous,
							"Readout aborted by user!" );
		}

		// Read the images
		while ( dPCIFrameCount < dNumOfFrames )
		{
			if ( bAbort )
			{
				ThrowException( METHOD_Continuous,
								"Readout aborted by user!" );
			}

			Send( "%s::%s", CLASS_CArcDevice, METHOD_GetFrameCount );

			sRecv = Recv();
			CHECK_EXCEPTION_OCCURRED( sRecv );

			dPCIFrameCount = atoi( sRecv.c_str() );

			if ( bAbort )
			{
				ThrowException( METHOD_Continuous,
								"Readout aborted by user!" );
			}

			if ( dFPBCount >= dFramesPerBuffer )
			{
					dFPBCount = 0;
			}

			if ( dPCIFrameCount > dLastPCIFrameCount )
			{
				// Call external deinterlace and fits file functions here
				if ( pConIFace != NULL )
				{
					pConIFace->FrameCallback(
									dFPBCount,
									dPCIFrameCount,
									dRows,
									dCols,
									( ( unsigned char * )pMapFd ) + dFPBCount * dBoundedImageSize );
				}

				dLastPCIFrameCount = dPCIFrameCount;
				dFPBCount++;
			}
		}

		// Set back to single image mode
		Send( "%s::%s %d %d %d",
			   CLASS_CArcDevice,
			   METHOD_Command,
			   TIM_ID,
			   SNF,
			   1 );

		sRecv = Recv();
		CHECK_EXCEPTION_OCCURRED( sRecv );

		dRetVal = atoi( sRecv.c_str() );

		if ( dRetVal != DON )
		{
			ostringstream oss;

			oss << "Failed to set number of frames (SNF) to 1. Reply: 0x"
				<< hex << dRetVal << dec << ends;

			ThrowException( METHOD_Continuous, oss.str() );
		}
	}
	catch ( std::runtime_error& e )
	{
		// Set back to single image mode
		StopExposure();

		Send( "%s::%s %d %d %d",
			   CLASS_CArcDevice,
			   METHOD_Command,
			   TIM_ID,
			   SNF,
			   1 );

		sRecv = Recv();
		CHECK_EXCEPTION_OCCURRED( sRecv );

		dRetVal = atoi( sRecv.c_str() );

		if ( dRetVal != DON )
		{
			ostringstream oss;

			oss << "Failed to set number of frames (SNF) to 1. Reply: 0x"
				<< hex << dRetVal << dec << ends;

			ThrowException( METHOD_Continuous, oss.str() );
		}

		throw e;
	}
}

void CArcAPIClient::StopContinuous()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_StopContinuous );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

bool CArcAPIClient::IsReadout()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_IsReadout );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return ( atoi( sRecv.c_str() ) != 0 );
}

int CArcAPIClient::GetPixelCount()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetPixelCount );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

int CArcAPIClient::GetCRPixelCount()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetCRPixelCount );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

int CArcAPIClient::GetFrameCount()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetFrameCount );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

double CArcAPIClient::GetArrayTemperature()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetArrayTemperature );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atof( sRecv.c_str() );
}

double CArcAPIClient::GetArrayTemperatureDN()
{
	Send( "%s::%s", CLASS_CArcDevice, METHOD_GetArrayTemperatureDN );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atof( sRecv.c_str() );
}

void CArcAPIClient::SetArrayTemperature( double gTempVal )
{
	Send( "%s::%s %f",
		   CLASS_CArcDevice,
		   METHOD_SetArrayTemperature,
		   gTempVal );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::LoadTemperatureCtrlData( const char* szFilename )
{
	Send( "%s::%s %s",
		   CLASS_CArcDevice,
		   METHOD_LoadTemperatureCtrlData,
		   szFilename );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::SaveTemperatureCtrlData( const char* szFilename )
{
	Send( "%s::%s %s",
		   CLASS_CArcDevice,
		   METHOD_SaveTemperatureCtrlData,
		   szFilename );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::SetLogCmds( bool bOnOff )
{
	Send( "%s::%s %d",
		   CLASS_CArcDevice,
		   METHOD_SetLogCmds,
		   static_cast<int>( bOnOff ) );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

const char* CArcAPIClient::GetNextLoggedCmd()
{
	Send( "%s::%s",
		   CLASS_CArcDevice,
		   METHOD_GetNextLoggedCmd );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	//
	// Scoping requires that string be copied to
	// class buffer before passing out of library.
	//
#ifdef WIN32
	CopyMemory( m_szTemp,
				sRecv.c_str(),
				sRecv.length() );
#else
	strcpy( m_szTemp, sRecv.c_str() );
#endif

	return m_szTemp;
}

int CArcAPIClient::GetLoggedCmdCount()
{
	Send( "%s::%s",
		   CLASS_CArcDevice,
		   METHOD_GetLoggedCmdCount );

	string sRecv = Recv();

	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

//
// /////////////////////////////////////////////////////////////////////////////
// /  CDeinterlace Methods                                                     /
// /////////////////////////////////////////////////////////////////////////////
//
void CArcAPIClient::Deinterlace( int dRows, int dCols, int dAlgorithm, int dArg, int dPixOffset )
{
	Send( "%s::%s %d %d %d %d %d",
		   CLASS_CDeinterlace,
		   METHOD_RunAlg,
		   dRows,
		   dCols,
		   dAlgorithm,
		   dArg,
		   dPixOffset );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::DeinterlaceFits( const char* szFitsFile, int dAlgorithm, int dArg )
{
	Send( "%s::%s %s %d %d",
		   CLASS_CDeinterlace,
		   METHOD_RunAlgFits,
		   szFitsFile,
		   dAlgorithm,
		   dArg );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::DeinterlaceFits3D( const char* szFitsFile, int dAlgorithm, int dArg )
{
	Send( "%s::%s %s %d %d",
		   CLASS_CDeinterlace,
		   METHOD_RunAlgFits3D,
		   szFitsFile,
		   dAlgorithm,
		   dArg );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

//
// /////////////////////////////////////////////////////////////////////////////
// /  CFitsFile Methods                                                        /
// /////////////////////////////////////////////////////////////////////////////
//
void CArcAPIClient::CFitsFile( const char *c_filename, int dMode )
{
	Send( "%s::%s %s %d",
		   CLASS_CFitsFile,
		   METHOD_CFitsFile,
		   c_filename,
		   dMode );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::CFitsFile( const char *c_filename, int dRows, int dCols, int dBitsPerPixel, bool bIs3D )
{
	Send( "%s::%s %s %d %d %d %d",
		   CLASS_CFitsFile,
		   METHOD_CFitsFile,
		   c_filename,
		   dRows,
		   dCols,
		   dBitsPerPixel,
		   static_cast<int>( bIs3D ) );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::CloseFits()
{
	Send( "%s::%s", CLASS_CFitsFile, METHOD_CloseFits );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

const std::string CArcAPIClient::GetFitsFilename()
{
	Send( "%s::%s", CLASS_CFitsFile, METHOD_GetFitsFilename );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	return sRecv;
}

// User MUST free the returned buffer!
char** CArcAPIClient::GetFitsHeader( int* pKeyCount )
{
	Send( "%s::%s", CLASS_CFitsFile, METHOD_GetFitsHeader );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	*pKeyCount = atoi( sRecv.c_str() );

	char** pHeader = NULL;
	int dHeaderSize = 100;

	if ( *pKeyCount >  0 )
	{
		pHeader = new char*[ *pKeyCount ];

		for ( int i=0; i<*pKeyCount; i++ )
		{
			pHeader[ i ] = new char[ dHeaderSize ];

			SendOK();
			sRecv = Recv();
			CHECK_EXCEPTION_OCCURRED( sRecv );

#ifdef WIN32
			strcpy_s( pHeader[ i ], dHeaderSize, sRecv.c_str() );
#else
			strcpy( pHeader[ i ], sRecv.c_str() );
#endif
		}
	}

	return pHeader;
}

void CArcAPIClient::WriteFitsKeyword( char* szKey, void* pKeyVal, int dValType, char* szComment )
{
	string sComment( "" );

	if ( szComment != NULL )
	{
		sComment = szComment;
		PrepMultiStringData( sComment );
	}

	switch ( dValType )
	{
		case CFitsFile::FITS_STRING_KEY:
		{
			string sKeyVal( ( char * )pKeyVal );
			PrepMultiStringData( sKeyVal );

			Send( "%s::%s %s %s %d %s",
				   CLASS_CFitsFile,
				   METHOD_WriteFitsKeyword,
				   szKey,
				   sKeyVal.c_str(),
				   dValType,
				   sComment.c_str() );
		}
		break;

		case CFitsFile::FITS_COMMENT_KEY:
		case CFitsFile::FITS_HISTORY_KEY:
		{
			string sKeyVal( ( char * )pKeyVal );
			PrepMultiStringData( sKeyVal );

			Send( "%s::%s NULL %s %d NULL",
				   CLASS_CFitsFile,
				   METHOD_WriteFitsKeyword,
				   sKeyVal.c_str(),
				   dValType );
		}
		break;

		case CFitsFile::FITS_DATE_KEY:
		{
			Send( "%s::%s NULL NULL %d NULL",
				   CLASS_CFitsFile,
				   METHOD_WriteFitsKeyword,
				   dValType );
		}
		break;

		case CFitsFile::FITS_INTEGER_KEY:
		{
			Send( "%s::%s %s %d %d %s",
				   CLASS_CFitsFile,
				   METHOD_WriteFitsKeyword,
				   szKey,
				   int( *( ( int * )pKeyVal ) ),
				   dValType,
				   sComment.c_str() );
		}
		break;

		case CFitsFile::FITS_LOGICAL_KEY:
		{
			Send( "%s::%s %s %d %d %s",
				   CLASS_CFitsFile,
				   METHOD_WriteFitsKeyword,
				   szKey,
				   static_cast<int>( *( ( bool * )pKeyVal ) ),
				   dValType,
				   sComment.c_str() );
		}
		break;

		case CFitsFile::FITS_DOUBLE_KEY:
		{
			Send( "%s::%s %s %f %d %s",
				   CLASS_CFitsFile,
				   METHOD_WriteFitsKeyword,
				   szKey,
				   double( *( ( double * )pKeyVal ) ),
				   dValType,
				   sComment.c_str() );
		}
		break;

		default:
		{
				ostringstream oss;

				oss << "Invalid FITS keyword type: "
					<< dValType;

				ThrowException( METHOD_WriteFitsKeyword, oss.str() );
		}
	}

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::UpdateFitsKeyword( char* szKey, void* pKeyVal, int dValType, char* szComment )
{
	string sComment( "" );

	if ( szComment != NULL )
	{
		sComment = szComment;
		PrepMultiStringData( sComment );
	}

	switch ( dValType )
	{
		case CFitsFile::FITS_STRING_KEY:
		{
			string sKeyVal( ( char * )pKeyVal );
			PrepMultiStringData( sKeyVal );

			Send( "%s::%s %s %s %d %s",
				   CLASS_CFitsFile,
				   METHOD_UpdateFitsKeyword,
				   szKey,
				   sKeyVal.c_str(),
				   dValType,
				   sComment.c_str() );
		}
		break;

		case CFitsFile::FITS_COMMENT_KEY:
		case CFitsFile::FITS_HISTORY_KEY:
		{
			string sKeyVal( ( char * )pKeyVal );
			PrepMultiStringData( sKeyVal );

			Send( "%s::%s NULL %s %d NULL",
				   CLASS_CFitsFile,
				   METHOD_UpdateFitsKeyword,
				   sKeyVal.c_str(),
				   dValType );
		}
		break;

		case CFitsFile::FITS_DATE_KEY:
		{
			Send( "%s::%s NULL NULL %d NULL",
				   CLASS_CFitsFile,
				   METHOD_UpdateFitsKeyword,
				   dValType );
		}
		break;

		case CFitsFile::FITS_INTEGER_KEY:
		{
			Send( "%s::%s %s %d %d %s",
				   CLASS_CFitsFile,
				   METHOD_UpdateFitsKeyword,
				   szKey,
				   int( *( ( int * )pKeyVal ) ),
				   dValType,
				   sComment.c_str() );
		}
		break;

		case CFitsFile::FITS_LOGICAL_KEY:
		{
			Send( "%s::%s %s %d %d %s",
				   CLASS_CFitsFile,
				   METHOD_UpdateFitsKeyword,
				   szKey,
				   static_cast<int>( *( ( bool * )pKeyVal ) ),
				   dValType,
				   sComment.c_str() );
		}
		break;

		case CFitsFile::FITS_DOUBLE_KEY:
		{
			Send( "%s::%s %s %f %d %s",
				   CLASS_CFitsFile,
				   METHOD_UpdateFitsKeyword,
				   szKey,
				   double( *( ( double * )pKeyVal ) ),
				   dValType,
				   sComment.c_str() );
		}
		break;

		default:
		{
				ostringstream oss;

				oss << "Invalid FITS keyword type: "
					<< dValType;

				ThrowException( METHOD_UpdateFitsKeyword, oss.str() );
		}
	}

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::GetFitsParameters( long* pNAxes, int* pNAxis, int* pBitsPerPixel )
{
	Send( "%s::%s", CLASS_CFitsFile, METHOD_GetFitsParameters );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	istringstream iss( sRecv );
	iss >> pNAxes[ 0 ] >> pNAxes[ 1 ] >> pNAxes[ 2 ];

	if ( pNAxis != NULL && iss.good() )
	{
		iss >> *pNAxis;
	}

	if ( pBitsPerPixel != NULL && iss.good() )
	{
		iss >> *pBitsPerPixel;
	}
}

void CArcAPIClient::GenerateFitsTestData()
{
	Send( "%s::%s", CLASS_CFitsFile, METHOD_GenerateFitsTestData );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::ReOpenFits()
{
	Send( "%s::%s", CLASS_CFitsFile, METHOD_ReOpenFits );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

bool CArcAPIClient::CompareFits( arc::CFitsFile& anotherCFitsFile )
{
	Send( "%s::%s", CLASS_CFitsFile, METHOD_CompareFits );

	string sRecvd = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecvd );

	return ( atoi( sRecvd.c_str() ) != 0 );
}

void CArcAPIClient::ResizeFits( int dRows, int dCols )
{
	Send( "%s::%s %d %d",
		   CLASS_CFitsFile,
		   METHOD_ResizeFits,
		   dRows,
		   dCols );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::WriteFits()
{
	Send( "%s::%s",
		   CLASS_CFitsFile,
		   METHOD_WriteFits );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::WriteFits( unsigned int udBytesToSkip, unsigned int udBytesToWrite, int dFPixl )
{
	Send( "%s::%s %d %d %d",
		   CLASS_CFitsFile,
		   METHOD_WriteFits,
		   udBytesToSkip,
		   udBytesToWrite,
		   dFPixl );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::WriteFitsSubImage( int dRowStart, int dColStart, int dRowEnd, int dColEnd, void* pData )
{
	Send( "%s::%s %d %d %d %d %l",
		   CLASS_CFitsFile,
		   METHOD_WriteFitsSubImage,
		   dRowStart,
		   dColStart,
		   dRowEnd,
		   dColEnd,
		   long( pData ) );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void* CArcAPIClient::ReadFitsSubImage( int llrow, int llcol, int urrow, int urcol )
{
	Send( "%s::%s", CLASS_CFitsFile, METHOD_ReadFitsSubImage );

	CHECK_EXCEPTION_OCCURRED( Recv() );

	return ( void * )NULL;
}

void* CArcAPIClient::ReadFits()
{
	Send( "%s::%s", CLASS_CFitsFile, METHOD_ReadFits );

	CHECK_EXCEPTION_OCCURRED( Recv() );

	return ( void * )NULL;
}

void CArcAPIClient::WriteFits3D( unsigned int udByteOffset )
{
	Send( "%s::%s %d",
		   CLASS_CFitsFile,
		   METHOD_WriteFits3D,
		   udByteOffset );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::ReWriteFits3D( unsigned int udByteOffset, int dImageNumber )
{
	Send( "%s::%s %d %d",
		   CLASS_CFitsFile,
		   METHOD_ReWriteFits3D,
		   udByteOffset,
		   dImageNumber );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void *CArcAPIClient::ReadFits3D( int dImageNumber )
{
	Send( "%s::%s", CLASS_CFitsFile, METHOD_ReadFits3D );

	CHECK_EXCEPTION_OCCURRED( Recv() );

	return ( void * )NULL;
}

//
// /////////////////////////////////////////////////////////////////////////////
// /  CTiffFile Methods                                                        /
// /////////////////////////////////////////////////////////////////////////////
//
void CArcAPIClient::CTiffFile( const char *c_filename, int dMode )
{
	Send( "%s::%s %s %d",
		   CLASS_CTiffFile,
		   METHOD_CTiffFile,
		   c_filename,
		   dMode );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::CloseTiff()
{
	Send( "%s::%s", CLASS_CTiffFile, METHOD_CloseTiff );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

int CArcAPIClient::GetTiffRows()
{
	Send( "%s::%s", CLASS_CTiffFile, METHOD_GetTiffRows );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

int CArcAPIClient::GetTiffCols()
{
	Send( "%s::%s", CLASS_CTiffFile, METHOD_GetTiffCols );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	return atoi( sRecv.c_str() );
}

void CArcAPIClient::WriteTiff( int dRows, int dCols )
{
	Send( "%s::%s %d %d",
		   CLASS_CTiffFile,
		   METHOD_WriteTiff,
		   dRows,
		   dCols );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void* CArcAPIClient::ReadTiff()
{
	Send( "%s::%s", CLASS_CTiffFile, METHOD_ReadTiff );

	CHECK_EXCEPTION_OCCURRED( Recv() );

	return ( void * )NULL;
}

//
// /////////////////////////////////////////////////////////////////////////////
// /  CImage Methods                                                           /
// /////////////////////////////////////////////////////////////////////////////
//
int* CArcAPIClient::Histogram( int& pHistSize, void *pMem, int dRow1, int dRow2,
							   int dCol1, int dCol2, int dRows, int dCols, int dBpp )
{
	Send( "%s::%s %l %d %d %d %d %d %d %d",
		   CLASS_CImage,
		   METHOD_Histogram,
		   long( pMem ),
		   dRow1,
		   dRow2,
		   dCol1,
		   dCol2,
		   dRows,
		   dCols,
		   dBpp );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	pHistSize = atoi( sRecv.c_str() );

	try
	{
		FreeHistogram();
		m_pHist = new int[ pHistSize ];
	}
	catch ( std::bad_alloc &ba )
	{
		ostringstream oss;

		oss << "Failed to allocate histogram buffer! "
			<< "Tried to allocate " << pHistSize
			<< " element buffer!" << std::endl << ba.what();

		ThrowException( METHOD_Histogram, oss.str() );
	}

	SendOK();

	Recv( m_pHist, ( pHistSize * sizeof( int ) ) );

	return m_pHist;
}

int* CArcAPIClient::Histogram( int& pHistSize, void *pMem, int dRows, int dCols, int dBpp )
{
	Send( "%s::%s %l %d %d %d",
		   CLASS_CImage,
		   METHOD_Histogram,
		   long( pMem ),
		   dRows,
		   dCols,
		   dBpp );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	pHistSize = atoi( sRecv.c_str() );

	try
	{
		FreeHistogram();
		m_pHist = new int[ pHistSize ];
	}
	catch ( std::bad_alloc &ba )
	{
		ostringstream oss;

		oss << "Failed to allocate histogram buffer! "
			<< "Tried to allocate " << pHistSize
			<< " element buffer!" << std::endl << ba.what();

		ThrowException( METHOD_Histogram, oss.str() );
	}

	SendOK();

	Recv( m_pHist, ( pHistSize * sizeof( int ) ) );

	return m_pHist;
}

void CArcAPIClient::FreeHistogram()
{
	if ( m_pHist != NULL )
	{
		delete [] m_pHist;
		m_pHist = NULL;
	}
}

// User MUST free the returned buffer!
void* CArcAPIClient::GetImageRow( int dRow, int dCol1, int dCol2, int dRows, int dCols, int& dElemCount )
{
	Send( "%s::%s %d %d %d %d %d",
		   CLASS_CImage,
		   METHOD_GetImageRow,
		   dRow,
		   dCol1,
		   dCol2,
		   dRows,
		   dCols );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	dElemCount = atoi( sRecv.c_str() );

	unsigned short* pRow = NULL;

	try
	{
		pRow = new unsigned short[ dElemCount ];
	}
	catch ( std::bad_alloc &ba )
	{
		ostringstream oss;

		oss << "Failed to allocate row buffer! "
			<< "Tried to allocate " << dElemCount
			<< " element buffer!" << std::endl << ba.what();

		ThrowException( METHOD_GetImageRow, oss.str() );
	}

	SendOK();

	Recv( pRow, ( dElemCount * sizeof( unsigned short ) ) );

	return ( void * )pRow;
}

// User MUST free the returned buffer!
void* CArcAPIClient::GetImageCol( int dCol, int dRow1, int dRow2, int dRows, int dCols, int& dElemCount )
{
	Send( "%s::%s %d %d %d %d %d",
		   CLASS_CImage,
		   METHOD_GetImageCol,
		   dCol,
		   dRow1,
		   dRow2,
		   dRows,
		   dCols );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	dElemCount = atoi( sRecv.c_str() );

	unsigned short* pCol = NULL;

	try
	{
		pCol = new unsigned short[ dElemCount ];
	}
	catch ( std::bad_alloc &ba )
	{
		ostringstream oss;

		oss << "Failed to allocate column buffer! "
			<< "Tried to allocate " << dElemCount
			<< " element buffer!" << std::endl << ba.what();

		ThrowException( METHOD_GetImageCol, oss.str() );
	}

	SendOK();

	Recv( pCol, ( dElemCount * sizeof( unsigned short ) ) );

	return ( void * )pCol;
}

// User MUST free the returned buffer!
void* CArcAPIClient::GetImageRowArea( int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int& dElemCount )
{
	Send( "%s::%s %d %d %d %d %d %d",
		   CLASS_CImage,
		   METHOD_GetImageRowArea,
		   dRow1,
		   dRow2,
		   dCol1,
		   dCol2,
		   dRows,
		   dCols );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	dElemCount = atoi( sRecv.c_str() );

	float* pRows = NULL;

	try
	{
		pRows = new float[ dElemCount ];
	}
	catch ( std::bad_alloc &ba )
	{
		ostringstream oss;

		oss << "Failed to allocate row buffer! "
			<< "Tried to allocate " << dElemCount
			<< " element buffer!" << std::endl << ba.what();

		ThrowException( METHOD_GetImageRowArea, oss.str() );
	}

	SendOK();

	Recv( pRows, ( dElemCount * sizeof( float ) ) );

	return ( void * )pRows;
}

// User MUST free the returned buffer!
void* CArcAPIClient::GetImageColArea( int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int& dElemCount )
{
	Send( "%s::%s %d %d %d %d %d %d",
		   CLASS_CImage,
		   METHOD_GetImageColArea,
		   dRow1,
		   dRow2,
		   dCol1,
		   dCol2,
		   dRows,
		   dCols );

	string sRecv = Recv();
	CHECK_EXCEPTION_OCCURRED( sRecv );

	dElemCount = atoi( sRecv.c_str() );

	float* pCols = NULL;

	try
	{
		pCols = new float[ dElemCount ];
	}
	catch ( std::bad_alloc &ba )
	{
		ostringstream oss;

		oss << "Failed to allocate column buffer! "
			<< "Tried to allocate " << dElemCount
			<< " element buffer!" << std::endl << ba.what();

		ThrowException( METHOD_GetImageColArea, oss.str() );
	}

	SendOK();

	Recv( pCols, ( dElemCount * sizeof( float ) ) );

	return ( void * )pCols;
}

void CArcAPIClient::FreeImageData( void* pBuf, int dTypeSize )
{
	if ( pBuf != NULL )
	{
		if ( dTypeSize == sizeof( unsigned short ) )
		{
			delete [] ( ( unsigned short * )pBuf );
		}

		else if ( dTypeSize == sizeof( float ) )
		{
			delete [] ( ( float * )pBuf );
		}
	}
}

void CArcAPIClient::SubtractImageHalves( int dRows, int dCols )
{
	Send( "%s::%s %d %d",
		   CLASS_CImage,
		   METHOD_SubtractImageHalves,
		   dRows,
		   dCols );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

void CArcAPIClient::VerifyImageAsSynthetic( int dRows, int dCols )
{
	Send( "%s::%s %d %d",
		   CLASS_CImage,
		   METHOD_VerifyImageAsSynthetic,
		   dRows,
		   dCols );

	CHECK_EXCEPTION_OCCURRED( Recv() );
}

CImage::CImgStats CArcAPIClient::GetStats( void* pMem, int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int dBpp )
{
	Send( "%s::%s %p %d %d %d %d %d %d %d",
		   CLASS_CImage,
		   METHOD_GetStats,
		   ULONG_PTR( pMem ),
		   dRow1,
		   dRow2,
		   dCol1,
		   dCol2,
		   dRows,
		   dCols,
		   dBpp );

	CImage::CImgStats cImgStats;
	Recv( &cImgStats, sizeof( CImage::CImgStats ) );

	return cImgStats;
}

CImage::CImgStats CArcAPIClient::GetStats( void* pMem, int dRows, int dCols, int dBpp )
{
	Send( "%s::%s %p %d %d %d",
		   CLASS_CImage,
		   METHOD_GetStats,
		   ULONG_PTR( pMem ),
		   dRows,
		   dCols,
		   dBpp );

	CImage::CImgStats cImgStats;
	Recv( &cImgStats, sizeof( CImage::CImgStats ) );

	return cImgStats;
}

CImage::CImgStats CArcAPIClient::GetStats( const char* szFitsFile, int dRow1, int dRow2, int dCol1, int dCol2 )
{
	Send( "%s::%s %s %d %d %d %d",
		   CLASS_CImage,
		   METHOD_GetFitsStats,
		   szFitsFile,
		   dRow1,
		   dRow2,
		   dCol1,
		   dCol2 );

	CImage::CImgStats cImgStats;
	Recv( &cImgStats, sizeof( CImage::CImgStats ) );

	return cImgStats;
}

CImage::CImgDifStats CArcAPIClient::GetDiffStats( void *pMem1, void *pMem2, int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int dBpp )
{
	Send( "%s::%s %l %l %d %d %d %d %d %d %d",
		   CLASS_CImage,
		   METHOD_GetDiffStats,
		   long( pMem1 ),
		   long( pMem2 ),
		   dRow1,
		   dRow2,
		   dCol1,
		   dCol2,
		   dRows,
		   dCols,
		   dBpp );

	CImage::CImgDifStats cImgDifStats;
	Recv( &cImgDifStats, sizeof( CImage::CImgDifStats ) );

	return cImgDifStats;
}

CImage::CImgDifStats CArcAPIClient::GetDiffStats( void *pMem1, void *pMem2, int dRows, int dCols, int dBpp )
{
	Send( "%s::%s %l %l %d %d %d",
		   CLASS_CImage,
		   METHOD_GetDiffStats,
		   long( pMem1 ),
		   long( pMem2 ),
		   dRows,
		   dCols,
		   dBpp );

	CImage::CImgDifStats cImgDifStats;
	Recv( &cImgDifStats, sizeof( CImage::CImgDifStats ) );

	return cImgDifStats;
}

CImage::CImgDifStats CArcAPIClient::GetDiffStats( const char* szFitsFile1, const char* szFitsFile2, int dRow1,
												  int dRow2, int dCol1, int dCol2 )
{
	Send( "%s::%s %s %s %d %d %d %d",
		   CLASS_CImage,
		   METHOD_GetFitsDiffStats,
		   szFitsFile1,
		   szFitsFile2,
		   dRow1,
		   dRow2,
		   dCol1,
		   dCol2 );

	CImage::CImgDifStats cImgDifStats;
	Recv( &cImgDifStats, sizeof( CImage::CImgDifStats ) );

	return cImgDifStats;
}

//
// /////////////////////////////////////////////////////////////////////////////
// /  Debug Console Methods                                                    /
// /////////////////////////////////////////////////////////////////////////////
//
void CArcAPIClient::ShowDebugConsole()
{
#ifdef WIN32
	if ( m_hStdOutConsole == INVALID_HANDLE_VALUE )
	{
		BOOL bOk = AllocConsole();

		if ( !bOk )
		{
			::MessageBox( NULL,
						  TEXT( "Failed to allocate a console window" ),
						  TEXT( "" ),
						  MB_OK );

			return;
		}

		m_hStdOutConsole = GetStdHandle( STD_OUTPUT_HANDLE );

	   TCHAR szOldTitle[ MAX_PATH ];
	   TCHAR szNewTitle[ MAX_PATH ];

		if( GetConsoleTitle( szOldTitle, MAX_PATH ) )
		{
			StringCchPrintf( szNewTitle,
							 MAX_PATH,
							 TEXT( "CameraAPIClient Debug: %s" ),
							 szOldTitle );

			SetConsoleTitle( szNewTitle );
		}

		SetConsoleTextAttribute( m_hStdOutConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY );
		//BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED
	}
#endif
}

void CArcAPIClient::WriteToConsole( const string sMsg )
{
#ifdef WIN32
	if ( m_hStdOutConsole != INVALID_HANDLE_VALUE )
	{
		DWORD dwNumberOfCharsWritten = 0;

		BOOL bOk = WriteConsoleA( m_hStdOutConsole,
								  ( const VOID * )sMsg.c_str(),
								  sMsg.length(),
								  &dwNumberOfCharsWritten,
								  NULL );

		if ( !bOk )
		{
				::MessageBox( NULL,
							  TEXT( "Failed to write to console window" ),
							  TEXT( "" ),
							  MB_OK );
		}
	}
#endif
}

void CArcAPIClient::FreeDebugConsole()
{
#ifdef WIN32
	if ( m_hStdOutConsole != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hStdOutConsole );
		FreeConsole();

		m_hStdOutConsole = INVALID_HANDLE_VALUE;
	}
#endif
}

**************************/


//
// /////////////////////////////////////////////////////////////////////////////
// /  Send/Receive Methods                                                     /
// /////////////////////////////////////////////////////////////////////////////
//
void CArcAPIClient::PrepMultiStringData( string& sData )
{
	size_t tPos = 0;

	for ( size_t i=0; i<sData.size(); i++ )
	{
		if ( ( tPos = sData.find( " ", tPos ) ) != string::npos )
		sData = sData.replace( tPos, 1, "+-+" );
	}
}

int CArcAPIClient::Recv( void* pBuffer, int dSize )
{
	char* pBuf = ( char * )pBuffer;
	ZeroMem( pBuf, dSize );

	//StartTimer();

	int nret = recv( m_tSocket,
					 pBuf,
					 dSize,				// Complete size of buffer
					 MSG_WAITALL );

	//KillTimer( NULL, m_Timer );

	if ( nret == SOCKET_ERROR )
	{
#ifdef WIN32
		DWORD dwError = GetError();

		// Check that the MSG_WAITALL flag is supported!
		if ( dwError == WSAEOPNOTSUPP )
		{
			nret = recv( m_tSocket,
						 pBuf,
						 dSize,				// Complete size of buffer
						 0 );

			if ( nret == SOCKET_ERROR )
			{
				ThrowException( "Recv",
								"recv() returned error: " +
								 GetSystemMessage( GetError() ) );
			}
		}
		else
		{
			ThrowException( "Recv",
							"recv() returned error: " +
							 GetSystemMessage( GetError() ) );
		}
#else
			ThrowException( "Recv",
							"recv() returned error: " +
							 GetSystemMessage( GetError() ) );
#endif
	}

	if ( nret != dSize )
	{
		ostringstream oss;

		oss << "Insufficient data transfer. Expected: "
			<< dSize << " bytes.  Received: " << nret
			<< " bytes.";

		ThrowException( "Recv", oss.str() );
	}

	return nret;
}

string CArcAPIClient::Recv()
{
	char *buffer = new char[ 1024 ];
	ZeroMem( buffer, 1024 );

	int nret = recv( m_tSocket,
					 buffer,
					 1024,		// Complete size of buffer
					 0 );

	string sBuffer( buffer );

	delete [] buffer;

	if ( nret == SOCKET_ERROR )
	{
		ThrowException( "Recv",
						"recv() returned error: " +
						 GetSystemMessage( GetError() ) );
	}

	if ( sBuffer.find( ERROR_STRING ) != string::npos )
	{
		ThrowException( "Recv", sBuffer );
	}

	return sBuffer;
}

int CArcAPIClient::BytesAvailable()
{
	char *szBuffer = new char[ 1024 ];

	return recv( m_tSocket,
				 szBuffer,
				 1024,		// Complete size of buffer
				 MSG_PEEK );
}

int CArcAPIClient::GetFileLength( string sFilename )
{
	ifstream ifs;
	ifs.open ( sFilename.c_str(), ios::binary );

	ifs.seekg( 0, ios::end );
	std::streamoff dLength = ifs.tellg();
	ifs.close();

	return static_cast<int>( dLength );
}

void CArcAPIClient::SendFile( string sFilename )
{
	ifstream ifs;
	ifs.open ( sFilename.c_str(), ios::binary );

	// get length of file:
	ifs.seekg( 0, ios::end );
	std::streamoff dLength = ifs.tellg();
	ifs.seekg( 0, ios::beg );

	// allocate memory:
	char* pBuffer = new char [ static_cast<int>( dLength ) ];

	// read data as a block:
	ifs.read( pBuffer, dLength );
	ifs.close();

	int nret = send( m_tSocket,
					 pBuffer,
					 static_cast<int>( dLength ),
					 0 );						// Most often is zero, but see MSDN for other options

	if ( nret == SOCKET_ERROR )
	{
		ThrowException( "SendFile",
						"send() returned error: " +
						 GetSystemMessage( GetError() ) );
	}

	delete [] pBuffer;
}

// +----------------------------------------------------------------------------
// |  Send
// +----------------------------------------------------------------------------
// |  Inserts a message into the log queue. It dumps the oldest message if
// |  the queue size is greater than or equal to Q_MAX.
// |
// |  <IN> -> fmt - C-printf style format (sort of):
// |				%d   -> Integer
// |				%f   -> Double
// |				%l   -> Long
// |				%s   -> Char *, std::string not accepted
// |				%x,X -> Integer as hex, produces uppercase only
// |				%e   -> Special case that produces a string message from
// |				        one of the system functions ::GetLastError or
// |				        strerror, depending on the OS.
// +----------------------------------------------------------------------------
void CArcAPIClient::Send( const char *fmt, ... )
{
	ostringstream oss;
	char *ch, *szVal;
	va_list ap;

	va_start( ap, fmt );

	for ( ch = ( char * )fmt; *ch; ch++ )
	{
		if ( *ch != '%' )
		{
			oss << *ch;
			continue;
		}

		switch ( *++ch )
		{
			case 'd':
				oss << va_arg( ap, int );
				break;

			case 'f':
				oss << va_arg( ap, double );
				break;

			case 'l':
				oss << va_arg( ap, long );
				break;

			case 's':
				for ( szVal = va_arg( ap, char * ); *szVal; szVal++ )
				{
					oss << *szVal;
				}
				break;

			case 'e':
				oss << GetSystemMessage( va_arg( ap, long ) );
				break;

			case 'X':
			case 'x':
				oss << uppercase << hex << va_arg( ap, int ) << dec;
				break;

			case 'p':
				oss << va_arg( ap, ULONG_PTR );
				break;

			default:
				oss << *ch;
				break;
		}
	}
	va_end( ap );

	if ( m_bEndOfLine )
	{
		oss << '\r' << '\n';
	}

	int nret = send( m_tSocket,
					 oss.str().c_str(),
					 oss.str().size(),
					 0 );

	if ( nret == SOCKET_ERROR )
	{
		// Save error code now because CloseConnections will change it!
		int dError = GetError();

		if ( dError == 10054 )
		{
			CloseConnections();
		}

		ThrowException( "Send",
						"send() returned error: " +
						 GetSystemMessage( dError ) );
	}
}

void CArcAPIClient::Send2( const char *fmt, va_list tArgList )
{
	ostringstream oss;
	char *ch, *szVal;

	//va_start( tArgList, fmt );

	for ( ch = ( char * )fmt; *ch; ch++ )
	{
		if ( *ch != '%' )
		{
			oss << *ch;
			continue;
		}

		switch ( *++ch )
		{
			case 'd':
				oss << va_arg( tArgList, int );
				break;

			case 'f':
				oss << va_arg( tArgList, double );
				break;

			case 'l':
				oss << va_arg( tArgList, long );
				break;

			case 's':
				for ( szVal = va_arg( tArgList, char * ); *szVal; szVal++ )
				{
					oss << *szVal;
				}
				break;

			case 'e':
				oss << GetSystemMessage( va_arg( tArgList, long ) );
				break;

			case 'X':
			case 'x':
				oss << uppercase << hex << va_arg( tArgList, int ) << dec;
				break;

			case 'p':
				oss << va_arg( tArgList, ULONG_PTR );
				break;

			default:
				oss << *ch;
				break;
		}
	}

//	va_end( tArgList );

	if ( m_bEndOfLine )
	{
		oss << '\r' << '\n';
	}

	cout  << "Sending -> " << oss.str() << " size -> " << oss.str().size() << std::endl;

	int nret = send( m_tSocket,
					 oss.str().c_str(),
					 oss.str().size(),
					 0 );

	if ( nret == SOCKET_ERROR )
	{
		// Save error code now because CloseConnections will change it!
		int dError = GetError();

		if ( dError == 10054 )
		{
			CloseConnections();
		}

		ThrowException( "Send",
						"send() returned error: " +
						 GetSystemMessage( dError ) );
	}
}

void CArcAPIClient::SendOK()
{
	Send( CLIENT_OK_STRING );
}

// +----------------------------------------------------------------------------
// |  Check the specified value for error replies ( TOUT, ERR, SYR, RST )
// +----------------------------------------------------------------------------
// |  Returns 'true' if the specified value matches 'TOUT, 'ERR', 'SYR' or 'RST'.
// |  Returns 'false' otherwise.
// +----------------------------------------------------------------------------
bool CArcAPIClient::ContainsError( int dWord )
{
	bool bContainsError = false;

	if ( dWord == TOUT || dWord == ERR || dWord == SYR || dWord == RST )
	{
		bContainsError = true;
	}

	return bContainsError;
}

// +----------------------------------------------------------------------------
// |  GetError
// +----------------------------------------------------------------------------
// |  Returns the last error that occured. Uses ::WSAGetLastError on windows
// |  and errno on linux/solaris.
// +----------------------------------------------------------------------------
int CArcAPIClient::GetError()
{
#ifdef WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

// +----------------------------------------------------------------------------
// |  ZeroMem
// +----------------------------------------------------------------------------
// |  Zero out the specified buffer. Uses ::ZeroMemory on windows and memset
// |  on linux/solaris.
// |
// |  <IN> -> pBuf  - The buffer to clear/set to zero.
// |  <IN> -> dSize - The size of the buffer in bytes.
// +----------------------------------------------------------------------------
void CArcAPIClient::ZeroMem( void* pBuf, int dSize )
{
#ifdef WIN32
	ZeroMemory( pBuf, dSize );
#else
	memset( pBuf, 0, dSize );
#endif
}

// +----------------------------------------------------------------------------
// |  GetSystemMessage
// +----------------------------------------------------------------------------
// |  Internal method that converts a system error code into a string. Uses
// |  ::GetLastError on windows and strerror on linux/solaris.
// |
// |  <IN>  -> dCode - The system dependent error code, ::GetLastError or errno
// |  <OUT> -> The error code message
// +----------------------------------------------------------------------------
string CArcAPIClient::GetSystemMessage( int dCode )
{
	ostringstream oss;

#ifdef WIN32

	char szBuffer[ 256 ];

	FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM |
				    FORMAT_MESSAGE_IGNORE_INSERTS,
				    NULL,
				    ( DWORD )dCode,
				    MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
				    szBuffer,
				    256,
				    NULL );

	oss << "[ " << dCode << " ]: " << szBuffer << ends;

#else

	if ( dCode != -1 )
	{
		oss << "( errno: " << dCode << " ) - " << strerror( dCode );
	}

#endif

	return oss.str();
}

void CArcAPIClient::ThrowException( string sMethodName, string sMsg )
{
		ostringstream oss;

		oss << "( CArcAPIClient::"
			<< ( sMethodName.empty() ? "???" : sMethodName )
			<< "() ): "
			<< sMsg
			<< ends;

		throw runtime_error( oss.str() );
}

void CArcAPIClient::CheckExceptionOccurred( string sRecvd )
{
	CHECK_EXCEPTION_OCCURRED( sRecvd );
}

void CArcAPIClient::DetectServers( unsigned short usPort )
{
#ifdef WIN32

	SOCKET RecvSocket;
	sockaddr_in RecvAddr;
	char RecvBuf[ 1024 ];
	int  BufLen = 1024;
	sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof( SenderAddr );

	//-----------------------------------------------
	// Create a receiver socket to receive datagrams
	RecvSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	//-----------------------------------------------
	// Bind the socket to any address and the specified port.
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons( usPort );
	RecvAddr.sin_addr.s_addr = htonl( INADDR_ANY );

	bind( RecvSocket,
		  ( SOCKADDR * )&RecvAddr,
		  sizeof( RecvAddr ) );

	for ( int i=2; i<255; i++ )
	{
		ostringstream oss;
		oss << "192.168.0." << i << ends;
	
		//---------------------------------------------
		// Set up the SenderAddr structure with the IP address of
		// the receiver (in this example case "192.168.1.1")
		// and the specified port number.
		SenderAddr.sin_family      = AF_INET;
		SenderAddr.sin_port        = htons( usPort );		// Change to network-byte order and

		auto retVal = inet_pton( SenderAddr.sin_family, oss.str().c_str(), &SenderAddr.sin_addr.s_addr );

		if ( retVal == 0 )
		{
			ThrowException( "DetectServers", "Invalid address format" );
		}

		else if ( retVal < 0 )
		{
			ThrowException( "DetectServers", "Failed to convert send address" );
		}

		sprintf_s( RecvBuf,
				   BufLen,
				   "%s %s", "arc::CArcAPIServer",
				   "Find" );
	
		//---------------------------------------------
		// Send a datagram to the receiver
		send( RecvSocket,
			  RecvBuf,
			  BufLen,
			  0 );
	
		//-----------------------------------------------
		// Call the recvfrom function to receive datagrams
		// on the bound socket.
		recv( RecvSocket,
			  RecvBuf,
			  BufLen,
			  0 );

	}		// END FOR LOOP

	//-----------------------------------------------
	// Close the socket when finished receiving datagrams
	closesocket( RecvSocket );

#endif
}
