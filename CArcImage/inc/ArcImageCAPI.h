// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  ArcImageCAPI.h  ( Gen3 )                                                                                 |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE: This file defines a C interface for all the CArcImage class methods.                                   |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 26, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2014 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+

#ifndef _ARC_GEN3_IMAGE_CAPI_H_
#define _ARC_GEN3_IMAGE_CAPI_H_


#include <stdlib.h>

#include <CArcImageDllMain.h>



// +------------------------------------------------------------------------------------------------------------------+
// | Status definitions                                                                                               |
// +------------------------------------------------------------------------------------------------------------------+
typedef unsigned int				ArcStatus_t;				/** Return status type */


#ifndef ARC_STATUS
#define ARC_STATUS

#define ARC_STATUS_NONE				( ArcStatus_t* )NULL		/** Invalid status */
#define ARC_STATUS_OK				( ArcStatus_t )1			/** Success status */
#define ARC_STATUS_ERROR			( ArcStatus_t )2			/** Error status   */

#define ARC_MSG_SIZE				64							/** General message size ( bytes ) */
#define ARC_ERROR_MSG_SIZE			256							/** Error message size ( bytes )   */

#endif



// +------------------------------------------------------------------------------------------------------------------+
// | Define image bits-per-pixel constants                                                                            |
// +------------------------------------------------------------------------------------------------------------------+
#ifdef __cplusplus
extern "C" {		// Using a C++ compiler
#endif

	GEN3_CARCIMAGE_API extern const unsigned int IMAGE_BPP16;			/** 16-bit bits-per-pixel image data */
	GEN3_CARCIMAGE_API extern const unsigned int IMAGE_BPP32;			/** 32-bit bits-per-pixel image data */

#ifdef __cplusplus
}
#endif



// +------------------------------------------------------------------------------------------------------------------+
// | Image statistics info structure                                                                                  |
// +------------------------------------------------------------------------------------------------------------------+
struct GEN3_CARCIMAGE_API CStats
{
	double gTotalPixels;
	double gMin;
	double gMax;
	double gMean;
	double gVariance;
	double gStdDev;
	double gSaturatedCount;
};


// +------------------------------------------------------------------------------------------------------------------+
// | Image difference statistics info structure                                                                       |
// +------------------------------------------------------------------------------------------------------------------+
struct GEN3_CARCIMAGE_API CDifStats
{
	struct CStats cStats1;
	struct CStats cStats2;
	struct CStats cDiffStats;
};



// +------------------------------------------------------------------------------------------------------------------+
// | CArcImage method definitions                                                                                     |
// |                                                                                                                  |
// | @see See CArcImage class definition ( CArcImage.h ) for method definitions.                                      |
// +------------------------------------------------------------------------------------------------------------------+

