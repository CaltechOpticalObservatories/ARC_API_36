// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  CArcImage.h  ( Gen3 )                                                                                    |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE: This file defines the standard ARC image operations interface.                                         |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 26, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2014 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+
/**< @file CArcImage.h */

#ifndef _GEN3_CARCIMAGE_H_
#define _GEN3_CARCIMAGE_H_

#ifdef _WINDOWS
#pragma warning( disable: 4251 )
#endif

#include <stdexcept>
#include <cstdint>
#include <memory>

#include <functional>

#include <CArcImageDllMain.h>
#include <CArcBase.h>



namespace arc
{
	namespace gen3
	{
		namespace image
		{

			/** \typedef BPP_16
			 *  16 bits-per-pixel image data
			 */
			using BPP_16 = std::uint16_t;


			/** \typedef BPP_32
			 *  32 bits-per-pixel image data
			 */
			using BPP_32 = std::uint32_t;


			/** @class CAvgStats
			 *  Average image statistics info class
			 */
			class GEN3_CARCIMAGE_API CAvgStats
			{
				public:

					/** Constructor
					 *  @param uiChannelCount - The number of channels in the image(s)
					 */
					CAvgStats( std::uint32_t uiChannelCount )
					{
						this->uiChannelCount = uiChannelCount;

						gMeanOfMeans = gStdDevOfMeans = gAvgAvgStdDev = 0;

						pAverageMin = new double[ uiChannelCount ];
						pAverageMax = new double[ uiChannelCount ];
						pAverageMean = new double[ uiChannelCount ];
						pAverageStdDev = new double[ uiChannelCount ];

						std::memset( pAverageMin, 0, ( uiChannelCount * sizeof( double ) ) );
						std::memset( pAverageMax, 0, ( uiChannelCount * sizeof( double ) ) );
						std::memset( pAverageMean, 0, ( uiChannelCount * sizeof( double ) ) );
						std::memset( pAverageStdDev, 0, ( uiChannelCount * sizeof( double ) ) );
					}

					/** Destructor
					 */
					~CAvgStats( void )
					{
						if ( pAverageMin != nullptr ) { delete[] pAverageMin; }
						if ( pAverageMax != nullptr ) { delete[] pAverageMax; }
						if ( pAverageMean != nullptr ) { delete[] pAverageMean; }
						if ( pAverageStdDev != nullptr ) { delete[] pAverageStdDev; }
					}

					double* pAverageMin;				/**< Average of the minimum value */
					double* pAverageMax;				/**< Average of the maximum value */
					double* pAverageMean;				/**< Average of the mean value */
					double* pAverageStdDev;				/**< Average of the standard deviation */

					double gMeanOfMeans;				/**< Average of the average of the mean values */
					double gStdDevOfMeans;				/**< Standard deviation of the mean values */
					double gAvgAvgStdDev;				/**< Average of the average standard deviation value */

					std::uint32_t uiChannelCount;		/**< Number of channels in the image(s) */
					std::uint32_t uiImageCount;			/**< Number of images used for the averaging */
			};


			/** @class CStats
			 *  Image statistics info class
			 */
			class GEN3_CARCIMAGE_API CStats
			{
				public:

					/** Default constructor
					 */
					CStats( void )
					{
						gMin = gMax = gMean = gVariance = gStdDev = gTotalPixels = gSaturatedCount = 0;
					}

					/** Copy constructor
					 *  @param rCStats - The CStats object to copy
					 */
					CStats( const CStats& rCStats )
					{
						gTotalPixels = rCStats.gTotalPixels;
						gMin = rCStats.gMin;
						gMax = rCStats.gMax;
						gMean = rCStats.gMean;
						gVariance = rCStats.gVariance;
						gStdDev = rCStats.gStdDev;
						gSaturatedCount = rCStats.gSaturatedCount;
					}

					/** Default destructor
					 */
					~CStats( void ) = default;

					/** Copies another instance of this class to this one.
					 *  @param rCStats - The CStats object to copy
					 */
					void set( const CStats& rCStats )
					{
						gTotalPixels = rCStats.gTotalPixels;
						gMin = rCStats.gMin;
						gMax = rCStats.gMax;
						gMean = rCStats.gMean;
						gVariance = rCStats.gVariance;
						gStdDev = rCStats.gStdDev;
						gSaturatedCount = rCStats.gSaturatedCount;
					}

