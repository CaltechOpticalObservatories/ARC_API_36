// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  ArcDeinterlaceCAPI.cpp  ( Gen3 )                                                                         |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE: This file implements a C interface for all the CArcDeinterlace class methods.                          |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 27, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2014 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+

#include <iostream>

#include <cstring>
#include <cstdint>
#include <climits>
#include <memory>

#include <ArcDeinterlaceCAPI.h>
#include <CArcDeinterlace.h>



// +------------------------------------------------------------------------------------------------------------------+
// | Define status constants                                                                                          |
// +------------------------------------------------------------------------------------------------------------------+
const ArcError_t* ARC_STATUS_NONE = ( const ArcError_t* )NULL;
const ArcError_t  ARC_STATUS_OK = 1;
const ArcError_t  ARC_STATUS_ERROR = 2;

const std::uint32_t ARC_MSG_SIZE = 64;
const std::uint32_t ARC_ERROR_MSG_SIZE = 256;


// +------------------------------------------------------------------------------------------------------------------+
// | Initialize status pointer macro                                                                                  |
// +------------------------------------------------------------------------------------------------------------------+
#define INIT_STATUS( status, value )			if ( status != nullptr )						\
												{												\
													*status = value;							\
												}


// +------------------------------------------------------------------------------------------------------------------+
// | Set the status pointer and associated message macro                                                              |
// +------------------------------------------------------------------------------------------------------------------+
#define SET_ERROR_STATUS( status, except )	if ( status != nullptr && g_pErrMsg != nullptr )		\
										{														\
											*status = ARC_STATUS_ERROR;							\
											g_pErrMsg->assign( except.what() );					\
										}


#define VERIFY_INSTANCE_HANDLE( handle )	if ( handle == 0 ||															\
											   ( handle != reinterpret_cast< std::uint64_t >( g_pDLace16.get() ) &&		\
												 handle != reinterpret_cast< std::uint64_t >( g_pDLace32.get() ) ) )	\
											{																			\
												THROW( "Invalid deinterlace handle: 0x%X", ulHandle );					\
											}


#define GET_PLUGIN_MANAGER( handle )		( handle != reinterpret_cast< std::uint64_t >( g_pDLace16.get() ) ?		\
											  g_pDLace16->getPluginManager() :										\
											  g_pDLace32->getPluginManager() )


// +------------------------------------------------------------------------------------------------------------------+
// |  Define explicit deinterlace objects                                                                             |
// +------------------------------------------------------------------------------------------------------------------+
std::unique_ptr<arc::gen3::CArcDeinterlace<arc::gen3::dlace::BPP_16>> g_pDLace16( new arc::gen3::CArcDeinterlace<arc::gen3::dlace::BPP_16>() );
std::unique_ptr<arc::gen3::CArcDeinterlace<arc::gen3::dlace::BPP_32>> g_pDLace32( new arc::gen3::CArcDeinterlace<arc::gen3::dlace::BPP_32>() );


// +------------------------------------------------------------------------------------------------------------------+
// | Define message buffers                                                                                           |
// +------------------------------------------------------------------------------------------------------------------+
std::unique_ptr<std::string>	g_pErrMsg( new std::string(), std::default_delete<std::string>() );
std::unique_ptr<char[]>			g_pVerBuf( new char[ ARC_MSG_SIZE ], std::default_delete<char[]>() );


// +------------------------------------------------------------------------------------------------------------------+
// | Define plugin list storage                                                                                       |
// +------------------------------------------------------------------------------------------------------------------+
auto vecDeleter = []( std::vector<const char*>* p ) { if ( p != nullptr ) { delete p; } };
std::unique_ptr<std::vector<const char*>, decltype( vecDeleter )> g_pPluginList( nullptr, vecDeleter );


// +------------------------------------------------------------------------------------------------------------------+
// | Define algorithm constants                                                                                       |
// +------------------------------------------------------------------------------------------------------------------+
const unsigned int DLACE_BPP16 = sizeof( arc::gen3::dlace::BPP_16 ); // 16;
const unsigned int DLACE_BPP32 = sizeof( arc::gen3::dlace::BPP_32 ); // 32;


// +------------------------------------------------------------------------------------------------------------------+
// | Define deinterlace algorithm no argument constants                                                               |
// +------------------------------------------------------------------------------------------------------------------+
const unsigned int DLACE_NO_ARG = UINT_MAX;


