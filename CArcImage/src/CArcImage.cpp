// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  CArcImage.cpp  ( Gen3 )                                                                                  |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE:  Defines the exported functions for the CArcImage DLL.                                                 |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 26, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2014 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+

#include <iostream>
#include <type_traits>
#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>
#include <cmath>

#include <CArcImage.h>




namespace arc
{
	namespace gen3
	{


		// +----------------------------------------------------------------------------------------------------------+
		// |  Macro - VERIFY_BUFFER                                                                                   |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Verifies that the specified buffer is not equal to nullptr. Throws exception on error.                  |
		// |                                                                                                          |
		// |  <IN> -> pBuf	- Pointer to the buffer to check.                                                         |
		// +----------------------------------------------------------------------------------------------------------+
		#define VERIFY_BUFFER( pBuf )													\
							if ( pBuf == nullptr )										\
							{															\
								THROW( "Invalid buffer parameter ( nullptr )!" );		\
							}


		// +----------------------------------------------------------------------------------------------------------+
		// |  Macro - VERIFY_ROW                                                                                      |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Verifies that the specified row value is less than the total number of rows. Throws exception on error. |
		// |                                                                                                          |
		// |  <IN> -> row	- The row to check.                                                                       |
		// |  <IN> -> rows	- The total row length ( i.e. image row count )                                           |
		// +----------------------------------------------------------------------------------------------------------+
		#define VERIFY_ROW( row, rows )																	\
							if ( ( row < 0 ) || ( row > rows ) ) 										\
							{																			\
								THROW( "Invalid row [ %u ]! Must be between 0 and %u!", row, rows );	\
							}


		// +----------------------------------------------------------------------------------------------------------+
		// |  Macro - VERIFY_ROWS                                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Verifies that the specified number of rows is not equal to zero. Throws exception on error.             |
		// |                                                                                                          |
		// |  <IN> -> rows - The total row length ( i.e. image row count )                                            |
		// +----------------------------------------------------------------------------------------------------------+
		#define VERIFY_ROWS( rows )																		\
							if ( rows <= 0 )															\
							{																			\
								THROW( "Invalid row count [ %u ]! Cannot be zero!", rows );				\
							}


		// +----------------------------------------------------------------------------------------------------------+
		// |  Macro - VERIFY_COLS                                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Verifies that the specified number of cols is not equal to zero. Throws exception on error.             |
		// |                                                                                                          |
		// |  <IN> -> cols - The total column length ( i.e. image column count )                                      |
		// +----------------------------------------------------------------------------------------------------------+
		#define VERIFY_COLS( cols )																		\
							if ( cols <= 0 )															\
							{																			\
								THROW( "Invalid column count [ %u ]! Cannot be zero!", cols );			\
							}


		// +----------------------------------------------------------------------------------------------------------+
		// |  Macro - VERIFY_COL                                                                                      |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Verifies that the specified column value is less than the total number of columns. Throws exception on  |
		// |  error.                                                                                                  |
		// |                                                                                                          |
		// |  <IN> -> col	- The column to check.                                                                    |
		// |  <IN> -> cols	- The total column length ( i.e. image column count )                                     |
		// +----------------------------------------------------------------------------------------------------------+
		#define VERIFY_COL( col, cols )																		\
							if ( ( col < 0 ) || ( col > cols ) )											\
							{																				\
								THROW( "Invalid column [ %u ]! Must be between 0 and %u!", col, cols );		\
							}


		// +----------------------------------------------------------------------------------------------------------+
		// |  Macro - VERIFY_RANGE_ORDER                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Verifies that value1 is less than value2. Throws exception on error.                                    |
		// |                                                                                                          |
		// |  <IN> -> value1 - The first ( lesser ) range value.                                                      |
		// |  <IN> -> value2 - The second ( higher ) range value.                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		#define VERIFY_RANGE_ORDER( value1, value2 )																\
						if ( value2 < value1 )																		\
						{																							\
							THROW( "Invalid range order [ %u < %u ]! Values must be reversed!", value2, value1 );	\
						}


		// +----------------------------------------------------------------------------------------------------------+
		// |  Macro - TYPE_SIZE_OF                                                                                    |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the correct byte size associated with the specified bits-per-pixel.                             |
		// |                                                                                                          |
		// |  <IN> -> eBpp - The bits-per-pixel value.                                                                |
		// +----------------------------------------------------------------------------------------------------------+
		#define TYPE_SIZE_OF( bpp )		( ( bpp == arc::gen3::CArcBase::e_BPP::BPP16 ) ?					\
											static_cast<std::uint32_t>( sizeof( std::uint16_t ) ) :			\
											static_cast<std::uint32_t>( sizeof( std::uint32_t ) ) )


		// +----------------------------------------------------------------------------------------------------------+
		// |  Constant - DEG2RAD                                                                                      |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Degree to radian conversion factor.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		#define DEG2RAD		( 3.14159 / 180 )


		// +----------------------------------------------------------------------------------------------------------+
		// |  GenIV image channel type                                                                                |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Image channel insertion pointers.                                                                       |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		struct imageChannel_t
		{
			T* pStart;
			T* pEnd;
			T* pInserter;
		};


		// +----------------------------------------------------------------------------------------------------------+
		// | Library build and version info                                                                           |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> const std::string CArcImage<T>::m_sVersion = std::string( "ARC Gen III Image API Library v3.6.    " ) +

		#ifdef _WINDOWS
			CArcBase::formatString( "[ Compiler Version: %d, Built: %s ]", _MSC_VER, __TIMESTAMP__ );
		#else
			arc::gen3::CArcBase::formatString( "[ Compiler Version: %d.%d.%d, Built: %s %s ]", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __DATE__, __TIME__ );
		#endif


