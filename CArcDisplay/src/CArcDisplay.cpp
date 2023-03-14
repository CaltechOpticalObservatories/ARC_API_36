// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  CArcDisplay.cpp  ( Gen3 )                                                                                |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE: This file implements the ARC image display interface.                                                  |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 26, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2013 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+
#include <sstream>
#include <cstdlib>
#include <cstring>

#ifdef _WINDOWS
	#include <tchar.h>
#endif

#if defined( linux ) || defined( __linux )
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <unistd.h>
	#include <link.h>
	#include <dlfcn.h>
#endif

#ifdef __APPLE__
	#include <csignal>
	#include <unistd.h>
	#include <dlfcn.h>
#endif

#include <CArcDisplay.h>



namespace arc
{
	namespace gen3
	{

	#ifdef _WINDOWS
		#define VERIFY_PID		if ( m_processInfo.dwProcessId == 0 ) { return; }
	#else
		#define VERIFY_PID		if ( m_pid == 0 ) { return; }
	#endif


		// +----------------------------------------------------------------------------------------------------------+
		// | Library build and version info                                                                           |
		// +----------------------------------------------------------------------------------------------------------+
		const std::string CArcDisplay::m_sVersion = std::string( "ARC Gen III Display API Library v3.6.  " ) +

		#ifdef _WINDOWS
			CArcBase::formatString( "[ Compiler Version: %d, Built: %s ]", _MSC_VER, __TIMESTAMP__ );
		#else
			arc::gen3::CArcBase::formatString( "[ Compiler Version: %d.%d.%d, Built: %s %s ]", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __DATE__, __TIME__ );
		#endif


