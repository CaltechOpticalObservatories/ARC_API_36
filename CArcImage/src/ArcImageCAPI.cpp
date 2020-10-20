// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  ArcImageCAPI.cpp  ( Gen3 )                                                                               |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE: This file implements a C interface for all the CArcImage class methods.                                |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 26, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2014 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+

#include <iostream>

#include <cstdint>
#include <climits>
#include <cstring>
#include <memory>

#include <ArcImageCAPI.h>
#include <CArcImage.h>



// +------------------------------------------------------------------------------------------------------------------+
// | Initialize status pointer macro                                                                                  |
// +------------------------------------------------------------------------------------------------------------------+
#define INIT_STATUS( status, value )		if ( status != nullptr )								\
											{														\
												*status = value;									\
											}


// +------------------------------------------------------------------------------------------------------------------+
// | Set the status pointer and associated message macro                                                              |
// +------------------------------------------------------------------------------------------------------------------+
#define SET_ERROR_STATUS( status, except )	if ( status != nullptr && g_pErrMsg != nullptr )		\
											{														\
												*status = ARC_STATUS_ERROR;							\
												g_pErrMsg->assign( except.what() );					\
											}


// +------------------------------------------------------------------------------------------------------------------+
// |  Define explicit image objects                                                                                   |
// +------------------------------------------------------------------------------------------------------------------+
static std::uint32_t g_uiCurrentBpp = 0;


// +------------------------------------------------------------------------------------------------------------------+
// | Define message buffers                                                                                           |
// +------------------------------------------------------------------------------------------------------------------+
std::unique_ptr<std::string>	g_pErrMsg( new std::string(), std::default_delete<std::string>() );
std::unique_ptr<char[]>			g_pVerBuf( new char[ ARC_MSG_SIZE ], std::default_delete<char[]>() );


// +------------------------------------------------------------------------------------------------------------------+
// | Define algorithm constants                                                                                       |
// +------------------------------------------------------------------------------------------------------------------+
const unsigned int IMAGE_BPP16 = sizeof( arc::gen3::image::BPP_16 ); // 16;
const unsigned int IMAGE_BPP32 = sizeof( arc::gen3::image::BPP_32 ); // 32;



// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_getInstance                                                                                            |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns a handle to the image object appropriate for the specified bits-per-pixel.                              |
// |                                                                                                                  |
// |  <IN> -> uiBpp - The number of bits-per-pixel in the image.                                                      |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_selectInstance( unsigned int uiBpp, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( uiBpp != IMAGE_BPP16 && uiBpp != IMAGE_BPP32 )
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32.", g_uiCurrentBpp );
		}

		g_uiCurrentBpp = uiBpp;
	}
	catch ( const std::exception& e )
	{
		g_uiCurrentBpp = 0;

		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_version                                                                                                |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns a textual representation of the class version. See CArcImage::version() for details.                    |
// |                                                                                                                  |
// |  <OUT> -> pStatus : Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                     |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API const char* ArcImage_version( ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	memset( g_pVerBuf.get(), 0, ARC_MSG_SIZE );

	try
	{
	#ifdef _WINDOWS
		sprintf_s( g_pVerBuf.get(), ARC_MSG_SIZE, "%s", arc::gen3::CArcImage<>::version().c_str() );
	#else
		snprintf( g_pVerBuf.get(), ARC_MSG_SIZE, "%s", arc::gen3::CArcImage<>::version().c_str() );
	#endif
	}
	catch ( std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}

	return const_cast< const char* >( g_pVerBuf.get() );
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcDLace_fill                                                                                                   |
// +------------------------------------------------------------------------------------------------------------------+
// |  Fills the specified buffer with the specified value.                                                            |
// |                                                                                                                  |
// |  <IN>  -> uHandle	- A reference to an image object.                                                             |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <IN>  -> uiValue	- The value to fill the buffer with.                                                          |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_fill( void* pBuf, unsigned int uiCols, unsigned int uiRows, unsigned int uiValue, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			arc::gen3::CArcImage<>::fill( static_cast< std::uint16_t* >( pBuf ),
										  uiCols,
										  uiRows,
										  static_cast< std::uint16_t >( uiValue ) );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			arc::gen3::CArcImage<arc::gen3::image::BPP_32>::fill( static_cast< std::uint32_t* >( pBuf ),
																  uiCols,
																  uiRows,
																  static_cast< std::uint32_t >( uiValue ) );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_fillWithGradient                                                                                       |