		// +----------------------------------------------------------------------------------------------------------+
		// |  Destructor                                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Destroys the class.                                                                                     |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> CArcImage<T>::~CArcImage( void )
		{
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  version                                                                                                 |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns a textual representation of the library version.                                                |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> const std::string CArcImage<T>::version( void )
		{
			return m_sVersion;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  fill                                                                                                    |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Fills the specified buffer with the specified value.                                                    |
		// |                                                                                                          |
		// |  <IN> -> pBuf	  - Pointer to the image data buffer.                                                     |
		// |  <IN> -> uiCols  - The image column size ( in pixels ).                                                  |
		// |  <IN> -> uiRows  - The image row size ( in pixels ).                                                     |
		// |  <IN> -> uiValue - The value to fill the buffer with.                                                    |
		// |                                                                                                          |
		// |  Throws std::invalid_argument, std::invalid_argument                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcImage<T>::fill( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows, T uiValue )
		{
			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid buffer reference ( nullptr )." );
			}

			if ( uiValue >= maxTVal() )
			{
				THROW_OUT_OF_RANGE( uiValue, std::make_pair( 0, ( maxTVal() - 1 ) ) );
			}

			for ( std::uint32_t i = 0; i < ( uiCols * uiRows ); i++ )
			{
				pBuf[ i ] = uiValue;
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  fill                                                                                                    |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Fills the specified buffer with the specified value.                                                    |
		// |                                                                                                          |
		// |  <IN> -> pBuf	  - Pointer to the image data buffer.                                                     |
		// |  <IN> -> uiBytes - The number of bytes in the image data buffer.                                         |
		// |  <IN> -> uiValue - The value to fill the buffer with.                                                    |
		// |                                                                                                          |
		// |  Throws std::invalid_argument, std::invalid_argument                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcImage<T>::fill( T* pBuf, std::uint32_t uiBytes, T uiValue )
		{
			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid buffer reference ( nullptr )." );
			}

			if ( uiValue >= maxTVal() )
			{
				THROW_OUT_OF_RANGE( uiValue, std::make_pair( 0, ( maxTVal() - 1 ) ) );
			}

			for ( std::uint32_t i = 0; i < ( uiBytes / sizeof( T ) ); i++ )
			{
				pBuf[ i ] = uiValue;
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  fillWithGradient                                                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Fills the specified buffer with a gradient pattern.                                                     |
		// |                                                                                                          |
		// |  <IN> -> pBuf	  - Pointer to the image data buffer.                                                     |
		// |  <IN> -> uiCols  - The image column size ( in pixels ).                                                  |
		// |  <IN> -> uiRows  - The image row size ( in pixels ).                                                     |
		// |                                                                                                          |
		// |  Throws std::invalid_argument on error.                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcImage<T>::fillWithGradient( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid buffer reference ( nullptr )." );
			}

			zeroMemory( pBuf, ( ( uiCols * uiRows ) * sizeof( T ) ) );

			T uiValue = 0;

			for ( decltype( uiRows ) r = 0; r < uiRows; r++ )
			{
				for ( decltype( uiCols ) c = 0; c < uiCols; c++ )
				{
					pBuf[ c + r * uiCols ] = uiValue;
				}

				uiValue += static_cast< T >( ( maxTVal() - 1 ) / uiRows );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  fillWithSmiley                                                                                          |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Fills the specified buffer with zeroes and puts a smiley face at the center.                            |
		// |                                                                                                          |
		// |  <IN> -> pBuf	  - Pointer to the image data buffer.                                                     |
		// |  <IN> -> uiCols  - The image column size ( in pixels ).                                                  |
		// |  <IN> -> uiRows  - The image row size ( in pixels ).                                                     |
		// |                                                                                                          |
		// |  Throws std::invalid_argument on error.                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcImage<T>::fillWithSmiley( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid buffer reference ( nullptr )." );
			}

			zeroMemory( pBuf, ( ( uiCols * uiRows ) * sizeof( T ) ) );

			std::uint32_t uiRadius = std::min( ( uiRows / 2 ), ( uiCols / 2 ) ) - 10;

			//  Draw Head
			// +---------------------------------------------------------------------------- +
			drawGradientFillCircle( ( uiCols / 2 ),
									( uiRows / 2 ),
									uiRadius,
									uiCols,
									uiRows,
									pBuf );

			//  Draw Left Eye
			// +---------------------------------------------------------------------------- +
			std::uint32_t uiRowFactor = static_cast< std::uint32_t >( uiRadius / 2.5 );

			drawFillCircle( ( uiCols / 2 ) - uiRowFactor,
							( uiRows / 2 ) + uiRowFactor,
							( uiRadius / 5 ),
							uiCols,
							pBuf );

			//  Draw Right Eye
			// +---------------------------------------------------------------------------- +
			drawFillCircle( ( uiCols / 2 ) + uiRowFactor,
							( uiRows / 2 ) + uiRowFactor,
							( uiRadius / 5 ),
							uiCols,
							pBuf );

			//  Draw Mouth
			// +---------------------------------------------------------------------------- +
			for ( decltype( uiRadius ) i = 0; i < ( uiRadius / 2 ); i++ )
			{
				drawSemiCircle( ( uiCols / 2 ),
								( uiRows / 2 ) - ( uiRowFactor / 2 ),
								i,
								180,
								360,
								uiCols,
								pBuf );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  fillWithRamp                                                                                            |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Fills the specified buffer with a ramp image. Data has the form 0, 1, 2, ..., 65535, 0, 1, ....         |
		// |                                                                                                          |
		// |  <IN> -> pBuf	 - Pointer to the image data buffer.                                                      |
		// |  <IN> -> uiCols - The image column size ( in pixels ).                                                   |
		// |  <IN> -> uiRows - The image row size ( in pixels ).                                                      |
		// |                                                                                                          |
		// |  Throws std::invalid_argument on error.                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcImage<T>::fillWithRamp( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid buffer reference ( nullptr )." );
			}

			T uiValue = 0;

			for ( std::uint32_t i = 0; i < ( uiCols * uiRows ); i++ )
			{
				pBuf[ i ] = uiValue;

				uiValue++;

				if ( uiValue >= maxTVal() ) { uiValue = 0; }
			}

			containsValidRamp( pBuf, uiCols, uiRows );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  containsValidRamp                                                                                       |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Verify a ramp synthetic image. Data has the form 0, 1, 2, ..., 65535, 0, 1, ....                        |
		// |                                                                                                          |
		// |  <IN> -> pBuf	 - Pointer to the image data buffer.                                                      |
		// |  <IN> -> uiCols - The image column size ( in pixels ).                                                   |
		// |  <IN> -> uiRows - The image row size ( in pixels ).                                                      |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcImage<T>::containsValidRamp( const T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid buffer reference ( nullptr )." );
			}

			T uiValue = pBuf[ 0 ];

			std::uint32_t uiPixel = 0;

			for ( std::uint32_t r = 0; r < uiRows; r++ )
			{
				for ( std::uint32_t c = 0; c < uiCols; c++ )
				{
					if ( pBuf[ uiPixel ] != uiValue )
					{
						THROW( "Invalid ramp image. Expected %u at col %u row %u, found %u",
								uiValue,
								c,
								r,
								pBuf[ uiPixel ] );
					}

					uiPixel++;
					uiValue++;

					if ( uiValue >= maxTVal() )
					{
						uiValue = pBuf[ c + r * uiCols + 1 ];
					}
				}
			}

			//for ( std::uint32_t i = 0; i < ( uiCols * uiRows ); i++ )
			//{
			//	if ( pBuf[ i ] != uiValue )
			//	{
			//		std::uint32_t uiRowNum = ( i / uiCols );
			//		std::uint32_t uiColNum = ( i % uiCols );

			//		std::cout << ""
			//		THROW( "Invalid ramp image. Expected %u at col %u row %u, found %u",
			//				uiValue,
			//				uiColNum,
			//				uiRowNum,
			//				pBuf[ i ] );
			//	}

			//	uiValue++;

			//	//if ( uiValue >= T_SIZE( T ) ) { uiValue = 0; }
			//	if ( uiValue >= maxTVal() ) { uiValue = 0; }
			//}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  countPixels                                                                                             |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Count the number of pixels having the specified value.                                                  |
		// |                                                                                                          |
		// |  <IN> -> pBuf	   - Pointer to the image data buffer.                                                     |
		// |  <IN> -> uiCols  - The image column size ( in pixels ).                                                  |
		// |  <IN> -> uiRows  - The image row size ( in pixels ).                                                     |
		// |  <IN> -> uwValue - The pixel value to include in the count.                                              |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		std::uint32_t CArcImage<T>::countPixels( const T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows, std::uint16_t uwValue )
		{
			return countPixels( pBuf, ( uiCols * uiRows ), uwValue );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  countPixels                                                                                             |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Count the number of pixels having the specified value.                                                  |
		// |                                                                                                          |
		// |  <IN> -> pBuf	    - Pointer to the image data buffer.                                                   |
		// |  <IN> -> uiBufSize - The image buffer size ( in pixels ).                                                |
		// |  <IN> -> uwValue   - The pixel value to include in the count.                                            |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		std::uint32_t CArcImage<T>::countPixels( const T* pBuf, std::uint32_t uiBufSize, std::uint16_t uwValue )
		{
			std::uint32_t uiCount = 0;

			VERIFY_BUFFER( pBuf )

			if ( pBuf != nullptr )
			{
				for ( decltype( uiBufSize ) i = 0; i < uiBufSize; i++ )
				{
					if ( pBuf[ i ] == uwValue )
					{
						uiCount++;
					}
				}
			}
			
			return uiCount;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getPixel                                                                                                |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the value of a pixel at the specified row and column.                                           |
		// |                                                                                                          |
		// |  <IN> -> pBuf		- Pointer to the image data buffer.                                                   |
		// |  <IN> -> uiCol		- The pixel column number.                                                            |
		// |  <IN> -> uiRow		- The pixel row number.                                                               |
		// |  <IN> -> uiCols	- The image column size ( in pixels ).                                                |
		// |  <IN> -> uiRows	- The image row size ( in pixels ).                                                   |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> T
		CArcImage<T>::getPixel( const T* pBuf, std::uint32_t uiCol, std::uint32_t uiRow, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			VERIFY_BUFFER( pBuf )

			VERIFY_COL( uiCol, uiCols )

			VERIFY_ROW( uiRow, uiRows )

			return pBuf[ uiCol + ( uiRow * uiCols ) ];
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getRegion                                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		// |   Returns a region of an image.                                                                          |
		// |                                                                                                          |
		// |  <IN> -> pBuf		- Pointer to the image data buffer.                                                   |
		// |  <IN> -> uiCol1	- The start column.                                                                   |
		// |  <IN> -> uiCol2	- The end column.                                                                     |
		// |  <IN> -> uiRow1	- The start row.                                                                      |
		// |  <IN> -> uiRow2	- The end row.                                                                        |
		// |  <IN> -> uiCols	- The image column size ( in pixels ).                                                |
		// |  <IN> -> uiRows	- The image row size ( in pixels ).                                                   |
		// |  <OUT>-> uiCount	- The pixel count of the returned array.                                              |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>  std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>>
		CArcImage<T>::getRegion( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow1,
								 std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount )
		{
			VERIFY_BUFFER( pBuf )

			VERIFY_COL( uiCol1, uiCols )

			VERIFY_COL( uiCol2, uiCols )

			VERIFY_ROW( uiRow1, uiRows )

			VERIFY_ROW( uiRow2, uiRows )

			VERIFY_RANGE_ORDER( uiCol1, uiCol2 )

			VERIFY_RANGE_ORDER( uiRow1, uiRow2 )

			uiCount = ( ( uiCol2 - uiCol1 ) * ( uiRow2 - uiRow1 ) );

			std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>> pRegion( new T[ uiCount ], arc::gen3::image::ArrayDeleter<T>() );

			if ( pRegion == nullptr )
			{
				THROW( "Failed to allocate region data buffer!" );
			}

			auto iRegionIndex = 0;

			for ( auto r = uiRow1; r < uiRow2; r++ )
			{
				for ( auto c = uiCol1; c < uiCol2; c++ )
				{
					pRegion[ iRegionIndex ] = pBuf[ c + r * uiCols ];

					iRegionIndex++;
				}
			}

			return std::move( pRegion );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getRow                                                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns all or part of a image row.                                                                     |
		// |                                                                                                          |
		// |  <IN> -> pBuf	  - Pointer to the image data buffer.                                                     |
		// |  <IN> -> uiCol1  - The start column.                                                                     |
		// |  <IN> -> uiCol2  - The end column.                                                                       |
		// |  <IN> -> uiRow   - The row to read from.                                                                 |
		// |  <IN> -> uiCols  - The image column size ( in pixels ).                                                  |
		// |  <IN> -> uiRows  - The image row size ( in pixels ).                                                     |
		// |  <IN> -> uiCount - The pixel count of the returned array.                                                |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>>
		CArcImage<T>::getRow( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount )
		{
			VERIFY_BUFFER( pBuf )

			VERIFY_RANGE_ORDER( uiCol1, uiCol2 )

			VERIFY_ROW( uiRow, uiRows )

			uiCount = ( ( uiCol2 - uiCol1 ) == 0 ? 1 : ( uiCol2 - uiCol1 ) );

			std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>> pRow( new T[ uiCount ], arc::gen3::image::ArrayDeleter<T>() );

			if ( pRow == nullptr )
			{
				THROW( "Failed to allocate row data buffer!" );
			}

			copyMemory( reinterpret_cast< void* >( pRow.get() ),
						reinterpret_cast< void* >( const_cast< T* >( &pBuf[ uiCol1 + uiRow * uiCols ] ) ),
						( uiCount * sizeof( T ) ) );

			return std::move( pRow );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getCol                                                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns all or part of a image row.                                                                     |
		// |                                                                                                          |
		// |  <IN> -> pBuf	  - Pointer to the image data buffer.                                                     |
		// |  <IN> -> uiCol   - The column to read from                                                               |
		// |  <IN> -> uiRow1  - The start row.                                                                        |
		// |  <IN> -> uiRow2  - The end row.                                                                          |
		// |  <IN> -> uiCols  - The image column size ( in pixels ).                                                 |
		// |  <IN> -> uiRows  - The image row size ( in pixels ).                                                     |
		// |  <IN> -> uiCount - The pixel count of the returned array.                                                |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>>
		CArcImage<T>::getCol( const T* pBuf, std::uint32_t uiCol, std::uint32_t uiRow1, std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount )
		{
			VERIFY_BUFFER( pBuf )

			VERIFY_RANGE_ORDER( uiRow1, uiRow2 )

			VERIFY_COL( uiCol, uiCols )

			uiCount = ( ( uiRow2 - uiRow1 ) == 0 ? 1 : ( uiRow2 - uiRow1 ) );

			std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>> pCol( new T[ uiCount ], arc::gen3::image::ArrayDeleter<T>() );

			if ( pCol == nullptr )
			{
				THROW( "Failed to allocate row data buffer!" );
			}

			for ( std::uint32_t row = uiRow1, i = 0; row < uiRow2; row++, i++ )
			{
				pCol.get()[ i ] = pBuf[ uiCol + row * uiCols ];
			}
			
			return std::move( pCol );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getRowArea                                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns a row of pixel data where each value is the average over the specified range of columns.        |
		// |                                                                                                          |
		// |  <IN> -> pBuf	  - Pointer to the image data buffer.                                                     |
		// |  <IN> -> uiCol1  - The start column.                                                                     |
		// |  <IN> -> uiCol2  - The end column.                                                                       |
		// |  <IN> -> uiRow1  - The start row.                                                                        |
		// |  <IN> -> uiRow2  - The end row.                                                                          |
		// |  <IN> -> uiCols  - The image column size ( in pixels ).                                                 |
		// |  <IN> -> uiRows  - The image row size ( in pixels ).                                                     |
		// |  <IN> -> uiCount - The pixel count of the returned array.                                                |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<double[], arc::gen3::image::ArrayDeleter<double>>
		CArcImage<T>::getRowArea( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow1,
								  std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount )
		{
			double  gRowSum = 0.0;

			VERIFY_ROW( uiRow1, uiRows )

			VERIFY_ROW( uiRow2, uiRows )

			VERIFY_COL( uiCol1, uiCols )

			VERIFY_COL( uiCol2, uiCols )

			VERIFY_RANGE_ORDER( uiCol1, uiCol2 )

			VERIFY_RANGE_ORDER( uiRow1, uiRow2 )

			VERIFY_BUFFER( pBuf )

			uiCount = ( ( uiRow2 - uiRow1 ) == 0 ? 1 : ( uiRow2 - uiRow1 ) );
			
			std::unique_ptr<double[], arc::gen3::image::ArrayDeleter<double>> pAreaBuf( new double[ uiCount ], arc::gen3::image::ArrayDeleter<double>() );

			if ( pAreaBuf == nullptr )
			{
				THROW( "Failed to allocate row data buffer!" );
			}

			for ( std::uint32_t row = uiRow1, i = 0; row < uiRow2; row++, i++ )
			{
				gRowSum = 0;

				for ( std::uint32_t col = uiCol1; col < uiCol2; col++ )
				{
					gRowSum += pBuf[ col + row * uiCols ];
				}

				pAreaBuf.get()[ i ] = gRowSum / ( static_cast< double >( uiCol2 - uiCol1 ) );
			}

			return pAreaBuf;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getColArea                                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns a column of pixel data where each value is the average over the specified range of rows.        |
		// |                                                                                                          |
		// |  <IN> -> pBuf	  - Pointer to the image data buffer.                                                     |
		// |  <IN> -> uiCol1  - The start column.                                                                     |
		// |  <IN> -> uiCol2  - The end column.                                                                       |
		// |  <IN> -> uiRow1  - The start row.                                                                        |
		// |  <IN> -> uiRow2  - The end row.                                                                          |
		// |  <IN> -> uiCols  - The image column size ( in pixels ).                                                 |
		// |  <IN> -> uiRows  - The image row size ( in pixels ).                                                     |
		// |  <IN> -> uiCount - The pixel count of the returned array.                                                |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<double[], arc::gen3::image::ArrayDeleter<double>>
		CArcImage<T>::getColArea( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow1,
								  std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount )
		{
			double  gColSum = 0.0;

			VERIFY_ROW( uiRow1, uiRows )
				
			VERIFY_ROW( uiRow2, uiRows )

			VERIFY_COL( uiCol1, uiCols )

			VERIFY_COL( uiCol2, uiCols )

			VERIFY_RANGE_ORDER( uiCol1, uiCol2 )

			VERIFY_RANGE_ORDER( uiRow1, uiRow2 )

			VERIFY_BUFFER( pBuf )

			uiCount = ( ( uiCol2 - uiCol1 ) == 0 ? 1 : ( uiCol2 - uiCol1 ) );

			std::unique_ptr<double[], arc::gen3::image::ArrayDeleter<double>> pAreaBuf( new double[ uiCount ], arc::gen3::image::ArrayDeleter<double>() );

			if ( pAreaBuf == nullptr )
			{
				THROW( "Failed to allocate column data buffer!" );
			}

			for ( std::uint32_t col = uiCol1, i = 0; col < uiCol2; col++, i++ )
			{
				gColSum = 0;

				for ( std::uint32_t row = uiRow1; row < uiRow2; row++ )
				{
					gColSum += pBuf[ col + row * uiCols ];
				}

				pAreaBuf.get()[ i ] = gColSum / ( static_cast< double >( uiRow2 - uiRow1 ) );
			}

			return pAreaBuf;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getStats                                                                                                |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Calculates the image min, max, mean, variance, standard deviation, total pixel count and saturated      |
		// |  pixel count over the specified image buffer cols and rows.                                              |
		// |                                                                                                          |
		// |  <IN> -> pBuf	 - Pointer to the image data buffer.                                                      |
		// |  <IN> -> uiCol1 - The start column.                                                                      |
		// |  <IN> -> uiCol2 - The end column.                                                                        |
		// |  <IN> -> uiRow1 - The start row.                                                                         |
		// |  <IN> -> uiRow2 - The end row.                                                                           |
		// |  <IN> -> uiCols - The image column size ( in pixels ).                                                   |
		// |  <IN> -> uiRows - The image row size ( in pixels ).                                                      |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<arc::gen3::image::CStats>
		CArcImage<T>::getStats( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow1, std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			double gDevSqrdSum = 0.0;
			double gVal = 0.0;
			double gSum = 0.0;

			VERIFY_ROW( uiRow1, uiRows )
				
			VERIFY_ROW( uiRow2, uiRows )

			VERIFY_COL( uiCol1, uiCols )

			VERIFY_COL( uiCol2, uiCols )
				
			VERIFY_RANGE_ORDER( uiCol1, uiCol2 )

			VERIFY_RANGE_ORDER( uiRow1, uiRow2 )

			VERIFY_BUFFER( pBuf )

			//double gMaxBpp = T_SIZE( T );
			double gMaxBpp = maxTVal();

			std::unique_ptr<arc::gen3::image::CStats> pStats( new arc::gen3::image::CStats() );

			if ( pStats == nullptr )
			{
				THROW( "Failed to allocate stats data buffer!" );
			}

			pStats->gMin = gMaxBpp;
			
			if ( uiRow1 == uiRow2 ) { uiRow2++; }
			if ( uiCol1 == uiCol2 ) { uiCol2++; }

			double gTotalPixelCount = ( ( uiRow2 - uiRow1 ) * ( uiCol2 - uiCol1 ) );

			pStats->gTotalPixels = gTotalPixelCount;

			for ( std::uint32_t i = uiRow1; i < uiRow2; i++ )
			{
				for ( std::uint32_t j = uiCol1; j < uiCol2; j++ )
				{
					gVal = static_cast< double >( pBuf[ j + i * uiCols ] );

					//
					// Determine min/max values
					//
					if ( gVal < pStats->gMin )
					{
						pStats->gMin = gVal;
					}

					else if ( gVal > pStats->gMax )
					{
						pStats->gMax = gVal;
					}

					//
					// Monitor for saturated pixels
					//
					//if ( gVal >= ( T_SIZE( T ) - 1 ) )
					if ( gVal >= ( maxTVal() - 1 ) )
					{
						pStats->gSaturatedCount++;
					}

					gSum += gVal;
				}
			}

			// Calculate mean
			pStats->gMean = gSum / gTotalPixelCount;

			for ( std::uint32_t i = uiRow1; i < uiRow2; i++ )
			{
				for ( std::uint32_t j = uiCol1; j < uiCol2; j++ )
				{
					double gPixVal = static_cast< double >( pBuf[ j + i * uiCols ] );
					
					gDevSqrdSum += std::pow( ( gPixVal - pStats->gMean ), 2 );
				}
			}

			pStats->gVariance = gDevSqrdSum / gTotalPixelCount;
			pStats->gStdDev = std::sqrt( pStats->gVariance );

			return pStats;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getStats                                                                                                |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Calculates the image min, max, mean, variance, standard deviation, total pixel count and saturated      |
		// |  pixel count over the entire image.                                                                      |
		// |                                                                                                          |
		// |  <IN> -> pBuf	 - Pointer to the image data buffer.                                                      |
		// |  <IN> -> uiCol1 - The start column.                                                                      |
		// |  <IN> -> uiCol2 - The end column.                                                                        |
		// |  <IN> -> uiRow1 - The start row.                                                                         |
		// |  <IN> -> uiRow2 - The end row.                                                                           |
		// |  <IN> -> uiCols - The image column size ( in pixels ).                                                   |
		// |  <IN> -> uiRows - The image row size ( in pixels ).                                                      |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		std::unique_ptr<arc::gen3::image::CStats> CArcImage<T>::getStats( const T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			return getStats( pBuf, 0, uiCols, 0, uiRows, uiCols, uiRows );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getDiffStats                                                                                            |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Calculates the min, max, mean, variance, standard deviation, total pixel count and saturated pixel      |
		// |  count for each image as well as the difference mean, variance and standard deviation over the specified |
		// |  image buffer cols and rows. This is used for photon transfer curves( PTC ).The two images MUST be the   |
		// |  same size or the methods behavior is undefined as this cannot be verified using the given parameters.   |
		// |                                                                                                          |
		// |  <IN> -> pBuf1	 - Pointer to the first image buffer.                                                     |
		// |  <IN> -> pBuf2	 - Pointer to the second image buffer.                                                    |
		// |  <IN> -> uiCol1 - The start column.                                                                      |
		// |  <IN> -> uiCol2 - The end column.                                                                        |
		// |  <IN> -> uiRow1 - The start row.                                                                         |
		// |  <IN> -> uiRow2 - The end row.                                                                           |
		// |  <IN> -> uiCols - The image column size ( in pixels ).                                                   |
		// |  <IN> -> uiRows - The image row size ( in pixels ).                                                      |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<arc::gen3::image::CDifStats>
		CArcImage<T>::getDiffStats( const T* pBuf1, const T* pBuf2, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow1, std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			double gSum		= 0.0;
			double gDifSum	= 0.0;
			double gVal1	= 0.0;
			double gVal2	= 0.0;

			VERIFY_ROW( uiRow1, uiRows )

			VERIFY_ROW( uiRow2, uiRows )

			VERIFY_COL( uiCol1, uiCols )

			VERIFY_COL( uiCol2, uiCols )

			VERIFY_RANGE_ORDER( uiCol1, uiCol2 )

			VERIFY_RANGE_ORDER( uiRow1, uiRow2 )

			VERIFY_BUFFER( pBuf1 )

			VERIFY_BUFFER( pBuf2 )

			if ( uiRow1 == uiRow2 ) { uiRow2++; }
			if ( uiCol1 == uiCol2 ) { uiCol2++; }

			double gTotalPixelCount = ( ( uiRow2 - uiRow1 ) * ( uiCol2 - uiCol1 ) );

			std::unique_ptr<arc::gen3::image::CDifStats> pDifStats( new arc::gen3::image::CDifStats() );

			pDifStats->cStats1.set( *getStats( pBuf1, uiCol1, uiCol2, uiRow1, uiRow2, uiCols, uiRows ).get() );

			pDifStats->cStats2.set( *getStats( pBuf2, uiCol1, uiCol2, uiRow1, uiRow2, uiCols, uiRows ).get() );

			for ( std::uint32_t i = uiRow1; i < uiRow2; i++ )
			{
				for ( std::uint32_t j = uiCol1; j < uiCol2; j++ )
				{
					gVal1 = static_cast< double >( pBuf1[ j + i * uiCols ] );

					gVal2 = static_cast< double >( pBuf2[ j + i * uiCols ] );

					gSum += ( gVal1 - gVal2 );

					gDifSum += ( std::pow( ( pDifStats->cStats2.gMean - gVal2 ) - ( pDifStats->cStats1.gMean - gVal1 ), 2 ) );
				}
			}

			pDifStats->cDiffStats.gMean = std::fabs( gSum / gTotalPixelCount );
			pDifStats->cDiffStats.gVariance = gDifSum / gTotalPixelCount;
			pDifStats->cDiffStats.gStdDev = std::sqrt( pDifStats->cDiffStats.gVariance );

			return pDifStats;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getDiffStats                                                                                            |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Calculates the min, max, mean, variance, standard deviation, total pixel count and saturated pixel      |
		// |  count for each image as well as the difference mean, variance and standard deviation over the entire    |
		// |  image buffer. This is used for photon transfer curves( PTC ).The two images MUST be the same size or    |
		// |  the methods behavior is undefined as this cannot be verified using the given parameters.                |
		// |                                                                                                          |
		// |  <IN> -> pBuf1	 - Pointer to the first image buffer.                                                     |
		// |  <IN> -> pBuf2	 - Pointer to the second image buffer.                                                    |
		// |  <IN> -> uiCols - The image column size ( in pixels ).                                                   |
		// |  <IN> -> uiRows - The image row size ( in pixels ).                                                      |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<arc::gen3::image::CDifStats>
		CArcImage<T>::getDiffStats( const T* pBuf1, const T* pBuf2, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			return getDiffStats( pBuf1, pBuf2, 0, uiCols, 0, uiRows, uiCols, uiRows );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  histogram                                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Calculates the histogram over the specified image buffer columns and rows.                              |
		// |                                                                                                          |
		// |  <IN> -> pBuf		- Pointer to the image buffer.                                                        |
		// |  <IN> -> uiCol1	- The start column.                                                                   |
		// |  <IN> -> uiCol2	- The end column.                                                                     |
		// |  <IN> -> uiRow1	- The start row.                                                                      |
		// |  <IN> -> uiRow2	- The end row.                                                                        |
		// |  <IN> -> uiCols	- The image column size ( in pixels ).                                                |
		// |  <IN> -> uiRows	- The image row size ( in pixels ).                                                   |
		// |  <IN> -> uiCount	- The element count of the returned array. The size of the array depends on the       |
		// |                      image data type.                                                                    |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<std::uint32_t[], arc::gen3::image::ArrayDeleter<std::uint32_t>>
		CArcImage<T>::histogram( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow1,
								 std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount )
		{
			VERIFY_ROW( uiRow1, uiRows )

			VERIFY_ROW( uiRow2, uiRows )

			VERIFY_COL( uiCol1, uiCols )

			VERIFY_COL( uiCol2, uiCols )

			VERIFY_RANGE_ORDER( uiCol1, uiCol2 )

			VERIFY_RANGE_ORDER( uiRow1, uiRow2 )

			VERIFY_BUFFER( pBuf )

			std::unique_ptr<std::uint32_t[], arc::gen3::image::ArrayDeleter<std::uint32_t>>
			pHist( new std::uint32_t[ maxTVal() ], arc::gen3::image::ArrayDeleter<std::uint32_t>() );

			if ( pHist == nullptr )
			{
				THROW( "Failed to allocate histogram data buffer!" );
			}

			//uiCount = T_SIZE( T );
			uiCount = maxTVal();

			zeroMemory( pHist.get(), uiCount * sizeof( std::uint32_t ) );

			if ( uiRow1 == uiRow2 ) { uiRow2++; }
			if ( uiCol1 == uiCol2 ) { uiCol2++; }

			for ( std::uint32_t i = uiRow1; i < uiRow2; i++ )
			{
				for ( std::uint32_t j = uiCol1; j < uiCol2; j++ )
				{
					pHist.get()[ pBuf[ j + i * uiCols ] ]++;
				}
			}
			
			return pHist;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  histogram                                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Calculates the histogram over the entire image buffer.                                                  |
		// |                                                                                                          |
		// |  <IN> -> pBuf		- Pointer to the image buffer.                                                        |
		// |  <IN> -> uiCols	- The image column size ( in pixels ).                                                |
		// |  <IN> -> uiRows	- The image row size ( in pixels ).                                                   |
		// |  <IN> -> uiCount	- The element count of the returned array. The size of the array depends on the       |
		// |                      image data type.                                                                    |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<std::uint32_t[], arc::gen3::image::ArrayDeleter<std::uint32_t>>
		CArcImage<T>::histogram( const T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount )
		{
			return histogram( pBuf, 0, uiCols, 0, uiRows, uiCols, uiRows, uiCount );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  add                                                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Adds two image buffers together pixel by pixel. The result is a pointer to a 64-bit buffer.             |
		// |                                                                                                          |
		// |  <IN> -> pBuf1	 - Pointer to the first image buffer.                                                     |
		// |  <IN> -> pBuf2	 - Pointer to the second image buffer.                                                    |
		// |  <IN> -> uiCols - The image column size ( in pixels ).                                                   |
		// |  <IN> -> uiRows - The image row size ( in pixels ).                                                      |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<std::uint64_t[], arc::gen3::image::ArrayDeleter<std::uint64_t>>
		CArcImage<T>::add( const T* pBuf1, const T* pBuf2, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			VERIFY_BUFFER( pBuf1 )
	
			VERIFY_BUFFER( pBuf2 )

			std::uint32_t uiLength = ( uiCols * uiRows );

			std::unique_ptr< std::uint64_t[], arc::gen3::image::ArrayDeleter< std::uint64_t > > pAdd( new std::uint64_t[ uiLength ], arc::gen3::image::ArrayDeleter< std::uint64_t >() );

			if ( pAdd == nullptr )
			{
				THROW( "Failed to allocate addition data buffer!" );
			}

			for ( decltype( uiLength ) i = 0; i < uiLength; i++ )
			{
				pAdd.get()[ i ] = static_cast< std::uint64_t >( pBuf1[ i ] ) + static_cast< std::uint64_t >( pBuf2[ i ] );
			}

			return pAdd;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  subtract                                                                                                |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Subtracts two image buffers pixel by pixel. The result is a pointer to a buffer with the same data      |
		// |  type as the two images being subtracted. Buffer two is subtracted from buffer one.                      |
		// |                                                                                                          |
		// |  <IN> -> pBuf1	 - Pointer to the first image buffer.                                                     |
		// |  <IN> -> pBuf2	 - Pointer to the second image buffer.                                                    |
		// |  <IN> -> uiCols - The image column size ( in pixels ).                                                   |
		// |  <IN> -> uiRows - The image row size ( in pixels ).                                                      |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>>
		CArcImage<T>::subtract( const T* pBuf1, const T* pBuf2, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			VERIFY_BUFFER( pBuf1 )
	
			VERIFY_BUFFER( pBuf2 )

			std::uint32_t uiLength = ( uiCols * uiRows );

			std::unique_ptr< T[], arc::gen3::image::ArrayDeleter< T > > pSub( new T[ uiLength ], arc::gen3::image::ArrayDeleter< T >() );

			if ( pSub == nullptr )
			{
				THROW( "Failed to allocate subtraction data buffer!" );
			}

			for ( decltype( uiLength ) i = 0; i < uiLength; i++ )
			{
				pSub.get()[ i ] = pBuf1[ i ] - pBuf2[ i ];
			}

			return pSub;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  subtractHalves                                                                                          |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Subtracts one half of an image from the other. The first half of the image buffer is replaced with      |
		// |  the new image.                                                                                          |
		// |                                                                                                          |
		// |  <IN> -> pBuf	 - Pointer to the image buffer. Result is placed in this buffer.                          |
		// |  <IN> -> uiCols - The image column size ( in pixels ).                                                   |
		// |  <IN> -> uiRows - The image row size ( in pixels ).                                                      |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcImage<T>::subtractHalves( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			VERIFY_BUFFER( pBuf )

			if ( ( uiRows % 2 ) != 0 )
			{
				THROW_INVALID_ARGUMENT( "Image must have an even number of rows [ %u ]", uiRows );
			}

			T* pBuf1 = pBuf;

			T* pBuf2 = pBuf + ( ( uiRows / 2 ) * uiCols );

			for ( std::uint32_t i = 0; i < ( ( uiRows / 2 ) * uiCols ); i++ )
			{
				pBuf1[ i ] -= pBuf2[ i ];
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  divide                                                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Divides two image buffers pixel by pixel.                                                               |
		// |                                                                                                          |
		// |  <IN> -> pBuf1	 - Pointer to the first image buffer. Result is placed in this buffer.                    |
		// |  <IN> -> pBuf2	 - Pointer to the second image buffer.                                                    |
		// |  <IN> -> uiCols - The image column size ( in pixels ).                                                   |
		// |  <IN> -> uiRows - The image row size ( in pixels ).                                                      |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>>
		CArcImage<T>::divide( const T* pBuf1, const T* pBuf2, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			VERIFY_BUFFER( pBuf1 )
	
			VERIFY_BUFFER( pBuf2 )

			std::uint32_t uiLength = ( uiCols * uiRows );

			std::unique_ptr< T[], arc::gen3::image::ArrayDeleter< T > > pDiv( new T[ uiLength ], arc::gen3::image::ArrayDeleter< T >() );

			if ( pDiv == nullptr )
			{
				THROW( "Failed to allocate division data buffer!" );
			}

			for ( decltype( uiLength ) i = 0; i < uiLength; i++ )
			{
				if ( pBuf2[ i ] != 0 )
				{
					pDiv.get()[ i ] = pBuf1[ i ] / pBuf2[ i ];
				}

				else
				{
					pDiv.get()[ i ] = 0;
				}
			}

			return pDiv;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  copy                                                                                                    |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Copies the source image buffer to the destination image buffer. The source buffer must be less than or  |
		// |  equal in dimensions to the destination buffer.                                                          |
		// |                                                                                                          |
		// |  <IN> -> pDstBuf - Pointer to the destination image buffer. Result is placed in this buffer.             |
		// |  <IN> -> pSrcBuf - Pointer to the source image buffer.                                                   |
		// |  <IN> -> uiCols  - The number of image columns to copy ( in pixels ).                                    |
		// |  <IN> -> uiRows  - The number of image rows to copy ( in pixels ).                                       |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcImage<T>::copy( T* pDstBuf, const T* pSrcBuf, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			VERIFY_BUFFER( pDstBuf )
	
			VERIFY_BUFFER( pSrcBuf )

			copy( pDstBuf, pSrcBuf, ( uiCols * uiRows * sizeof( T ) ) );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  copy                                                                                                    |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Copies the source image buffer to the destination image buffer. The source buffer must be less than or  |
		// |  equal in dimensions to the destination buffer.                                                          |
		// |                                                                                                          |
		// |  <IN> -> pDstBuf	- Pointer to the destination image buffer. Result is placed in this buffer.           |
		// |  <IN> -> pSrcBuf	- Pointer to the source image buffer.                                                 |
		// |  <IN> -> uiSize	- The number of image bytes to copy.                                                  |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcImage<T>::copy( T* pDstBuf, const T* pSrcBuf, std::uint32_t uiSize )
		{
			VERIFY_BUFFER( pDstBuf )
	
			VERIFY_BUFFER( pSrcBuf )

			copyMemory( pDstBuf, const_cast< T* >( pSrcBuf ), uiSize );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  maxTVal                                                                                                 |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Determines the maximum value for the data type currently in use.                                        |
		// |                                                                                                          |
		// |  Example, for std::uint16_t: 2^16 = 65536.                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::uint32_t CArcImage<T>::maxTVal()
		{
			auto gExponent = ( ( sizeof( T ) == sizeof( arc::gen3::image::BPP_32 ) ) ? 20 : ( sizeof( T ) * 8 ) );

			return static_cast< std::uint32_t >( std::pow( 2.0, gExponent ) );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  drawSemiCircle                                                                                          |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Draws a semi-circle on the specified buffer.                                                            |
		// |                                                                                                          |
		// |  <IN> -> uiXCenter		- uiX position of circle center point.                                            |
		// |  <IN> -> uiYCenter		- uiY position of circle center point.                                            |
		// |  <IN> -> uiRadius		- The radius of the circle.                                                       |
		// |  <IN> -> gStartAngle	- The start angle of the semi-circle.                                             |
		// |  <IN> -> gEndAngle		- The end angle of the semi-circle.                                               |
		// |  <IN> -> uiCols		- The image column size ( in pixels ).                                            |
		// |  <IN> -> pBuf			- Pointer to the image data buffer.                                               |
		// |  <IN> -> uiColor		- Color to draw circle ( default = 0 ).                                           |
		// |                                                                                                          |
		// |  Throws std::invalid_argument                                                                            |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcImage<T>::drawSemiCircle( std::uint32_t uiXCenter, std::uint32_t uiYCenter,
																 std::uint32_t uiRadius, double gStartAngle, double gEndAngle,
																 std::uint32_t uiCols, T* pBuf, T uiColor )
		{
			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid buffer reference ( nullptr )." );
			}

			for ( double angle = gStartAngle; angle < gEndAngle; angle += 0.1 )
			{
				std::uint32_t uiX = static_cast< std::uint32_t >( uiRadius * std::cos( angle * DEG2RAD ) + uiXCenter );
				std::uint32_t uiY = static_cast< std::uint32_t >( uiRadius * std::sin( angle * DEG2RAD ) + uiYCenter );

				pBuf[ uiX + uiY * uiCols ] = static_cast< T >( uiColor );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  drawFillCircle                                                                                          |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Draws a filled circle on the specified buffer.                                                          |
		// |                                                                                                          |
		// |  <IN> -> uiXCenter	- uiX position of circle center point.                                                |
		// |  <IN> -> uiYCenter	- uiY position of circle center point.                                                |
		// |  <IN> -> uiRadius	- The radius of the circle.                                                           |
		// |  <IN> -> uiCols	- The image column size ( in pixels ).                                                |
		// |  <IN> -> pBuf		- Pointer to the image data buffer.                                                   |
		// |  <IN> -> uiRows	- The image row size ( in pixels ).                                                   |
		// |  <IN> -> uiColor	- Color to draw circle ( default = 0 ).                                               |
		// |                                                                                                          |
		// |  Throws std::invalid_argument                                                                            |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		void CArcImage<T>::drawFillCircle( std::uint32_t uiXCenter, std::uint32_t uiYCenter, std::uint32_t uiRadius, std::uint32_t uiCols, T* pBuf, T uiColor )
		{
			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid buffer reference ( nullptr )." );
			}

			for ( decltype( uiRadius ) r = 0; r < uiRadius; r++ )
			{
				for ( double angle = 0; angle < 360; angle += 0.1 )
				{
					std::uint32_t uiX = static_cast< std::uint32_t >( r * std::cos( angle * DEG2RAD ) + uiXCenter );
					std::uint32_t uiY = static_cast< std::uint32_t >( r * std::sin( angle * DEG2RAD ) + uiYCenter );

					pBuf[ uiX + uiY * uiCols ] = static_cast< T >( uiColor );
				}
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  drawGradientFillCircle                                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Draws a gradient filled circle on the specified buffer.                                                 |
		// |                                                                                                          |
		// |  <IN> -> uiXCenter	- uiX position of circle center point.                                                |
		// |  <IN> -> uiYCenter	- uiY position of circle center point.                                                |
		// |  <IN> -> uiRadius	- The radius of the circle.                                                           |
		// |  <IN> -> uiCols	- The image column size ( in pixels ).                                                |
		// |  <IN> -> uiRows	- The image row size ( in pixels ).                                                   |
		// |  <IN> -> pBuf		- Pointer to the image data buffer.                                                   |
		// |                                                                                                          |
		// |  Throws std::invalid_argument                                                                            |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		void CArcImage<T>::drawGradientFillCircle( std::uint32_t uiXCenter, std::uint32_t uiYCenter, std::uint32_t uiRadius, std::uint32_t uiCols, std::uint32_t uiRows, T* pBuf )
		{
			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid buffer reference ( nullptr )." );
			}

			for ( decltype( uiRadius ) r = 0; r < uiRadius; r++ )
			{
				drawCircle( ( uiCols / 2 ),
					( uiRows / 2 ),
					( uiRadius - r ),
					uiCols,
					pBuf,
					static_cast< std::uint32_t >( r + ( ( maxTVal() - 1 ) / uiRadius ) ) );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  drawCircle                                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Draws a circle on the specified buffer.                                                                 |
		// |                                                                                                          |
		// |  <IN> -> uiXCenter	- uiX position of circle center point.                                                |
		// |  <IN> -> uiYCenter	- uiY position of circle center point.                                                |
		// |  <IN> -> uiRadius	- The radius of the circle.                                                           |
		// |  <IN> -> uiCols	- The image column size ( in pixels ).                                                |
		// |  <IN> -> pBuf		- Pointer to the image data buffer.                                                   |
		// |  <IN> -> uiColor	- Color to draw circle ( default = 0 ).                                               |
		// |                                                                                                          |
		// |  Throws std::invalid_argument                                                                            |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		void CArcImage<T>::drawCircle( std::uint32_t uiXCenter, std::uint32_t uiYCenter, std::uint32_t uiRadius, std::uint32_t uiCols, T* pBuf, T uiColor )
		{
			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid buffer reference ( nullptr )." );
			}

			for ( double angle = 0; angle < 360; angle += 0.1 )
			{
				std::uint32_t uiX = static_cast< std::uint32_t >( uiRadius * std::cos( angle * DEG2RAD ) + uiXCenter );
				std::uint32_t uiY = static_cast< std::uint32_t >( uiRadius * std::sin( angle * DEG2RAD ) + uiYCenter );

				pBuf[ uiX + uiY * uiCols ] = static_cast< T >( uiColor );
			}
		}


	}	// end gen3 namespace
}		// end arc namespace


/** Explicit instantiations - These are the only allowed instantiations of this class */
template class arc::gen3::CArcImage<arc::gen3::image::BPP_16>;
template class arc::gen3::CArcImage<arc::gen3::image::BPP_32>;



// +------------------------------------------------------------------------------------------------+
// |  default_delete definition                                                                     |
// +------------------------------------------------------------------------------------------------+
// |  Creates a modified version of the std::default_delete class for use by all                    |
// |  std::unique_ptr's that wrap a CStats/CDifStats object.                                        |
// +------------------------------------------------------------------------------------------------+
namespace std
{

	void default_delete<arc::gen3::image::CStats>::operator()( arc::gen3::image::CStats* pObj )
	{
		if ( pObj != nullptr )
		{
			delete pObj;
		}
	}


	void default_delete<arc::gen3::image::CDifStats>::operator()( arc::gen3::image::CDifStats* pObj )
	{
		if ( pObj != nullptr )
		{
			delete pObj;
		}
	}

}
