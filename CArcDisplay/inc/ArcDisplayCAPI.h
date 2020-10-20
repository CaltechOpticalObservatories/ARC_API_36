#ifndef _ARC_DISPLAY_CAPI_H_
#define _ARC_DISPLAY_CAPI_H_

#include <CArcDisplayDllMain.h>


// +------------------------------------------------------------------------------------+
// | Status/Error constants
// +------------------------------------------------------------------------------------+
#ifndef ARC_STATUS
#define ARC_STATUS

#define ARC_STATUS_OK			0
#define ARC_STATUS_ERROR		1
#define ARC_ERROR_MSG_SIZE		128

#endif


#ifdef __cplusplus
extern "C" {		// Using a C++ compiler
#endif

// +----------------------------------------------------------------------------------+
// |  C Interface - Deinterlace Functions                                             |
// +----------------------------------------------------------------------------------+
	GEN3_CARCDISPLAY_API void ArcDisplay_Launch( int dMSDelay, int* pStatus );
	GEN3_CARCDISPLAY_API void ArcDisplay_ShowFits( const char* pszFitsFile, int* pStatus );
	GEN3_CARCDISPLAY_API void ArcDisplay_Clear( int dFrame, int* pStatus );
	GEN3_CARCDISPLAY_API void ArcDisplay_Terminate( int* pStatus );

	GEN3_CARCDISPLAY_API const char* ArcDisplay_GetLastError();

#ifdef __cplusplus
}
#endif

#endif		// _ARC_DISPLAY_CAPI_H_





//#ifndef _ARC_DISPLAY_CAPI_H_
//#define _ARC_DISPLAY_CAPI_H_
//
//#include <CArcDisplayDllMain.h>
//
//
//// +------------------------------------------------------------------------------------+
//// | Status/Error constants
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
//// +----------------------------------------------------------------------------------+
//// |  C Interface - Deinterlace Functions                                             |
//// +----------------------------------------------------------------------------------+
//DLLDISPLAY_API void ArcDisplay_Launch( int dMSDelay, int* pStatus );
//DLLDISPLAY_API void ArcDisplay_Show( void *pBuffer, int dRows, int dCols, int* pStatus );
//DLLDISPLAY_API void ArcDisplay_ShowFits( const char* pszFitsFile, int* pStatus );
//DLLDISPLAY_API void ArcDisplay_Clear( int dFrame, int* pStatus );
//DLLDISPLAY_API void ArcDisplay_Terminate( int* pStatus );
//
//DLLDISPLAY_API const char* ArcDisplay_GetLastError();
//
//#ifdef __cplusplus
//   }
//#endif
//
//#endif		// _ARC_DISPLAY_CAPI_H_
