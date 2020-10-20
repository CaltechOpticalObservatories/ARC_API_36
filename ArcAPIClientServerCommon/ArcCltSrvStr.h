#ifndef _CLT_SVR_STR_H_
#define _CLT_SVR_STR_H_

namespace arc
{
	#define METHOD_LogMsgOnServer				"LogMsgOnServer"
	#define METHOD_EnableServerLog				"EnableServerLog"
	#define METHOD_IsServerLogging				"IsServerLogging"
	#define METHOD_GetServerVersion				"GetServerVersion"

	#define METHOD_ToString						"ToString"
	#define METHOD_GetDeviceList				"GetDeviceList"
	#define METHOD_FreeDeviceList				"FreeDeviceList"
	#define METHOD_UseDevices					"UseDevices"

	#define METHOD_IsOpen						"IsOpen"
	#define METHOD_Open							"Open"
	#define METHOD_Close						"Close"
	#define METHOD_Reset						"Reset"

	#define METHOD_MapCommonBuffer				"MapCommonBuffer"
	#define METHOD_UnMapCommonBuffer			"UnMapCommonBuffer"
	#define METHOD_ReMapCommonBuffer			"ReMapCommonBuffer"
	#define METHOD_GetCommonBufferProperties	"GetCommonBufferProperties"
	#define METHOD_FillCommonBuffer				"FillCommonBuffer"
	#define METHOD_CommonBufferVA				"CommonBufferVA"
	#define METHOD_CommonBufferPA				"CommonBufferPA"
	#define METHOD_CommonBufferSize				"CommonBufferSize"
	#define METHOD_CommonBufferPixels			"CommonBufferPixels"

	#define METHOD_GetId						"GetId"
	#define METHOD_GetStatus					"GetStatus"
	#define METHOD_ClearStatus					"ClearStatus"
	#define METHOD_Set2xFOTransmitter			"Set2xFOTransmitter"
	#define METHOD_LoadDeviceFile				"LoadDeviceFile"

	#define METHOD_GetCfgSpByte					"GetCfgSpByte"
	#define METHOD_GetCfgSpWord					"GetCfgSpWord"
	#define METHOD_GetCfgSpDWord				"GetCfgSpDWord"

	#define METHOD_SetCfgSpByte					"SetCfgSpByte"
	#define METHOD_SetCfgSpWord					"SetCfgSpWord"
	#define METHOD_SetCfgSpDWord				"SetCfgSpDWord"

	#define METHOD_Command						"Command"
	#define METHOD_GetControllerId				"GetControllerId"
	#define METHOD_ResetController				"ResetController"
	#define METHOD_IsControllerConnected		"IsControllerConnected"
	#define METHOD_SetupController				"SetupController"
	#define METHOD_LoadControllerFile			"LoadControllerFile"
	#define METHOD_SetImageSize					"SetImageSize"
	#define METHOD_GetImageRows					"GetImageRows"
	#define METHOD_GetImageCols					"GetImageCols"
	#define METHOD_GetCCParams					"GetCCParams"
	#define METHOD_IsCCParamSupported			"IsCCParamSupported"
	#define METHOD_IsCCD						"IsCCD"

	#define METHOD_IsBinningSet					"IsBinningSet"
	#define METHOD_SetBinning					"SetBinning"
	#define METHOD_UnSetBinning					"UnSetBinning"

	#define METHOD_SetSubArray					"SetSubArray"
	#define METHOD_UnSetSubArray				"UnSetSubArray"

	#define METHOD_IsSyntheticImageMode			"IsSyntheticImageMode"
	#define METHOD_SetSyntheticImageMode		"SetSyntheticImageMode"
	#define METHOD_VerifyImageAsSynthetic		"VerifyImageAsSynthetic"

	#define METHOD_SetOpenShutter				"SetOpenShutter"
	#define METHOD_Expose						"Expose"
	#define METHOD_StopExposure					"StopExposure"
	#define METHOD_Continuous					"Continuous"
	#define METHOD_StopContinuous				"StopContinuous"
	#define METHOD_IsReadout					"IsReadout"
	#define METHOD_GetPixelCount				"GetPixelCount"
	#define METHOD_GetCRPixelCount				"GetCRPixelCount"
	#define METHOD_GetFrameCount				"GetFrameCount"

	#define METHOD_SubtractImageHalves			"SubtractImageHalves"

