#ifndef _CARCAPI_CLIENT_H_
#define _CARCAPI_CLIENT_H_


#ifdef WIN32
#include <Winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#endif

#include <vector>
#include <ArcOSDefs.h>
#include <ArcDefs.h>
#include <CArcAPIClientDllMain.h>


namespace arc
{
	#ifndef WIN32
		#define INVALID_SOCKET			-1
		#define SOCKET_ERROR			-1
		#define NI_MAXHOST				512
		#define NI_MAXSERV				512
		typedef int						SOCKET;
		typedef struct sockaddr_in		SOCKADDR_IN;
		typedef const struct sockaddr*	LPSOCKADDR;
		typedef unsigned long			ULONG_PTR;
		typedef long					HANDLE;
	#endif


	// +------------------------------------------------------+
	// | CArcAPIClient class definition                       |
	// +------------------------------------------------------+
	class CARCAPICLIENT_API CArcAPIClient
	{
		public:
			CArcAPIClient( void );
			virtual ~CArcAPIClient( void );

			const std::string ToString();

			void DetectServers( unsigned short usPort = DEFAULT_PORT );

			//   Server Connection Methods
			// +--------------------------------------------------------------+
			void Connect( const char* pszIPAddr, unsigned short u16Port = CArcAPIClient::DEFAULT_PORT );
			void CloseConnections();
			void SendInvalidCommand( const char* szCmd );
			void SetEndOfLine( bool bOnOff );

			//   General Server Methods
			// +---------------------------------------------------------------+
			void   GetDirListing( const std::string sTargetDir, std::vector<std::string>& vecDirList, bool bSearchSubDir = false );
			void   LogMsgOnServer( const char* szMsg );
			void   EnableServerLog( bool bEnable );
			bool   IsServerLogging();
			double GetServerVersion();