#ifdef __cplusplus
extern "C" {		// Using a C++ compiler
#endif

	/** Returns a handle to the image object appropriate for the specified bits-per-pixel.
	 *  @param uiBpp - The number of bits-per-pixel in the image.
	 *  @return A reference to an image object.
	 */
	GEN3_CARCIMAGE_API void ArcImage_selectInstance( unsigned int uiBpp, ArcStatus_t* pStatus );

	/** Returns the library build and version info.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 *  @return A strng representation of the library version.
	 */
	GEN3_CARCIMAGE_API const char* ArcImage_version( ArcStatus_t* pStatus );

	/** Fills the specified buffer with the specified value.
	 *  @param pBuf		- Pointer to the image buffer.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param uiValue	- The value to fill the buffer with.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_fill( void* pBuf, unsigned int uiCols, unsigned int uiRows, unsigned int uiValue, ArcStatus_t* pStatus );

	/** Fills the specified buffer with a gradient pattern.
	 *  @param pBuf		- Pointer to the image buffer.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_fillWithGradient( void* pBuf, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Fills the specified buffer with zeroes and puts a smiley face at the center.
	 *  @param pBuf		- Pointer to the image buffer.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_fillWithSmiley( void* pBuf, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Fills the specified buffer with a ramp image. Data has the form 0, 1, 2, ..., 65535, 0, 1, ....
	 *  @param pBuf		- Pointer to the image buffer.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_fillWithRamp( void* pBuf, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Verify a ramp synthetic image. Data has the form 0, 1, 2, ..., 65535, 0, 1, ....
	 *  @param pBuf		- Pointer to the image data buffer.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_containsValidRamp( const void* pBuf, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Returns all or part of an image row.
	 *  @param pRow		- A pointer to a row buffer that is at least ( col2 - col1 ) in length.
	 *  @param pBuf		- Pointer to the image data buffer.
	 *  @param uiCol1	- The start column.
	 *  @param uiCol2	- The end column.
	 *  @param uiRow	- The row to read from.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param uCount	- The pixel count of the returned array.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_getRow( void* pRow, const void* pBuf, unsigned int uiCol1, unsigned int uiCol2, unsigned int uiRow,
		unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Returns all or part of an image column.
	 *  @param pCol	- A pointer to a column buffer that is at least ( row2 - row 1 ) in length.
	 *  @param pBuf		- Pointer to the image data buffer.
	 *  @param uiCol	- The column to read from.
	 *  @param uiRow1	- The start row.
	 *  @param uiRow2	- The end row.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param uCount	- The pixel count of the returned array.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void	ArcImage_getCol( void* pCol, const void* pBuf, unsigned int uiCol, unsigned int uiRow1, unsigned int uiRow2,
		unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Returns a row of pixel data where each value is the average over the specified range of columns.
	 *  @param pArea	- A pointer to an area buffer that is at least ( row2 - row 1 ) * ( col2 - col1 ) in length.
	 *  @param pBuf		- Pointer to the image data buffer.
	 *  @param uiCol1	- The start column.
	 *  @param uiCol2	- The end column.
	 *  @param uiRow1	- The start row.
	 *  @param uiRow2	- The end row.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param uCount	- The pixel count of the returned array.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_getRowArea( double* pArea, const void* pBuf, unsigned int uiCol1, unsigned int uiCol2, unsigned int uiRow1,
		unsigned int uiRow2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Returns a column of pixel data where each value is the average over the specified range of rows.
	 *  @param pArea	- A pointer to an area buffer that is at least ( row2 - row 1 ) * ( col2 - col1 ) in length.
	 *  @param pBuf		- Pointer to the image data buffer.
	 *  @param uiCol1	- The start column.
	 *  @param uiCol2	- The end column.
	 *  @param uiRow1	- The start row.
	 *  @param uiRow2	- The end row.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param uCount	- The pixel count of the returned array.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void	ArcImage_getColArea( double* pArea, const void* pBuf, unsigned int uiCol1, unsigned int uiCol2, unsigned int uiRow1,
		unsigned int uiRow2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Calculates the image min, max, mean, variance, standard deviation, total pixel count and saturated
	 *  pixel count over the specified image buffer cols and rows.
	 *  @param pStats	- A pointer to a CStats structure.
	 *  @param pBuf		- Pointer to the image data buffer.
	 *  @param uiCol1	- The start column.
	 *  @param uiCol2	- The end column.
	 *  @param uiRow1	- The start row.
	 *  @param uiRow2	- The end row.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_getStats( struct CStats* pStats, const void* pBuf, unsigned int uiCol1, unsigned int uiCol2, unsigned int uiRow1,
		unsigned int uiRow2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Calculates the min, max, mean, variance, standard deviation, total pixel count and saturated pixel count for
	 *  each image as well as the difference mean, variance and standard deviation over the specified image buffer
	 *  cols and rows. This is used for photon transfer curves ( PTC ). The two images MUST be the same size or the
	 *  methods behavior is undefined as this cannot be verified using the given parameters.
	 *  @param pStats	- A pointer to a CDifStats structure.
	 *  @param pBuf1	- Pointer to the first image buffer.
	 *  @param pBuf2	- Pointer to the second image buffer.
	 *  @param uiCol1	- The start column.
	 *  @param uiCol2	- The end column.
	 *  @param uiRow1	- The start row.
	 *  @param uiRow2	- The end row.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_getDiffStats( struct CDifStats* pStats, const void* pBuf1, void* pBuf2, unsigned int uiCol1, unsigned int uiCol2,
		unsigned int uiRow1, unsigned int uiRow2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Calculates the histogram over the specified image buffer columns and rows.
	 *  @param pHist	- A pointer to an unsigned int histogram array.
	 *  @param pBuf		- Pointer to the image buffer.
	 *  @param uiCol1	- The start column.
	 *  @param uiCol2	- The end column.
	 *  @param uiRow1	- The start row.
	 *  @param uiRow2	- The end row.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param puiCount	- The element count of the returned array.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void	ArcImage_histogram( unsigned int* pHist, const void* pBuf, unsigned int uiCol1, unsigned int uiCol2, unsigned int uiRow1, unsigned int uiRow2,
		unsigned int uiCols, unsigned int uiRows, unsigned int* puiCount, ArcStatus_t* pStatus );

	/** Adds two image buffers together pixel by pixel. The result is a pointer to a 64-bit buffer.
	 *  @param pAdd		- A pointer to a 64-bit array.
	 *  @param pBuf1	- Pointer to the first image buffer.
	 *  @param pBuf2	- Pointer to the second image buffer.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_add( unsigned long long* pAdd, const void* pBuf1, const void* pBuf2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Subtracts two image buffers pixel by pixel. Buffer two is subtracted from buffer one.
	 *  @param pSub		- Pointer to a buffer with the same data type as the two images being subtracted.
	 *  @param pBuf1	- Pointer to the first image buffer.
	 *  @param pBuf2	- Pointer to the second image buffer.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_subtract( void* pSub, const void* pBuf1, const void* pBuf2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Subtracts one half of an image from the other. The first half of the image buffer is replaced with the new image.
	 *  @param pBuf		- Pointer to the image buffer. Result is placed in this buffer.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_subtractHalves( void* pBuf, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Divide two image buffers pixel by pixel.
	 *  @param pDiv		- Pointer to a buffer with the same data type as the two images being divided.
	 *  @param pBuf1	- Pointer to the first image buffer. Result is placed in this buffer.
	 *  @param pBuf2	- Pointer to the second image buffer.
	 *  @param uiCols	- The image column size ( in pixels ).
	 *  @param uiRows	- The image row size ( in pixels ).
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_divide( void* pDiv, const void* pBuf1, const void* pBuf2, unsigned int uiCols, unsigned int uiRows, ArcStatus_t* pStatus );

	/** Copies the source image buffer to the destination image buffer. The source buffer must be less than or equal
	 *  in dimensions to the destination buffer.
	 *  @param pBuf1	- Pointer to the destination image buffer. Result is placed in this buffer.
	 *  @param uiCols1	- The destination image buffer column size ( in pixels ).
	 *  @param uiRows1	- The destination image buffer row size ( in pixels ).
	 *  @param pBuf2	- Pointer to the source image buffer.
	 *  @param uiCols2	- The source image buffer size ( in bytes ).
	 *  @param uiRows2	- The source image buffer size ( in bytes ).
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCIMAGE_API void ArcImage_copy( void* pBuf1, unsigned int uiCols1, unsigned int uiRows1, const void* pBuf2, unsigned int uiCols2, unsigned int uiRows2 );

	/** Determines the maximum value for the current data type in use. Example, for unsigned short: 2^16 = 65536.
	 *  @return The maximum value for the data type currently in use.
	 */
	GEN3_CARCIMAGE_API unsigned int ArcImage_maxTVal( ArcStatus_t* pStatus );

	/** Returns the last reported error message.
	 *  @return The last error message.
	 */
	GEN3_CARCIMAGE_API const char* ArcImage_getLastError( void );


#ifdef __cplusplus
}
#endif