		// +----------------------------------------------------------------------------------------------------------+
		// |  Default constructor                                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		CArcDisplay::CArcDisplay( void )
		{
			#ifdef _WINDOWS
				ZeroMemory( &m_processInfo, sizeof( m_processInfo ) );
			#else
				m_pid = 0;
			#endif
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  Destructor                                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		CArcDisplay::~CArcDisplay( void )
		{
			#ifdef _WINDOWS

				CloseHandle( m_processInfo.hProcess );
				CloseHandle( m_processInfo.hThread );

			#elif defined( __APPLE__ ) || defined( linux ) || defined( __linux )

				kill( m_pid, 1 );

			#endif
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  version                                                                                                 |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns a textual representation of the library version.                                                |
		// +----------------------------------------------------------------------------------------------------------+
		const std::string CArcDisplay::version( void )
		{
			return m_sVersion;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  launch                                                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Attempts to launch the DS9 application as a new process.                                                |
		// |                                                                                                          |
		// |  <IN> uiMSDelay - The amount of time to delay before attempting to access the program ( Default = 1000 ) |
		// |                                                                                                          |
		// |  Throws std::runtime_error if DS9 cannot be started or found.                                            |
		// +----------------------------------------------------------------------------------------------------------+
		void CArcDisplay::launch( std::uint32_t uiMSDelay )
		{
			#ifdef _WINDOWS

				//
				// Make sure the process isn't already running.
				//
				DWORD dwExitCode = 0;

				GetExitCodeProcess( m_processInfo.hProcess, &dwExitCode );

				if ( dwExitCode != STILL_ACTIVE )
				{
					runAndSaveProcess( "ds9.exe" );
				}

			#else

				char* pszEnv = getenv( "ARC_DS9_PATH" );

				if ( pszEnv == nullptr )
				{
					std::ostringstream oss;

					oss << "Failed to locate DS9 path environment variable. "
						<< "Set \"ARC_DS9_PATH\" to location of ds9 executable."
						<< std::ends;

					THROW( oss.str() );
				}

				std::string sPath = std::string( pszEnv ) + std::string( "/ds9" );

				runAndSaveProcess( sPath.c_str() );

				//
				// LINUX/APPLE ONLY: Sleep for 1 sec to allow DS9 to start
				//
				usleep( uiMSDelay * 1000 );

			#endif

			//
			// Set the default region to "box"
			//
			//setBoxShape();
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  show                                                                                                    |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Displays the specified FITS image data in DS9.                                                          |
		// |                                                                                                          |
		// |  <IN> sFitsFile - FITS file to display.                                                                  |
		// |                                                                                                          |
		// |  Throws std::runtime_error                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		void CArcDisplay::show( const std::string& sFitsFile )
		{
			VERIFY_PID

			std::ostringstream oss;

			#ifdef _WINDOWS
				oss << "xpaset -p ds9 file \"{" << sFitsFile << "}\"" << std::ends;
			#else
				oss << "xpaset -p ds9 fits " << sFitsFile << std::ends;
			#endif

			runProcess( oss.str() );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  clear                                                                                                   |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Clears the specified frame or all if no frame number is specified.                                      |
		// |                                                                                                          |
		// |  <IN> iFrame - Frame number to clear or -1 to clear all ( default = -1 ).                                |
		// |                                                                                                          |
		// |  Throws std::runtime_error                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		void CArcDisplay::clear( std::int32_t iFrame )
		{
			VERIFY_PID

			std::ostringstream oss;

			oss << "xpaset -p ds9 frame clear " << ( ( iFrame > 0 ) ? std::to_string( iFrame ) : "all" ) << std::ends;

			runProcess( oss.str() );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  terminate                                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Forces the termination of the display process.                                                          |
		// |                                                                                                          |
		// |  Throws std::runtime_error                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		void CArcDisplay::terminate( void )
		{
			VERIFY_PID

			//			std::cout << "terminating DS9" << std::std::endl;

			#ifdef _WINDOWS

				runProcess( "xpaset -p ds9 exit" );

			#else

				kill( m_pid, 1 );

			#endif
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  setBoxShape                                                                                             |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Sets the "region" shape to box [].                                                                      |
		// |                                                                                                          |
		// |  Throws std::runtime_error                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		void CArcDisplay::setBoxShape( void )
		{
			VERIFY_PID

			runProcess( "xpaset -p ds9 regions shape box" );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getVersion                                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the DS9 version number.                                                                         |
		// |                                                                                                          |
		// |  Throws std::runtime_error                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		void CArcDisplay::getVersion( void )
		{
			VERIFY_PID

			runProcess( "xpaget -p ds9 version" );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  runProcess ( _WINDOWS ONLY )                                                                            |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Creates a new _WINDOWS process.                                                                         |
		// |                                                                                                          |
		// |  <IN> sBuf - Process string to execute.                                                                  |
		// |                                                                                                          |
		// |  Throws std::runtime_error                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		void CArcDisplay::runProcess( const std::string& sBuf )
		{
			#ifdef _WINDOWS

				STARTUPINFOA si;
				PROCESS_INFORMATION pi;

				ZeroMemory( &si, sizeof( si ) );
				si.cb = sizeof( si );
				ZeroMemory( &pi, sizeof( pi ) );

				BOOL bOK = CreateProcessA(  nullptr,
											( LPSTR )sBuf.c_str(),
											nullptr,
											nullptr,
											FALSE,
											( NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW ),
											nullptr,
											nullptr,
											&si,
											&pi );

				if ( !bOK )
				{
					THROW( "Failed to run \"%s\"! Verify path!", sBuf.c_str() );
				}

				CloseHandle( pi.hProcess );
				CloseHandle( pi.hThread );

			#else

				FILE* handle = popen( sBuf.c_str(), "r" );

				if ( handle == NULL )
				{
					THROW( "%e", errno );
				}

				char buf[ 64 ];

				size_t readn = 0;

				while ( ( readn = fread( buf, 1, sizeof( buf ), handle ) ) > 0 )
				{
					fwrite( buf, 1, readn, stdout );
				}

				if ( readn > 0 )
				{
					THROW( "%s", buf );
				}

				pclose( handle );

			#endif
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  runAndSaveProcess                                                                                       |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Creates a new process and saves PROCESS_INFORMATION ( _WINDOWS ) or pid_t.                                 |
		// |                                                                                                          |
		// |  <IN> sBuf - Process string to execute.                                                                  |
		// |                                                                                                          |
		// |  Throws std::runtime_error                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		void CArcDisplay::runAndSaveProcess( const std::string& sBuf )
		{
			#ifdef _WINDOWS

				STARTUPINFOA si;
				ZeroMemory( &si, sizeof( si ) );
				si.cb = sizeof( si );
				si.wShowWindow = SW_HIDE;

				ZeroMemory( &m_processInfo, sizeof( m_processInfo ) );

				BOOL bOK = CreateProcessA(  nullptr,
											( LPSTR )sBuf.c_str(),
											nullptr,
											nullptr,
											FALSE,
											( NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW ),
											nullptr,
											nullptr,
											&si,
											&m_processInfo );

				if ( !bOK )
				{
					ZeroMemory( &m_processInfo, sizeof( m_processInfo ) );

					THROW( "Failed to run \"%s\"! Verify path!", sBuf.c_str() );
				}

				// Wait until child process exits.
				WaitForSingleObject( m_processInfo.hProcess, 5000 );

			#else

				m_pid = fork();

				switch ( m_pid )
				{
					// +------------------------+
					// |  Error                 |
					// +------------------------+
				case -1:
				{
					THROW( "Failed to create process for: \"%s\"! Verify path!", sBuf.c_str() );
				}
				break;

				// +------------------------+
				// |  Child Process         |
				// +------------------------+
				case 0:
				{
					execl( ( char* )sBuf.c_str(), ( char* )sBuf.c_str(), ( char* )nullptr );
				}
				break;

				// +------------------------+
				// |  Parent Process        |
				// +------------------------+
				default:
				{
					std::int32_t iStatus = 0;

					if ( waitpid( m_pid, &iStatus, WNOHANG ) == -1 )
					{
						THROW( "Failed to launch DS9 process! %e", CArcBase::getSystemError() );
					}
				}
				break;
				}

			#endif
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getModuleDirectory                                                                                      |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Gets the path of THIS module. Returns 'true' on success; 'false' othewise.                              |
		// |                                                                                                          |
		// |  <OUT> pszBuf   - Buffer to hold module path. Set to '\0' on failure.                                    |
		// |  <IN > tBufSize - The byte size of the obuf buffer.                                                      |
		// |                                                                                                          |
		// |  Throws std::runtime_error                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		#ifdef _WINDOWS
		bool CArcDisplay::getModuleDirectory( TCHAR* pszBuf, size_t tBufSize )
		{
			if ( !GetModuleFileName( 0, pszBuf, static_cast< DWORD >( tBufSize ) ) )
			{
				*pszBuf = '\0';	// Ensure it's nullptr terminated

				return false;
			}

			// Run through looking for the *last* slash in this path.
			// if we find it, NUL it out to truncate the following
			// filename part.
			TCHAR* lastslash = 0;

			for ( ; *pszBuf; pszBuf++ )
			{
				if ( *pszBuf == '\\' || *pszBuf == '/' )
				{
					lastslash = pszBuf;
				}
			}

			if ( lastslash ) { *lastslash = '\0'; }

			return true;
		}

		#else
		bool CArcDisplay::getModuleDirectory( char* pszBuf, size_t tBufSize )
		{
			Dl_info	rInfo;

			*pszBuf = 0;

			memset( &rInfo, 0, sizeof( rInfo ) );

			if ( !dladdr( ( void* )&getModuleStub, &rInfo ) || !rInfo.dli_fname )
			{
				return false;
			}

			strncpy( pszBuf, rInfo.dli_fname, tBufSize );

			char* pLastSlash = 0;

			for ( ; *pszBuf; pszBuf++ )
			{
				if ( *pszBuf == '\\' || *pszBuf == '/' )
				{
					pLastSlash = pszBuf;
				}
			}

			if ( pLastSlash ) { *pLastSlash = '\0'; }

			return true;
		}

		#endif

	}	// end gen3 namespace
}		// end arc namespace



























//// +---------------------------------------------------------------------------+
//// |  CArcDisplay.cpp                                                          |
//// +---------------------------------------------------------------------------+
//// |  Displays the image using SAOImage DS9.                                   |
//// |                                                                           |
//// |  Scott Streit | Astronomical Research Cameras, Inc. | Aug 24, 2012        |
//// +---------------------------------------------------------------------------+
//#include <iostream>
//
//#include <sstream>
//#include <cstdlib>
//#include <cstring>
//
//#ifdef _WINDOWS
//	#include <tchar.h>
//#endif
//
//#if defined( linux ) || defined( __linux )
//	#include <sys/types.h>
//	#include <sys/wait.h>
//	#include <unistd.h>
//	#include <link.h>
//	#include <dlfcn.h>
//#endif
//
//#ifdef __APPLE__
//	#include <csignal>
//	#include <unistd.h>
//	#include <dlfcn.h>
//#endif
//
//#include <CArcDisplay.h>
//
//
//#define NXPA	10
//
//using namespace std;
//using namespace arc::display;
//
//
//#ifdef _WINDOWS
//	#define VERIFY_PID		if ( m_processInfo.dwProcessId == 0 ) { return; }
//#else
//	#define VERIFY_PID		if ( m_pid == 0 ) { return; }
//#endif
//
//
//// +---------------------------------------------------------------------------+
//// |  Default constructor                                                      |
//// +---------------------------------------------------------------------------+
//CArcDisplay::CArcDisplay()
//{
//#if defined( linux ) || defined( __linux ) || defined( __APPLE__ )
//
//	m_pid = 0;
//
//#elif defined( _WINDOWS )
//
//	SecureZeroMemory( &m_processInfo, sizeof( PROCESS_INFORMATION ) );
//
//#endif
//}
//
//// +---------------------------------------------------------------------------+
//// |  Class destructor                                                         |
//// +---------------------------------------------------------------------------+
//// |  Destroys the class. Deallocates any header and data buffers. Closes any  |
//// |  open FITS pointers.                                                      |
//// |                                                                           |
//// |  Throws CFitsException on input parameter error or CFitsException         |
//// |  on cfitsio library error.                                                |
//// +---------------------------------------------------------------------------+
//CArcDisplay::~CArcDisplay()
//{
//#ifdef _WINDOWS
//
//	CloseHandle( m_processInfo.hProcess );
//	CloseHandle( m_processInfo.hThread );
//
//#elif defined( linux ) || defined( __linux ) || defined( __APPLE__ )
//
//	kill( m_pid, 1 );
//
//#endif
//}
//
//// +---------------------------------------------------------------------------+
//// | Launch                                                                    |
//// +---------------------------------------------------------------------------+
//// |  Attempts to launch the DS9 program.                                      |
//// |                                                                           |
//// |  Throws std::runtime_error if DS9 cannot be started or found.             |
//// |                                                                           |
//// |  <IN> -> dMSDelay - The amount of time to delay before attempting to      |
//// |                     access the application. Default: 1 second             |
//// +---------------------------------------------------------------------------+
//void CArcDisplay::Launch( int dMSDelay )
//{
//#ifdef _WINDOWS
//
//	//
//	// Make sure the process isn't already running.
//	//
//	DWORD dwExitCode = 0;
//
//	GetExitCodeProcess( m_processInfo.hProcess, &dwExitCode );
//
//	if ( dwExitCode != STILL_ACTIVE )
//	{
//		RunAndSaveProcess( "ds9.exe" );
//	}
//
//#else
//
//	char* pszEnv = getenv( "ARC_DS9_PATH" );
//
//	if ( pszEnv == nullptr )
//	{
//		std::ostringstream oss;
//
//		oss << "Failed to locate DS9 path environment variable. "
//			<< "Set \"ARC_DS9_PATH\" to location of ds9 executable."
//			<< std::ends;
//
//		ThrowException( "Launch", oss.str() );
//	}
//
//	std::string sPath = std::string( pszEnv ) + std::string( "/ds9" );
//
//	RunAndSaveProcess( sPath.c_str() );
//
//	//
//	// LINUX/APPLE ONLY: Sleep for 1 sec to allow DS9 to start
//	//
//	usleep( dMSDelay * 1000 );
//
//#endif
//
//	//
//	// Set the default region to "box"
//	//
//	SetBoxShape();
//}
//
//
//// +---------------------------------------------------------------------------+
//// | Show                                                                      |
//// +---------------------------------------------------------------------------+
//// |  Displays the specified FITS image data in DS9.                           |
//// |                                                                           |
//// |  Throws std::runtime_error on error                                       |
//// |  <IN> -> sFitsFile - FITS file to display.                                |
//// +---------------------------------------------------------------------------+
//void CArcDisplay::Show( const std::string& sFitsFile )
//{
//	VERIFY_PID
//
//	ostringstream oss;
//
//	oss << "xpaset -p ds9 file \"{" << sFitsFile << "}\"" << std::ends;
//
//	RunProcess( oss.str() );
//}
//
//// +---------------------------------------------------------------------------+
//// | Clear                                                                     |
//// +---------------------------------------------------------------------------+
//// |  Clears the specified frame or all if no frame number is specified.       |
//// |                                                                           |
//// |  <IN> -> dFrame - Frame number to clear or -1 to clear all ( default ).   |
//// +---------------------------------------------------------------------------+
//void CArcDisplay::Clear( int dFrame )
//{
//	VERIFY_PID
//
//	ostringstream oss;
//
//	oss << "xpaset -p ds9 frame clear " << ( ( dFrame > 0 ) ? std::to_string( dFrame ) : "all" ) << std::ends;
//
//	RunProcess( oss.str() );
//}
//
//// +---------------------------------------------------------------------------+
//// |  Terminate                                                                |
//// +---------------------------------------------------------------------------+
//// |  Forces the termination of the display process.                           |
//// +---------------------------------------------------------------------------+
//void CArcDisplay::Terminate()
//{
//	VERIFY_PID
//
//	RunProcess( "xpaset -p ds9 exit" );
//
//#ifndef _WINDOWS
//
//	kill( m_pid, 1 );
//
//#endif
//}
//
//
//// +---------------------------------------------------------------------------+
//// |  SetBoxShape                                                              |
//// +---------------------------------------------------------------------------+
//// |  Sets the "region" shape to box [].                                       |
//// +---------------------------------------------------------------------------+
//void CArcDisplay::SetBoxShape()
//{
//	VERIFY_PID
//
//	RunProcess( "xpaset -p ds9 regions shape box" );
//}
//
//
//// +---------------------------------------------------------------------------+
//// | RunProcess ( _WINDOWS ONLY )                                              |
//// +---------------------------------------------------------------------------+
//// |  Creates a new _WINDOWS process.                                          |
//// |                                                                           |
//// |  <IN> -> sBuf - Process string to execute.                                |
//// +---------------------------------------------------------------------------+
//void CArcDisplay::RunProcess( const std::string& sBuf )
//{
//#ifdef _WINDOWS
//
//	STARTUPINFOA si;
//	PROCESS_INFORMATION pi;
//
//	ZeroMemory( &si, sizeof( si ) );
//	si.cb = sizeof( si );
//	ZeroMemory( &pi, sizeof( pi ) );
//
//	BOOL bOK = CreateProcessA( nullptr,
//							  ( LPSTR )sBuf.c_str(),
//							   nullptr,
//							   nullptr,
//							   FALSE,
//							   ( NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW ),
//							   nullptr,
//							   nullptr,
//							   &si,
//							   &pi );
//
//	if ( !bOK )
//	{
//		ostringstream oss;
//
//		oss << "Failed to run \""
//			<< sBuf
//			<< "\"! Verify path!"
//			<< ends;
//
//		ThrowException( "RunProcess", oss.str().c_str() );
//	}
//
//	CloseHandle( pi.hProcess );
//	CloseHandle( pi.hThread );
//
//#else
//
//	FILE* handle = popen( sBuf.c_str(), "r" );
//
//	if ( handle == NULL )
//	{
//		ostringstream oss;
//
//		oss << "Failed to run \""
//			<< sBuf
//			<< "\"! Verify path!"
//			<< " errno: " << errno
//			<< ends;
//
//		ThrowException( "RunProcess", oss.str().c_str() );
//	}
//
//	char buf[ 64 ];
//
//	size_t readn = 0;
//
//	while ( ( readn = fread( buf, 1, sizeof( buf ), handle ) ) > 0 )
//	{
//		fwrite( buf, 1, readn, stdout );
//	}
//
//	if ( readn > 0 )
//	{
//		ostringstream oss;
//
//		oss << "Failed to run \""
//			<< sBuf
//			<< "\"! Verify path!"
//			<< " " << buf
//			<< ends;
//
//		ThrowException( "RunProcess", oss.str().c_str() );
//	}
//
//	pclose( handle );
//
//#endif
//}
//
//// +---------------------------------------------------------------------------+
//// | RunAndSaveProcess                                                         |
//// +---------------------------------------------------------------------------+
//// |  Creates a new process and saves PROCESS_INFORMATION ( _WINDOWS ) or pid_t|
//// |                                                                           |
//// |  <IN> -> sBuf - Process string to execute.                                |
//// +---------------------------------------------------------------------------+
//void CArcDisplay::RunAndSaveProcess( const std::string& sBuf )
//{
//#ifdef _WINDOWS
//
//	STARTUPINFOA si;
//
//	ZeroMemory( &si, sizeof( si ) );
//	si.cb = sizeof( si );
//	ZeroMemory( &m_processInfo, sizeof( m_processInfo ) );
//
//	BOOL bOK = CreateProcessA( nullptr,
//							   ( LPSTR )sBuf.c_str(),
//							   nullptr,
//							   nullptr,
//							   FALSE,
//							   ( NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW ),
//							   nullptr,
//							   nullptr,
//							   &si,
//							   &m_processInfo );
//
//	if ( !bOK )
//	{
//		ostringstream oss;
//
//		oss << "Failed to run \""
//			<< sBuf
//			<< "\"! Verify path!"
//			<< ends;
//
//		ThrowException( "RunAndSaveProcess", oss.str().c_str() );
//	}
//
//	// Wait until child process exits.
//	WaitForSingleObject( m_processInfo.hProcess, 5000 );
//
//#else
//
//	m_pid = fork();
//
//	switch( m_pid )
//	{
//		// +------------------------+
//		// |  Error                 |
//		// +------------------------+
//		case -1:
//		{
//			ostringstream oss;
//
//			oss << "Failed to create process for: \""
//				<< sBuf << "\"! Verify path!" << ends;
//
//			ThrowException( "RunAndSaveProcess",
//							 oss.str().c_str() );
//		}
//		break;
//
//		// +------------------------+
//		// |  Child Process         |
//		// +------------------------+
//		case 0:
//		{
//			//char* pszArg[] = { 0, 0 };
//
//			//pszArg[ 0 ] = ( char * )sBuf.c_str();
//
//			//execv( pszArg[ 0 ], pszArg );
//
//			execl( ( char * )sBuf.c_str(),
//				   ( char * )sBuf.c_str(),
//				   ( char * )NULL );
//		}
//		break;
//
//		// +------------------------+
//		// |  Parent Process        |
//		// +------------------------+
//		default:
//		{
//			int dStatus = 0;
//
//			waitpid( m_pid, &dStatus, WNOHANG );
//		}
//		break;
//	}
//
//#endif
//}
//
//// +---------------------------------------------------------------------------+
//// | GetModuleDirectory                                                        |
//// +---------------------------------------------------------------------------+
//// |  Gets the path of THIS module. Returns 'true' on success; 'false'         |
//// |  otherwise.                                                               |
//// |                                                                           |
//// |  <OUT> -> pszBuf   - Buffer to hold module path. Set to '\0' on failure.  |
//// |  <IN > -> tBufSize - The byte size of the obuf buffer.                    |
//// +---------------------------------------------------------------------------+
//#ifdef _WINDOWS
//bool CArcDisplay::GetModuleDirectory( TCHAR* pszBuf, size_t tBufSize )
//{
//	if ( !GetModuleFileName( 0, pszBuf, static_cast<DWORD>( tBufSize ) ) )
//	{
//		*pszBuf = '\0';	// Ensure it's NULL terminated
//
//		return false;
//	}
//
//	// Run through looking for the *last* slash in this path.
//	// if we find it, NUL it out to truncate the following
//	// filename part.
//	TCHAR* lastslash = 0;
//
//	for ( ; *pszBuf; pszBuf++ )
//	{
//		if ( *pszBuf == '\\' || *pszBuf == '/' )
//		{
//			lastslash = pszBuf;
//		}
//	}
//
//	if ( lastslash ) { *lastslash = '\0'; }
//
//	return true;
//}
//
//#else
//bool CArcDisplay::GetModuleDirectory( char* pszBuf, size_t tBufSize )
//{
//	Dl_info	rInfo;
//
//	*pszBuf = 0;
//
//	memset( &rInfo, 0, sizeof( rInfo ) );
//
//	if ( !dladdr( ( void * )&arc::display::CArcDisplay::GetModuleStub, &rInfo ) || !rInfo.dli_fname )
//	{
//		return false;
//	}
//
//	strncpy( pszBuf, rInfo.dli_fname, tBufSize );
//
//	char* pLastSlash = 0;
//
//	for ( ; *pszBuf; pszBuf++ )
//	{
//		if ( *pszBuf == '\\' || *pszBuf == '/' )
//		{
//			pLastSlash = pszBuf;
//		}
//	}
//
//	if ( pLastSlash ) { *pLastSlash = '\0'; }
//
//	return true;
//}
//
//#endif
//
//// +----------------------------------------------------------------------------
//// |  ThrowException
//// +----------------------------------------------------------------------------
//// |  Method to throw a "const char *" exception.
//// |
//// |  <IN> -> sMethodName : The name of the method where the error occurred.
//// |  <IN> -> sMsg        : A std::string to use for the exception message.
//// +----------------------------------------------------------------------------
//void CArcDisplay::ThrowException( const std::string& sMethodName, const std::string& sMsg )
//{
//	throw std::runtime_error( string( "( CArcDisplay::" ) + 
//							  ( sMethodName.empty() ? "???" : sMethodName ) +
//							  "() ): " + sMsg );
//}
//
///*******************************************************************************
//  DS9 command list and syntax
// *******************************************************************************
//
//		* about
//		* analysis
//		* array
//		* bin
//		* blink
//		* cd
//		* cmap
//		* contour
//		* crosshair
//		* cursor
//		* data
//		* datacube
//		* dss
//		* exit
//		* file
//		* fits
//		* frame
//		* grid
//		* iconify
//		* iis
//		* imexam
//		* lock
//		* lower
//		* match
//		* minmax
//		* mode
//		* nameserver
//		* orient
//		* page setup
//		* pan
//		* pixeltable
//		* plot
//		* prefs
//		* preserve
//		* print
//		* quit
//		* raise
//		* regions
//		* rgb
//		* rotate
//		* scale
//		* saveimage
//		* savefits
//		* savempeg
//		* shm
//		* single
//		* source
//		* tcl
//		* tile
//		* update
//		* version
//		* view
//		* vo
//		* wcs
//		* web
//		* zoom
//
//	about
//	Get DS9 credits.
//	Syntax:
//	about
//	Example:
//	$xpaget ds9 about
//
//
//	analysis
//	Control external analysis tasks. Tasks are numbered as they are loaded, starting with 1. Can also be used to display a message and display text in the text dialog window. To plot data, use the plot xpa point. Current tasks can be cleared via the clear command and new tasks can be loaded via the load command.
//	Syntax:
//	analysis [<task number>]
//			 [clear]
//			 [clear][load <filename>]
//			 [message ok|okcancel|yesno {<message>}
//			 [entry {<message>}]
//			 [text]
//
//	Example:
//	$xpaget ds9 analysis
//	$xpaset -p ds9 analysis 0 # invoke first analysis task
//	$xpaset -p ds9 analysis clear
//	$xpaset -p ds9 analysis load my.analysis
//	$xpaset -p ds9 analysis clear load my.analysis
//	$xpaset -p ds9 analysis message ok '{This is a test}'
//	$xpaget ds9 analysis entry '{Please enter something}'
//	$cat my.analysis | xpaset ds9 analysis load
//	$cat foo.txt | xpaset ds9 analysis text
// 
//
//   array
//	Load raw data array from stdin. If new is specified, a new frame is created first, before loading.
//	Syntax:
//	array [new][[xdim=<x>,ydim=<y>|dim=<dim>],zdim=<z>,bitpix=<b>,skip=<s>,
//		arch=[littleendian|bigendian]]
//	array [new] rgb [[xdim=<x>,ydim=<y>|dim=<dim>],zdim=<z>,bitpix=<b>,skip=<s>,
//		arch=[littleendian|bigendian]]
//	Example:
//	$xpaget ds9 array
//	$cat foo.arr | xpaset ds9 array [dim=512,bitpix=16]
//	$cat rgb.arr | xpaset ds9 array rgb [dim=200,zdim=3,bitpix=8]
//	$cat bar.arr | xpaset ds9 array [xdim=512,ydim=512,zdim=1,bitpix=16] # load 512x512 short
//	$cat bar.arr | xpaset ds9 array [dim=256,bitpix=-32,skip=4] # load 256x256 float with 4 byte head
//	$cat bar.arr | xpaset ds9 array [dim=512,bitpix=32,arch=littleendian] # load 512x512 long, intel
//
//
//	bin
//	Controls binning factor, binning buffer size, and  binning function for binning FITS bin tables. The access point blocking is provided for backward compatibility.
//	Syntax:
//	bin [about <x> <y>]
//		[buffersize <value>]
//		[cols <x> <y>]
//		[factor <value> [<vector>]]
//		[depth <value>]
//		[filter <string>]
//		[function average|sum]
//		[to fit]
//		[smooth yes|no]
//		[smooth function boxcar|tophat|gaussian]
//		[smooth radius <value>]
//	Example:
//	$xpaget ds9 bin about
//	$xpaget ds9 bin buffersize
//	$xpaget ds9 bin cols
//	$xpaget ds9 bin factor
//	$xpaget ds9 bin depth
//	$xpaget ds9 bin filter
//	$xpaget ds9 bin function
//	$xpaget ds9 bin smooth
//	$xpaget ds9 bin smooth function
//	$xpaget ds9 bin smooth radius
//	$xpaset -p ds9 bin about 4096 4096
//	$xpaset -p ds9 bin buffersize 512
//	$xpaset -p ds9 bin cols detx dety
//	$xpaset -p ds9 bin factor 4
//	$xpaset -p ds9 bin factor 4 2
//	$xpaset -p ds9 bin depth 10
//	$xpaset -p ds9 bin filter '{pha > 5}'
//	$xpaset -p ds9 bin function sum
//	$xpaset -p ds9 bin to fit
//	$xpaset -p ds9 bin smooth yes
//	$xpaset -p ds9 bin smooth boxcar
//	$xpaset -p ds9 bin smooth radius 3
// 
//
//   blink
//	Select Blink Display Mode
//	Syntax:
//	blink
//	Example:
//	$xpaget ds9 blink
//	$xpaset -p ds9 blink
//
//
//	cd
//	Sets/Returns the current working directory.
//	Syntax:
//	cd [<directory>]
//	Example:
//	$xpaget ds9 cd
//	$xpaset -p ds9 cd /home/mrbill
// 
//
//   cmap
//	Controls the colormap for the current frame. The colormap name is not case sensitive. A valid contrast value is  from 0 to 10 and bias value from 0 to 1.
//	Syntax:
//	cmap [<colormap>]
//		 [file <filename>]
//		 [invert yes|no]
//		 [value <constrast> <bias>]
//	Example:
//	$xpaget ds9 cmap
//	$xpaget ds9 cmap file
//	$xpaget ds9 cmap invert
//	$xpaget ds9 cmap value
//	$xpaset -p ds9 cmap Heat
//	$xpaset -p ds9 cmap file foo.sao
//	$xpaset -p ds9 cmap invert yes
//	$xpaset -p ds9 cmap value 5 .5
//  
//
//  contour
//	Controls contours in the current frame.
//	Syntax:
//	contour [yes|no]
//			[<coordinate system> [<sky frame>]]
//			[copy]
//			[clear]
//			[paste <coordinate system> [<sky frame>] <color> <width>]
//			[load <filename> <coordinate system> [<sky frame>] <color> <width>]
//			[save <filename> <coordinate system> [<sky frame>]]
//	Example:
//	$xpaget ds9 contour
//	$xpaget ds9 contour wcs fk5
//	$xpaset -p ds9 contour yes
//	$xpaset -p ds9 contour copy
//	$xpaset -p ds9 contour clear
//	$xpaset -p ds9 contour paste wc fk4 red 2
//	$xpaset -p ds9 contour load foo.con wcs fk5 yellow 2
//	$xpaset -p ds9 contour save foo.con wcs fk5
//
//
//   crosshair
//	Controls the current position of the crosshair in the current frame. DS9 is placed in crosshair mode when the crosshair is set.
//	Syntax:
//	crosshair [x y <coordinate system> [<sky frame>][<sky format>]]
//	Example:
//	$xpaget ds9 crosshair # get crosshair in physical coords
//	$xpaget ds9 crosshair wcs fk4 sexagesimal # get crosshair in wcs coords
//	$xpaset -p ds9 crosshair 100 100 physical # set crosshair in physical
//	$xpaset -p ds9 crosshair 345 58.8 wcs fk5 # set crosshair in wcs coords
//	$xpaset -p ds9 crosshair 23:01:00 +58:52:51 wcs fk5
// 
//
//   cursor
//	Move mouse pointer or crosshair in image pixels in the current frame. Note, this will move selected Regions also.
//	Syntax:
//	cursor [x y]
//	Example:
//	$xpaset -p ds9 cursor 10 10
// 
//
//   data
//	Return an array of data values given a lower left corner and a width and height in specified coordinate system. The last argument indicates if the coordinates are listed or just the data values.
//	Syntax:
//	data [<coordinate system> [<sky frame>] <x> <y> <w> <h> [yes|no]]
//	Example:
//	$xpaget ds9 data image 450 520 3 3 yes
//	$xpaget ds9 data physical 899 1039 6 6 no
//	$xpaget ds9 data fk5 202.47091 47.196811 0.00016516669 0.00016516669 no
//	$xpaget ds9 data wcs fk5 13:29:53.018 +47:11:48.52 0.00016516669 0.00016516669 no
// 
//
//   datacube
//	Controls FITS datacube.
//	Syntax:
//	datacube [play|stop|next|prev|first|last]
//			 [#]
//			 [interval #]
//	Example:
//	$xpaget ds9 datacube
//	$xpaget ds9 datacube interval
//	$xpaset -p ds9 datacube play
//	$xpaset -p ds9 datacube last
//	$xpaset -p ds9 datacube 3
//	$xpaset -p ds9 datacube interval 2
//
//
//	dss
//	Support for Digital Sky Survey. The first three options will download an image based on the name or specified fk5 coordinate. The remaining options are used to configure downloads.
//	Syntax:
//	dss [<object>]
//		[name <object>]
//		[coordinate <ra> <dec>] # in wcs fk5
//
//		[server sao|stsci|eso]
//		[survey dss|dss2red|dss2blue]
//		[size <width> <height>] # in arcmin
//
//	Example:
//	$xpaget ds9 dss name
//	$xpaget ds9 dss coord
//	$xpaget ds9 dss server
//	$xpaget ds9 dss survey
//	$xpaget ds9 dss size
//	$xpaset -p ds9 dss m31
//	$xpaset -p ds9 dss name m31
//	$xpaset -p ds9 dss coord 00:42:44.404 +41:16:08.78
//	$xpaset -p ds9 dss server eso
//	$xpaset -p ds9 dss survey dss2red
//	$xpaset -p ds9 dss size 10 10
//
//
//	exit
//	Exit DS9.
//	Syntax:
//	exit
//	Example:
//	$xpaset -p ds9 exit
//
//
//	file
//	file Load a FITS image, FITS Mosaic image(s), or array from a file into the current frame, or return the current file name(s) loaded for the current frame.
//	Syntax:
//	file [new][<filename>]
//		 [new][fits <filename>]
//		 [new][sfits <filename> <filename>]
//		 [new][medatacube <filename>]
//		 [new][mosaicimage [iraf|wcs|wcsa...wcsz|wfpc2] <filename>]
//		 [new][mosaicimagenext [wcs|wcsa...wcsz] <filename>]
//		 [new][mosaic [iraf|wcs|wcsa...wcsz] <filename>]
//		 [new][smosaic [iraf|wcs|wcsa...wcsz] <filename> <filename>]
//		 [new][rgbcube <filename>]
//		 [new][srgbcube <filename> <filename>]
//		 [new][rgbimage <filename>]
//		 [new][rgbarray <filename>[[xdim=<x>,ydim=<y>|dim=<dim>],zdim=3,bitpix=<b>,[skip=<s>]]]
//		 [new][array <filename>[[xdim=<x>,ydim=<y>|dim=<dim>],zdim=<z>,bitpix=<b>,[skip=<s>]]]
//		 [new][url <url>]
//		 [save <filename>]
//		 [save gz <filename>]
//		 [save resample <filename>]
//		 [save resample gz <filename>]
//	Example:
//	$xpaget ds9 file
//	$xpaset -p ds9 file foo.fits
//	$xpaset -p ds9 file fits foo.fits
//	$xpaset -p ds9 file sfits foo.hdr foo.arr
//	$xpaset -p ds9 file medatacube foo.fits
//	$xpaset -p ds9 file mosaicimage iraf bar.fits
//	$xpaset -p ds9 file mosaicimage wcs bar.fits
//	$xpaset -p ds9 file mosaicimage wcsa bar.fits
//	$xpaset -p ds9 file mosaicimage wfpc2 hst.fits
//	$xpaset -p ds9 file mosaicimagenext wcs bar.fits
//	$xpaset -p ds9 file mosaic iraf foo.fits
//	$xpaset -p ds9 file mosaic wcs bar.fits
//	$xpaset -p ds9 file smosaic iraf foo.hdr foo.arr
//	$xpaset -p ds9 file smosaic wcs bar.hdr bar.arr
//	$xpaset -p ds9 file rgbcube rgb.fits
//	$xpaset -p ds9 file srgbcube rgb.hdr rgb.arr
//	$xpaset -p ds9 file rgbimage rgb.fits
//	$xpaset -p ds9 file rgbarray rgb.arr[dim=200,zdim=3,bitpix=-32]
//	$xpaset -p ds9 file array array.arr[dim=512,bitpix=-32]
//	$xpaset -p ds9 file url 'ftp://foo.bar.edu/img.fits'
//	$xpaset -p ds9 file save foo.fits # save the current frame as FITS Image
//	$xpaset -p ds9 file save gz foo.fits.gz # save as compressed FITS Image
//	$xpaset -p ds9 file save resample foo.fits # save current pan/zoom/rotate as FITS Image
//	$xpaset -p ds9 file save resample gz foo.fits.gz # save as compressed FITS Image
//
//
//	fits
//	Load a FITS image from stdin into the current frame. Options can include the FITS extension or binning instructions. xpaget returns the FITS image in the current frame. If new is specified, a new frame is created before loading.
//	Syntax:
//	fits [size]
//		 [size [image|physical|wcs|wcsa...wcsz] [degrees|arcmin|arcsecs]]
//		 [depth]
//		 [header [#] [keyword <string>]]
//		 [type]
//		 [image|table|resample] [gz]
//		 [new][<options>]
//		 [new][medatacube <options>]
//		 [new][mosaicimage [iraf|wcs|wcsa...wcsz|wfpc2] <options>]
//		 [new][mosaicimagenext [wcs|wcsa...wcsz] <options>]
//		 [new][mosaic [iraf|wcs|wcsa...wcsz] <options>]
//		 [new][rgbcube <options>]
//		 [new][rgbimage <options>]
//		 [save resample gz <filename>]
//	Example:
//	$xpaget ds9 fits > foo.fits
//	$xpaget ds9 fits size
//	$xpaget ds9 fits size wcs arcmin
//	$xpaget ds9 fits depth
//	$xpaget ds9 fits header # primary
//	$xpaget ds9 fits header 2 # hdu 2
//	$xpaget ds9 fits header -2 # hdu 2 with inherit
//	$xpaget ds9 fits header keyword "'BITPIX'"
//	$xpaget ds9 fits header 1 keyword "'BITPIX'"
//	$xpaget ds9 fits type
//	$xpaget ds9 fits image > foo.fits
//	$xpaget ds9 fits image gz > foo.fits.gz
//	$xpaget ds9 fits table > bar.fits
//	$xpaget ds9 fits table gz > bar.fits.gz
//	$xpaget ds9 fits resample > bar.fits
//	$xpaget ds9 fits resample gz > bar.fits.gz
//	$cat foo.fits | xpaset ds9 fits
//	$cat abc.fits | xpaset ds9 fits [2]
//	$cat bar.fits | xpaset ds9 fits new [bin=detx,dety]
//	$cat foo.fits | xpaset ds9 fits medatacube
//	$cat bar.fits | xpaset ds9 fits mosaicimage iraf
//	$cat bar.fits | xpaset ds9 fits mosaicimage wcs
//	$cat bar.fits | xpaset ds9 fits mosaicimage wcsa
//	$cat hst.fits | xpaset ds9 fits mosaicimage wfpc2
//	$cat bar.fits | xpaset ds9 fits mosaicimagenext wcs
//	$cat bar.fits | xpaset ds9 fits mosaic iraf
//	$cat bar.fits | xpaset ds9 fits mosaic wcs
//	$cat rgb.fits | xpaset ds9 fits rgbcube
//	$cat rgb.fits | xpaset ds9 fits rgbimage
//
//
//	frame
//	Controls frame functions. Frames may be created, deleted, reset, and centered. While return the current frame number. If you goto a frame that does not exists, it will be created. If the frame is hidden, it will be shown. The 'frameno' option is available for backward compatibility.
//	Syntax:
//	frame [center [#|all]]
//		  [clear [#|all]]
//		  [new [rgb]]
//		  [delete [#|all]]
//		  [reset [#|all]]
//		  [refresh [#|all]]
//		  [hide [#|all]]
//		  [show [#|all]]
//		  [first]
//		  [next]
//		  [prev]
//		  [last]
//		  [frameno #]
//		  [#]
//	Example:
//	$xpaget ds9 frame # returns the id of the current frame
//	$xpaget ds9 frame frameno # returns the id of the current frame
//	$xpaget ds9 frame all # returns the id of all frames
//	$xpaget ds9 frame active # returns the id of all active frames
//	$xpaset -p ds9 frame center # center current frame
//	$xpaset -p ds9 frame center 1 # center 'Frame1'
//	$xpaset -p ds9 frame center all # center all frames
//	$xpaset -p ds9 frame clear # clear current frame
//	$xpaset -p ds9 frame new # create new frame
//	$xpaset -p ds9 frame new rgb # create new rgb frame
//	$xpaset -p ds9 frame delete # delete current frame
//	$xpaset -p ds9 frame reset # reset current frame
//	$xpaset -p ds9 frame refresh # refresh current frame
//	$xpaset -p ds9 frame hide # hide current frame
//	$xpaset -p ds9 frame show 1 # show frame 'Frame1'
//	$xpaset -p ds9 frame first # goto first frame
//	$xpaset -p ds9 frame next # goto next frame
//	$xpaset -p ds9 frame prev # goto prev frame
//	$xpaset -p ds9 frame last # goto last frame
//	$xpaset -p ds9 frame frameno 4 # goto frame 'Frame4', create if needed
//	$xpaset -p ds9 frame 3 # goto frame 'Frame3', create if needed
//
//
//	grid
//	Controls coordinate grid.
//	Syntax:
//	grid [yes|no]
//		 [load <filename>]
//		 [save <filename>]
//		 [system <coordinate system>]
//		 [sky <sky frame>]
//		 [skyformat <skyformat>]
//		 [type analysis|publication]
//		 [type axes interior|exterior]
//		 [type numerics interior|exterior]
//		 [view grid|axes|title|border|vertical yes|no]
//		 [view axes numbers|tickmarks|label yes|no]
//	Example:
//	$xpaget ds9 grid
//	$xpaget ds9 grid system
//	$xpaget ds9 grid sky
//	$xpaget ds9 grid skyformat
//	$xpaget ds9 grid type
//	$xpaget ds9 grid type axes
//	$xpaget ds9 grid type numerics
//	$xpaget ds9 grid view grid
//	$xpaget ds9 grid view axes
//	$xpaget ds9 grid view axes numbers
//	$xpaget ds9 grid view axes tickmarks
//	$xpaget ds9 grid view axes label
//	$xpaget ds9 grid view title
//	$xpaget ds9 grid view border
//	$xpaget ds9 grid view vertical
//	$xpaset -p ds9 grid yes
//	$xpaset -p ds9 grid load foo.grd
//	$xpaset -p ds9 grid save foo.grd
//	$xpaset -p ds9 grid system wcs
//	$xpaset -p ds9 grid sky fk5
//	$xpaset -p ds9 grid skyformat degrees
//	$xpaset -p ds9 grid type analysis
//	$xpaset -p ds9 grid type axes interior
//	$xpaset -p ds9 grid type numerics interior
//	$xpaset -p ds9 grid view grid yes
//	$xpaset -p ds9 grid view axes yes
//	$xpaset -p ds9 grid view axes numbers yes
//	$xpaset -p ds9 grid view axes tickmarks yes
//	$xpaset -p ds9 grid view axes label yes
//	$xpaset -p ds9 grid view title yes
//	$xpaset -p ds9 grid view border yes
//	$xpaset -p ds9 grid view vertical no
//
//
//	iconify
//	Toggles iconification.
//	Syntax:
//	iconify [yes|no]
//	Example:
//	$xpaget ds9 iconify
//	$xpaset -p ds9 iconify yes
//
//
//	iis
//	Set/Get IIS Filename.
//	Syntax:
//	iis [filename <filename> [#]]
//	Example:
//	$xpaget ds9 iis filename
//	$xpaget ds9 iis filename 4
//	$xpaset -p ds9 iis filename foo.fits
//	$xpaset -p ds9 iis filename bar.fits 4
//
//
//	imexam
//	Interactive examine function. A blinking cursor will indicate to the user to click on a point on an image. The specified information will be returned at that time.
//	Syntax:
//	imexam [coordinate <coordinate system> [<sky frame>] [<sky format>]]
//		   [data [width][height]]
//	Example:
//	$xpaget ds9 imexam coordinate image
//	$xpaget ds9 imexam coordinate wcs fk5 degrees
//	$xpaget ds9 imexam coordinate wcs galactic sexagesimal
//	$xpaget ds9 imexam coordinate fk5
//	$xpaget ds9 imexam data # return data value
//	$xpaget ds9 imexam data 3 3 # return all data in 3x3 box about selected point
// 
//
//   lock
//	Lock frames.
//	Syntax:
//	lock [crosshairs none|wcs|wcsa...wcsz|physical|image]
//	Example:
//	$xpaset -p ds9 lock crosshairs wcs
//
//
//	lower
//	Will lower ds9 in the window stacking order.
//	Syntax:
//	lower
//	Example:
//	$xpaset -p ds9 lower
//
//
//	match
//	Match all other frames to the current frame.
//	Syntax:
//	match [frames wcs|physical|image]
//		  [colorbars]
//		  [scales]
//	Example:
//	$xpaset -p ds9 match frames wcs
//	$xpaset -p ds9 match colorbars
//	$xpaset -p ds9 match scales
//
//
//	minmax
//	This is how DS9 determines  the min and max data values from the data. SCAN will scan all data. SAMPLE will sample the data every n samples. DATAMIN and IRAFMIN will use the values of the keywords if present. In general, it is recommended to use SCAN unless your computer is slow or your data files are very large. Select the increment  interval for determining the min and max data values during sampling. The larger the interval, the quicker the process.
//	Syntax:
//	minmax [scan|sample|datamin|irafmin]
//		   [mode scan|sample|datamin|irafmin]
//		   [interval <value>]
//	Example:
//	$xpaget ds9 minmax mode
//	$xpaget ds9 minmax interval
//	$xpaset -p ds9 minmax scan
//	$xpaset -p ds9 minmax mode sample
//	$xpaset -p ds9 minmax interval 10
// 
//
//   mode
//	Controls the first mouse button mode.
//	Syntax:
//	mode [none|pointer|crosshair|colorbar|pan|zoom|rotate|examine]
//	Example:
//	$xpaget ds9 mode
//	$xpaset -p ds9 mode crosshair
//	nameserver
//	Support Name Server functions. Coordinates are in fk5.
//	Syntax:
// 
//
//   nameserver [<object>]
//			   [name <object>]
//			   [server ned-sao|ned-eso|simbad-sao|simbad-eso]
//			   [skyformat degrees|sexagesimal]
//
//	Example:
//	$xpaget ds9 nameserver server
//	$xpaget ds9 nameserver skyformat
//	$xpaset -p ds9 nameserver m31
//	$xpaset -p ds9 nameserver name m31
//	$xpaset -p ds9 nameserver server ned-sao
//	$xpaset -p ds9 nameserver skyformat sexagesimal
// 
//
//   orient
//	Controls the orientation of the current frame.
//	Syntax:
//	orient [none|x|y|xy]
//	Example:
//	$xpaget ds9 orient
//	$xpaset -p ds9 orient xy
// 
//
//   page setup
//	Controls Page Setup options.
//	Syntax:
//	page setup [orientation portrait|landscape]
//			   [pagescale scaled|fixed]
//			   [pagesize letter|legal|tabloid|poster|a4]
//	Example:
//	$xpaget ds9 page setup orientation
//	$xpaget ds9 page setup pagescale
//	$xpaget ds9 page setup pagesize
//	$xpaset -p ds9 page setup orientation portrait
//	$xpaset -p ds9 page setup pagescale scaled
//	$xpaset -p ds9 page setup pagesize poster
// 
//
//   pan
//	Controls the current image cursor location for the current frame.
//	Syntax:
//	pan [x y <coordinate system> [<sky frame>][<sky format>]]
//		[to x y <coordinate system> [sky frame>][<sky format>]
//	Example:
//	$xpaget ds9 pan # get current image coords
//	$xpaget ds9 pan wcs fk4 sexagesimal # get current wcs coords
//	$xpaset -p ds9 pan 200 200 image # pan relative
//	$xpaset -p ds9 pan to 400 400 physical # pan to physical coords
//	$xpaset -p ds9 pan to 13:29:55 47:11:50 wcs fk5 # pan to wcs coords
// 
//
//   pixeltable
//	Display/Hide the pixel table.
//	Syntax:
//	pixeltable [yes|no]
//	Example:
//	$xpaget ds9 pixeltable
//	$xpaset -p ds9 pixeltable yes
//
//
//	plot
//	Display and configure data plots. All plot commands take an optional second command, the plot name. Use xpaget plot to retreive all plot names. If no plot name is specified, the last plot created is assumed. Plot data is assumed to be a pair of coordinates, with optional error values. The follow are valid data descriptions:
//
//		xy        x and y coordinates
//		xyex      x,y coordinates with x errors
//		xyey      x,y coordinates with y errors
//		xyexey    x,y coordinates with both x and y errors
//
//	To create a new plot, use the plot new command. If the second arg is stdin, the title, x axis title, y axis title, and dimension are assumed to be on the first line of the data.
//
//	Syntax:
//	# create new empty plot window
//	plot []
//		 [new [name <plotname>]]
//
//	# create new plot with data
//	plot [new [name <plotname>] stdin]
//		 [new [name <plotname>] <title> <xaxis label> <yaxis label>  xy|xyex|xyey|xyexey]
//
//	# load additional dataset into an existing plot
//	plot [<plotname>] [data xy|xyex|xyey|xyexey]
//
//	# edit existing plot
//	plot [<plotname>] [close]
//		 [<plotname>] [clear]
//
//		 [<plotname>] [load <filename> xy|xyex|xyey|xyexey]
//		 [<plotname>] [save <filename>]
//		 [<plotname>] [loadconfig <filename>]
//		 [<plotname>] [saveconfig <filename>]
//
//		 [<plotname>] [print]
//		 [<plotname>] [print destination printer|file]
//		 [<plotname>] [print command <command>]
//		 [<plotname>] [print filename <filename>]
//		 [<plotname>] [print palette color|gray|mono]
//		 [<plotname>] [page orientation portrait|landscape]
//		 [<plotname>] [page pagescale scaled|fixed]
//		 [<plotname>] [page pagesize letter|legal|tabloid|poster|a4]
//
//		 [<plotname>] [graph grid yes|no]
//		 [<plotname>] [graph scale linearlinear|linearlog|loglinear|loglog]
//		 [<plotname>] [graph range x|y auto yes|no]
//		 [<plotname>] [graph range x|y min <value>]
//		 [<plotname>] [graph range x|y max <value>]
//		 [<plotname>] [graph labels title|xaxis|yaxis <value>]
//
//		 [<plotname>] [font numbers|labels|title font times|helvetica|symbol|courier]
//		 [<plotname>] [font numbers|labels|title size <value>]
//		 [<plotname>] [font numbers|labels|title style plain|bold|italic]
//
//	# edit current dataset
//	plot [<plotname>] [dataset #]
//		 [<plotname>] [view discrete|line|step|quadratic|errorbar yes|no]
//		 [<plotname>] [color discrete|line|step|quadratic|errorbar <color>]
//		 [<plotname>] [line discrete circle|diamond|plus|cross]
//		 [<plotname>] [line line|step|quadratic|errorbar width <value>]
//		 [<plotname>] [line line|step|quadratic dash yes|no]
//		 [<plotname>] [line errorbar style 1|2]
//
//	Example:
//	$xpaget ds9 plot # return all plotnames
//
//	# create new empty plot window
//	$xpaset -p ds9 plot
//	$xpaset -p ds9 plot new
//	$xpaset -p ds9 plot new name foo
//
//	# create new plot with data
//	$cat foo.dat | xpaset ds9 plot new stdin
//	$cat foo.dat | xpaset ds9 plot new name foo stdin
//	$cat bar.dat | xpaset ds9 plot new "{The Title}" "{X}" "{Y}" xy
//	$cat bar.dat | xpaset ds9 plot new name foo "{The Title}" "{X}" "{Y}" xy
//
//	# load additional dataset into an existing plot
//	$cat bar.dat | xpaset ds9 plot data xy # plot additional data
//	$cat bar.dat | xpaset ds9 plot foo data xy # plot additional data
//
//	# edit existing plot
//	$xpaset -p ds9 plot close # close last plot
//	$xpaset -p ds9 plot foo close # close plot foo
//	$xpaset -p ds9 plot clear # clear all datasets
//
//	$xpaset -p ds9 plot load foo.dat xy # load new dataset with dimension xy
//	$xpaset -p ds9 plot save bar.dat # save current dataset
//	$xpaset -p ds9 plot loadconfig foo.plt # load plot configuration
//	$xpaset -p ds9 plot saveconfig bar.plt # save current plot configuration
//
//	$xpaset -p ds9 plot print
//	$xpaset -p ds9 plot print destination file
//	$xpaset -p ds9 plot print command "lp"
//	$xpaset -p ds9 plot print filename "foo.ps"
//	$xpaset -p ds9 plot print palette gray
//	$xpaset -p ds9 plot page orientation portrait
//	$xpaset -p ds9 plot page pagescale scaled
//	$xpaset -p ds9 plot page pagesize letter
//
//	$xpaset -p ds9 plot graph grid yes
//	$xpaset -p ds9 plot graph scale loglog
//	$xpaset -p ds9 plot graph range x auto yes
//	$xpaset -p ds9 plot graph range x min 0
//	$xpaset -p ds9 plot graph range x max 100
//	$xpaset -p ds9 plot graph range y auto yes
//	$xpaset -p ds9 plot graph range y min 0
//	$xpaset -p ds9 plot graph range y max 100
//	$xpaset -p ds9 plot graph labels title {The Title}
//	$xpaset -p ds9 plot graph labels xaxis {X}
//	$xpaset -p ds9 plot graph labels yaxis {Y}
//
//	$xpaset -p ds9 plot font numbers font times
//	$xpaset -p ds9 plot font numbers size 12
//	$xpaset -p ds9 plot font numbers style bold
//	$xpaset -p ds9 plot font labels font times
//	$xpaset -p ds9 plot font title font times
//
//	# edit current dataset
//	$xpaset -p ds9 plot dataset 2 # set current dataset to the second dataset loaded
//	$xpaset -p ds9 plot view discrete yes
//	$xpaset -p ds9 plot color discrete red
//	$xpaset -p ds9 plot line discrete cross
//	$xpaset -p ds9 plot line step width 2
//	$xpaset -p ds9 plot line step dash yes
//	$xpaset -p ds9 plot line errorbar style 2
//
//
//	prefs
//	Controls various preference settings.
//	Syntax:
//	prefs [mosaicfast yes|no]
//		  [bgcolor white|black|red|green|blue|cyan|magenta|yellow]
//		  [nancolor white|black|red|green|blue|cyan|magenta|yellow]
//	Example:
//	$xpaget ds9 prefs mosaicfast
//	$xpaget ds9 prefs bgcolor
//	$xpaget ds9 prefs nancolor
//	$xpaget ds9 prefs wcsprojection
//	$xpaset -p ds9 prefs mosaicfast no
//	$xpaset -p ds9 prefs bgcolor black
//	$xpaset -p ds9 prefs nancolor red
//
//
//	preserve
//	Preserve the follow attributes while loading a new image.
//	Syntax:
//	preserve [scale yes|no]
//			 [pan yes|no]
//			 [regions yes|no]
//	Example:
//	$xpaget ds9 preserve scale
//	$xpaget ds9 preserve pan
//	$xpaget ds9 preserve regions
//	$xpaset -p ds9 preserve scale yes
//	$xpaset -p ds9 preserve pan yes
//	$xpaset -p ds9 preserve regions yes
//
//
//	print
//	Controls printing. Use print option to set printing options. Use print to actually print.
//	Syntax:
//	print [destination printer|file]
//		  [command <command>]
//		  [filename <filename>]
//		  [palette rgb|cmyk|gray]
//		  [level 1|2]
//		  [interpolate yes|no]
//		  [resolution 53|72|75|150|300|600]
//	Example:
//	$xpaget ds9 print destination
//	$xpaget ds9 print command
//	$xpaget ds9 print filename
//	$xpaget ds9 print palette
//	$xpaget ds9 print level
//	$xpaget ds9 print interpolate
//	$xpaget ds9 print resolution
//	$xpaset -p ds9 print
//	$xpaset -p ds9 print destination file
//	$xpaset -p ds9 print command '{gv -}'
//	$xpaset -p ds9 print filename foo.ps
//	$xpaset -p ds9 print palette cmyk
//	$xpaset -p ds9 print level 2
//	$xpaset -p ds9 print interpolate no
//	$xpaset -p ds9 print resolution 75
//
//
//	quit
//	Exit DS9.
//	Syntax:
//	quit
//	Example:
//	$xpaset -p ds9 quit
//
//
//	raise
//	Will raise ds9 in the window stacking order.
//	Syntax:
//	raise
//	Example:
//	$xpaset -p ds9 raise
//
//
//	regions
//	Controls regions in the current frame.
//	Syntax:
//	regions [fg|bg] [move front]
//			[fg|bg] [move back]
//			[fg|bg] [select all]
//			[fg|bg] [select none]
//			[fg|bg] [select group <groupname>]
//			[fg|bg] [delete all]
//			[fg|bg] [delete select]
//			[fg|bg] [load <filename>]
//			[fg|bg] [save <filename>]
//
//			[format]
//			[-format ds9|ciao|saotng|saoimage|pros|xy]
//			[system]
//			[-system image|physical|wcs|wcsa...wcsz]
//			[sky]
//			[-sky fk4|fk5|icrs|galactic|ecliptic]
//			[skyformat]
//			[-skyformat degrees|sexagesimal]
//			[strip]
//			[-strip yes|no]
//			[wcs]
//			[-wcs]
//			[shape <shape>]
//			[color white|black|red|green|blue|cyan|magenta|yellow]
//			[width <width>]
//			[delim [nl|<char>]]
//
//			[fg|bg] [include|exclude|source|background|selected]
//			[fg|bg] [-prop select|edit|move|rotate|delete|fixed|include|source 1|0]
//
//			[fg|bg] [groups]
//			[fg|bg] [-group <tag>]
//			[fg|bg] [group <tag> color [black|white|red|green|blue|cyan|magenta|yellow]]
//			[fg|bg] [group <tag> copy]
//			[fg|bg] [group <tag> delete]
//			[fg|bg] [group <tag> cut]
//			[fg|bg] [group <tag> font <font>]
//			[fg|bg] [group <tag> move <int> <int>]
//			[fg|bg] [group <tag> movefront]
//			[fg|bg] [group <tag> moveback]
//			[fg|bg] [group <tag> property <property> [yes|no]]
//			[fg|bg] [group <tag> select]
//	 Example:
//	$xpaget ds9 regions
//	$xpaget ds9 regions bg
//	$xpaget ds9 regions -format ds9 -coord wcs -sky fk5 -skyformat sexagesimal -prop edit 1 -group foo
//
//	$xpaget ds9 regions format
//	$xpaget ds9 regions system
//	$xpaget ds9 regions sky
//	$xpaget ds9 regions skyformat
//	$xpaget ds9 regions strip
//	$xpaget ds9 regions wcs
//	$xpaget ds9 regions shape
//	$xpaget ds9 regions color
//	$xpaget ds9 regions width
//	$xpaget ds9 regions delim
//	$xpaget ds9 regions source
//	$xpaget ds9 regions background
//	$xpaget ds9 regions include
//	$xpaget ds9 regions exclude
//	$xpaget ds9 regions selected
//	$xpaget ds9 regions -prop select yes
//	$xpaget ds9 regions groups
//	$xpaget ds9 regions -group foo
//
//	$echo "image; circle 100 100 20" | xpaset ds9 regions
//	$echo "image; circle 100 100 20" | xpaset ds9 regions bg
//	$echo "fk5; circle 13:29:55 47:11:50 .5'" | xpaset ds9 regions
//	$echo "physical; ellipse 100 100 20 40" | xpaset ds9 regions
//	$echo "box 100 100 20 40 25" | xpaset ds9 regions
//	$echo "image; line 100 100 200 400" | xpaset ds9 regions
//	$echo "physical; ruler 200 300 200 400" | xpaset ds9 regions
//	$echo "image; text 100 100 # text={Hello, World}" | xpaset ds9 regions
//	$echo "fk4; boxcircle point 13:29:55 47:11:50" | xpaset ds9 regions
//
//	$xpaset -p ds9 regions move back
//	$xpaset -p ds9 regions move front
//	$xpaset -p ds9 regions select all
//	$xpaset -p ds9 regions select none
//	$xpaset -p ds9 regions select group foo
//	$xpaset -p ds9 regions delete all
//	$xpaset -p ds9 regions delete select
//	$xpaset -p ds9 regions load foo.reg
//	$xpaset -p ds9 regions load regions.fits
//	$xpaset -p ds9 regions save foo.reg
//	$xpaset -p ds9 regions format ds9
//	$xpaset -p ds9 regions system wcs
//	$xpaset -p ds9 regions sky fk5
//	$xpaset -p ds9 regions skyformat degrees
//	$xpaset -p ds9 regions delim nl
//	$xpaset -p ds9 regions strip yes
//	$xpaset -p ds9 regions wcs yes
//	$xpaset -p ds9 regions shape ellipse
//	$xpaset -p ds9 regions color red
//	$xpaset -p ds9 regions width 3
//
//	$xpaset -p ds9 regions group foo color red
//	$xpaset -p ds9 regions group foo copy
//	$xpaset -p ds9 regions group foo delete
//	$xpaset -p ds9 regions group foo cut
//	$xpaset -p ds9 regions group foo font {times 14 bold}
//	$xpaset -p ds9 regions group foo move 100 100
//	$xpaset -p ds9 regions group foo movefront
//	$xpaset -p ds9 regions group foo moveback
//	$xpaset -p ds9 regions group foo property delete no
//	$xpaset -p ds9 regions group foo select
//
//
//	rgb
//	Create RGB frame and control RGB frame parameters.
//	Syntax:
//	rgb  # empty
//		 [red|green|blue]
//		 [channel [red|green|blue]]
//		 [view [red|green|blue] [yes|no]]
//		 [system <coordinate system>]
//		 [lock scale|bin|colorbar [yes|no]]
//	Example:
//	$xpaget ds9 rgb channel
//	$xpaget ds9 rgb lock bin
//	$xpaget ds9 rgb lock scale
//	$xpaget ds9 rgb lock colorbar
//	$xpaget ds9 rgb system
//	$xpaget ds9 rgb view red
//	$xpaget ds9 rgb view green
//	$xpaget ds9 rgb view blue
//	$xpaset -p ds9 rgb # create new rgb frame
//	$xpaset -p ds9 rgb red # set current channel to red
//	$xpaset -p ds9 rgb channel red # set current channel to red
//	$xpaset -p ds9 rgb view blue no # turn off blue channel
//	$xpaset -p ds9 rgb system wcs # set rgb coordinate system
//	$xpaset -p ds9 rgb lock scale yes # lock rgb channels for scaling
//	$xpaset -p ds9 rgb lock bin yes # lock rgb channels for binning
//	$xpaset -p ds9 rgb lock colorbar yes # lock rgb colorbar channels
//
//
//	rotate
//	Controls the rotation angle (in degrees) of the current frame.
//	Syntax:
//	rotate [<value>]
//		   [to <value>]
//	Example:
//	$xpaget ds9 rotate
//	$xpaset -p ds9 rotate 45
//	$xpaset -p ds9 rotate to 30
// 
//
//   saveimage
//	Save visible image(s) as a raster. If image is a data cube, the mpeg option will cycle thru each slice creating a mpeg movie.
//	Syntax:
//	saveimage [fits|jpeg|tiff|png|ppm|mpeg <filename>]
//	Example:
//	$xpaset -p ds9 saveimage jpeg ds9.jpg
//
//	savefits
//	Save current frame data as FITS. This differs from SAVEIMAGE in that the entire image of the current frame is saved as a FITS, without graphics.
//	Syntax:
//	savefits [<filename>]
//	Example:
//	$xpaset -p ds9 savefits ds9.fits
//
//	savempeg
//	Save all active frames as a mpeg movie.
//	Syntax:
//	savempeg [<filename>]
//	Example:
//	$xpaset -p ds9 savempeg ds9.mpg
//
//	scale
//	Controls the limits, color scale distribution, and use of DATASEC keyword.
//	Syntax:
//	scale [linear|log|squared|sqrt|histequ]
//		  [datasec yes|no]
//		  [limits <minvalue> <maxvalue>]
//		  [mode minmax|<value>|zscale|zmax]
//		  [scope local|global]
//	Example:
//	$xpaget ds9 scale
//	$xpaget ds9 scale datasec
//	$xpaget ds9 scale limits
//	$xpaget ds9 scale mode
//	$xpaget ds9 scale scope
//	$xpaset -p ds9 scale datasec yes
//	$xpaset -p ds9 scale histequ
//	$xpaset -p ds9 scale limits 1 100
//	$xpaset -p ds9 scale mode zscale
//	$xpaset -p ds9 scale mode 99.5
//	$xpaset -p ds9 scale scope local
//
//
//	shm
//	Load a shared memory segment into the current frame.
//	Syntax:
//	shm [<key> [<filename>]]
//		[key <key> [<filename>]]
//		[shmid <id> [<filename>]]
//
//		[fits [key|shmid] <id> [<filename>]]
//		[sfits [key|shmid] <id> <id> [<filename>]]
//		[mosaicimage [iraf|wcs|wcsa...wcsz|wfpc2] [key|shmid] <id> [<filename>]]
//		[mosaicimagenext [wcs|wcsa...wcsz] [key|shmid] <id> [<filename>]]
//		[mosaic [iraf|wcs|wcsa...wcsz] [key|shmid] <id> [<filename>]]
//		[smosaic [iraf|wcs|wcsa...wcsz] [key|shmid] <id> [<filename>]]
//		[rgbcube [key|shmid] <id> [<filename>]
//		[srgbcube [key|shmid] <id> [<filename>]
//		[rgbimage [key|shmid] <id> [<filename>]]
//		[rgbarray [key|shmid] <id> [xdim=<x>,ydim=<y>|dim=<dim>,zdim=3],bitpix=<b>,[skip=<s>]]
//		[array [key|shmid] <id> [xdim=<x>,ydim=<y>|dim=<dim>],bitpix=<b>,[skip=<s>]]
//		[startload|finishload]
//	Example:
//	$xpaget ds9 shm
//	$xpaset -p ds9 shm 102
//	$xpaset -p ds9 shm key 102
//	$xpaset -p ds9 shm shmid 102 foo
//
//	$xpaset -p ds9 shm fits key 100 foo
//	$xpaset -p ds9 shm sfits key 100 101 foo
//	$xpaset -p ds9 shm mosaicimage iraf key 100 foo
//	$xpaset -p ds9 shm mosaicimage wcs key 100 foo
//	$xpaset -p ds9 shm mosaicimage wcsa key 100 foo
//	$xpaset -p ds9 shm mosaicimage wfpc2 key 100 foo
//	$xpaset -p ds9 shm mosaicimagenext wcs key 100 foo
//	$xpaset -p ds9 shm mosaic iraf key 100 foo
//	$xpaset -p ds9 shm mosaic wcs key 100 foo
//	$xpaset -p ds9 shm smosaic wcs key 100 101 foo
//	$xpaset -p ds9 shm rgbcube key 100 foo
//	$xpaset -p ds9 shm srgbcube key 100 101 foo
//	$xpaset -p ds9 shm rgbimage key 100 foo
//	$xpaset -p ds9 shm rgbarray key 100 [dim=200,zdim=3,bitpix=-32]
//	$xpaset -p ds9 shm array shmid 102 [dim=32,bitpix=-32]
//	$xpaset -p ds9 shm startload # start a multiple load sequence without updating the display
//	$xpaset -p ds9 shm finishload # finish multiple load sequence
//
//	single
//	Select Single Display mode
//	Syntax:
//	single
//	Example:
//	$xpaget ds9 single
//	$xpaset -p ds9 single
//	 
//	source
//	Source tcl code from a file.
//	Syntax:
//	source [filename]
//	Example:
//	$xpaset -p ds9 source foo.tcl
// 
//
//   tcl
//	Execute one tcl command
//	Syntax:
//	tcl [<tcl command>]
//	Example:
//	$echo 'puts "Hello, World"' | xpaset ds9 tcl
// 
//
//   tile
//	Controls the tile display mode.
//	Syntax:
//	tile [yes|no]
//		 [mode grid|column|row]
//		 [grid]
//		 [grid mode [automatic|manual]]
//		 [grid layout <row> <col>]
//		 [grid gap <pixels>]
//		 [row]
//		 [column]
//	Example:
//	$xpaget ds9 tile
//	$xpaget ds9 tile mode
//	$xpaget ds9 tile grid mode
//	$xpaget ds9 tile grid layout
//	$xpaget ds9 tile grid gap
//	$xpaset -p ds9 tile
//	$xpaset -p ds9 tile mode row
//	$xpaset -p ds9 tile grid
//	$xpaset -p ds9 tile grid mode manual
//	$xpaset -p ds9 tile grid layout 5 5
//	$xpaset -p ds9 tile grid gap 10
//	$xpaset -p ds9 tile row
//	$xpaset -p ds9 tile column
// 
//
//   update
//	Updates the current frame or region of frame. In the second form, the first argument is the number of the fits HDU (starting with 1) and the remaining args are a bounding box in IMAGE coordinates. By default, the screen is updated the next available idle cycle. However, you may force an immediate update by specifying the NOW option.
//	Syntax:
//	update []
//		   [# x1 y1 x2 y2]
//		   [now]
//		   [now # x1 y1 x2 y2]
//		   [on]
//		   [off]
//	Example:
//	$xpaset -p ds9 update
//	$xpaset -p ds9 update 1 100 100 300 400
//	$xpaset -p ds9 update now
//	$xpaset -p ds9 update now 1 100 100 300 400
//	$xpaset -p ds9 update off # delay refreash of the screen while loading files
//	$xpaset -p ds9 update on # be sure to turn it on when you are finished loading
//
//
//	version
//	Returns the current version of DS9.
//	Syntax:
//	version
//	Example:
//	$xpaget ds9 version
// 
//
//   view
//	Controls the GUI.
//	Syntax:
//	view  [info yes|no]
//		  [panner yes|no]
//		  [magnifier yes|no]
//		  [buttons yes|no]
//		  [colorbar yes|no]
//		  [horzgraph yes|no]
//		  [vertgraph yes|no]
//		  [filename yes|no[
//		  [object yes|no]
//		  [minmax yes|no]
//		  [frame yes|no]
//		  [image|physical|wcs|wcsa...wcsz yes|no]
//		  [red yes|no]
//		  [green yes|no]
//		  [blue yes|no]
//	Example:
//	$xpaget ds9 view info
//	$xpaget ds9 view horzgraph
//	$xpaget ds9 view wcsa
//	$xpaset -p ds9 view info no
//	$xpaset -p ds9 view horzgraph yes
//	$xpaset -p ds9 view wcsa yes
//	$xpaset -p ds9 view red yes
//	$xpaset -p ds9 view green no
//	$xpaset -p ds9 view blue yes
//
//
//	vo
//	Invoke an connection to a Virtual Observatory site.
//	Syntax:
//	vo <url>
//	Example:
//	$xpaget ds9 vo
//	$xpaset -p ds9 vo chandra-ed.havard.edu
// 
//
//   wcs
//	Controls the World Coordinate System for the current frame. If the wcs system, skyframe, or skyformat is modified, the info panel, compass, grid, and alignment will be modified accordingly. Also, using this access point, a new WCS specification can be loaded and used by the current image regardless of the WCS that was contained in the image file. WCS specification can be sent to DS9 as an ASCII file . Please see WCS for more information.
//
//	Syntax:
//	wcs [wcs|wcsa...wcsz]
//		[system wcs|wcsa...wcsz]
//		[sky fk4|fk5|icrs|galactic|ecliptic]
//		[skyformat degrees|sexagesimal]
//		[align yes|no]
//		[reset]
//		[replace file <filename>]
//		[append file <filename>]
//		[replace]
//		[append]
//	Example:
//	$xpaget ds9 wcs
//	$xpaget ds9 wcs system
//	$xpaget ds9 wcs sky
//	$xpaget ds9 wcs skyformat
//	$xpaget ds9 wcs align
//	$xpaset -p ds9 wcs wcs
//	$xpaset -p ds9 wcs system wcs
//	$xpaset -p ds9 wcs sky fk5
//	$xpaset -p ds9 wcs skyformat sexagesimal
//	$xpaset -p ds9 wcs align yes
//	$xpaset -p ds9 wcs reset
//	$xpaset -p ds9 wcs replace file foo.wcs
//	$xpaset -p ds9 wcs append file foo.wcs
//	$cat foo.wcs | xpaset ds9 wcs replace
//	$cat foo.wcs | xpaset ds9 wcs append
//	$echo "OBJECT = 'foobar'" | xpaset ds9 wcs append
// 
//
//   web
//	Display specified URL in the web display.
//	Syntax:
//	web <url>
//	Example:
//	$xpaget ds9 web
//	$xpaset -p ds9 web <url>
// 
//
//   zoom
//	Controls the current zoom value for the current frame.
//	Syntax:
//	zoom [<value>]
//		 [to <value>]
//		 [to fit]
//	Example:
//	$xpaget ds9 zoom
//	$xpaset -p ds9 zoom 2
//	$xpaset -p ds9 zoom to 4
//	$xpaset -p ds9 zoom to fit
//
//***********************************************************************/