// +------------------------------------------------------------------------------------------------------------------+
// | Define algorithm constants                                                                                       |
// +------------------------------------------------------------------------------------------------------------------+
const unsigned int DLACE_NONE_ALG = static_cast< unsigned int >( arc::gen3::dlace::e_Alg::NONE );
const unsigned int DLACE_PARALLEL_ALG = static_cast< unsigned int >( arc::gen3::dlace::e_Alg::PARALLEL );
const unsigned int DLACE_SERIAL_ALG = static_cast< unsigned int >( arc::gen3::dlace::e_Alg::SERIAL );
const unsigned int DLACE_QUAD_CCD_ALG = static_cast< unsigned int >( arc::gen3::dlace::e_Alg::QUAD_CCD );
const unsigned int DLACE_QUAD_IR_ALG = static_cast< unsigned int >( arc::gen3::dlace::e_Alg::QUAD_IR );
const unsigned int DLACE_QUAD_IR_CDS_ALG = static_cast< unsigned int >( arc::gen3::dlace::e_Alg::QUAD_IR_CDS );
const unsigned int DLACE_HAWAII_RG_ALG = static_cast< unsigned int >( arc::gen3::dlace::e_Alg::HAWAII_RG );
const unsigned int DLACE_STA1600_ALG = static_cast< unsigned int >( arc::gen3::dlace::e_Alg::STA1600 );
const unsigned int DLACE_CUSTOM_ALG = static_cast< unsigned int >( arc::gen3::dlace::e_Alg::CUSTOM );