// +------------------------------------------------------------------------------------------------------------------+
// |  Fills the specified buffer with a gradient pattern.                                                             |
// |                                                                                                                  |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <IN>  -> uiValue	- The value to fill the buffer with.                                                          |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_fillWithGradient( void* pBuf, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			arc::gen3::CArcImage<>::fillWithGradient( static_cast< std::uint16_t* >( pBuf ), uiCols, uiRows );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			arc::gen3::CArcImage<arc::gen3::image::BPP_32>::fillWithGradient( static_cast< std::uint32_t* >( pBuf ), uiCols, uiRows );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_fillWithSmiley                                                                                         |
// +------------------------------------------------------------------------------------------------------------------+
// |  Fills the specified buffer with zeroes and puts a smiley face at the center.                                    |
// |                                                                                                                  |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <IN>  -> uiValue	- The value to fill the buffer with.                                                          |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_fillWithSmiley( void* pBuf, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			arc::gen3::CArcImage<>::fillWithSmiley( static_cast< std::uint16_t* >( pBuf ), uiCols, uiRows );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			arc::gen3::CArcImage<arc::gen3::image::BPP_32>::fillWithSmiley( static_cast< std::uint32_t* >( pBuf ), uiCols, uiRows );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_fillWithRamp                                                                                           |
// +------------------------------------------------------------------------------------------------------------------+
// |  Fills the specified buffer with a ramp image. Data has the form 0, 1, 2, ..., 65535, 0, 1, ....                 |
// |                                                                                                                  |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <IN>  -> uiValue	- The value to fill the buffer with.                                                          |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_fillWithRamp( void* pBuf, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			arc::gen3::CArcImage<>::fillWithRamp( static_cast< std::uint16_t* >( pBuf ), uiCols, uiRows );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			arc::gen3::CArcImage<arc::gen3::image::BPP_32>::fillWithRamp( static_cast< std::uint32_t* >( pBuf ), uiCols, uiRows );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_containsValidRamp                                                                                      |
// +------------------------------------------------------------------------------------------------------------------+
// |  Verify a ramp synthetic image. Data has the form 0, 1, 2, ..., 65535, 0, 1, ....                                |
// |                                                                                                                  |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <IN>  -> uiValue	- The value to fill the buffer with.                                                          |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_containsValidRamp( const void* pBuf, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			arc::gen3::CArcImage<>::containsValidRamp( static_cast< const std::uint16_t* >( pBuf ), uiCols, uiRows );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			arc::gen3::CArcImage<arc::gen3::image::BPP_32>::containsValidRamp( static_cast< const std::uint32_t* >( pBuf ), uiCols, uiRows );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_getRow                                                                                                 |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns all or part of an image row.                                                                            |
// |                                                                                                                  |
// |  <IN>  -> pRow		- A pointer to a row buffer that is at least ( col2 - col1 ) in length.                       |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCol1	- The start column.                                                                           |
// |  <IN>  -> uiCol2	- The end column.                                                                             |
// |  <IN>  -> uiRow		- The row to read from.                                                                       |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_getRow( void* pRow, const void* pBuf, unsigned int uiCol1, unsigned int uiCol2, unsigned int uiRow,
	unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		std::uint32_t uiCount = 0;

		if ( pRow == nullptr )
		{
			THROW( "Invalid row pointer parameter [ NULL ]." );
		}

		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			auto pImgRow = arc::gen3::CArcImage<>::getRow( static_cast< const std::uint16_t* >( pBuf ),
				uiCol1,
				uiCol2,
				uiRow,
				uiCols,
				uiRows,
				uiCount );

			std::memcpy( pRow, pImgRow.get(), ( uiCount * sizeof( std::uint16_t ) ) );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			auto pImgRow = arc::gen3::CArcImage<arc::gen3::image::BPP_32>::getRow( static_cast< const std::uint32_t* >( pBuf ),
				uiCol1,
				uiCol2,
				uiRow,
				uiCols,
				uiRows,
				uiCount );

			std::memcpy( pRow, pImgRow.get(), ( uiCount * sizeof( std::uint32_t ) ) );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_getCol                                                                                                 |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns all or part of an image column.                                                                         |