					double gTotalPixels;		/**< The total number of pixels in the image */
					double gMin;				/**< The minimum value found in the image */
					double gMax;				/**< The maximum value found in the image */
					double gMean;				/**< The mean value of the pixels in the image */
					double gVariance;			/**< The variance of the pixels in the image. Measures how far spread out the pixel values are from their mean. */
					double gStdDev;				/**< The standard deviation of the pixels in the image. Measures the dispersion of the pixel values. */
					double gSaturatedCount;		/**< The number of saturated pixels in the iamge. Saturation pixel value is 65535 for 16-bit images */
			};


			/** @class CDifStats
			 *  Image difference statistics info class
			 */
			class GEN3_CARCIMAGE_API CDifStats
			{
				public:

					/** Default constructor
					 */
					CDifStats( void ) = default;

					/** Default destructor
					 */
					~CDifStats( void ) = default;

					CStats cStats1;			/**< Statistics values for image #1 */
					CStats cStats2;			/**< Statistics values for image #2 */
					CStats cDiffStats;		/**< Difference values for the cStats1 and cStats2 members of this class */
			};


			/** @struct ArrayDeleter
			 *  Returned array deleter
			 */
			template <typename T>
			struct GEN3_CARCIMAGE_API ArrayDeleter
			{
				void operator()( T* p ) const
				{
					if ( p != nullptr )
					{
						delete[] p;
					}
				}
			};

		};	// end image namespace



		/** @class CArcImage
		 *  ARC image processing class. WARNING - All methods within this class perform destructive operations
		 *  on the original image buffer. i.e. Original buffer contents will be modified in place.
		 *  @see arc::gen3::CArcBase
		 */
		template <typename T = arc::gen3::image::BPP_16>
		class GEN3_CARCIMAGE_API CArcImage : public arc::gen3::CArcBase
		{
		public:

			/** Constructor
			 *  Creates an image operations object.
			 */
			CArcImage( void ) = default;

			/** Destructor
			 */
			virtual ~CArcImage( void );

			/** Returns a textual representation of the library version.
			 *  @return A string representation of the library version.
			 */
			static const std::string version( void );

			/** Fills the specified buffer with the specified value.
			 *  @param pBuf		- Pointer to the image buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @param uiValue	- The value to fill the buffer with.
			 *  @throws std::invalid_argument
			 *  @throws std::out_of_range
			 */
			static void fill( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows, T uiValue );

			/** Fills the specified buffer with the specified value.
			 *  @param pBuf		- Pointer to the image buffer.
			 *  @param uiBytes	- The number of bytes in the image data buffer.
			 *  @param uiValue	- The value to fill the buffer with.
			 *  @throws std::invalid_argument
			 *  @throws std::out_of_range
			 */
			static void fill( T* pBuf, std::uint32_t uiBytes, T uiValue );

