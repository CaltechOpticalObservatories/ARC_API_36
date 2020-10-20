// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  CArcDevice.h  ( Gen3 )                                                                                   |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE: This file defines the standard ARC deinterlace interface.                                              |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 27, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2013 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+

#ifndef _GEN3_CARCDEINTERLACE_H_
#define _GEN3_CARCDEINTERLACE_H_

#ifdef _WINDOWS
	#pragma warning( disable: 4251 )
	#include <windows.h>
#endif

#include <initializer_list>
#include <cstdint>
#include <cstdarg>
#include <memory>
#include <string>
#include <vector>

#include <CArcDeinterlaceDllMain.h>
#include <CArcPluginManager.h>
#include <CArcBase.h>



// This class is exported from the CArcDeinterlace.dll
namespace arc
{
	namespace gen3
	{

		namespace dlace
		{

			/** 16 bits-per-pixel image data. */
			using BPP_16 = std::uint16_t;


			/** 32 bits-per-pixel image data. */
			using BPP_32 = std::uint32_t;


			/** @enum e_Alg
			*  Defines the allowable deinterlace algorithms.
			*/
			enum class e_Alg : std::uint32_t
			{
				NONE = 0,
				PARALLEL,
				SERIAL,
				QUAD_CCD,
				QUAD_IR,
				QUAD_IR_CDS,
				HAWAII_RG,
				STA1600,
				CUSTOM
			};

		}	// end dlace namespace


		/** @class CArcDeinterlace
		 *  Image deinterlace class.
		 *  @see arc::gen3::CArcBase()
		 */
		template <typename T = arc::gen3::dlace::BPP_16>
		class GEN3_CARCDEINTERLACE_API CArcDeinterlace : public arc::gen3::CArcBase
		{
		public:

			/** Constructor
			 */
			CArcDeinterlace( void );

			/** Destructor
			 */
			virtual ~CArcDeinterlace( void );

			/** Returns a textual representation of the library version.
			 *  @return A string representation of the library version.
			 */
			static const std::string version( void );

			/** Deinterlace the buffer data using the specified algorithm.
			 *  @param pBuf		- Pointer to the buffer to deinterlace.
			 *  @param uiCols	- The number of columns in the buffer.
			 *  @param uiRows	- The number of rows in the buffer.
			 *  @param eAlg		- The algorithm to use to deinterlace the buffer.
			 *  @param tArgList	- A reference to a list of algorithm dependent arguments ( default = {}, empty list ).
			 *  @see CArcDeinterlace::e_Alg
			 *  @throws std::exception on error.
			 */
			void run( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows, arc::gen3::dlace::e_Alg eAlg, const std::initializer_list<std::uint32_t>& tArgList = {} );

			/** Deinterlace the buffer data using a custom algorithm loaded through the plugin manager.
			 *  @param pBuf		- Pointer to the buffer to deinterlace.
			 *  @param uiCols	- The number of columns in the buffer.
			 *  @param uiRows	- The number of rows in the buffer.
			 *  @param sAlg		- The name of the algorithm to use for deinterlacing the buffer.
			 *  @param tArgList	- A reference to a list of algorithm dependent arguments ( default = {}, empty list ).
			 *  @throws std::exception on error.
			 */
			void run( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows, const std::string& sAlg, const std::initializer_list<std::uint32_t>& tArgList = {} );

			/** Returns the deinterlace plugin manager.
			 *  @return The plugin manager.
			 */
			static arc::gen3::CArcPluginManager* getPluginManager( void );

			/** Determines the maximum value for a specific data type. Example, for std::uint16_t: 2^16 = 65536.
			 *  @return The maximum value for the data type currently in use.
			 */
			std::uint32_t maxTVal( void );

		protected:

			///** Intermediate buffer deleter.
			// *  @param p - Pointer to buffer to delete.
			// */
			//template<typename T> static void arrayDeleter( T* p );