// +------------------------------------------------------------------------------------------------------------------+
// |  ArcDLace_getInstance                                                                                            |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns a handle to the deinterlace object appropriate for the specified bits-per-pixel.                        |
// |                                                                                                                  |
// |  <IN>  -> uiBpp	- The number of bits-per-pixel in the image.                                                  |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCDEINTERLACE_API unsigned long long	ArcDLace_getInstance( unsigned int uiBpp, ArcStatus_t* pStatus )
{
	unsigned long long ulHandle = 0;

	INIT_STATUS( pStatus, ARC_STATUS_OK )

		try
	{
		if ( uiBpp != DLACE_BPP16 && uiBpp != DLACE_BPP32 )
		{
			THROW_INVALID_ARGUMENT( "Invalid bits-per-pixel setting [ %d ]. Must be DLACE_BPP16 or DLACE_BPP32.", uiBpp );
		}

		if ( uiBpp == DLACE_BPP16 )
		{
			if ( g_pDLace16.get() != nullptr )
			{
				ulHandle = reinterpret_cast< std::uint64_t >( g_pDLace16.get() );
			}
		}

		else if ( uiBpp == DLACE_BPP32 )
		{
			if ( g_pDLace32.get() != nullptr )
			{
				ulHandle = reinterpret_cast< std::uint64_t >( g_pDLace32.get() );
			}
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}

	return ulHandle;
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcDLace_version                                                                                                |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns a textual representation of the class version. See CArcDeinterlace::version() for details.              |
// |                                                                                                                  |
// |  <OUT> -> pStatus : Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                     |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCDEINTERLACE_API const char* ArcDLace_version( ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

		memset( g_pVerBuf.get(), 0, ARC_MSG_SIZE );

	try
	{
#ifdef _WINDOWS
		sprintf_s( g_pVerBuf.get(), ARC_MSG_SIZE, "%s", arc::gen3::CArcDeinterlace<>::version().c_str() );
#else
		sprintf( g_pVerBuf.get(), "%s", arc::gen3::CArcDeinterlace<>::version().c_str() );
#endif
	}
	catch ( std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}

	return const_cast< const char* >( g_pVerBuf.get() );
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcDLace_run                                                                                                    |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns a textual representation of the class. See CArcDevice::toString() for details.                          |
// |                                                                                                                  |
// |  <IN>  -> ulHandle	- A reference to a deinterlace object.                                                        |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <IN>  -> uiAlg	- The deinterlace algorithm.                                                                  |
// |  <IN>  -> uiArg	- An algorithm dependent argument. Use DLACE_NO_ARG if not needed.                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCDEINTERLACE_API void ArcDLace_run( unsigned long long ulHandle, void* pBuf, unsigned int uiCols, unsigned int uiRows,
	unsigned int uiAlg, unsigned int uiArg, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

		auto list = ( uiArg != DLACE_NO_ARG ? std::initializer_list<std::uint32_t>{ uiArg } : std::initializer_list<std::uint32_t>{} );

	try
	{
		VERIFY_INSTANCE_HANDLE( ulHandle )

			if ( ulHandle == reinterpret_cast< std::uint64_t >( g_pDLace16.get() ) )
			{
				g_pDLace16->run( static_cast< unsigned short* >( pBuf ), uiCols, uiRows, static_cast< arc::gen3::dlace::e_Alg >( uiAlg ), list );

				//if ( uiArg == DLACE_NO_ARG )
				//{
				//	g_pDLace16->run( static_cast< unsigned short * >( pBuf ), uiCols, uiRows, static_cast< arc::gen3::dlace::e_Alg >( uiAlg ) );
				//}

				//else
				//{
				//	g_pDLace16->run( static_cast< unsigned short * >( pBuf ), uiCols, uiRows, static_cast< arc::gen3::dlace::e_Alg >( uiAlg ), { uiArg } );
				//}
			}

			else if ( ulHandle == reinterpret_cast< std::uint64_t >( g_pDLace32.get() ) )
			{
				g_pDLace32->run( static_cast< unsigned int* >( pBuf ), uiCols, uiRows, static_cast< arc::gen3::dlace::e_Alg >( uiAlg ), list );

				//if ( uiArg == DLACE_NO_ARG )
				//{
				//	g_pDLace32->run( static_cast< unsigned int * >( pBuf ), uiCols, uiRows, static_cast< arc::gen3::dlace::e_Alg >( uiAlg ) );
				//}

				//else
				//{
				//	g_pDLace32->run( static_cast< unsigned int * >( pBuf ), uiCols, uiRows, static_cast< arc::gen3::dlace::e_Alg >( uiAlg ), { uiArg } );
				//}
			}
	}
	catch ( std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcDLace_getLastError                                                                                           |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns the last reported error message.                                                                        |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCDEINTERLACE_API const char* ArcDLace_getLastError( void )
{
	return const_cast< const char* >( g_pErrMsg->data() );
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcDLace_maxTVal                                                                                                |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns the maximum value for a specific data type. Example, for an unsigned short: 2 ^ 16 = 65536.             |
// |                                                                                                                  |
// |  <IN>  -> ulHandle	- A reference to a deinterlace object.                                                        |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCDEINTERLACE_API unsigned int ArcDLace_maxTVal( unsigned long long ulHandle, ArcStatus_t* pStatus )
{
	unsigned int uiValue = 0;

	try
	{
		VERIFY_INSTANCE_HANDLE( ulHandle )

			INIT_STATUS( pStatus, ARC_STATUS_OK );

		if ( ulHandle == reinterpret_cast< std::uint64_t >( g_pDLace16.get() ) )
		{
			uiValue = g_pDLace16->maxTVal();
		}

		else if ( ulHandle == reinterpret_cast< std::uint64_t >( g_pDLace32.get() ) )
		{
			uiValue = g_pDLace32->maxTVal();
		}
	}
	catch ( std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}

	return uiValue;
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcDLace_findPlugins                                                                                            |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns true [ 1 ] if plugins were found; false [ 0 ] otherwise.                                                |
// |                                                                                                                  |
// |  <IN>  -> ulHandle	- A reference to a deinterlace object.                                                        |
// |  <IN>  -> pszDir   - The directory to search for libraries in.                                                   |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCDEINTERLACE_API unsigned int ArcDLace_findPlugins( unsigned long long ulHandle, const char* pszDir, ArcStatus_t* pStatus )
{
	unsigned int uiSuccess = 0;

	INIT_STATUS( pStatus, ARC_STATUS_OK )

		try
	{
		VERIFY_INSTANCE_HANDLE( ulHandle )

			auto pluginManager = GET_PLUGIN_MANAGER( ulHandle );

		uiSuccess = ( pluginManager->findPlugins( pszDir ) == true ? 1 : 0 );
	}
	catch ( std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}

	return uiSuccess;
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcDLace_pluginCount                                                                                            |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns the number of loaded plugins.                                                                           |
// |                                                                                                                  |
// |  <IN>  -> ulHandle	- A reference to a deinterlace object.                                                        |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCDEINTERLACE_API unsigned int ArcDLace_pluginCount( unsigned long long ulHandle, ArcStatus_t* pStatus )
{
	unsigned int uiCount = 0;

	INIT_STATUS( pStatus, ARC_STATUS_OK )

		try
	{
		VERIFY_INSTANCE_HANDLE( ulHandle )

			auto pluginManager = GET_PLUGIN_MANAGER( ulHandle );

		uiCount = pluginManager->pluginCount();
	}
	catch ( std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}

	return uiCount;
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcDLace_pluginList                                                                                             |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns the list of algorithms supported by the plugin.                                                         |
// |                                                                                                                  |
// |  <IN>  -> ulHandle	- A reference to a deinterlace object.                                                        |
// |  <IN>  -> uiPlugin - The plugin index. The range is 0 to ArcDLace_pluginCount().                                 |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCDEINTERLACE_API const char** ArcDLace_pluginList( unsigned long long ulHandle, unsigned int uiPlugin, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

		try
	{
		VERIFY_INSTANCE_HANDLE( ulHandle )

			auto pluginManager = GET_PLUGIN_MANAGER( ulHandle );

		if ( uiPlugin >= pluginManager->pluginCount() )
		{
			THROW_INVALID_ARGUMENT( "Invalid plugin value [ %u ], expected range: 0 to %u", uiPlugin, pluginManager->pluginCount() );
		}

		auto pList = pluginManager->getPluginObject( uiPlugin )->getNameList();

		g_pPluginList.reset( new std::vector<const char*> );

		for ( std::uint32_t i = 0; i < pList->length(); i++ )
		{
			g_pPluginList->push_back( pList->at( i ).c_str() );
		}
	}
	catch ( std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}

	return g_pPluginList->data();
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcDLace_pluginListCount                                                                                        |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns the number of algorithms supported by the plugin.                                                       |
// |                                                                                                                  |
// |  <IN>  -> ulHandle	- A reference to a deinterlace object.                                                        |
// |  <IN>  -> uiPlugin - The plugin index. The range is 0 to ArcDLace_pluginCount().                                 |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCDEINTERLACE_API unsigned int ArcDLace_pluginListCount( unsigned long long ulHandle, unsigned int uiPlugin, ArcStatus_t* pStatus )
{
	unsigned int uiCount = 0;

	INIT_STATUS( pStatus, ARC_STATUS_OK )

		try
	{
		VERIFY_INSTANCE_HANDLE( ulHandle )

			auto pluginManager = GET_PLUGIN_MANAGER( ulHandle );

		if ( uiPlugin >= pluginManager->pluginCount() )
		{
			THROW_INVALID_ARGUMENT( "Invalid plugin value [ %u ], expected range: 0 to %u", uiPlugin, pluginManager->pluginCount() );
		}

		uiCount = pluginManager->getPluginObject( uiPlugin )->getCount();
	}
	catch ( std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}

	return uiCount;
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcDLace_pluginRun                                                                                              |
// +------------------------------------------------------------------------------------------------------------------+
// |  Executes a custom plugin deinterlace algorithm on the specified image buffer.                                   |
// |                                                                                                                  |
// |  <IN>  -> ulHandle	- A reference to a deinterlace object.                                                        |
// |  <IN>  -> uiPlugin - The plugin index. The range is 0 to ArcDLace_pluginCount().                                 |
// |  <IN>  -> pBuf		- The image data buffer.                                                                      |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <IN>  -> pszAlg	- The deinterlace algorithm name. One of the strings returned from the pluginList() function. |
// |  <IN>  -> uiArg	- An algorithm dependent argument. Use DLACE_NO_ARG if not needed.                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCDEINTERLACE_API void ArcDLace_pluginRun( unsigned long long ulHandle, unsigned int uiPlugin, void* pBuf, unsigned int uiCols,
	unsigned int uiRows, const char* pszAlg, unsigned int uiArg, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

		try
	{
		VERIFY_INSTANCE_HANDLE( ulHandle )

			auto pluginManager = GET_PLUGIN_MANAGER( ulHandle );

		if ( uiPlugin >= pluginManager->pluginCount() )
		{
			THROW_INVALID_ARGUMENT( "Invalid plugin value [ %u ], expected range: 0 to %u", uiPlugin, g_pDLace16->getPluginManager()->pluginCount() );
		}

		pluginManager->getPluginObject( uiPlugin )->run( pBuf,
			uiCols,
			uiRows,
			( 8 * ( ulHandle == reinterpret_cast< std::uint64_t >( g_pDLace16.get() ) ? DLACE_BPP16 : DLACE_BPP32 ) ),
			pszAlg,
			uiArg );

		//pluginManager->getPluginObject( uiPlugin )->run( pBuf,
		//												uiCols,
		//												uiRows,
		//												( ulHandle == reinterpret_cast<std::uint64_t>( g_pDLace16.get() ) ? DLACE_BPP16 : DLACE_BPP32 ),
		//												pszAlg,
		//												uiArg );
	}
	catch ( std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


//GEN3_CARCDEINTERLACE_API void ArcDLace_pluginRun( unsigned int uiPlugin, void* pBuf, unsigned int uiCols, unsigned int uiRows,
//												  unsigned int uiBpp, const char* pszAlg, unsigned int uiArg, ArcStatus_t* pStatus )
//{
//	INIT_STATUS( pStatus, ARC_STATUS_OK )
//
//	try
//	{
//		auto pluginManager = ( uiBpp == DLACE_BPP16 ? g_pDLace16->getPluginManager() : g_pDLace32->getPluginManager() );
//
//		if ( uiPlugin >= pluginManager->pluginCount() )
//		{
//			THROW( "Invalid plugin value [ %u ], expected range: 0 to %u", uiPlugin, g_pDLace16->getPluginManager()->pluginCount() );
//		}
//
//		if ( uiArg == DLACE_NO_ARG )
//		{
//			pluginManager->getPluginObject( uiPlugin )->run( pBuf, uiCols, uiRows, uiBpp, pszAlg );
//		}
//
//		else
//		{
//			pluginManager->getPluginObject( uiPlugin )->run( pBuf, uiCols, uiRows, uiBpp, pszAlg, { uiArg } );
//		}
//
////		if ( uiBpp == DLACE_BPP16 )
////		{
////			if ( g_pDLace16 == nullptr )
////			{
////				THROW( "Invalid class pointer!" );
////			}
////
////			if ( uiArg == DLACE_NO_ARG )
////			{
////				g_pDLace16->getPluginManager()->getPluginObject( uiPlugin )->run( static_cast< unsigned short * >( pBuf ), uiCols, uiRows, pszAlg );
////			}
////
////			else
////			{
////				g_pDLace16->getPluginManager()->getPluginObject( uiPlugin )->run( static_cast< unsigned short * >( pBuf ), uiCols, uiRows, pszAlg, { uiArg } );
////			}
////		}
////
////		else
////		{
////			if ( g_pDLace32 == nullptr )
////			{
////				THROW( "Invalid class pointer!" );
////			}
////
////			std::cout << "########################################" << std::std::endl
////				<< "uiPlugin -> " << uiPlugin << std::std::endl
////				<< "pBuf - > " << ( pBuf == nullptr ? "NULL" : "OK" ) << std::std::endl
////				<< "uiCols -> " << uiCols << " uiRows -> " << uiRows << std::std::endl
////				<< "pszAlg -> " << pszAlg << std::std::endl
////				<< "g_pDLace32 - > " << ( g_pDLace32 == nullptr ? "NULL" : "OK" ) << std::std::endl
////				<< "getPluginManager - > " << ( g_pDLace32->getPluginManager() == nullptr ? "NULL" : "OK" ) << std::std::endl
////				<< "getPluginObject - > " << ( g_pDLace32->getPluginManager()->getPluginObject( uiPlugin ) == nullptr ? "NULL" : "OK" ) << std::std::endl;
////
////			if ( uiArg == DLACE_NO_ARG )
////			{
//////				g_pDLace32->getPluginManager()->getPluginObject( uiPlugin )->run( static_cast< unsigned int * >( pBuf ), uiCols, uiRows, pszAlg );
////				arc::gen3::CArcDeinterlace<arc::gen3::dlace::BPP_16>::getPluginManager()->getPluginObject( uiPlugin )->run( static_cast< unsigned int * >( pBuf ), uiCols, uiRows, pszAlg );
////			}
////
////			else
////			{
////				g_pDLace32->getPluginManager()->getPluginObject( uiPlugin )->run( static_cast< unsigned int * >( pBuf ), uiCols, uiRows, pszAlg, { uiArg } );
////			}
////		}
//	}
//	catch ( std::exception& e )
//	{
//		SET_ERROR_STATUS( pStatus, e );
//	}
//}