#endif		// _ARC_GEN3_IMAGE_CAPI_H_







//#ifndef _ARC_IMAGE_CAPI_H_
//#define _ARC_IMAGE_CAPI_H_
//
//#include <CArcImageDllMain.h>
//
//
//// +------------------------------------------------------------------------------------+
//// | Status/Error constants                                                             |
//// +------------------------------------------------------------------------------------+
//#ifndef ARC_STATUS
//#define ARC_STATUS
//
//	#define ARC_STATUS_OK			0
//	#define ARC_STATUS_ERROR		1
//	#define ARC_ERROR_MSG_SIZE		128
//
//#endif
//
//
//// +------------------------------------------------------------------------------------+
//// | Image statistics structs                                                           |
//// +------------------------------------------------------------------------------------+
//#ifndef ARC_IMG_STATS
//#define ARC_IMG_STATS
//
//	struct CImgStats
//	{
//		double gTotalPixels;
//		double gMin;
//		double gMax;
//		double gMean;
//		double gVariance;
//		double gStdDev;
//		double gSaturatedPixCnt;
//	};
//
//	struct CImgDifStats
//	{
//		CImgStats cImg1Stats;
//		CImgStats cImg2Stats;
//		CImgStats cImgDiffStats;
//	};
//
//#endif
//
//#ifdef __cplusplus
//   extern "C" {		// Using a C++ compiler
//#endif
//
//// +------------------------------------------------------------------------------------+
//// |  C Interface - Image bits-per-pixel constants                                      |
//// +------------------------------------------------------------------------------------+
//DLLARCIMAGE_API extern const int IMAGE_BPP16;
//DLLARCIMAGE_API extern const int IMAGE_BPP32;
//
//
//// +------------------------------------------------------------------------------------+
//// |  Used to free ANY and ALL pointers returned by methods                             |
//// +------------------------------------------------------------------------------------+
//DLLARCIMAGE_API void ArcImage_FreeU8( unsigned char* ptr, int* pStatus );
//DLLARCIMAGE_API void ArcImage_FreeFlt( float* ptr, int* pStatus );
//DLLARCIMAGE_API void ArcImage_FreeS32( int* ptr, int* pStatus );
//
//
//// +------------------------------------------------------------------------------------+
//// |  Image Data                                                                        |
//// +------------------------------------------------------------------------------------+
//DLLARCIMAGE_API unsigned char* ArcImage_GetRow( void* pBuf, int dRow, int dCol1, int dCol2, int dRows, int dCols, int* pCount, int dBpp, int* pStatus );
//DLLARCIMAGE_API unsigned char* ArcImage_GetCol( void* pBuf, int dCol, int dRow1, int dRow2, int dRows, int dCols, int* pCount, int dBpp, int* pStatus );
//
//DLLARCIMAGE_API float* ArcImage_GetRowArea( void* pBuf, int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int* pCount, int dBpp, int* pStatus );
//DLLARCIMAGE_API float* ArcImage_GetColArea( void* pBuf, int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int* pCount, int dBpp, int* pStatus );
//
//
//// +------------------------------------------------------------------------------------+
//// |  Image Statistics                                                                  |
//// +------------------------------------------------------------------------------------+
//DLLARCIMAGE_API CImgDifStats ArcImage_GetDiffStats( void* pBuf1, void* pBuf2, int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int dBpp, int* pStatus );
//DLLARCIMAGE_API CImgDifStats ArcImage_GetImageDiffStats( void* pBuf1, void* pBuf2, int dRows, int dCols, int dBpp, int* pStatus );
//
//DLLARCIMAGE_API CImgStats ArcImage_GetStats( void* pBuf, int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int dBpp, int* pStatus );
//DLLARCIMAGE_API CImgStats ArcImage_GetImageStats( void* pBuf, int dRows, int dCols, int dBpp, int* pStatus );
//
//
//// +------------------------------------------------------------------------------------+
//// |  Histogram                                                                         |
//// +------------------------------------------------------------------------------------+
//DLLARCIMAGE_API int* ArcImage_Histogram( int* pCount, void* pBuf, int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int dBpp, int* pStatus );
//DLLARCIMAGE_API int* ArcImage_ImageHistogram( int* pCount, void* pBuf, int dRows, int dCols, int dBpp, int* pStatus );
//
//
//// +------------------------------------------------------------------------------------+
//// |  Image Buffer Manipulation                                                         |
//// +------------------------------------------------------------------------------------+
//DLLARCIMAGE_API void ArcImage_Add( unsigned int* pU32Buf, unsigned short* pU16Buf1, unsigned short* pU16Buf2, int dRows, int dCols, int* pStatus );
//DLLARCIMAGE_API void ArcImage_Subtract( void* pBuf1, void* pBuf2, int dRows, int dCols, int dBpp, int* pStatus );
//DLLARCIMAGE_API void ArcImage_SubtractHalves( void* pBuf, int dRows, int dCols, int dBpp, int* pStatus );
//DLLARCIMAGE_API void ArcImage_Divide( void* pBuf1, void* pBuf2, int dRows, int dCols, int dBpp, int* pStatus );
//
//
//// +------------------------------------------------------------------------------------+
//// |  Image Buffer Copy                                                                 |
//// +------------------------------------------------------------------------------------+
//DLLARCIMAGE_API void ArcImage_Copy( void* pDstBuf, int dDstRows, int dDstCols, void* pSrcBuf, int dSrcRows, int dSrcCols, int dBpp, int* pStatus );
//DLLARCIMAGE_API void ArcImage_CopyBySize( void* pDstBuf, int dDstSize, void* pSrcBuf, int dSrcSize, int* pStatus );
//
//
//// +------------------------------------------------------------------------------------+
//// |  Image Buffer Fill                                     |
//// +------------------------------------------------------------------------------------+
//DLLARCIMAGE_API void ArcImage_Fill( void* pBuf, int dRows, int dCols, int dValue, int dBpp, int* pStatus );
//DLLARCIMAGE_API void ArcImage_GradientFill( void* pBuf, int dRows, int dCols, int dBpp, int* pStatus );
//DLLARCIMAGE_API void ArcImage_SmileyFill( void* pBuf, int dRows, int dCols, int dBpp, int* pStatus );
//
//
//// +------------------------------------------------------------------------------------+
//// |  VerifyAsSynthetic                                                                 |
//// +------------------------------------------------------------------------------------+
//DLLARCIMAGE_API void ArcImage_VerifyAsSynthetic( void* pBuf, int dRows, int dCols, int dBpp, int* pStatus );
//
//DLLARCIMAGE_API const char* ArcImage_GetLastError();
//
//#ifdef __cplusplus
//   }
//#endif
//
//#endif		// _ARC_IMAGE_CAPI_H_