			//   Method Call Methods
			// +---------------------------------------------------------------+
			std::string CallMethod( const std::string sClazz, const std::string sMethod, const std::string sFormat, ... );

/****
			//   CArcDevice Methods
			// +---------------------------------------------------------------+
			char** GetDeviceList( int& dCount );
			void   FreeDeviceList();
			void   UseDevices( const char** pszDeviceList, int dListCount );

			bool IsOpen();
			void Open( const char* pszDevice );
			void Open( const char* pszDevice, int dBytes );
			void Close();
			void Reset();

			void  MapCommonBuffer( int dBytes = 0 );
			void  UnMapCommonBuffer();
			void  ReMapCommonBuffer( int dBytes = 0 );
			bool  GetCommonBufferProperties();
			void  FillCommonBuffer( unsigned short u16Value = 0 );
			void* CommonBufferVA();
			ulong CommonBufferPA();
			int   CommonBufferSize();
			int	  CommonBufferPixels( void* pBuffer, int dSize );

			int  GetId();
			int  GetStatus();
			void ClearStatus();

			void Set2xFOTransmitter( bool bOnOff );
			void LoadDeviceFile( const char* pszFile );

			int  GetCfgSpByte( int dOffset );
			int  GetCfgSpWord( int dOffset );
			int  GetCfgSpDWord( int dOffset );

			void SetCfgSpByte( int dOffset, int dVal );
			void SetCfgSpWord( int dOffset, int dVal );
			void SetCfgSpDWord( int dOffset, int dVal );

			//  Setup & General commands
			// +-------------------------------------------------+
			int  Command( int dBoardId, int dCommand, int dArg1 = -1, int dArg2 = -1, int dArg3 = -1, int dArg4 = -1 );
			int  GetControllerId();
			void ResetController();
			bool IsControllerConnected();

			void SetupController( bool bReset, bool bTdl, bool bPower, int dRows, int dCols, const char* pszTimFile,
							      const char* pszUtilFile = NULL, const char* pszPciFile = NULL );

			void LoadControllerFile( const char* pszFileName, bool bValidate = true );
			void SetImageSize( int dRows, int dCols );
			int  GetImageRows();
			int  GetImageCols();
			int  GetCCParams();
			bool IsCCParamSupported( int dParameter );
			bool IsCCD();

			bool IsBinningSet();
			void SetBinning( int dRows, int dCols, int dRowFactor, int dColFactor, int* pBinRows = NULL, int* pBinCols = NULL );
			void UnSetBinning( int dRows, int dCols );

			void SetSubArray( int& dOldRows, int& dOldCols, int dRow, int dCol, int dSubRows, int dSubCols, int dBiasOffset, int dBiasWidth );
			void UnSetSubArray( int dRows, int dCols );

			bool IsSyntheticImageMode();
			void SetSyntheticImageMode( bool bMode );

			//  Expose commands
			// +-------------------------------------------------+
			void SetOpenShutter( bool bShouldOpen );

			void Expose( float fExpTime, int dRows, int dCols, const bool& bAbort = false,
						 CExpIFace* pExpIFace = NULL, bool bOpenShutter = true );

			void StopExposure();

			void Continuous( int dRows, int dCols, int dNumOfFrames, float fExpTime, const bool& bAbort = false,
							 CConIFace* pConIFace = NULL, bool bOpenShutter = true );

			void StopContinuous();

			bool IsReadout();
			int  GetPixelCount();
			int  GetCRPixelCount();
			int  GetFrameCount();

			double GetArrayTemperature();
			double GetArrayTemperatureDN();
			void   SetArrayTemperature( double gTempVal );
			void   LoadTemperatureCtrlData( const char* szFilename );
			void   SaveTemperatureCtrlData( const char* szFilename );

			const char *GetNextLoggedCmd();
			int         GetLoggedCmdCount();
			void        SetLogCmds( bool bOnOff );

			//   CDeinterlace Methods
			// +---------------------------------------------------------------+
			void Deinterlace( int dRows, int dCols, int dAlgorithm, int dArg = -1, int dPixOffset = 0 );
			void DeinterlaceFits( const char* szFitsFile, int dAlgorithm, int dArg = -1 );
			void DeinterlaceFits3D( const char* szFitsFile, int dAlgorithm, int dArg = -1 );

			//   CFitsFile Methods
			// +---------------------------------------------------------------+
			void   CFitsFile( const char *c_filename, int dMode = CFitsFile::READMODE );
			void   CFitsFile( const char *c_filename, int dRows, int dCols, int dBitsPerPixel = CFitsFile::BPP16, bool bIs3D = false );
			void   CloseFits();
			const  std::string GetFitsFilename();
			char** GetFitsHeader( int* pKeyCount );
			void   WriteFitsKeyword( char* szKey, void* pKeyVal, int dValType, char* szComment );
			void   UpdateFitsKeyword( char* szKey, void* pKeyVal, int dValType, char* szComment );
			void   GetFitsParameters( long* pNAxes, int* pNAxis = NULL, int* pBitsPerPixel = NULL );
			void   GenerateFitsTestData();
			void   ReOpenFits();
			bool   CompareFits( arc::CFitsFile& anotherCFitsFile );
			void   ResizeFits( int dRows, int dCols );
			void   WriteFits();
			void   WriteFits( unsigned int udBytesToSkip, unsigned int udBytesToWrite, int dFPixl = -1 );
			void   WriteFitsSubImage( int dRowStart, int dColStart, int dRowEnd, int dColEnd, void* pData );
			void*  ReadFitsSubImage( int llrow, int llcol, int urrow, int urcol );
			void*  ReadFits();
			void   WriteFits3D( unsigned int udByteOffset );
			void   ReWriteFits3D( unsigned int udByteOffset, int dImageNumber );
			void*  ReadFits3D( int dImageNumber );

			//   CTiffFile Methods
			// +---------------------------------------------------------------+
			void  CTiffFile( const char *c_filename, int dMode = CTiffFile::WRITEMODE );
			void  CloseTiff();
			int   GetTiffRows();
			int   GetTiffCols();
			void  WriteTiff( int dRows, int dCols );
			void* ReadTiff();

			//   CImage Methods
			// +---------------------------------------------------------------+
			int* Histogram( int& pHistSize, void *pMem, int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int dBpp = arc::CImage::BPP16 );
			int* Histogram( int& pHistSize, void *pMem, int dRows, int dCols, int dBpp = arc::CImage::BPP16 );
			void FreeHistogram();

			void* GetImageRow( int dRow, int dCol1, int dCol2, int dRows, int dCols, int& dElemCount );
			void* GetImageCol( int dCol, int dRow1, int dRow2, int dRows, int dCols, int& dElemCount );

			void* GetImageRowArea( int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int& dElemCount );
			void* GetImageColArea( int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int& dElemCount );

			void FreeImageData( void* pBuf, int dTypeSize = sizeof( unsigned short ) );

			void SubtractImageHalves( int dRows, int dCols );
			void VerifyImageAsSynthetic( int dRows, int dCols );

			arc::CImage::CImgStats GetStats( void* pMem, int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int dBpp = arc::CImage::BPP16 );
			arc::CImage::CImgStats GetStats( void* pMem, int dRows, int dCols, int dBpp = arc::CImage::BPP16 );
			arc::CImage::CImgStats GetStats( const char* szFitsFile, int dRow1, int dRow2, int dCol1, int dCol2 );

			arc::CImage::CImgDifStats GetDiffStats( void* pMem1, void *pMem2, int dRow1, int dRow2, int dCol1, int dCol2, int dRows, int dCols, int dBpp = arc::CImage::BPP16 );
			arc::CImage::CImgDifStats GetDiffStats( void* pMem1, void *pMem2, int dRows, int dCols, int dBpp = arc::CImage::BPP16 );
			arc::CImage::CImgDifStats GetDiffStats( const char* szFitsFile1, const char* szFitsFile2, int dRow1, int dRow2, int dCol1, int dCol2 );

			//   Debug Console Methods
			// +---------------------------------------------------------------+
			void ShowDebugConsole();
			void WriteToConsole( const std::string sMsg );
			void FreeDebugConsole();
*****/

			//  Class Constants
			// +---------------------------------------------------------------+
			static const int DEFAULT_PORT			= 5000;

		protected:
			void ThrowException( std::string sMethodName, std::string sMsg );
			void CheckExceptionOccurred( std::string sRecvd );
			std::string GetSystemMessage( int dCode );
			bool ContainsError( int dWord );
			int  GetError();

			void SendFile( std::string sFilename );
			void Send( const char *fmt, ... );
			void Send2( const char *fmt, va_list tArgList );
			void SendOK();

			int  Recv( void* pBuffer, int dSize );
			std::string Recv();

			int  BytesAvailable();
			int  GetFileLength( std::string sFilename );
			void ZeroMem( void* pBuf, int dSize );
			void PrepMultiStringData( std::string& sData );

			////  Smart pointer array deleter
			//// +--------------------------------------------------------------------+
			//static void ArrayDeleter( void* p );

			#ifdef WIN32
			WSADATA m_wsaData;
			#endif

			HANDLE	m_hStdOutConsole;
			SOCKET	m_tSocket;
			bool	m_bEndOfLine;
			int*	m_pHist;
			char	m_szTemp[ 256 ];
			char**	m_pDevList;

			//std::tr1::shared_ptr<void> pRcvData;
	};

}	// end namespace

#endif
