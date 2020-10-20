// ArcDisplayCAPI.cpp : Defines the exported functions for the DLL application.
//
#include <memory>
#include <string>
#include <stdexcept>

#include <ArcDisplayCAPI.h>
#include <CArcDisplay.h>


// +------------------------------------------------------------------------------------+
// | Globals                                                                            |
// +------------------------------------------------------------------------------------+
static std::unique_ptr<arc::gen3::CArcDisplay> g_pCDisplay( static_cast<arc::gen3::CArcDisplay*>( nullptr ) );
static char g_szErrMsg[ ARC_ERROR_MSG_SIZE ];


// +------------------------------------------------------------------------------------+
// | Define system dependent macros                                                     |
// +------------------------------------------------------------------------------------+
#ifndef ARC_SPRINTF
#define ARC_SPRINTF

#ifdef _WINDOWS
	#define ArcSprintf( dst, size, fmt, msg )	sprintf_s( dst, size, fmt, msg )
#else
	#define ArcSprintf( dst, size, fmt, msg )	sprintf( dst, fmt, msg )
#endif

#endif


// +------------------------------------------------------------------------------------+
// | Verify class pointer macro                                                         |
// +------------------------------------------------------------------------------------+
#define VERIFY_CLASS_PTR( fnc, ptr )													\
						if ( ptr.get() == NULL )										\
						{																\
							ptr.reset( new arc::gen3::CArcDisplay() );					\
																						\
							if ( ptr.get() == NULL )									\
							{															\
								throw std::runtime_error( std::string( "( " ) + fnc +	\
								std::string( " ): Invalid class pointer!" ) );			\
							}															\
						}


// +------------------------------------------------------------------------------------+
// | Launch                                                                             |
// +------------------------------------------------------------------------------------+
// |  Attempts to launch the DS9 program.                                               |
// |                                                                                    |
// |  Throws std::runtime_error if DS9 cannot be started or found.                      |
// |                                                                                    |
// |  <IN> -> dMSDelay - The amount of time to delay before attempting to               |
// |                     access the application. Default: 1 second                      |
// |  <OUT> -> pStatus - Status equals ARC_STATUS_OK or ARC_STATUS_ERROR                |
// +------------------------------------------------------------------------------------+
GEN3_CARCDISPLAY_API void ArcDisplay_Launch( int dMSDelay, int* pStatus )
{
	*pStatus = ARC_STATUS_OK;

	try
	{
		VERIFY_CLASS_PTR( "ArcDisplay_Launch", g_pCDisplay )

		g_pCDisplay.get()->launch( dMSDelay );
	}
	catch ( std::exception& e )
	{
		*pStatus = ARC_STATUS_ERROR;

		ArcSprintf( g_szErrMsg, ARC_ERROR_MSG_SIZE, "%s", e.what() );
	}
}


//// +------------------------------------------------------------------------------------+
//// | Show                                                                               |
//// +------------------------------------------------------------------------------------+
//// |  Displays the specified image data in DS9.                                         |
//// |                                                                                    |
//// |  Throws std::runtime_error on input parameter error or on cfitsio library          |
//// |  error.                                                                            |
//// |                                                                                    |
//// |  <IN>  -> pBuffer - Image data buffer to display.                                  |
//// |  <IN>  -> dRows   - Number of rows in image buffer.                                |
//// |  <IN>  -> dCols   - Number of columns in image buffer.                             |
//// |  <OUT> -> pStatus - Status equals ARC_STATUS_OK or ARC_STATUS_ERROR                |
//// +------------------------------------------------------------------------------------+
//DLLDISPLAY_API void ArcDisplay_Show( void *pBuffer, int dRows, int dCols, int* pStatus )
//{
//	*pStatus = ARC_STATUS_OK;
//
//	try
//	{
//		VERIFY_CLASS_PTR( "ArcDisplay_Show", g_pCDisplay )
//
//		g_pCDisplay.get()->Show( pBuffer, dRows, dCols );
//	}
//	catch ( exception& e )
//	{
//		*pStatus = ARC_STATUS_ERROR;
//
//		ArcSprintf( g_szErrMsg, ARC_ERROR_MSG_SIZE, "%s", e.what() );
//	}
//}


