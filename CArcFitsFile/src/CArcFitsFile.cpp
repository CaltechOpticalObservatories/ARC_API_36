// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  CArcFitsFile.cpp  ( Gen3 )                                                                               |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE:  Defines the exported functions for the CArcFitsFile DLL.  Wraps the cfitsio library for convenience.  |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 25, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2013 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+

#include <type_traits>
#include <typeinfo>
#include <sstream>
#include <memory>
#include <cmath>

#include <CArcFitsFile.h>


#if defined( _WINDOWS ) || defined( __linux )
	#include <filesystem>
	#define ArcRemove( sFileName )		std::filesystem::remove( std::filesystem::path( sFileName ) )
#else
	#include <cstdio>
	#define ArcRemove( sFileName )		remove( sFileName.c_str() )
#endif




namespace arc
{
	namespace gen3
	{


		/** Ensures that the internal fits handle is valid.
		 *  @throws Throws an exception if the fits handle is not valid.
		 */
		#define VERIFY_FILE_HANDLE()		if ( m_pFits == nullptr )									\
											{															\
												THROW( "Invalid FITS handle, no file open" );			\
											}


		 // +----------------------------------------------------------------------------------------------------------+
		 // | Library build and version info                                                                           |
		 // +----------------------------------------------------------------------------------------------------------+
		template <typename T> const std::string CArcFitsFile<T>::m_sVersion = std::string( "ARC Gen IV FITS API Library.     " ) +

#ifdef _WINDOWS
			CArcBase::formatString( "[ Compiler Version: %d, Built: %s ]", _MSC_VER, __TIMESTAMP__ );
#else
			arc::gen3::CArcBase::formatString( "[ Compiler Version: %d.%d.%d, Built: %s %s ]", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __DATE__, __TIME__ );
#endif