			/** Fills the specified buffer with a gradient pattern.
			 *  @param pBuf		- Pointer to the image buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @throws std::invalid_argument
			 */
			static void fillWithGradient( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Fills the specified buffer with zeroes and puts a smiley face at the center.
			 *  @param pBuf		- Pointer to the image buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @throws std::invalid_argument
			 */
			static void fillWithSmiley( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Fills the specified buffer with a ramp image. Data has the form 0, 1, 2, ..., 65535, 0, 1, ....
			 *  @param pBuf		- Pointer to the image buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @throws std::invalid_argument
			 */
			static void fillWithRamp( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Verify a ramp synthetic image. Data has the form 0, 1, 2, ..., 65535, 0, 1, ....
			 *  @param pBuf		- Pointer to the image data buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @throws std::runtime_error
			 *  @throws std::invalid_argument
			 */
			static void containsValidRamp( const T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Count the number of pixels having the specified value.
			 *  @param pBuf		- Pointer to the image data buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @param uwValue	- The pixel value to include in the count.
			 *  @throws std::runtime_error
			 */
			static std::uint32_t countPixels( const T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows, std::uint16_t uwValue );

			/** Count the number of pixels having the specified value.
			 *  @param pBuf			- Pointer to the image data buffer.
			 *  @param uiBufSize	- The image buffer size ( in pixels ).
			 *  @param uwValue		- The pixel value to include in the count.
			 *  @throws std::runtime_error
			 */
			static std::uint32_t countPixels( const T* pBuf, std::uint32_t uiBufSize, std::uint16_t uwValue );

			/** Returns the value of a pixel at the specified row and column.
			 *  @param pBuf		- Pointer to the image buffer.
			 *  @param uiCol	- The pixel column number.
			 *  @param uiRow	- The pixel row number.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @return The pixel value at the specified row and column.
			 *  @throws std::runtime_error
			 */
			static T getPixel( const T* pBuf, std::uint32_t uiCol, std::uint32_t uiRow, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Returns a region of an image.
			 *  @param pBuf		- Pointer to the image data buffer.
			 *  @param uiCol1	- The start column.
			 *  @param uiCol2	- The end column.
			 *  @param uiRow1	- The start row.
			 *  @param uiRow2	- The end row.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @param uiCount	- The pixel count of the returned array.
			 *  @return A std::unique_ptr to an array of type T, where CArcImage<T>.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>>
			getRegion( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow1, std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount );

			/** Returns all or part of an image row.
			 *  @param pBuf		- Pointer to the image data buffer.
			 *  @param uiCol1	- The start column.
			 *  @param uiCol2	- The end column.
			 *  @param uiRow		- The row to read from.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @param uiCount	- The pixel count of the returned array.
			 *  @return A std::unique_ptr to an array of type T, where CArcImage<T>.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>>
			getRow( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount );

			/** Returns all or part of an image column.
			 *  @param pBuf		- Pointer to the image data buffer.
			 *  @param uiCol		- The column to read from.
			 *  @param uiRow1	- The start row.
			 *  @param uiRow2	- The end row.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @param uiCount	- The pixel count of the returned array.
			 *  @return A std::unique_ptr to an array of type T, where CArcImage<T>.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>>
			getCol( const T* pBuf, std::uint32_t uiCol, std::uint32_t uiRow1, std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount );

			/** Returns a row of pixel data where each value is the average over the specified range of columns.
			 *  @param pBuf		- Pointer to the image data buffer.
			 *  @param uiCol1	- The start column.
			 *  @param uiCol2	- The end column.
			 *  @param uiRow1	- The start row.
			 *  @param uiRow2	- The end row.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @param uiCount	- The pixel count of the returned array.
			 *  @return A std::unique_ptr to an array of doubles.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<double[], arc::gen3::image::ArrayDeleter<double>>
			getRowArea( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow1, std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount );

			/** Returns a column of pixel data where each value is the average over the specified range of rows.
			 *  @param pBuf		- Pointer to the image data buffer.
			 *  @param uiCol1	- The start column.
			 *  @param uiCol2	- The end column.
			 *  @param uiRow1	- The start row.
			 *  @param uiRow2	- The end row.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @param uiCount	- The pixel count of the returned array.
			 *  @return A std::unique_ptr to an array of doubles.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<double[], arc::gen3::image::ArrayDeleter<double>> getColArea( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2,
																								 std::uint32_t uiRow1, std::uint32_t uiRow2,
																								 std::uint32_t uiCols, std::uint32_t uiRows,
																								 std::uint32_t& uiCount );

			/** Calculates the image min, max, mean, variance, standard deviation, total pixel count and saturated
			 *  pixel count over the specified image buffer cols and rows.
			 *  @param pBuf		- Pointer to the image data buffer.
			 *  @param uiCol1	- The start column.
			 *  @param uiCol2	- The end column.
			 *  @param uiRow1	- The start row.
			 *  @param uiRow2	- The end row.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @return A std::unique_ptr to an arc::gen3::image::CStats object.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<arc::gen3::image::CStats>
			getStats( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow1, std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Calculates the image min, max, mean, variance, standard deviation, total pixel count and
			 *  saturated pixel count over the entire image.
			 *  @param pBuf		- Pointer to the image data buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @return A std::unique_ptr to an arc::gen3::image::CStats object.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<arc::gen3::image::CStats> getStats( const T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Calculates the min, max, mean, variance, standard deviation, total pixel count and saturated pixel count for
			 *  each image as well as the difference mean, variance and standard deviation over the specified image buffer
			 *  cols and rows. This is used for photon transfer curves ( PTC ). The two images MUST be the same size or the
			 *  methods behavior is undefined as this cannot be verified using the given parameters.
			 *  @param pBuf1	- Pointer to the first image buffer.
			 *  @param pBuf2	- Pointer to the second image buffer.
			 *  @param uiCol1	- The start column.
			 *  @param uiCol2	- The end column.
			 *  @param uiRow1	- The start row.
			 *  @param uiRow2	- The end row.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @return A std::unique_ptr to an arc::gen3::image::CDifStats object.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<arc::gen3::image::CDifStats>
			getDiffStats( const T* pBuf1, const T* pBuf2, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow1, std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Calculates the min, max, mean, variance, standard deviation, total pixel count and saturated pixel count for
			 *  each image as well as the difference mean, variance and standard deviation for the entire image. This is used
			 *  for photon transfer curves ( PTC ). The two images MUST be the same size or the methods behavior is undefined
			 *  as this cannot be verified using the given parameters.
			 *  @param pBuf1	- Pointer to the first image buffer.
			 *  @param pBuf2	- Pointer to the second image buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @return A std::unique_ptr to an arc::gen3::image::CDifStats object.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<arc::gen3::image::CDifStats> getDiffStats( const T* pBuf1, const T* pBuf2, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Calculates the histogram over the specified image buffer columns and rows.
			 *  @param pBuf		- Pointer to the image buffer.
			 *  @param uiCol1	- The start column.
			 *  @param uiCol2	- The end column.
			 *  @param uiRow1	- The start row.
			 *  @param uiRow2	- The end row.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @param uiCount	- The element count of the returned array.
			 *  @return A std::unique_ptr to an array of unsigned integers. The size of the array depends on the image data type.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<std::uint32_t[], arc::gen3::image::ArrayDeleter<std::uint32_t>>
			histogram( const T* pBuf, std::uint32_t uiCol1, std::uint32_t uiCol2, std::uint32_t uiRow1, std::uint32_t uiRow2, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount );

			/** Calculates the histogram over the entire image buffer.
			 *  @param pBuf		- Pointer to the image buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @param uiCount	- The element count of the returned array.
			 *  @return A std::unique_ptr to an array of unsigned integers. The size of the array depends on the image data type.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<std::uint32_t[], arc::gen3::image::ArrayDeleter<std::uint32_t>> histogram( const T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows, std::uint32_t& uiCount );

			/** Adds two buffers together pixel by pixel.
			 *  @param pBuf1	- Pointer to the first image buffer.
			 *  @param pBuf2	- Pointer to the second image buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @return A pointer to a 64-bit buffer.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<std::uint64_t[], arc::gen3::image::ArrayDeleter<std::uint64_t>> add( const T* pBuf1, const T* pBuf2, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Subtracts two image buffers pixel by pixel. Buffer two is subtracted from buffer one.
			 *  @param pBuf1	- Pointer to the first image buffer. Result is placed in this buffer.
			 *  @param pBuf2	- Pointer to the second image buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @return A pointer to a buffer with the same data type as the two images being subtracted.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>> subtract( const T* pBuf1, const T* pBuf2, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Subtracts one half of an image from the other. The first half of the image buffer is replaced with the new image.
			 *  @param pBuf		- Pointer to the image buffer. Result is placed in this buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @throws std::runtime_error
			 *  @throws std::invalid_argument
			 */
			static void subtractHalves( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Divide two image buffers pixel by pixel.
			 *  @param pBuf1	- Pointer to the first image buffer.
			 *  @param pBuf2	- Pointer to the second image buffer.
			 *  @param uiCols	- The image column size ( in pixels ).
			 *  @param uiRows	- The image row size ( in pixels ).
			 *  @return A pointer to a buffer with the same data type as the two images being divided.
			 *  @throws std::runtime_error
			 */
			static std::unique_ptr<T[], arc::gen3::image::ArrayDeleter<T>> divide( const T* pBuf1, const T* pBuf2, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Copies the source image buffer to the destination image buffer. The source buffer must be less than or equal
			 *  in dimensions to the destination buffer.
			 *  @param pDstBuf	- Pointer to the destination image buffer. Result is placed in this buffer.
			 *  @param pSrcBuf	- Pointer to the source image buffer.
			 *  @param uiCols	- The number of image column pixels to copy.
			 *  @param uiRows	- The number of image row pixels to copy.
			 *  @throws std::runtime_error
			 */
			static void copy( T* pDstBuf, const T* pSrcBuf, std::uint32_t uiCols, std::uint32_t uiRows );

			/** Copies the source image buffer to the destination image buffer. The source buffer must be less than or equal
			 *  in dimensions to the destination buffer.
			 *  @param pDstBuf	- Pointer to the destination image buffer. Result is placed in this buffer.
			 *  @param pSrcBuf	- Pointer to the source image buffer.
			 *  @param uiSize	- The number of image bytes to copy.
			 *  @throws std::runtime_error
			 */
			static void copy( T* pDstBuf, const T* pSrcBuf, std::uint32_t uiSize );

			/** Determines the maximum value for a specific data type. Example, for std::uint16_t: 2^16 = 65536.
			 *  @return The maximum value for the data type currently in use.
			 */
			static std::uint32_t maxTVal( void );

		private:

			/** Draws a semi-circle on the specified buffer.
			 *  @param uiXCenter	- x position of circle center point.
			 *  @param uiYCenter	- y position of circle center point.
			 *  @param uiRadius		- The radius of the circle.
			 *  @param gStartAngle	- The start angle of the semi-circle.
			 *  @param gEndAngle	- The end angle of the semi-circle.
			 *  @param uiCols		- The image column size ( in pixels ).
			 *  @param pBuf			- Pointer to the image data buffer.
			 *  @param uiColor		- Color to draw circle ( default = 0 ).
			 *  @throws std::invalid_argument
			 */
			static void drawSemiCircle( std::uint32_t uiXCenter, std::uint32_t uiYCenter, std::uint32_t uiRadius, double gStartAngle,
			double gEndAngle, std::uint32_t uiCols, T* pBuf, T uiColor = 0 );

			/** Draws a filled circle on the specified buffer.
			 *  @param uiXCenter	- x position of circle center point.
			 *  @param uiYCenter	- y position of circle center point.
			 *  @param uiRadius		- The radius of the circle.
			 *  @param uiCols		- The image column size ( in pixels ).
			 *  @param pBuf			- Pointer to the image data buffer.
			 *  @param uiColor		- Color to draw circle ( default = 0 ).
			 *  @throws std::invalid_argument
			 */
			static void drawFillCircle( std::uint32_t uiXCenter, std::uint32_t uiYCenter, std::uint32_t uiRadius, std::uint32_t uiCols, T* pBuf, T uiColor = 0 );

			/** Draws a gradient filled circle on the specified buffer.
			 *  @param uiXCenter	- x position of circle center point.
			 *  @param uiYCenter	- y position of circle center point.
			 *  @param uiRadius		- The radius of the circle.
			 *  @param uiCols		- The image column size ( in pixels ).
			 *  @param uiRows		- The image row size ( in pixels ).
			 *  @param pBuf			- Pointer to the image data buffer.
			 *  @param uiColor		- Color to draw circle ( default = 0 ).
			 *  @throws std::invalid_argument
			 */
			static void drawGradientFillCircle( std::uint32_t uiXCenter, std::uint32_t uiYCenter, std::uint32_t uiRadius, std::uint32_t uiCols, std::uint32_t uiRows, T* pBuf );

			/** Draws a gradient filled circle on the specified buffer.
			 *  @param uiXCenter	- x position of circle center point.
			 *  @param uiYCenter	- y position of circle center point.
			 *  @param uiRadius		- The radius of the circle.
			 *  @param uiCols		- The image column size ( in pixels ).
			 *  @param pBuf			- Pointer to the image data buffer.
			 *  @param uiColor		- Color to draw circle ( default = 0 ).
			 *  @throws std::invalid_argument
			 */
			static void drawCircle( std::uint32_t uiXCenter, std::uint32_t uiYCenter, std::uint32_t uiRadius, std::uint32_t uiCols, T* pBuf, T uiColor = 0 );

			/** version() text holder */
			static const std::string m_sVersion;
		};


	}	// end gen3 namespace
}		// end arc namespace



//
// Creates a modified version of the std::default_delete class for use by
// all std::unique_ptr's that wrap a CStats/CDifStats object.
//
namespace std
{

	/**
	 *  Creates a modified version of the std::default_delete class for use by
	 *  all std::unique_ptr's returned from CArcImage to delete CStats objects.
	 */
	template<>
	class GEN3_CARCIMAGE_API default_delete< arc::gen3::image::CStats >
	{
	public:

		/** Deletes the specified CStats object
		 *  @param pObj - The object to be deleted/destroyed.
		 */
		void operator()( arc::gen3::image::CStats* pObj );
	};

	/**
	 *  Creates a modified version of the std::default_delete class for use by
	 *  all std::unique_ptr's returned from CArcImage to delete CDifStats objects.
	 */
	template<>
	class GEN3_CARCIMAGE_API default_delete< arc::gen3::image::CDifStats >
	{
	public:

		/** Deletes the specified CDifStats object
		 *  @param pObj - The object to be deleted/destroyed.
		 */
		void operator()( arc::gen3::image::CDifStats* pObj );
	};
}



#endif		// _GEN3_CARCIMAGE_H_