// +------------------------------------------------------------------------------------+
// | ShowFits                                                                           |
// +------------------------------------------------------------------------------------+
// |  Displays the specified FITS image data in DS9.                                    |
// |                                                                                    |
// |  Throws std::runtime_error on input parameter error or on cfitsio library          |
// |  error.                                                                            |
// |                                                                                    |
// |  <IN>  -> sFitsFile - FITS file to display.                                        |
// |  <OUT> -> pStatus   - Status equals ARC_STATUS_OK or ARC_STATUS_ERROR              |
// +------------------------------------------------------------------------------------+
GEN3_CARCDISPLAY_API void ArcDisplay_ShowFits( const char* pszFitsFile, int* pStatus )
{
	*pStatus = ARC_STATUS_OK;

	try
	{
		VERIFY_CLASS_PTR( "ArcDisplay_ShowFits", g_pCDisplay )

		g_pCDisplay.get()->show( std::string( pszFitsFile ) );
	}
	catch ( std::exception& e )
	{
		*pStatus = ARC_STATUS_ERROR;

		ArcSprintf( g_szErrMsg, ARC_ERROR_MSG_SIZE, "%s", e.what() );
	}
}


// +------------------------------------------------------------------------------------+
// | Clear                                                                              |
// +------------------------------------------------------------------------------------+
// |  Clears the specified frame or all if no frame number is specified.                |
// |                                                                                    |
// |  Throws std::runtime_error on input parameter error or on cfitsio library          |
// |  error.                                                                            |
// |                                                                                    |
// |  <IN>  -> dFrame  - Frame number to clear or -1 to clear all ( default ).          |
// |  <OUT> -> pStatus - Status equals ARC_STATUS_OK or ARC_STATUS_ERROR                |
// +------------------------------------------------------------------------------------+
GEN3_CARCDISPLAY_API void ArcDisplay_Clear( int dFrame, int* pStatus )
{
	*pStatus = ARC_STATUS_OK;

	try
	{
		VERIFY_CLASS_PTR( "ArcDisplay_Clear", g_pCDisplay )

			g_pCDisplay.get()->clear( dFrame );
	}
	catch ( std::exception& e )
	{
		*pStatus = ARC_STATUS_ERROR;

		ArcSprintf( g_szErrMsg, ARC_ERROR_MSG_SIZE, "%s", e.what() );
	}
}


// +------------------------------------------------------------------------------------+
// | Terminate                                                                          |
// +------------------------------------------------------------------------------------+
// |  Forces the termination of the display process.                                    |
// |                                                                                    |
// |  Throws std::runtime_error on input parameter error or on cfitsio library          |
// |  error.                                                                            |
// |                                                                                    |
// |  <OUT> -> pStatus - Status equals ARC_STATUS_OK or ARC_STATUS_ERROR                |
// +------------------------------------------------------------------------------------+
GEN3_CARCDISPLAY_API void ArcDisplay_Terminate( int* pStatus )
{
	*pStatus = ARC_STATUS_OK;

	try
	{
		VERIFY_CLASS_PTR( "ArcDisplay_Terminate", g_pCDisplay )

		g_pCDisplay.get()->terminate();
	}
	catch ( std::exception& e )
	{
		*pStatus = ARC_STATUS_ERROR;

		ArcSprintf( g_szErrMsg, ARC_ERROR_MSG_SIZE, "%s", e.what() );
	}
}


// +--------------------------------------------------------------------------------------------
// | GetLastError
// +--------------------------------------------------------------------------------------------
// | Returns the last error message reported.
// +--------------------------------------------------------------------------------------------
GEN3_CARCDISPLAY_API const char* ArcDisplay_GetLastError()
{
	return const_cast< const char* >( g_szErrMsg );
}