			/** Parallel deinterlace algorithm.
			 *  @param pBuf   - Pointer to the buffer data to deinterlace.
			 *  @param uiCols - The number of columns in the buffer.
			 *  @param uiRows - The number of rows in the buffer.
			 *  @throws std::exception on error.
			 */
			void parallel( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Serial deinterlace algorithm.
			 *  @param pBuf   - Pointer to the buffer data to deinterlace.
			 *  @param uiCols - The number of columns in the buffer.
			 *  @param uiRows - The number of rows in the buffer.
			 *  @throws std::exception on error.
			 */
			void serial( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Quad CCD deinterlace algorithm.
			 *  @param pBuf   - Pointer to the buffer data to deinterlace.
			 *  @param uiCols - The number of columns in the buffer.
			 *  @param uiRows - The number of rows in the buffer.
			 *  @throws std::exception on error.
			 */
			void quadCCD( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Quad IR deinterlace algorithm.
			 *  @param pBuf   - Pointer to the buffer data to deinterlace.
			 *  @param uiCols - The number of columns in the buffer.
			 *  @param uiRows - The number of rows in the buffer.
			 *  @throws std::exception on error.
			 */
			void quadIR( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Quad IR CDS ( correlated double sampling ) deinterlace algorithm.
			 *  @param pBuf   - Pointer to the buffer data to deinterlace.
			 *  @param uiCols - The number of columns in the buffer.
			 *  @param uiRows - The number of rows in the buffer.
			 *  @throws std::exception on error.
			 */
			void quadIRCDS( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Hawaii RG deinterlace algorithm.
			 *  @param pBuf			- Pointer to the buffer data to deinterlace.
			 *  @param uiCols		- The number of columns in the buffer.
			 *  @param uiRows		- The number of rows in the buffer.
			 *  @param uiChannels	- The number of channels in the image ( 16, 32, ... ).
			 *  @throws std::exception on error.
			 */
			void hawaiiRG( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t uiChannels );

			/** STA 1600 deinterlace algorithm.
			 *  @param pBuf	  - Pointer to the buffer data to deinterlace.
			 *  @param uiCols - The number of columns in the buffer.
			 *  @param uiRows - The number of rows in the buffer.
			 *  @throws std::exception on error.
			 */
			void sta1600( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** version() text holder */
			static const std::string m_sVersion;

			/** Intermediate buffer */
			std::unique_ptr<T[]> m_pNewData;

			/** Intermediate buffer columns */
			std::uint32_t m_uiNewCols;

			/** Intermediate buffer rows */
			std::uint32_t m_uiNewRows;

			/** Deinterlace plugin manager */
			static std::unique_ptr<arc::gen3::CArcPluginManager> m_pPluginManager;

		};

	}		// end gen3 namespace
}			// end arc namespace


#endif		// _GEN3_CARCDEINTERLACE_H_



















///*
// *  CArcDeinterlace.h
// *  CArcDeinterlace
// *
// *  Created by Scott Streit on 2/9/11.
// *  Copyright 2011 Astronomical Research Cameras, Inc. All rights reserved.
// *
// */
//#ifndef _ARC_CDEINTERLACE_H_
//#define _ARC_CDEINTERLACE_H_
//
//#ifdef _WINDOWS
//#pragma warning( disable: 4251 )
//#endif
//
//#include <stdexcept>
//#include <string>
//#include <vector>
//#include <memory>
//#include <CArcDeinterlaceDllMain.h>
//
//#ifdef _WINDOWS
//	#include <windows.h>
//#endif
//
//
//namespace arc
//{
//	namespace deinterlace
//	{
//		// +------------------------------------------------------------+
//		// | Define OS dependent custom library handle                  |
//		// +------------------------------------------------------------+
//		#ifdef _WINDOWS
//			#define ArcCustomLib	HINSTANCE
//		#else
//			#define ArcCustomLib	void*
//		#endif
//
//
//		// +------------------------------------------------------------+
//		// | Define custom function pointers                            |
//		// +------------------------------------------------------------+
//		typedef int  ( *RunCustomAlgFUNC )( void *, int, int, int, int );
//		typedef void ( *GetCustomNameFUNC )( int, char*, int );
//		typedef void ( *GetErrorMsgFUNC )( char*, int );
//		typedef int  ( *GetCustomCountFUNC )( void );
//
//
//		// +------------------------------------------------------------+
//		// | CArcDeinterlace class definition                           |
//		// +------------------------------------------------------------+
//		class CDEINTERLACE_API CArcDeinterlace
//		{
//			public:
//				CArcDeinterlace();
//				virtual ~CArcDeinterlace();
//
//				//  Deinterlace the data with the specified algorithm
//				// +--------------------------------------------------------------------+
//				void RunAlg( void *pData, int dRows, int dCols, int dAlgorithm, int dArg = -1 );
//
//
//				//  Custom dAlgorithm interface
//				// +--------------------------------------------------------------------+
//				bool FindCustomLibrary( const std::string sLibPath );
//				void RunCustomAlg( void *pData, int dRows, int dCols, int dAlgorithm, int dArg = -1 );
//				void GetCustomInfo( int index, int& dAlgorithm, std::string& name );
//				int  GetCustomCount();
//
//
//				//  Algorithm definitions
//				// +--------------------------------------------------------------------+
//				const static int DEINTERLACE_NONE        = 0;
//				const static int DEINTERLACE_PARALLEL    = 1;
//				const static int DEINTERLACE_SERIAL      = 2;
//				const static int DEINTERLACE_CCD_QUAD    = 3;
//				const static int DEINTERLACE_IR_QUAD     = 4;
//				const static int DEINTERLACE_CDS_IR_QUAD = 5;
//				const static int DEINTERLACE_HAWAII_RG   = 6;
//				const static int DEINTERLACE_STA1600	 = 7;
//				const static int DEINTERLACE_CUSTOM      = 8;
//
//			private:
//				std::shared_ptr<unsigned short> m_pNewData;
//
//				ArcCustomLib					m_hCustomLib;
//				RunCustomAlgFUNC				m_fnRunCustomAlg;
//				GetCustomNameFUNC				m_fnGetCustomName;
//				GetCustomCountFUNC				m_fnGetCustomCount;
//				GetErrorMsgFUNC					m_fnGetErrorMsg;
//				int								m_dCustomCount;
//
//				//  Smart pointer deleter
//				// +-----------------------------------------------------------------+
//				template<typename T> static void ArrayDeleter( T* p );
//
//				void Parallel( unsigned short *pData, int dRows, int dCols );
//				void Serial( unsigned short *pData, int dRows, int dCols );
//				void CCDQuad( unsigned short *pData, int dRows, int dCols );
//				void IRQuad( unsigned short *pData, int dRows, int dCols );
//				void IRQuadCDS( unsigned short *pData, int dRows, int dCols );
//				void HawaiiRG( unsigned short *pData, int dRows, int dCols, int dArg );
//				void STA1600( unsigned short *pData, int dRows, int dCols );
//				void ThrowException( std::string sMethodName, std::string sMsg );
//
//				void GetDirList( const std::string& sPath, std::vector<std::string>& vDirs );
//				void LoadCustomLibrary( const std::string sLibPath, const std::string sLibName );
//				bool IsCustomLibrary( ArcCustomLib hCustomLib );
//				std::string GetCustomErrorMsg();
//		};
//
//	}	// end deinterlace namespace
//}	// end arc namespace
//
//
//#endif
