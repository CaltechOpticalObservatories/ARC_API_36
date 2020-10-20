// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  CArcDisplay.h  ( Gen3 )                                                                                  |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE: This file defines the standard ARC display interface.                                                  |
// |                                                                                                                  |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 26, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2013 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+
#ifndef _GEN3_CARCDISPLAY_H_
#define _GEN3_CARCDISPLAY_H_

#include <string>
#include <cstdint>

#ifdef _WINDOWS

	#include <windows.h>

#endif

#include <CArcDisplayDllMain.h>
#include <CArcBase.h>



namespace arc
{
	namespace gen3
	{

		/** @class CArcDisplay
		 *  SAOImage DS9 display class
		 *  @see arc::gen3::CArcBase
		 */
		class GEN3_CARCDISPLAY_API CArcDisplay : public CArcBase
		{
		public:

			/** Default constructor
			*/
			CArcDisplay( void );

			/** Destructor
			*/
			virtual ~CArcDisplay( void );

			/** Returns a textual representation of the library version.
			 *  @return A string representation of the library version.
			 */
			static const std::string version( void );

			/** Attempts to launch the DS9 application as a new process.
			 *  @param uiMSDelay - The amount of time to delay before attempting to access the program ( Default = 1000 ).
			 *  @throws std::runtime_error
			 */
			void launch( std::uint32_t uiMSDelay = 1000 );

			/** Displays the specified FITS image data in DS9.
			 *  @param sFitsFile - FITS file to display.
			 *  @throws std::runtime_error
			 */
			void show( const std::string& sFitsFile );

			/** Clears the specified frame or all if no frame number is specified.
			 *  @param iFrame - Frame number to clear or -1 to clear all ( default = -1 ).
			 *  @throws std::runtime_error
			 */
			void clear( std::int32_t iFrame = -1 );

			/** Forces the termination of the display process.
			 *  @throws std::runtime_error
			 */
			void terminate( void );

			/** Sets the "region" shape to box [].
			 *  @throws std::runtime_error
			 */
			void setBoxShape( void );

			/** Returns the DS9 version number
			 *  @return The DS9 version number
			 */
			void getVersion( void );

		private:

			/** Creates a new command process.
			 *  @param sBuf - Process string to execute.
			 *  @throws std::runtime_error
			 */
			void runProcess( const std::string& sBuf );

			/** Creates a new process and saves PROCESS_INFORMATION ( _WINDOWS ) or pid_t.
			 *  @param sBuf - Process string to execute.
			 *  @throws std::runtime_error
			 */
			void runAndSaveProcess( const std::string& sBuf );

			/** Gets the path of THIS module.
			 *  @param pszBuf	- Buffer to hold module path. Set to '\0' on failure.
			 *  @param tBufSize	- The byte size of the obuf buffer.
			 *  @return <i>true<i/> on success; <i>false</i> othewise.
			 *  @throws std::runtime_error
			 */
#ifdef _WINDOWS

			bool getModuleDirectory( TCHAR* pszBuf, size_t tBufSize );

#else

			bool getModuleDirectory( char* pszBuf, size_t tBufSize );

			// This method is used by GetModuleDirectory to
			// determine the path to this DLL.
			static void* getModuleStub( void ) { return NULL; }

#endif

			/** version() text holder */
			static const std::string m_sVersion;

#ifdef _WINDOWS

			/** DS9 process info structure
			 */
			PROCESS_INFORMATION m_processInfo;

#elif defined( __APPLE__ ) || defined( linux ) || defined( __linux )

			/** DS9 process id
			 */
			pid_t	m_pid;

#endif
		};

	}		// end gen3 namespace
}			// end arc namespace


#endif














//#ifndef _ARC_CDISPLAY_H_
//#define _ARC_CDISPLAY_H_
//
//#include <string>
//#include <cstdint>
//
//#ifdef _WINDOWS
//
//	#include <windows.h>
//
//#endif
//
//#include <CArcDisplayDllMain.h>
//
//
//
//namespace arc
//{
//	namespace display
//	{
//
//		class DLLDISPLAY_API CArcDisplay
//		{
//			public:
//				// Default constructor/destructor
//				// ------------------------------------------------------------------------
//				CArcDisplay();
//				virtual ~CArcDisplay();
//
//
//				// Display methods
//				// ------------------------------------------------------------------------
//				void Launch( int dMSDelay = 1000 );
//				void Show( const std::string& sFitsFile );
//				void Clear( int dFrame = -1 );
//				void Terminate();
//
//				void SetBoxShape();
//
//				const static int BUFSIZE	= 512;
//
//			private:
//				void RunProcess( const std::string& sBuf );
//				void RunAndSaveProcess( const std::string& sBuf );
//				void ThrowException( const std::string& sMethodName, const std::string& sMsg );
//
//			#ifdef _WINDOWS
//				bool GetModuleDirectory( TCHAR* pszBuf, size_t tBufSize );
//			#else
//				bool GetModuleDirectory( char* pszBuf, size_t tBufSize );
//
//				// This method is used by GetModuleDirectory to
//				// determine the path to this DLL.
//				static void* GetModuleStub( void ) { return NULL; }
//			#endif
//
//			#ifdef _WINDOWS
//
//				PROCESS_INFORMATION m_processInfo;
//
//			#elif defined( __APPLE__ ) || defined( linux ) || defined( __linux )
//
//				/** DS9 process id
//				 */
//				pid_t	m_pid;
//
//			#endif
//		};
//
//	}	// end display namespace
//}	// end arc namespace
//
//#endif