// |                                                                                                                  |
// |  <IN>  -> pCol		- A pointer to a column buffer that is at least ( row2 - row 1 ) in length.                   |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCol	- The column to read from.                                                                    |
// |  <IN>  -> uiRow1	- The start row.                                                                              |
// |  <IN>  -> uiRow2	- The end row.                                                                                |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void	ArcImage_getCol( void* pCol, const void* pBuf, unsigned int uiCol, unsigned int uiRow1,
	unsigned int uiRow2, unsigned int uiCols, unsigned int uiRows,
	ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		std::uint32_t uiCount = 0;

		if ( pCol == nullptr )
		{
			THROW( "Invalid column pointer parameter [ NULL ]." );
		}

		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			auto pImgCol = arc::gen3::CArcImage<>::getCol(  static_cast< const std::uint16_t* >( pBuf ),
															uiCol,
															uiRow1,
															uiRow2,
															uiCols,
															uiRows,
															uiCount );

			std::memcpy( pCol, pImgCol.get(), ( uiCount * sizeof( std::uint16_t ) ) );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			auto pImgCol = arc::gen3::CArcImage<arc::gen3::image::BPP_32>::getCol(  static_cast< const std::uint32_t* >( pBuf ),
																					uiCol,
																					uiRow1,
																					uiRow2,
																					uiCols,
																					uiRows,
																					uiCount );

			std::memcpy( pCol, pImgCol.get(), ( uiCount * sizeof( std::uint32_t ) ) );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_getRowArea                                                                                             |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns a row of pixel data where each value is the average over the specified range of columns.                |
// |                                                                                                                  |
// |  <IN>  -> pArea	- A pointer to an area buffer that is at least ( row2 - row 1 ) * ( col2 - col1 ) in length.  |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCol1	- The start column.                                                                           |
// |  <IN>  -> uiCol2	- The end column.                                                                             |
// |  <IN>  -> uiRow1	- The start row.                                                                              |
// |  <IN>  -> uiRow2	- The end row.                                                                                |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_getRowArea( double* pArea, const void* pBuf, unsigned int uiCol1, unsigned int uiCol2, unsigned int uiRow1,
	unsigned int uiRow2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		std::uint32_t uiCount = 0;

		if ( pArea == nullptr )
		{
			THROW( "Invalid area pointer parameter [ NULL ]." );
		}

		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			auto pImgArea = arc::gen3::CArcImage<>::getRowArea( static_cast< const std::uint16_t* >( pBuf ),
																uiCol1,
																uiCol2,
																uiRow1,
																uiRow2,
																uiCols,
																uiRows,
																uiCount );

			std::memcpy( pArea, pImgArea.get(), ( uiCount * sizeof( double ) ) );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			auto pImgArea = arc::gen3::CArcImage<arc::gen3::image::BPP_32>::getRowArea( static_cast< const std::uint32_t* >( pBuf ),
																						uiCol1,
																						uiCol2,
																						uiRow1,
																						uiRow2,
																						uiCols,
																						uiRows,
																						uiCount );

			std::memcpy( pArea, pImgArea.get(), ( uiCount * sizeof( double ) ) );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_getColArea                                                                                             |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns a column of pixel data where each value is the average over the specified range of rows.                |
// |                                                                                                                  |
// |  <IN>  -> pArea	- A pointer to an area buffer that is at least ( row2 - row 1 ) * ( col2 - col1 ) in length.  |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCol1	- The start column.                                                                           |
// |  <IN>  -> uiCol2	- The end column.                                                                             |
// |  <IN>  -> uiRow1	- The start row.                                                                              |
// |  <IN>  -> uiRow2	- The end row.                                                                                |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void	ArcImage_getColArea( double* pArea, const void* pBuf, unsigned int uiCol1, unsigned int uiCol2, unsigned int uiRow1,
	unsigned int uiRow2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		std::uint32_t uiCount = 0;

		if ( pArea == nullptr )
		{
			THROW( "Invalid area pointer parameter [ NULL ]." );
		}

		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			auto pImgArea = arc::gen3::CArcImage<>::getColArea( static_cast< const std::uint16_t* >( pBuf ),
																uiCol1,
																uiCol2,
																uiRow1,
																uiRow2,
																uiCols,
																uiRows,
																uiCount );

			std::memcpy( pArea, pImgArea.get(), ( uiCount * sizeof( double ) ) );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			auto pImgArea = arc::gen3::CArcImage<arc::gen3::image::BPP_32>::getColArea( static_cast< const std::uint32_t* >( pBuf ),
																						uiCol1,
																						uiCol2,
																						uiRow1,
																						uiRow2,
																						uiCols,
																						uiRows,
																						uiCount );

			std::memcpy( pArea, pImgArea.get(), ( uiCount * sizeof( double ) ) );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_getStats                                                                                               |