		// +----------------------------------------------------------------------------------------------------------+
		// |  CParam Class constructor                                                                                |
		// +----------------------------------------------------------------------------------------------------------+
		fits::CParam::CParam( void ) : m_iNAxis( 0 ), m_iBpp( 0 )
		{
			m_plCols = &m_lNAxes[ 0 ];  *m_plCols = 0;
			m_plRows = &m_lNAxes[ 1 ];  *m_plRows = 0;
			m_plFrames = &m_lNAxes[ 2 ];  *m_plFrames = 0;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  CParam Class destructor                                                                                 |
		// +----------------------------------------------------------------------------------------------------------+
		fits::CParam::~CParam( void )
		{
		}

		// +----------------------------------------------------------------------------------------------------------+
		// |  CParam Class getCols                                                                                    |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the number of image columns.                                                                    |
		// +----------------------------------------------------------------------------------------------------------+
		std::uint32_t fits::CParam::getCols( void )
		{
			return static_cast< std::uint32_t >( m_lNAxes[ 0 ] );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  CParam Class getRows                                                                                    |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the number of image rows.                                                                       |
		// +----------------------------------------------------------------------------------------------------------+
		std::uint32_t fits::CParam::getRows( void )
		{
			return static_cast< std::uint32_t >( m_lNAxes[ 1 ] );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  CParam Class getFrames                                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the number of frames in the file.                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		std::uint32_t fits::CParam::getFrames( void )
		{
			return static_cast< std::uint32_t >( m_lNAxes[ 2 ] );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  CParam Class getNAxis                                                                                   |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the number of axes in the file.                                                                 |
		// +----------------------------------------------------------------------------------------------------------+
		std::uint32_t fits::CParam::getNAxis( void )
		{
			return static_cast< std::uint32_t >( m_iNAxis );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  CParam Class getBpp                                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the number of bits-per-pixel in the file.                                                       |
		// +----------------------------------------------------------------------------------------------------------+
		std::uint32_t fits::CParam::getBpp( void )
		{
			return static_cast< std::uint32_t >( m_iBpp );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  Class constructor                                                                                       |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> CArcFitsFile<T>::CArcFitsFile( void ) : CArcBase(), m_i64Pixel( 0 ), m_iFrame( 0 ), m_pFits( nullptr )
		{
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  Class destructor                                                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> CArcFitsFile<T>::~CArcFitsFile( void )
		{
			try
			{
				close();
			}
			catch ( ... ) {}
		}


		template <typename T> std::string CArcFitsFile<T>::getType( void )
		{
			return typeid( T ).name();
		}
		//template <typename T> typeid CArcFitsFile<T>::getType( void )
		//{
		//	return typeid( T );
		//}


		// +----------------------------------------------------------------------------------------------------------+
		// |  version                                                                                                 |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns a textual representation of the library version.                                                |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> const std::string CArcFitsFile<T>::version( void )
		{
			return m_sVersion;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  version                                                                                                 |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns a textual representation of the library version.                                                |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> const std::string CArcFitsFile<T>::cfitsioVersion( void )
		{
			float fVersion = 0.f;

			fits_get_version( &fVersion );

			const std::string sVersion = CArcBase::formatString( "CFITSIO Library.                 [ Version: %f ]", fVersion );

			return sVersion;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  create                                                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Creates a new single image file on disk with the specified image dimensions.                            |
		// |                                                                                                          |
		// |  <IN> -> sFileName - The new file name.                                                                  |
		// |  <IN> -> uiCols	- The image column size ( in pixels ).                                                |
		// |  <IN> -> uiRows	- The image row size ( in pixels ).                                                   |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		void CArcFitsFile<T>::create( const std::string& sFileName, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			std::int32_t iImageType = ( ( sizeof( T ) == sizeof( std::uint16_t ) ) ? USHORT_IMG : ULONG_IMG );

			std::int32_t iStatus = 0;

			long a_lNAxes[] = { static_cast< long >( uiCols ), static_cast< long >( uiRows ) };

			if ( m_pFits != nullptr )
			{
				close();
			}

			//
			// Verify image dimensions
			//
			if ( uiRows == 0 )
			{
				THROW_INVALID_ARGUMENT( "Row dimension must be greater than zero!" );
			}

			if ( uiCols == 0 )
			{
				THROW_INVALID_ARGUMENT( "Column dimension must be greater than zero!" );
			}

			//
			// Verify filename
			//
			if ( sFileName.empty() )
			{
				THROW_INVALID_ARGUMENT( "Invalid file name : %s", sFileName.c_str() );
			}

			//
			// Delete the file if it exists. This is to prevent creation errors.
			//
			ArcRemove( sFileName );

			//
			// Create the file ( force overwrite )
			//
			fits_create_file( &m_pFits, const_cast< std::string& >( sFileName ).insert( 0, 1, '!' ).c_str(), &iStatus );

			if ( iStatus )
			{
				close();

				//
				// Delete the file if it exists. This is to prevent creation errors.
				//
				ArcRemove( sFileName );

				throwFitsError( iStatus );
			}

			fits_create_img( m_pFits, iImageType, 2, a_lNAxes, &iStatus );

			if ( iStatus )
			{
				close();

				//
				// Delete the file if it exists. This is to prevent creation errors.
				//
				ArcRemove( sFileName );

				throwFitsError( iStatus );
			}

			m_i64Pixel = 0;
			m_iFrame = 0;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  create3D                                                                                                |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Creates a new data cube file on disk with the specified image dimensions.                               |
		// |                                                                                                          |
		// |  <IN> -> sFileName - The new file name.                                                                  |
		// |  <IN> -> uiCols	- The image column size ( in pixels ).                                                |
		// |  <IN> -> uiRows	- The image row size ( in pixels ).                                                   |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		void CArcFitsFile<T>::create3D( const std::string& sFileName, std::uint32_t uiCols, std::uint32_t uiRows )
		{
			std::int32_t iImageType = ( ( sizeof( T ) == sizeof( std::uint16_t ) ) ? USHORT_IMG : ULONG_IMG );

			std::int32_t iStatus = 0;

			long a_lNAxes[] = { static_cast< long >( uiCols ), static_cast< long >( uiRows ), 1 };		// cols, rows, nof

			if ( m_pFits != nullptr )
			{
				close();
			}

			//
			// Verify image dimensions
			//
			if ( uiRows == 0 )
			{
				THROW_INVALID_ARGUMENT( "Row dimension must be greater than zero!" );
			}

			if ( uiCols == 0 )
			{
				THROW_INVALID_ARGUMENT( "Column dimension must be greater than zero!" );
			}

			//
			// Verify filename
			//
			if ( sFileName.empty() )
			{
				THROW_INVALID_ARGUMENT( "Invalid file name : %s", sFileName.c_str() );
			}

			//
			// Delete the file if it exists. This is to prevent creation errors.
			//
			ArcRemove( sFileName );

			//
			// Create the file ( force overwrite )
			//
			fits_create_file( &m_pFits, const_cast< std::string& >( sFileName ).insert( 0, 1, '!' ).c_str(), &iStatus );

			if ( iStatus )
			{
				close();

				//
				// Delete the file if it exists. This is to prevent creation errors.
				//
				ArcRemove( sFileName );

				throwFitsError( iStatus );
			}

			fits_create_img( m_pFits, iImageType, 3, a_lNAxes, &iStatus );

			if ( iStatus )
			{
				close();

				//
				// Delete the file if it exists. This is to prevent creation errors.
				//
				ArcRemove( sFileName );

				throwFitsError( iStatus );
			}

			m_i64Pixel = 0;
			m_iFrame = 0;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  open                                                                                                    |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Opens an existing file. Can be used to open a file containing a single image or data cube ( a file with |
		// |  multiple image planes ).                                                                                |
		// |                                                                                                          |
		// |  <IN> -> sFileName - The file name.                                                                      |
		// |  <IN> -> eMode - The mode ( read/write ) with which to open the file ( default = e_ReadMode::READMODE ). |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcFitsFile<T>::open( const std::string& sFileName, arc::gen3::fits::e_ReadMode eMode )
		{
			std::int32_t iStatus = 0;
			std::int32_t iExists = 0;

			if ( m_pFits != nullptr )
			{
				close();
			}

			//
			// Verify filename
			//
			if ( sFileName.empty() )
			{
				THROW_INVALID_ARGUMENT( "Invalid file name : %s", sFileName.c_str() );
			}

			//
			// Make sure the specified file exists
			//
			fits_file_exists( sFileName.c_str(), &iExists, &iStatus );

			if ( iStatus )
			{
				m_pFits = nullptr;

				throwFitsError( iStatus );
			}

			//
			// Open the FITS file
			//
			fits_open_file( &m_pFits, sFileName.c_str(), static_cast< int >( eMode ), &iStatus );

			if ( iStatus )
			{
				m_pFits = nullptr;

				throwFitsError( iStatus );
			}

			m_i64Pixel = 0;
			m_iFrame = 0;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  close                                                                                                   |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Closes the file. All subsequent methods, except for create and open will result in an error and an      |
		// |  exception will be thrown.                                                                               |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		void CArcFitsFile<T>::close( void )
		{
			//
			//  DeleteBuffer requires access to the file, so don't close the file until
			// after deleteBuffer has been called!!!
			//
			if ( m_pFits != nullptr )
			{
				std::int32_t iStatus = 0;

				fits_close_file( m_pFits, &iStatus );
			}

			m_pFits = nullptr;

			m_i64Pixel = 0;
			m_iFrame = 0;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getHeader                                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the FITS header as a list of strings.                                                           |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<arc::gen3::CArcStringList> CArcFitsFile<T>::getHeader( void )
		{
			std::int32_t iNumOfKeys = 0;
			std::int32_t iStatus = 0;

			VERIFY_FILE_HANDLE()

				fits_get_hdrspace( m_pFits, &iNumOfKeys, nullptr, &iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}

			std::unique_ptr<arc::gen3::CArcStringList> pList( new arc::gen3::CArcStringList() );

			std::string sCard( 100, ' ' );

			for ( int i = 0; i < iNumOfKeys; i++ )
			{
				fits_read_record( m_pFits,
					( i + 1 ),
					const_cast< char* >( sCard.data() ),
					&iStatus );

				if ( iStatus )
				{
					throwFitsError( iStatus );
				}

				*( pList.get() ) << sCard.c_str();
			}

			return pList;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getFileName                                                                                             |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the filename associated with this CArcFitsFile object.                                          |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> const std::string CArcFitsFile<T>::getFileName( void )
		{
			std::int32_t iStatus = 0;

			std::string sFileName( 150, ' ' );

			VERIFY_FILE_HANDLE()

				fits_file_name( m_pFits, const_cast< char* >( sFileName.data() ), &iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}

			return sFileName.c_str();
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  readKeyword                                                                                             |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Reads a FITS keyword value from the header.  The keyword must be valid or an exception will be thrown.  |
		// |                                                                                                          |
		// |  <IN> -> sKey  - The header keyword. Can be "".  Ex: SIMPLE                                              |
		// |  <IN> -> eType - The keyword type, as defined in CArcFitsFile.h                                          |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		arc::gen3::fits::keywordValue_t CArcFitsFile<T>::readKeyword( const std::string& sKey, arc::gen3::fits::e_Type eType )
		{
			std::int32_t iType = static_cast< std::int32_t >( arc::gen3::fits::e_Type::FITS_INVALID_KEY );
			std::int32_t iStatus = 0;

			VERIFY_FILE_HANDLE()

			arc::gen3::fits::keywordValue_t keyValue; // ( 0, 0, 0, 0, 0.0, std::string() );

			switch ( eType )
			{
				case arc::gen3::fits::e_Type::FITS_STRING_KEY:
				{
					iType = TSTRING;

					char szValue[ 80 ];

					fits_read_key( m_pFits, iType, sKey.c_str(), szValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = std::string( szValue );
					#else
						keyValue = std::make_tuple( 0, 0, 0, 0, 0.0, std::string( szValue ) );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_INT_KEY:
				{
					iType = TINT;

					std::int32_t iValue = 0;

					fits_read_key( m_pFits, iType, sKey.c_str(), &iValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = iValue;
					#else
						keyValue = std::make_tuple( 0, iValue, 0, 0, 0.0, std::string() );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_UINT_KEY:
				{
					iType = TUINT;

					std::uint32_t uiValue = 0;

					fits_read_key( m_pFits, iType, sKey.c_str(), &uiValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = uiValue;
					#else
						keyValue = std::make_tuple( uiValue, 0, 0, 0, 0.0, std::string() );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_SHORT_KEY:
				{
					iType = TSHORT;

					std::int16_t wValue = 0;

					fits_read_key( m_pFits, iType, sKey.c_str(), &wValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = static_cast< std::int32_t >( wValue );
					#else
						keyValue = std::make_tuple( 0, static_cast< std::int32_t >( wValue ), 0, 0, 0.0, std::string() );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_USHORT_KEY:
				{
					iType = TUSHORT;

					std::uint16_t uwValue = 0;

					fits_read_key( m_pFits, iType, sKey.c_str(), &uwValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = static_cast< std::uint32_t >( uwValue );
					#else
						keyValue = std::make_tuple( static_cast< std::uint32_t >( uwValue ), 0, 0, 0, 0.0, std::string() );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_FLOAT_KEY:
				{
					iType = TFLOAT;

					float fValue = 0;

					fits_read_key( m_pFits, iType, sKey.c_str(), &fValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = static_cast< double >( fValue );
					#else
						keyValue = std::make_tuple( 0, 0, 0, 0, static_cast< double >( fValue ), std::string() );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_DOUBLE_KEY:
				{
					iType = TDOUBLE;

					double gValue = 0;

					fits_read_key( m_pFits, iType, sKey.c_str(), &gValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = gValue;
					#else
						keyValue = std::make_tuple( 0, 0, 0, 0, gValue, std::string() );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_BYTE_KEY:
				{
					iType = TBYTE;

					std::int8_t iValue = 0;

					fits_read_key( m_pFits, iType, sKey.c_str(), &iValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = static_cast< std::uint32_t >( iValue );
					#else
						keyValue = std::make_tuple( 0, static_cast< std::uint32_t >( iValue ), 0, 0, 0.0, std::string() );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_LONG_KEY:
				{
					iType = TLONG;

					std::int32_t iValue = 0;

					fits_read_key( m_pFits, iType, sKey.c_str(), &iValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = iValue;
					#else
						keyValue = std::make_tuple( 0, static_cast< std::int32_t >( iValue ), 0, 0, 0.0, std::string() );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_ULONG_KEY:
				{
					iType = TULONG;

					std::uint32_t uiValue = 0;

					fits_read_key( m_pFits, iType, sKey.c_str(), &uiValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = uiValue;
					#else
						keyValue = std::make_tuple( uiValue, 0, 0, 0, 0.0, std::string() );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_LONGLONG_KEY:
				{
					iType = TLONGLONG;

					std::int64_t iValue = 0;

					fits_read_key( m_pFits, iType, sKey.c_str(), &iValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = iValue;
					#else
						keyValue = std::make_tuple( 0, 0, 0, iValue, 0.0, std::string() );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_LOGICAL_KEY:
				{
					iType = TLOGICAL;

					std::int32_t iValue = 0;

					fits_read_key( m_pFits, iType, sKey.c_str(), &iValue, NULL, &iStatus );

					#ifndef __APPLE__
						keyValue = iValue;
					#else
						keyValue = std::make_tuple( 0, iValue, 0, 0, 0.0, std::string() );
					#endif
				}
				break;

				case arc::gen3::fits::e_Type::FITS_COMMENT_KEY:
				case arc::gen3::fits::e_Type::FITS_HISTORY_KEY:
				case arc::gen3::fits::e_Type::FITS_DATE_KEY:
				{
				}
				break;

				default:
				{
					iType = static_cast< std::int32_t >( arc::gen3::fits::e_Type::FITS_INVALID_KEY );

					THROW_INVALID_ARGUMENT( "Invalid FITS keyword type. See CArcFitsFile.h for valid type list" );
				}
			}

			return keyValue;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  writeKeyword                                                                                            |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Writes a FITS keyword to the header.  The keyword must be valid or an exception will be thrown.         |
		// |                                                                                                          |
		// |  'HIERARCH' keyword NOTE: This text will be prefixed to any keyword by the cfitsio library if the        |
		// |                           keyword is greater than 8 characters, which is the standard FITS keyword       |
		// |                           length. See the link below for details:                                        |
		// |                                                                                                          |
		// |   http://heasarc.gsfc.nasa.gov/docs/software/fitsio/c/f_user/node28.html                                 |
		// |                                                                                                          |
		// |   HIERARCH examples:                                                                                     |
		// |   -----------------                                                                                      |
		// |   HIERARCH LongKeyword = 47.5 / Keyword has > 8 characters & mixed case                                  |
		// |   HIERARCH XTE$TEMP = 98.6 / Keyword contains the '$' character                                          |
		// |   HIERARCH Earth is a star = F / Keyword contains embedded spaces                                        |
		// |                                                                                                          |
		// |  <IN> -> sKey     - The header keyword. Can be "".  Ex: SIMPLE                                           |
		// |  <IN> -> pvalue   - The value associated with the key. Ex: T                                             |
		// |  <IN> -> eType    - The keyword type, as defined in CArcFitsFile.h                                       |
		// |  <IN> -> sComment - The comment to attach to the keyword.  Can be "".                                    |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		void CArcFitsFile<T>::writeKeyword( const std::string& sKey, void* pValue, arc::gen3::fits::e_Type eType, const std::string& sComment )
		{
			std::int32_t iType = static_cast< std::int32_t >( arc::gen3::fits::e_Type::FITS_INVALID_KEY );
			std::int32_t iStatus = 0;

			VERIFY_FILE_HANDLE()

			//
			// Verify value pointer
			//
			if ( eType != arc::gen3::fits::e_Type::FITS_DATE_KEY && pValue == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid FITS key value, cannot be nullptr" );
			}

			//
			// Update the fits header with the specified keys
			//
			switch ( eType )
			{
				case arc::gen3::fits::e_Type::FITS_STRING_KEY:
				{
					iType = TSTRING;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_INT_KEY:
				{
					iType = TINT;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_UINT_KEY:
				{
					iType = TUINT;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_SHORT_KEY:
				{
					iType = TSHORT;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_USHORT_KEY:
				{
					iType = TUSHORT;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_FLOAT_KEY:
				{
					iType = TFLOAT;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_DOUBLE_KEY:
				{
					iType = TDOUBLE;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_BYTE_KEY:
				{
					iType = TBYTE;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_LONG_KEY:
				{
					iType = TLONG;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_ULONG_KEY:
				{
					iType = TULONG;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_LONGLONG_KEY:
				{
					iType = TLONGLONG;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_LOGICAL_KEY:
				{
					iType = TLOGICAL;
				}
				break;

				case arc::gen3::fits::e_Type::FITS_COMMENT_KEY:
				case arc::gen3::fits::e_Type::FITS_HISTORY_KEY:
				case arc::gen3::fits::e_Type::FITS_DATE_KEY:
				{
				}
				break;

				default:
				{
					iType = static_cast< std::int32_t >( arc::gen3::fits::e_Type::FITS_INVALID_KEY );

					THROW_INVALID_ARGUMENT( "Invalid FITS keyword type. See CArcFitsFile.h for valid type list" );
				}
			}

			//
			// write ( append ) a COMMENT keyword to the header. The comment
			// string will be continued over multiple keywords if it is
			// longer than 70 characters. 
			//
			if ( eType == arc::gen3::fits::e_Type::FITS_COMMENT_KEY )
			{
				fits_write_comment( m_pFits, static_cast< char* >( pValue ), &iStatus );
			}

			//
			// write ( append ) a HISTORY keyword to the header. The history
			// string will be continued over multiple keywords if it is
			// longer than 70 characters. 
			//
			else if ( eType == arc::gen3::fits::e_Type::FITS_HISTORY_KEY )
			{
				fits_write_history( m_pFits, static_cast< char* >( pValue ), &iStatus );
			}

			//
			// write the DATE keyword to the header. The keyword value will contain
			// the current system date as a character string in 'yyyy-mm-ddThh:mm:ss'
			// format. If a DATE keyword already exists in the header, then this
			// routine will simply update the keyword value with the current date.
			//
			else if ( eType == arc::gen3::fits::e_Type::FITS_DATE_KEY )
			{
				fits_write_date( m_pFits, &iStatus );
			}

			//
			// write a keyword of the appropriate data type into the header
			//
			else
			{
				fits_update_key( m_pFits,
					iType,
					sKey.c_str(),
					pValue,
					( sComment == "" ? nullptr : sComment.c_str() ),
					&iStatus );
			}

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  updateKeyword                                                                                           |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Updates an existing FITS header keyword.  The keyword must be valid or an exception will be thrown.     |
		// |                                                                                                          |
		// |  'HIERARCH' keyword NOTE: This text will be prefixed to any keyword by the cfitsio library if the        |
		// |                           keyword is greater than 8 characters, which is the standard FITS keyword       |
		// |                           length. See the link below for details:                                        |
		// |   http://heasarc.gsfc.nasa.gov/docs/software/fitsio/c/f_user/node28.html                                 |
		// |                                                                                                          |
		// |   HIERARCH examples:                                                                                     |
		// |   -----------------                                                                                      |
		// |   HIERARCH LongKeyword = 47.5 / Keyword has > 8 characters & mixed case                                  |
		// |   HIERARCH XTE$TEMP = 98.6 / Keyword contains the '$' character                                          |
		// |   HIERARCH Earth is a star = F / Keyword contains embedded spaces                                        |
		// |                                                                                                          |
		// |  <IN> -> sKey     - The header keyword. Can be nullptr. Ex: SIMPLE                                       |
		// |  <IN> -> pKey     - The value associated with the key. Ex: T                                             |
		// |  <IN> -> iType    - The keyword type, as defined in CArcFitsFile.h                                       |
		// |  <IN> -> sComment - The comment to attach to the keyword.  Can be nullptr.                               |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		void CArcFitsFile<T>::updateKeyword( const std::string& sKey, void* pKey, arc::gen3::fits::e_Type eType, const std::string& sComment )
		{
			writeKeyword( sKey, pKey, eType, sComment );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getParameters                                                                                           |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns a pointer to an arc::gen3::fits::CParam class that contains all the image parameters, such as   |
		// |  number of cols, rows, frames, dimensions and bits-per-pixel. May return nullptr.                        |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<arc::gen3::fits::CParam> CArcFitsFile<T>::getParameters( void )
		{
			std::int32_t iStatus = 0;

			VERIFY_FILE_HANDLE()

			std::unique_ptr<arc::gen3::fits::CParam> pParam( new arc::gen3::fits::CParam() );

			if ( pParam == nullptr )
			{
				THROW( "Failed to create parameters structure!" );
			}

			//
			// Get the image parameters
			//
			fits_get_img_param( m_pFits, 3, &pParam->m_iBpp, &pParam->m_iNAxis, pParam->m_lNAxes, &iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}

			return pParam;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getNumberOfFrames                                                                                       |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the number of frames.  A single image file will return a value of 0.                            |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::uint32_t CArcFitsFile<T>::getNumberOfFrames( void )
		{
			return static_cast< std::uint32_t >( getParameters()->getFrames() );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getRows                                                                                                 |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the number of rows in the image.                                                                |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::uint32_t CArcFitsFile<T>::getRows( void )
		{
			return static_cast< std::uint32_t >( getParameters()->getRows() );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getCols																							      |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the number of columns in the image.                                                             |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::uint32_t CArcFitsFile<T>::getCols( void )
		{
			return static_cast< std::uint32_t >( getParameters()->getCols() );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getNAxis                                                                                                |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the number of dimensions in the image.                                                          |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::uint32_t CArcFitsFile<T>::getNAxis( void )
		{
			return static_cast< std::uint32_t >( getParameters()->getNAxis() );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  getBitsPerPixel                                                                                         |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns the image bits-per-pixel value.                                                                 |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::uint32_t CArcFitsFile<T>::getBitsPerPixel( void )
		{
			return static_cast< std::uint32_t >( getParameters()->getBpp() );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  generateTestData                                                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Generates a ramp test pattern image within the file. The size of the image is determined by the image   |
		// |  dimensions supplied during the create() method call.This method is only valid for single image files.   |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcFitsFile<T>::generateTestData( void )
		{
			std::size_t  iNElements = 0;
			std::int64_t i64FPixel = 1;
			std::int32_t iStatus = 0;

			VERIFY_FILE_HANDLE()

				auto pParam = getParameters();

			if ( pParam->getNAxis() > 2 )
			{
				THROW( "This method only supports single 2-D image files." );
			}

			//
			// Set number of pixels to write
			//
			iNElements = static_cast< std::int64_t >( pParam->getCols() * pParam->getRows() );

			std::unique_ptr<T[]> pBuf( new T[ iNElements ] );

			T uiValue = 0;

			for ( std::size_t i = 0; i < iNElements; i++ )
			{
				pBuf.get()[ i ] = uiValue;

				uiValue++;

				if ( uiValue >= maxTVal() )
				{
					uiValue = 0;
				}
			}

			fits_write_img( m_pFits,
				( sizeof( T ) == sizeof( std::uint16_t ) ? TUSHORT : TUINT ),
				i64FPixel,
				iNElements,
				pBuf.get(),
				&iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  reOpen                                                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Effectively closes and re-opens the underlying disk file.                                               |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcFitsFile<T>::reOpen( void )
		{
			std::int32_t	iStatus = 0;
			std::int32_t	iIOMode = 0;

			std::string sFilename = getFileName();

			fits_file_mode( m_pFits, &iIOMode, &iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}

			close();

			fits_open_file( &m_pFits, sFilename.c_str(), iIOMode, &iStatus );

			if ( iStatus )
			{
				close();

				throwFitsError( iStatus );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  flush                                                                                                   |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Causes all internal data buffers to write data to the disk file.                                        |
		// |                                                                                                          |
		// |  Throws std::runtime_error on error.                                                                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcFitsFile<T>::flush( void )
		{
			std::int32_t iStatus = 0;

			VERIFY_FILE_HANDLE()

				fits_flush_file( m_pFits, &iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  compare ( Single Images )                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Compares this file image data to another. This method does not check headers, except for size values,   |
		// | and does not throw any exceptions.See the error message parameter if a <i>false< / i> value is returned. |
		// |                                                                                                          |
		// |  <IN> - > cFitsFile - Reference to FITS file to compare.	                                              |
		// |  <IN> - > psErrMsg - The reason for failure, should it occur. ( default = nullptr ).                     |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> bool CArcFitsFile<T>::compare( CArcFitsFile<T>& cFitsFile, std::string* psErrMsg )
		{
			bool bMatch = true;

			try
			{
				auto pParam = cFitsFile.getParameters();

				auto pThisParams = getParameters();

				//
				//  Verify file dimensions
				//
				if ( pThisParams->getNAxis() != pParam->getNAxis() )
				{
					THROW( "Comparison file dimensions DO NOT match! This: %u Passed: %u.",
						pThisParams->getNAxis(),
						pParam->getNAxis() );
				}

				//
				//  Verify image dimensions
				//
				if ( ( pThisParams->getCols() != pParam->getCols() ) || ( pThisParams->getRows() != pParam->getRows() ) )
				{
					THROW( "Image dimensions of comparison files DO NOT match! This: %ux%u Passed: %ux%u.",
						pThisParams->getCols(),
						pThisParams->getRows(),
						pParam->getCols(),
						pParam->getRows() );
				}

				if ( pThisParams->getBpp() != pParam->getBpp() )
				{
					THROW( "Image bits-per-pixel of comparison files DO NOT match! This: %u Passed: %u.",
						pThisParams->getBpp(),
						pParam->getBpp() );
				}

				//
				//  Read input file image buffer
				//
				auto pBuf = cFitsFile.read();

				if ( pBuf.get() == nullptr )
				{
					THROW( "Failed to read passed FITS file image data!" );
				}

				//
				//  Read this file image buffer
				//
				auto pThisBuf = read();

				if ( pThisBuf.get() == nullptr )
				{
					THROW( "Failed to read this FITS file image data!" );
				}

				//
				//  compare image buffers
				//
				for ( std::uint32_t r = 0; r < pThisParams->getRows(); r++ )
				{
					for ( std::uint32_t c = 0; c < pThisParams->getCols(); c++ )
					{
						auto uIndex = c + r * pThisParams->getCols();

						if ( pThisBuf.get()[ uIndex ] != pBuf.get()[ uIndex ] )
						{
							THROW( "Images do not match at col: %u, row: %u, this: %u, passed: %u",
								c,
								r,
								pThisBuf.get()[ uIndex ],
								pBuf.get()[ uIndex ] );
						}
					}
				}
			}
			catch ( ... )
			{
				bMatch = false;
			}

			return bMatch;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  reSize ( Single Image )                                                                                 |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Resizes a single image file by modifying the NAXES keyword and increasing the image data portion of     |
		// |  the file.                                                                                               |
		// |                                                                                                          |
		// |  <IN> -> uiCols - The number of image columns the new file will have.                                    |
		// |  <IN> -> uiRows - The number of image rows the new file will have.                                       |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcFitsFile<T>::reSize( std::uint32_t uiCols, std::uint32_t uiRows )
		{
			std::int32_t  iStatus = 0;

			VERIFY_FILE_HANDLE()

				auto pParam = getParameters();

			//
			// Verify NAXIS parameter
			//
			if ( pParam->getNAxis() != 2 )
			{
				THROW_INVALID_ARGUMENT( "Invalid NAXIS value. This method is only valid for a file containing a single image." );
			}

			*pParam->m_plCols = uiCols;
			*pParam->m_plRows = uiRows;

			fits_resize_img( m_pFits,
				pParam->m_iBpp,
				pParam->m_iNAxis,
				pParam->m_lNAxes,
				&iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  write ( Single Image )                                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Writes image data to a single image file.                                                               |
		// |                                                                                                          |
		// |  <IN> -> pBuf - Pointer to the image data.                                                               |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcFitsFile<T>::write( T* pBuf )
		{
			std::int64_t i64NElements = 0;
			std::int64_t i64FPixel = 1;
			std::int32_t iStatus = 0;

			VERIFY_FILE_HANDLE()

				//
				// Verify buffer pointer
				//
				if ( pBuf == nullptr )
				{
					THROW_INVALID_ARGUMENT( "Invalid data buffer." );
				}

			//
			// Verify parameters
			//
			auto pParam = getParameters();

			if ( pParam->getNAxis() != 2 )
			{
				THROW_INVALID_ARGUMENT( "Invalid NAXIS value. This method is only valid for a file containing a single image." );
			}

			//
			// Set number of pixels to write
			//
			i64NElements = ( pParam->getCols() * pParam->getRows() );

			//
			// Write image data
			//
			fits_write_img( m_pFits,
				( sizeof( T ) == sizeof( std::uint16_t ) ? TUSHORT : TUINT ),
				i64FPixel,
				i64NElements,
				pBuf,
				&iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}

			//
			// Force data flush for more real-time performance
			//
			fits_flush_file( m_pFits, &iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  write ( Single Image )                                                                                  |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Writes the specified number of bytes to a single image file. The start position of the data within the  |
		// |  file image can be specified.                                                                            |
		// |                                                                                                          |
		// |  <IN> -> pBuf		- Pointer to the data to write.                                                       |
		// |  <IN> -> i64Bytes  - The number of bytes to write.                                                       |
		// |  <IN> -> i64Pixel  - The start pixel within the file image. This parameter is optional. If -1, then the  |
		// |                      next write position will be at zero.                                                |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> void CArcFitsFile<T>::write( T* pBuf, std::int64_t i64Bytes, std::int64_t i64Pixel )
		{
			std::int64_t i64NElements	= 0;
			std::int32_t iStatus 		= 0;

			bool bMultiWrite = false;

			VERIFY_FILE_HANDLE()

			auto pParam = getParameters();

			//
			// Verify parameters
			//
			if ( pParam->getNAxis() != 2 )
			{
				THROW_INVALID_ARGUMENT( "Invalid NAXIS value. This method is only valid for a file containing a single image." );
			}

			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid data buffer." );
			}

			if ( i64Pixel >= static_cast< std::int64_t >( pParam->getCols() ) * static_cast< std::int64_t >( pParam->getRows() ) )
			{
				THROW_INVALID_ARGUMENT( "Invalid start position, pixel position outside image size." );
			}

			//
			// Set the start pixel ( position ) within the file.
			//
			if ( i64Pixel < 0 && m_i64Pixel == 0 )
			{
				m_i64Pixel = 1;

				bMultiWrite = true;
			}

			else if ( i64Pixel == 0 && m_i64Pixel != 0 )
			{
				m_i64Pixel = 1;

				bMultiWrite = true;
			}

			else if ( i64Pixel < 0 && m_i64Pixel != 0 )
			{
				bMultiWrite = true;
			}

			else
			{
				m_i64Pixel = i64Pixel + 1;

				bMultiWrite = false;
			}

			//
			// Verify the start position
			//
			if ( m_i64Pixel >= static_cast< std::int64_t >( pParam->getCols() ) * static_cast< std::int64_t >( pParam->getRows() ) )
			{
				THROW( "Invalid start position, pixel position outside image size." );
			}

			//
			// Number of pixels to write
			//
			i64NElements = i64Bytes / sizeof( T );

			fits_write_img( m_pFits,
							( sizeof( T ) == sizeof( std::uint16_t ) ? TUSHORT : TUINT ),
							static_cast< LONGLONG >( m_i64Pixel ),
							static_cast< LONGLONG >( i64NElements ),
							pBuf,
							&iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}

			if ( bMultiWrite )
			{
				m_i64Pixel += i64NElements;
			}

			//
			// Force data flush for more real-time performance
			//
			fits_flush_file( m_pFits, &iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}

		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  writeSubImage ( Single Image )                                                                          |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Writes a sub-image of the specified buffer to a single image file.                                      |
		// |                                                                                                          |
		// |  <IN> -> pBuf	          - The pixel data.                                                               |
		// |  <IN> -> lowerLeftPoint  - The lower left point { col, row } of the sub-image.                           |
		// |  <IN> -> upperRightPoint - The upper right point { col, row } of the sub-image.                          |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T>
		void CArcFitsFile<T>::writeSubImage( T* pBuf, arc::gen3::fits::Point lowerLeftPoint, arc::gen3::fits::Point upperRightPoint )
		{
			std::int32_t iStatus = 0;

			VERIFY_FILE_HANDLE()

			auto pParam = getParameters();

			//
			// Verify parameters
			//
			if ( pParam->getNAxis() != 2 )
			{
				THROW_INVALID_ARGUMENT( "Invalid NAXIS value. This method is only valid for a file containing a single image." );
			}

			if ( lowerLeftPoint.second > upperRightPoint.second )
			{
				THROW_INVALID_ARGUMENT( "Invalid LOWER LEFT ROW parameter!" );
			}

			if ( lowerLeftPoint.first > upperRightPoint.first )
			{
				THROW_INVALID_ARGUMENT( "Invalid LOWER LEFT COLUMN parameter!" );
			}

			if ( lowerLeftPoint.second < 0 || lowerLeftPoint.second >= static_cast< long >( pParam->getRows() ) )
			{
				THROW_INVALID_ARGUMENT( "Invalid LOWER LEFT ROW parameter!" );
			}

			if ( upperRightPoint.second < 0 || upperRightPoint.second >= static_cast< long >( pParam->getRows() ) )
			{
				THROW_INVALID_ARGUMENT( "Invalid UPPER RIGHT ROW parameter!" );
			}

			if ( lowerLeftPoint.first < 0 || lowerLeftPoint.first >= static_cast< long >( pParam->getCols() ) )
			{
				THROW_INVALID_ARGUMENT( "Invalid LOWER LEFT COLUMN parameter!" );
			}

			if ( upperRightPoint.first < 0 || upperRightPoint.first >= static_cast< long >( pParam->getCols() ) )
			{
				THROW_INVALID_ARGUMENT( "Invalid UPPER RIGHT COLUMN parameter!" );
			}

			if ( pBuf == nullptr )
			{
				THROW_INVALID_ARGUMENT( "Invalid data buffer." );
			}

			//
			// Set the subset start pixels
			//
			long lFirstPixel[] = { lowerLeftPoint.first + 1, lowerLeftPoint.second + 1 };
			long lLastPixel[] = { upperRightPoint.first + 1, upperRightPoint.second + 1 };

			fits_write_subset( m_pFits,
							  ( sizeof( T ) == sizeof( std::uint16_t ) ? TUSHORT : TUINT ),
							  lFirstPixel,
							  lLastPixel,
							  pBuf,
							  &iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}

			//
			// Force data flush for more real-time performance
			//
			fits_flush_file( m_pFits, &iStatus );

			if ( iStatus )
			{
				throwFitsError( iStatus );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// |  readSubImage ( Single Image )                                                                           |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Reads a sub-image from a single image file.                                                             |
		// |                                                                                                          |
		// |  <IN> -> lowerLeftPoint  - The lower left point { col, row } of the sub-image.                           |
		// |  <IN> -> upperRightPoint - The upper right point { col, row } of the sub-image.                          |
		// |                                                                                                          |
		// |  Throws std::runtime_error, std::invalid_argument                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		template <typename T> std::unique_ptr<T[], arc::gen3::fits::ArrayDeleter<T>>
			CArcFitsFile<T>::readSubImage( arc::gen3::fits::Point lowerLeftPoint, arc::gen3::fits::Point upperRightPoint )
			{
				std::int32_t iStatus = 0;
				std::int32_t iAnyNul = 0;

				VERIFY_FILE_HANDLE()

				auto pParam = getParameters();

				//
				// Verify parameters
				//
				if ( pParam->getNAxis() != 2 )
				{
					THROW_INVALID_ARGUMENT( "Invalid NAXIS value. This method is only valid for a file containing a single image." );
				}

				if ( lowerLeftPoint.second > upperRightPoint.second )
				{
					THROW_INVALID_ARGUMENT( "Invalid LOWER LEFT ROW parameter!" );
				}

				if ( lowerLeftPoint.first > upperRightPoint.first )
				{
					THROW_INVALID_ARGUMENT( "Invalid LOWER LEFT COLUMN parameter!" );
				}

				if ( lowerLeftPoint.second < 0 || lowerLeftPoint.second >= static_cast< long >( pParam->getRows() ) )
				{
					THROW_INVALID_ARGUMENT( "Invalid LOWER LEFT ROW parameter!" );
				}

				if ( upperRightPoint.second < 0 || upperRightPoint.second >= static_cast< long >( pParam->getRows() ) )
				{
					THROW_INVALID_ARGUMENT( "Invalid UPPER RIGHT ROW parameter!" );
				}

				if ( lowerLeftPoint.first < 0 || lowerLeftPoint.first >= static_cast< long >( pParam->getCols() ) )
				{
					THROW_INVALID_ARGUMENT( "Invalid LOWER LEFT COLUMN parameter!" );
				}

				if ( upperRightPoint.first < 0 || upperRightPoint.first >= static_cast< long >( pParam->getCols() ) )
				{
					THROW_INVALID_ARGUMENT( "Invalid UPPER RIGHT COLUMN parameter!" );
				}

				if ( upperRightPoint.first == static_cast< long >( pParam->getCols() ) )
				{
					upperRightPoint.first = ( static_cast< long >( pParam->getCols() ) - 1 );
				}

				if ( upperRightPoint.second == static_cast< long >( pParam->getRows() ) )
				{
					upperRightPoint.second = ( static_cast< long >( pParam->getRows() ) - 1 );
				}

				//
				// Set the subset start pixels
				//
				long lFirstPixel[] = { lowerLeftPoint.first + 1, lowerLeftPoint.second + 1 };
				long lLastPixel[] = { upperRightPoint.first + 1, upperRightPoint.second + 1 };

				//
				// The read routine also has an inc parameter which can be used to
				// read only every inc-th pixel along each dimension of the image.
				// Normally inc[0] = inc[1] = 1 to read every pixel in a 2D image.
				// To read every other pixel in the entire 2D image, set 
				//
				long lInc[] = { 1, 1 };

				//
				// Set the data length ( in pixels )
				//
				std::uint32_t uiDataLength = ( pParam->getCols() * pParam->getRows() );

				std::unique_ptr<T[], arc::gen3::fits::ArrayDeleter<T>> pSubBuf( new T[ uiDataLength ], arc::gen3::fits::ArrayDeleter<T>() );

				if ( pSubBuf.get() == nullptr )
				{
					THROW( "Failed to allocate buffer for image pixel data." );
				}

				fits_read_subset( m_pFits,
								 ( sizeof( T ) == sizeof( std::uint16_t ) ? TUSHORT : TUINT ),
								 lFirstPixel,
								 lLastPixel,
								 lInc,
								 0,
								 pSubBuf.get(),
								 &iAnyNul,
								 &iStatus );

				if ( iStatus )
				{
					throwFitsError( iStatus );
				}

				return pSubBuf;
			}


			// +----------------------------------------------------------------------------------------------------------+
			// |  Read ( Single Image )                                                                                   |
			// +----------------------------------------------------------------------------------------------------------+
			// |  Read the image from a single image file. Returns a pointer to the image data.                           |
			// |                                                                                                          |
			// |  Throws std::runtime_error, std::invalid_argument                                                        |
			// +----------------------------------------------------------------------------------------------------------+
			template <typename T> std::unique_ptr<T[], arc::gen3::fits::ArrayDeleter<T>> CArcFitsFile<T>::read( void )
			{
				std::int32_t iStatus = 0;

				VERIFY_FILE_HANDLE()

				auto pParam = getParameters();

				//
				// Verify NAXIS parameter
				//
				if ( pParam->getNAxis() != 2 )
				{
					THROW_INVALID_ARGUMENT( "Invalid NAXIS value. This method is only valid for a file containing a single image." );
				}

				//
				// Set the data length ( in pixels )
				//
				std::uint32_t uiDataLength = ( pParam->getCols() * pParam->getRows() );

				std::unique_ptr<T[], arc::gen3::fits::ArrayDeleter<T>> pImgBuf( new T[ uiDataLength ], arc::gen3::fits::ArrayDeleter<T>() );

				if ( pImgBuf.get() == nullptr )
				{
					THROW( "Failed to allocate buffer for image pixel data." );
				}

				//
				// Read the image data
				//
				fits_read_img( m_pFits,
								( sizeof( T ) == sizeof( std::uint16_t ) ? TUSHORT : TUINT ),
								1,
								static_cast< long >( uiDataLength ),
								nullptr,
								pImgBuf.get(),
								nullptr,
								&iStatus );

				if ( iStatus )
				{
					throwFitsError( iStatus );
				}

				return pImgBuf;
			}


			// +----------------------------------------------------------------------------------------------------------+
			// |  Read ( Single Image )                                                                                   |
			// +----------------------------------------------------------------------------------------------------------+
			// |  Read the image from a single image file into the user supplied buffer.                                  |
			// |                                                                                                          |
			// |  <IN> -> pBuf   - A pointer to the user supplied image buffer.                                           |
			// |  <IN> -> uiCols - The column length of the user image buffer ( in pixels ).                              |
			// |  <IN> -> uiRows - The row length of the user image buffer ( in pixels ).                                 |
			// |                                                                                                          |
			// |  Throws std::runtime_error, std::invalid_argument, std::length_error                                     |
			// +----------------------------------------------------------------------------------------------------------+
			template <typename T> void CArcFitsFile<T>::read( T* pBuf, std::uint32_t uiCols, std::uint32_t uiRows )
			{
				std::int32_t iStatus = 0;

				VERIFY_FILE_HANDLE()

				auto pParam = getParameters();

				//
				// Verify NAXIS parameter
				//
				if ( pParam->getNAxis() != 2 )
				{
					THROW_INVALID_ARGUMENT( "Invalid NAXIS value. This method is only valid for a file containing a single image." );
				}

				//
				// Set the data length ( in pixels )
				//
				std::uint32_t uiDataLength = ( pParam->getCols() * pParam->getRows() );

				//
				// Verify the buffer dimensions
				//
				if ( uiDataLength > ( uiCols * uiRows ) )
				{
					THROW_LENGTH_ERROR( "Error, user supplied buffer is too small. Expected: %d bytes, Supplied: %d bytes.", uiDataLength, ( uiCols * uiRows ) );
				}

				if ( pBuf == nullptr )
				{
					THROW_INVALID_ARGUMENT( "Invalid image buffer parameter ( nullptr )." );
				}

				//
				// Read the image data
				//
				fits_read_img( m_pFits,
								( sizeof( T ) == sizeof( std::uint16_t ) ? TUSHORT : TUINT ),
								1,
								static_cast< long >( uiDataLength ),
								nullptr,
								pBuf,
								nullptr,
								&iStatus );

				if ( iStatus )
				{
					throwFitsError( iStatus );
				}
			}


			// +----------------------------------------------------------------------------------------------------------+
			// |  write3D ( Data Cube )                                                                                   |
			// +----------------------------------------------------------------------------------------------------------+
			// |  Writes an image to the end of a data cube file.                                                         |
			// |                                                                                                          |
			// |  <IN> -> pBuf - A pointer to the image data.                                                             |
			// |                                                                                                          |
			// |  Throws std::runtime_error, std::invalid_argument                                                        |
			// +----------------------------------------------------------------------------------------------------------+
			template <typename T> void CArcFitsFile<T>::write3D( T* pBuf )
			{
				std::int64_t i64NElements	= 0;
				std::int32_t iStatus 		= 0;

				VERIFY_FILE_HANDLE()

				auto pParam = getParameters();

				*pParam->m_plFrames = ++m_iFrame;

				//
				// Verify parameters
				//
				if ( pParam->getNAxis() != 3 )
				{
					THROW_INVALID_ARGUMENT( "Invalid NAXIS value. This method is only valid for a FITS data cube." );
				}

				if ( pBuf == nullptr )
				{
					THROW_INVALID_ARGUMENT( "Invalid data buffer ( write3D )." );
				}

				//
				// Set number of pixels to write
				//
				i64NElements = static_cast< std::int64_t >( pParam->getCols() * pParam->getRows() );

				if ( m_i64Pixel == 0 )
				{
					m_i64Pixel = 1;
				}

				fits_write_img( m_pFits,
								( sizeof( T ) == sizeof( std::uint16_t ) ? TUSHORT : TUINT ),
								m_i64Pixel,
								i64NElements,
								pBuf,
								&iStatus );

				if ( iStatus )
				{
					throwFitsError( iStatus );
				}

				//
				// Update the start pixel
				//
				m_i64Pixel += i64NElements;

				//
				// Increment the image number and update the key
				//
				fits_update_key( m_pFits,
								 TINT,
								 "NAXIS3",
								 &m_iFrame,
								 nullptr,
								 &iStatus );

				if ( iStatus )
				{
					throwFitsError( iStatus );
				}

				//
				// Force data flush for more real-time performance
				//
				fits_flush_file( m_pFits, &iStatus );

				if ( iStatus )
				{
					throwFitsError( iStatus );
				}
			}


			// +----------------------------------------------------------------------------------------------------------+
			// |  reWrite3D ( Data Cube )                                                                                 |
			// +----------------------------------------------------------------------------------------------------------+
			// |  Re-writes an existing image in a FITS data cube. The image data MUST match in size to the exising       |
			// |  images within the data cube. The image size is NOT checked for by this method.                          |
			// |                                                                                                          |
			// |  <IN> -> pBuf			- A pointer to the image data.                                                    |
			// |  <IN> -> uiImageNumber	- The number of the data cube image to replace.                                   |
			// |                                                                                                          |
			// |  Throws std::runtime_error, std::invalid_argument                                                        |
			// +----------------------------------------------------------------------------------------------------------+
			template <typename T> void CArcFitsFile<T>::reWrite3D( T* pBuf, std::uint32_t uiImageNumber )
			{
				std::int64_t i64NElements 	= 0;
				std::int64_t i64Pixel 		= 0;
				std::int32_t iStatus 		= 0;

				VERIFY_FILE_HANDLE()

					auto pParam = getParameters();

				//
				// Verify parameters
				//
				if ( pParam->getNAxis() != 3 )
				{
					THROW_INVALID_ARGUMENT( "Invalid NAXIS value. This method is only valid for a FITS data cube." );
				}

				if ( pBuf == nullptr )
				{
					THROW_INVALID_ARGUMENT( "Invalid data buffer." );
				}

				//
				// Set number of pixels to write; also set the start position
				//
				i64NElements = static_cast< std::int64_t >( pParam->getCols() ) * static_cast< std::int64_t >( pParam->getRows() );

				i64Pixel = i64NElements * uiImageNumber + 1;

				fits_write_img( m_pFits,
								( sizeof( T ) == sizeof( std::uint16_t ) ? TUSHORT : TUINT ),
								i64Pixel,
								i64NElements,
								pBuf,
								&iStatus );

				if ( iStatus )
				{
					throwFitsError( iStatus );
				}

				//
				// Force data flush for more real-time performance
				//
				fits_flush_file( m_pFits, &iStatus );

				if ( iStatus )
				{
					throwFitsError( iStatus );
				}
			}


			// +----------------------------------------------------------------------------------------------------------+
			// |  read3D ( Data Cube )                                                                                    |
			// +----------------------------------------------------------------------------------------------------------+
			// |  Reads an image from a data cube file.                                                                   |
			// |                                                                                                          |
			// |  <IN>  -> uiImageNumber - The image number to read.                                                      |
			// |                                                                                                          |
			// |  Throws std::runtime_error, std::invalid_argument                                                        |
			// +----------------------------------------------------------------------------------------------------------+
			template <typename T>
			std::unique_ptr<T[], arc::gen3::fits::ArrayDeleter<T>> CArcFitsFile<T>::read3D( std::uint32_t uiImageNumber )
			{
				std::size_t  iNElements = 0;
				std::int64_t i64Pixel 	= 0;
				std::int32_t iStatus 	= 0;

				VERIFY_FILE_HANDLE()

					auto pParam = getParameters();

				if ( pParam->getNAxis() != 3 )
				{
					THROW_INVALID_ARGUMENT( "Invalid NAXIS value. This method is only valid for a FITS data cube." );
				}

				//
				// Verify parameters
				//
				if ( ( uiImageNumber + 1 ) > pParam->getFrames() )
				{
					THROW_INVALID_ARGUMENT( "Invalid image number. File contains %u images.", pParam->getFrames() );
				}

				//
				// Set number of pixels to read
				//
				iNElements = static_cast< std::int64_t >( pParam->getCols() * pParam->getRows() );

				i64Pixel = iNElements * uiImageNumber + 1;

				std::unique_ptr<T[], arc::gen3::fits::ArrayDeleter<T>> pImgBuf( new T[ iNElements ], arc::gen3::fits::ArrayDeleter<T>() );

				if ( pImgBuf.get() == nullptr )
				{
					THROW( "Failed to allocate buffer for image pixel data." );
				}

				//
				// Read the image data
				//
				fits_read_img( m_pFits,
							  ( sizeof( T ) == sizeof( std::uint16_t ) ? TUSHORT : TUINT ),
							  i64Pixel,
							  iNElements,
							  nullptr,
							  pImgBuf.get(),
							  nullptr,
							  &iStatus );

				if ( iStatus )
				{
					throwFitsError( iStatus );
				}

				return pImgBuf;
			}


			// +----------------------------------------------------------------------------------------------------------+
			// |  getBaseFile                                                                                             |
			// +----------------------------------------------------------------------------------------------------------+
			// |  Returns the underlying cfitsio file pointer. May return nullptr.                                        |
			// +----------------------------------------------------------------------------------------------------------+
			template <typename T> fitsfile* CArcFitsFile<T>::getBaseFile( void )
			{
				return m_pFits;
			}


			// +----------------------------------------------------------------------------------------------------------+
			// |  maxTVal                                                                                                 |
			// +----------------------------------------------------------------------------------------------------------+
			// |  Determines the maximum value for a specific data type. Example, for std::uint16_t: 2^16 = 65536.        |
			// +----------------------------------------------------------------------------------------------------------+
			template <typename T> std::uint32_t CArcFitsFile<T>::maxTVal( void )
			{
				auto gExponent = ( ( sizeof( T ) == sizeof( arc::gen3::fits::BPP_32 ) ) ? 20 : ( sizeof( T ) * 8 ) );

				return static_cast< std::uint32_t >( std::pow( 2.0, gExponent ) );
			}


			// +----------------------------------------------------------------------------------------------------------+
			// |  ThrowException                                                                                          |
			// +----------------------------------------------------------------------------------------------------------+
			// |  Throws a std::runtime_error based on the supplied cfitsio status value.                                 |
			// |                                                                                                          |
			// |  <IN> -> iStatus : cfitsio error value.                                                                  |
			// +----------------------------------------------------------------------------------------------------------+
			template <typename T> void CArcFitsFile<T>::throwFitsError( std::int32_t iStatus )
			{
				char szFitsMsg[ 100 ];

				fits_get_errstatus( iStatus, szFitsMsg );

				THROW( "%s", szFitsMsg );
			}


	}	// end gen3 namespace
}		// end arc namespace



/** Explicit instantiations - These are the only allowed instantiations of this class */
template class arc::gen3::CArcFitsFile<arc::gen3::fits::BPP_16>;
template class arc::gen3::CArcFitsFile<arc::gen3::fits::BPP_32>;



// +------------------------------------------------------------------------------------------------+
// |  default_delete definition                                                                     |
// +------------------------------------------------------------------------------------------------+
// |  Creates a modified version of the std::default_delete class for use by all std::unique_ptr's  |
// |  that wrap a CStats/CDifStats object.                                                          |
// +------------------------------------------------------------------------------------------------+
namespace std
{

	void default_delete<arc::gen3::fits::CParam>::operator()( arc::gen3::fits::CParam* pObj )
	{
		if ( pObj != nullptr )
		{
			delete pObj;
		}
	}

}