	#define METHOD_GetArrayTemperature			"GetArrayTemperature"
	#define METHOD_GetArrayTemperatureDN		"GetArrayTemperatureDN"
	#define METHOD_SetArrayTemperature			"SetArrayTemperature"
	#define METHOD_LoadTemperatureCtrlData		"LoadTemperatureCtrlData"
	#define METHOD_SaveTemperatureCtrlData		"SaveTemperatureCtrlData"

	#define METHOD_GetNextLoggedCmd				"GetNextLoggedCmd"
	#define METHOD_GetLoggedCmdCount			"GetLoggedCmdCount"
	#define METHOD_SetLogCmds					"SetLogCmds"

	//#define METHOD_Deinterlace					"Deinterlace"
	//#define METHOD_DeinterlaceFits				"DeinterlaceFits"
	//#define METHOD_DeinterlaceFits3D			"DeinterlaceFits3D"
	#define METHOD_RunAlg						"RunAlg"
	#define METHOD_RunAlgFits					"RunAlgFits"
	#define METHOD_RunAlgFits3D					"RunAlgFits3D"

	#define METHOD_CFitsFile					"CFitsFile"
	#define METHOD_CloseFits					"CloseFits"
	#define METHOD_GetFitsFilename				"GetFitsFilename"
	#define METHOD_GetFitsHeader				"GetFitsHeader"
	#define METHOD_WriteFitsKeyword				"WriteFitsKeyword"
	#define METHOD_UpdateFitsKeyword			"UpdateFitsKeyword"
	#define METHOD_GetFitsParameters			"GetFitsParameters"
	#define METHOD_GenerateFitsTestData			"GenerateFitsTestData"
	#define METHOD_ReOpenFits					"ReOpenFits"
	#define METHOD_CompareFits					"CompareFits"
	#define METHOD_ResizeFits					"ResizeFits"
	#define METHOD_WriteFits					"WriteFits"
	#define METHOD_WriteFitsSubImage			"WriteFitsSubImage"
	#define METHOD_ReadFitsSubImage				"ReadFitsSubImage"
	#define METHOD_ReadFits						"ReadFits"
	#define METHOD_WriteFits3D					"WriteFits3D"
	#define METHOD_ReWriteFits3D				"ReWriteFits3D"
	#define METHOD_ReadFits3D					"ReadFits3D"

	#define METHOD_CTiffFile					"CTiffFile"
	#define METHOD_CloseTiff					"CloseTiff"
	#define METHOD_GetTiffRows					"GetTiffRows"
	#define METHOD_GetTiffCols					"GetTiffCols"
	#define METHOD_WriteTiff					"WriteTiff"
	#define METHOD_ReadTiff						"ReadTiff"

	#define METHOD_Histogram					"Histogram"
	#define METHOD_FreeHistogram				"FreeHistogram"
	#define METHOD_GetImageRow					"GetImageRow"
	#define METHOD_GetImageCol					"GetImageCol"
	#define METHOD_GetImageRowArea				"GetImageRowArea"
	#define METHOD_GetImageColArea				"GetImageColArea"
	#define METHOD_FreeImageData				"FreeImageData"
	#define METHOD_GetStats						"GetStats"
	#define METHOD_GetDiffStats					"GetDiffStats"
	#define METHOD_GetFitsStats					"GetFitsStats"
	#define METHOD_GetFitsDiffStats				"GetFitsDiffStats"

	//#define SERVER_STRING						"arc::CArcAPIServer"
	//#define DEVICE_STRING						"arc::CArcDevice"
	//#define DEINTERLACE_STRING					"arc::CDeinterlace"
	//#define FITSFILE_STRING						"arc::CFitsFile"
	//#define TIFFFILE_STRING						"arc::CTiffFile"
	//#define IMAGE_STRING						"arc::CImage"
	#define ERROR_STRING						"EXCEPTION"
	#define CLIENT_OK_STRING					"CLIENT OK"
	#define API_OK_STRING						"ARC API OK"

	#define CLASS_CArcAPIServer					"arc::CArcAPIServer"
	#define CLASS_CArcDevice					"arc::CArcDevice"
	#define CLASS_CDeinterlace					"arc::CDeinterlace"
	#define CLASS_CFitsFile						"arc::CFitsFile"
	#define CLASS_CTiffFile						"arc::CTiffFile"
	#define CLASS_CImage						"arc::CImage"

	//#define METHOD_ID_ToString					0xAAAA0000
	//#define METHOD_ID_GetDeviceList				0xAAAA0001
	//#define METHOD_ID_FreeDeviceList			0xAAAA0002
	//#define METHOD_ID_UseDevices				0xAAAA0003
}

#endif