// +------------------------------------------------------------------------------------------------------------------+
// |  Calculates the image min, max, mean, variance, standard deviation, total pixel count and saturated pixel count  |
// |  over the specified image buffer cols and rows.                                                                  |
// |                                                                                                                  |
// |  <IN>  -> pStats	- A pointer to a CStats structure.                                                            |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCol1	- The start column.                                                                           |
// |  <IN>  -> uiCol2	- The end column.                                                                             |
// |  <IN>  -> uiRow1	- The start row.                                                                              |
// |  <IN>  -> uiRow2	- The end row.                                                                                |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_getStats( struct CStats* pStats, const void* pBuf, unsigned int uiCol1, unsigned int uiCol2,
	unsigned int uiRow1, unsigned int uiRow2, unsigned int uiCols, unsigned int uiRows,
	ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( pStats == nullptr )
		{
			THROW( "Invalid CStats pointer parameter [ NULL ]." );
		}

		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			auto pImgStats = arc::gen3::CArcImage<>::getStats(  static_cast< const std::uint16_t* >( pBuf ),
																uiCol1,
																uiCol2,
																uiRow1,
																uiRow2,
																uiCols,
																uiRows );

			pStats->gSaturatedCount = pImgStats->gSaturatedCount;
			pStats->gTotalPixels = pImgStats->gTotalPixels;
			pStats->gVariance = pImgStats->gVariance;
			pStats->gMax = pImgStats->gMax;
			pStats->gMin = pImgStats->gMin;
			pStats->gMean = pImgStats->gMean;
			pStats->gStdDev = pImgStats->gStdDev;
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			auto pImgStats = arc::gen3::CArcImage<arc::gen3::image::BPP_32>::getStats(  static_cast< const std::uint32_t* >( pBuf ),
																						uiCol1,
																						uiCol2,
																						uiRow1,
																						uiRow2,
																						uiCols,
																						uiRows );

			pStats->gSaturatedCount = pImgStats->gSaturatedCount;
			pStats->gTotalPixels = pImgStats->gTotalPixels;
			pStats->gVariance = pImgStats->gVariance;
			pStats->gMax = pImgStats->gMax;
			pStats->gMin = pImgStats->gMin;
			pStats->gMean = pImgStats->gMean;
			pStats->gStdDev = pImgStats->gStdDev;
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_getDiffStats                                                                                           |
// +------------------------------------------------------------------------------------------------------------------+
// |  Calculates the min, max, mean, variance, standard deviation, total pixel count and saturated pixel count for    |
// |  each image as well as the difference mean, variance and standard deviation over the specified image buffer      |
// |  cols and rows.This is used for photon transfer curves( PTC ).The two images MUST be the same size or the        |
// |  methods behavior is undefined as this cannot be verified using the given parameters.                            |
// |                                                                                                                  |
// |  <IN>  -> pStats	- A pointer to a CDifStats structure.                                                         |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCol1	- The start column.                                                                           |
// |  <IN>  -> uiCol2	- The end column.                                                                             |
// |  <IN>  -> uiRow1	- The start row.                                                                              |
// |  <IN>  -> uiRow2	- The end row.                                                                                |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_getDiffStats( struct CDifStats* pStats, const void* pBuf1, void* pBuf2, unsigned int uiCol1,
	unsigned int uiCol2, unsigned int uiRow1, unsigned int uiRow2, unsigned int uiCols,
	unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( pStats == nullptr )
		{
			THROW( "Invalid CDifStats pointer parameter [ NULL ]." );
		}

		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			auto pImgStats = arc::gen3::CArcImage<>::getDiffStats(  static_cast< const std::uint16_t* >( pBuf1 ),
																	static_cast< const std::uint16_t* >( pBuf1 ),
																	uiCol1,
																	uiCol2,
																	uiRow1,
																	uiRow2,
																	uiCols,
																	uiRows );

			std::memcpy( &pStats->cDiffStats, &pImgStats->cDiffStats, sizeof( CDifStats ) );
			std::memcpy( &pStats->cStats1, &pImgStats->cStats1, sizeof( CStats ) );
			std::memcpy( &pStats->cStats2, &pImgStats->cStats2, sizeof( CStats ) );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			auto pImgStats = arc::gen3::CArcImage<arc::gen3::image::BPP_32>::getDiffStats(  static_cast< const std::uint32_t* >( pBuf1 ),
																							static_cast< const std::uint32_t* >( pBuf2 ),
																							uiCol1,
																							uiCol2,
																							uiRow1,
																							uiRow2,
																							uiCols,
																							uiRows );

			std::memcpy( &pStats->cDiffStats, &pImgStats->cDiffStats, sizeof( CDifStats ) );
			std::memcpy( &pStats->cStats1, &pImgStats->cStats1, sizeof( CStats ) );
			std::memcpy( &pStats->cStats2, &pImgStats->cStats2, sizeof( CStats ) );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_histogram                                                                                              |
