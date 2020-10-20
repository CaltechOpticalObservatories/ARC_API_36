// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  ArcDeinterlaceCAPI.h  ( Gen3 )                                                                           |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE: This file defines a C interface for all the CArcDeinterlace class methods.                             |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 27, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2014 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+

#ifndef _ARC_GEN3_DLACE_CAPI_H_
#define _ARC_GEN3_DLACE_CAPI_H_


#include <stdlib.h>

#include <CArcDeinterlaceDllMain.h>


// +------------------------------------------------------------------------------------------------------------------+
// | Status definitions                                                                                               |
// +------------------------------------------------------------------------------------------------------------------+

typedef unsigned int ArcStatus_t;				/** Return status type */


#ifdef __cplusplus
extern "C" {		// Using a C++ compiler
#endif

	typedef uint32_t ArcError_t;

	extern GEN3_CARCDEINTERLACE_API const ArcError_t* ARC_STATUS_NONE;
	extern GEN3_CARCDEINTERLACE_API const ArcError_t  ARC_STATUS_OK;
	extern GEN3_CARCDEINTERLACE_API const ArcError_t  ARC_STATUS_ERROR;

	extern GEN3_CARCDEINTERLACE_API const ArcError_t  ARC_MSG_SIZE;
	extern GEN3_CARCDEINTERLACE_API const ArcError_t  ARC_ERROR_MSG_SIZE;

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
extern "C" {		// Using a C++ compiler
#endif

	// +------------------------------------------------------------------------------------------------------------------+
	// | Define deinterlace image bits-per-pixel constants                                                                |
	// +------------------------------------------------------------------------------------------------------------------+
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_BPP16;			/** 16-bit bits-per-pixel image data */
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_BPP32;			/** 32-bit bits-per-pixel image data */


	// +------------------------------------------------------------------------------------------------------------------+
	// | Define deinterlace algorithms                                                                                    |
	// +------------------------------------------------------------------------------------------------------------------+
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_NONE_ALG;
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_PARALLEL_ALG;
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_SERIAL_ALG;
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_QUAD_CCD_ALG;
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_QUAD_IR_ALG;
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_QUAD_IR_CDS_ALG;
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_HAWAII_RG_ALG;
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_STA1600_ALG;
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_CUSTOM_ALG;


	// +------------------------------------------------------------------------------------------------------------------+
	// | Define deinterlace algorithm no argument constants                                                               |
	// +------------------------------------------------------------------------------------------------------------------+
	GEN3_CARCDEINTERLACE_API extern const unsigned int DLACE_NO_ARG;

#ifdef __cplusplus
}
#endif


// +------------------------------------------------------------------------------------------------------------------+
// | CArcDeinterlace method definitions                                                                               |
// |                                                                                                                  |
// | @see See CArcDeinterlace class definition ( CArcDeinterlace.h ) for method definitions.                          |
// +------------------------------------------------------------------------------------------------------------------+

#ifdef __cplusplus
extern "C" {		// Using a C++ compiler
#endif

	/** Returns a handle to the deinterlace object appropriate for the specified bits-per-pixel.
	 *  @param uiBpp	- The number of bits-per-pixel in the image.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 *  @return A reference to a deinterlace object.
	 */
	GEN3_CARCDEINTERLACE_API unsigned long long	ArcDLace_getInstance( unsigned int uiBpp, ArcStatus_t* pStatus );

	/** Returns the library build and version info.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 *  @return A strng representation of the library version.
	 */
	GEN3_CARCDEINTERLACE_API const char* ArcDLace_version( ArcStatus_t* pStatus );

	/** Returns a textual representation of the class. See CArcDevice::toString() for details.
	 *  @param ulHandle	- A reference to a deinterlace object.
	 *  @param pBuf		- The image buffer data.
	 *  @param uiCols	- The number of columns in the image.
	 *  @param uiRows	- The number of rows in the image.
	 *  @param uiAlg	- The deinterlace algorithm.
	 *  @param uiArg	- An algorithm dependent argument. Use DLACE_NO_ARG if not needed.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCDEINTERLACE_API void ArcDLace_run( unsigned long long ulHandle, void* pBuf, unsigned int uiCols, unsigned int uiRows,
		unsigned int uiAlg, unsigned int uiArg, ArcStatus_t* pStatus );

	/** Returns the last reported error message.
	 *  @return The last error message.
	 */
	GEN3_CARCDEINTERLACE_API const char* ArcDLace_getLastError( void );

	/** Returns the maximum value for a specific data type. Example, for an unsigned short: 2 ^ 16 = 65536.
	 *  @param ulHandle	- A reference to a deinterlace object.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCDEINTERLACE_API unsigned int ArcDLace_maxTVal( unsigned long long ulHandle, ArcStatus_t* pStatus );

	/** Returns true [ 1 ] if plugins were found; false [ 0 ] otherwise.
	 *  @param ulHandle	- A reference to a deinterlace object.
	 *  @param pszDir	- The directory to search for libraries in.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCDEINTERLACE_API unsigned int ArcDLace_findPlugins( unsigned long long ulHandle, const char* pszDir, ArcStatus_t* pStatus );

	/** Returns the number of loaded plugins.
	 *  @param ulHandle	- A reference to a deinterlace object.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCDEINTERLACE_API unsigned int ArcDLace_pluginCount( unsigned long long ulHandle, ArcStatus_t* pStatus );

	/** Returns the list of algorithms supported by the plugin.
	 *  @param ulHandle	- A reference to a deinterlace object.
	 *  @param uiPlugin  - The plugin index. The range is 0 to ArcDLace_pluginCount().
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCDEINTERLACE_API const char** ArcDLace_pluginList( unsigned long long ulHandle, unsigned int uiPlugin, ArcStatus_t* pStatus );

	/** Returns the number of algorithms supported by the plugin.
	 *  @param ulHandle	- A reference to a deinterlace object.
	 *  @param uiPlugin  - The plugin index. The range is 0 to ArcDLace_pluginCount().
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCDEINTERLACE_API unsigned int ArcDLace_pluginListCount( unsigned long long ulHandle, unsigned int uiPlugin, ArcStatus_t* pStatus );

	/** Executes a custom plugin deinterlace algorithm on the specified image buffer.
	 *  @param ulHandle	- A reference to a deinterlace object.
	 *  @param uiPlugin - The plugin index. The range is 0 to ArcDLace_pluginCount().
	 *  @param pBuf		- The image buffer data.
	 *  @param uiCols	- The number of columns in the image.
	 *  @param uiRows	- The number of rows in the image.
	 *  @param pszAlg	- The deinterlace algorithm name. One of the strings returned from the pluginList() function.
	 *  @param uiArg	- An algorithm dependent argument. Use DLACE_NO_ARG if not needed.
	 *  @param pStatus	- Success state; equals ARC_STATUS_OK or ARC_STATUS_ERROR.
	 */
	GEN3_CARCDEINTERLACE_API void ArcDLace_pluginRun( unsigned long long ulHandle, unsigned int uiPlugin, void* pBuf, unsigned int uiCols,
		unsigned int uiRows, const char* pszAlg, unsigned int uiArg, ArcStatus_t* pStatus );

#ifdef __cplusplus
}
#endif


#endif		// _ARC_GEN3_DLACE_CAPI_H_


















//#ifndef _ARC_DEINTERLACE_CAPI_H_
//#define _ARC_DEINTERLACE_CAPI_H_
//
//#include <CArcDeinterlaceDllMain.h>
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
//#ifdef __cplusplus
//   extern "C" {		// Using a C++ compiler
//#endif
//
//// +------------------------------------------------------------------------------------+
//// |  C Interface - Deinterlace Alogrithms                                              |
//// +------------------------------------------------------------------------------------+
//CDEINTERLACE_API extern const int DEINTERLACE_NONE;
//CDEINTERLACE_API extern const int DEINTERLACE_PARALLEL;
//CDEINTERLACE_API extern const int DEINTERLACE_SERIAL;
//CDEINTERLACE_API extern const int DEINTERLACE_CCD_QUAD;
//CDEINTERLACE_API extern const int DEINTERLACE_IR_QUAD;
//CDEINTERLACE_API extern const int DEINTERLACE_CDS_IR_QUAD;
//CDEINTERLACE_API extern const int DEINTERLACE_HAWAII_RG;
//CDEINTERLACE_API extern const int DEINTERLACE_STA1600;
//
//
//// +------------------------------------------------------------------------------------+
//// |  C Interface - Deinterlace Functions                                               |
//// +------------------------------------------------------------------------------------+
//CDEINTERLACE_API void ArcDeinterlace_RunAlg( void* pData, int dRows, int dCols, int dAlgorithm, int* pStatus );
//
//CDEINTERLACE_API void ArcDeinterlace_RunAlgWArg( void* pData, int dRows, int dCols, int dAlgorithm, int dArg, int* pStatus );
//
//CDEINTERLACE_API const char* ArcDeinterlace_GetLastError();
//
//#ifdef __cplusplus
//   }
//#endif
//
//#endif		// _ARC_DEINTERLACE_CAPI_H_