// +------------------------------------------------------------------------------------------------------------------+
// |  Calculates the histogram over the specified image buffer columns and rows.                                      |
// |                                                                                                                  |
// |  <IN>  -> pHist	- A pointer to an unsigned int histogram array.                                               |
// |  <IN>  -> pBuf		- The image buffer data.                                                                      |
// |  <IN>  -> uiCol1	- The start column.                                                                           |
// |  <IN>  -> uiCol2	- The end column.                                                                             |
// |  <IN>  -> uiRow1	- The start row.                                                                              |
// |  <IN>  -> uiRow2	- The end row.                                                                                |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <IN>	-> puiCount	- The element count of the returned array.                                                    |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void	ArcImage_histogram( unsigned int* pHist, const void* pBuf, unsigned int uiCol1, unsigned int uiCol2,
	unsigned int uiRow1, unsigned int uiRow2, unsigned int uiCols, unsigned int uiRows,
	unsigned int* puiCount, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		std::uint32_t uiCount = 0;

		if ( pHist == nullptr )
		{
			THROW( "Invalid histogram pointer parameter [ NULL ]." );
		}

		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			auto pImgHist = arc::gen3::CArcImage<>::histogram(  static_cast< const std::uint16_t* >( pBuf ),
																uiCol1,
																uiCol2,
																uiRow1,
																uiRow2,
																uiCols,
																uiRows,
																uiCount );

			std::memcpy( pHist, pImgHist.get(), ( uiCount * sizeof( std::uint32_t ) ) );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			auto pImgHist = arc::gen3::CArcImage<arc::gen3::image::BPP_32>::histogram(  static_cast< const std::uint32_t* >( pBuf ),
																						uiCol1,
																						uiCol2,
																						uiRow1,
																						uiRow2,
																						uiCols,
																						uiRows,
																						uiCount );

			std::memcpy( pHist, pImgHist.get(), ( uiCount * sizeof( std::uint32_t ) ) );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}

		if ( puiCount != nullptr )
		{
			*puiCount = uiCount;
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_add                                                                                                    |
// +------------------------------------------------------------------------------------------------------------------+
// |  Adds two image buffers together pixel by pixel. The result is a pointer to a 64-bit buffer.                     |
// |                                                                                                                  |
// |  <IN>  -> pAdd  	- A pointer to a 64-bit array.                                                                |
// |  <IN>  -> pBuf1	- Pointer to the first image buffer.                                                          |
// |  <IN>  -> pBuf2	- Pointer to the second image buffer.                                                         |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_add( unsigned long long* pAdd, const void* pBuf1, const void* pBuf2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( pAdd == nullptr )
		{
			THROW( "Invalid addition pointer parameter [ NULL ]." );
		}

		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			auto pImgAdd = arc::gen3::CArcImage<>::add( static_cast< const std::uint16_t* >( pBuf1 ),
														static_cast< const std::uint16_t* >( pBuf2 ),
														uiCols,
														uiRows );

			std::memcpy( pAdd, pImgAdd.get(), ( uiCols * uiRows * sizeof( std::uint64_t ) ) );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			auto pImgAdd = arc::gen3::CArcImage<arc::gen3::image::BPP_32>::add( static_cast< const std::uint32_t* >( pBuf1 ),
																				static_cast< const std::uint32_t* >( pBuf2 ),
																				uiCols,
																				uiRows );

			std::memcpy( pAdd, pImgAdd.get(), ( uiCols * uiRows * sizeof( std::uint64_t ) ) );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_subtract                                                                                               |
// +------------------------------------------------------------------------------------------------------------------+
// |  Subtracts two image buffers pixel by pixel. Buffer two is subtracted from buffer one.                           |
// |                                                                                                                  |
// |  <IN>  -> pSub  	- Pointer to a buffer with the same data type as the two images being subtracted.             |
// |  <IN>  -> pBuf1	- Pointer to the first image buffer.                                                          |
// |  <IN>  -> pBuf2	- Pointer to the second image buffer.                                                         |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_subtract( void* pSub, const void* pBuf1, const void* pBuf2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( pSub == nullptr )
		{
			THROW( "Invalid subtraction pointer parameter [ NULL ]." );
		}

		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			auto pImgSub = arc::gen3::CArcImage<>::subtract( static_cast< const std::uint16_t* >( pBuf1 ),
															 static_cast< const std::uint16_t* >( pBuf2 ),
															 uiCols,
															 uiRows );

			std::memcpy( pSub, pImgSub.get(), ( uiCols * uiRows * sizeof( std::uint16_t ) ) );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			auto pImgSub = arc::gen3::CArcImage<arc::gen3::image::BPP_32>::subtract( static_cast< const std::uint32_t* >( pBuf1 ),
																					 static_cast< const std::uint32_t* >( pBuf2 ),
																					 uiCols,
																					 uiRows );

			std::memcpy( pSub, pImgSub.get(), ( uiCols * uiRows * sizeof( std::uint32_t ) ) );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_subtractHalves                                                                                         |
// +------------------------------------------------------------------------------------------------------------------+
// |  Subtracts one half of an image from the other. The first half of the image buffer is replaced with the new.     |
// |                                                                                                                  |
// |  <IN>  -> pBuf		- Pointer to the image buffer. Result is placed in this buffer.                               |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_subtractHalves( void* pBuf, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			arc::gen3::CArcImage<>::subtractHalves( static_cast< std::uint16_t* >( pBuf ), uiCols, uiRows );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			arc::gen3::CArcImage<arc::gen3::image::BPP_32>::subtractHalves( static_cast< std::uint32_t* >( pBuf ), uiCols, uiRows );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_divide                                                                                                 |
// +------------------------------------------------------------------------------------------------------------------+
// |  Divide two image buffers pixel by pixel. The result replaces the original data in first buffer.                 |
// |                                                                                                                  |
// |  <IN>  -> pDiv		- Pointer to a buffer with the same data type as the two images being divided.                |
// |  <IN>  -> pBuf1	- Pointer to the first image buffer.                                                          |
// |  <IN>  -> pBuf2	- Pointer to the second image buffer.                                                         |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_divide( void* pDiv, const void* pBuf1, const void* pBuf2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus )
{
	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			auto pImgDiv = arc::gen3::CArcImage<>::divide( static_cast< const std::uint16_t* >( pBuf1 ),
														   static_cast< const std::uint16_t* >( pBuf2 ),
														   uiCols,
														   uiRows );

			std::memcpy( pDiv, pImgDiv.get(), ( uiCols * uiRows * sizeof( std::uint16_t ) ) );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			auto pImgDiv = arc::gen3::CArcImage<arc::gen3::image::BPP_32>::divide( static_cast< const std::uint32_t* >( pBuf1 ),
																				   static_cast< const std::uint32_t* >( pBuf2 ),
																				   uiCols,
																				   uiRows );

			std::memcpy( pDiv, pImgDiv.get(), ( uiCols * uiRows * sizeof( std::uint32_t ) ) );
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_copy                                                                                                   |
// +------------------------------------------------------------------------------------------------------------------+
// |  Copies the source image buffer to the destination image buffer. The source buffer must be less than or equal in |
// |  dimensions to the destination buffer.                                                                           |
// |                                                                                                                  |
// |  <IN>  -> pDiv		- Pointer to a buffer with the same data type as the two images being divided.                |
// |  <IN>  -> pBuf1	- Pointer to the first image buffer.                                                          |
// |  <IN>  -> pBuf2	- Pointer to the second image buffer.                                                         |
// |  <IN>  -> uiCols	- The number of columns in the image.                                                         |
// |  <IN>  -> uiRows	- The number of rows in the image.                                                            |
// |  <OUT> -> pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.                                    |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API void ArcImage_copy( void* pBuf1, unsigned int uiCols1, unsigned int uiRows1, const void* pBuf2, unsigned int uiCols2, unsigned int uiRows2, ArcStatus_t* pStatus )
{
	std::uint32_t uiSize1 = 0;
	std::uint32_t uiSize2 = 0;

	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			uiSize1 = ( uiCols1 * uiRows1 * sizeof( arc::gen3::image::BPP_16 ) );
			uiSize2 = ( uiCols2 * uiRows2 * sizeof( arc::gen3::image::BPP_16 ) );

			//  Verify image dimensions
			// +--------------------------------------------+
			if ( uiSize2 > uiSize1 )
			{
				THROW( "Source buffer must be less than or equal to destination buffer size!\nSource size: %u\nDestination size: %u",
					uiSize1,
					uiSize2 );
			}

			arc::gen3::CArcImage<arc::gen3::image::BPP_16>::copyMemory( pBuf1, const_cast< void* >( pBuf2 ), uiSize1 );
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			uiSize1 = ( uiCols1 * uiRows1 * sizeof( arc::gen3::image::BPP_32 ) );
			uiSize2 = ( uiCols2 * uiRows2 * sizeof( arc::gen3::image::BPP_32 ) );

			//  Verify image dimensions
			// +--------------------------------------------+
			if ( uiSize2 > uiSize1 )
			{
				THROW( "Source buffer must be less than or equal to destination buffer size!\nSource size: %u\nDestination size: %u",
					uiSize1,
					uiSize2 );
			}

			arc::gen3::CArcImage<arc::gen3::image::BPP_32>::copyMemory( pBuf1, const_cast< void* >( pBuf2 ), uiSize1 );
		}

		else
		{
			THROW(
				"Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().",
				g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_maxTVal                                                                                                |
// +------------------------------------------------------------------------------------------------------------------+
// |  Determines the maximum value for the current data type in use. Example, for unsigned short: 2^16 = 65536.       |
// |  Returns the maximum value for the data type currently in use.                                                   |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API unsigned int ArcImage_maxTVal( ArcStatus_t* pStatus )
{
	std::uint32_t uiTVal = 0;

	INIT_STATUS( pStatus, ARC_STATUS_OK )

	try
	{
		if ( g_uiCurrentBpp == IMAGE_BPP16 )
		{
			uiTVal = arc::gen3::CArcImage<arc::gen3::image::BPP_16>::maxTVal();
		}

		else if ( g_uiCurrentBpp == IMAGE_BPP32 )
		{
			uiTVal = arc::gen3::CArcImage<arc::gen3::image::BPP_32>::maxTVal();
		}

		else
		{
			THROW( "Invalid bits-per-pixel setting [ %d ]. Must be IMAGE_BPP16 or IMAGE_BPP32. See ArcImage_selectInstance().", g_uiCurrentBpp );
		}
	}
	catch ( const std::exception& e )
	{
		SET_ERROR_STATUS( pStatus, e );
	}

	return uiTVal;
}


// +------------------------------------------------------------------------------------------------------------------+
// |  ArcImage_getLastError                                                                                           |
// +------------------------------------------------------------------------------------------------------------------+
// |  Returns the last reported error message.                                                                        |
// +------------------------------------------------------------------------------------------------------------------+
GEN3_CARCIMAGE_API const char* ArcImage_getLastError( void )
{
	return const_cast< const char* >( g_pErrMsg->data() );
}
