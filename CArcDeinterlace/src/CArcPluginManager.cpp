// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  CArcPluginManager.cpp  ( Gen3 )                                                                          |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE: This file implements the ARC plugin manager interface.                                                 |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 27, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2013 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+

#if defined( linux ) || defined( __linux ) || defined( __APPLE__ )

	#include <dirent.h>
	#include <dlfcn.h>

#endif

#include <CArcPluginManager.h>



namespace arc
{
	namespace gen3
	{

		// +----------------------------------------------------------------------------------------------------------+
		// | OS dependent macros                                                                                      |
		// +----------------------------------------------------------------------------------------------------------+
		#ifdef _WINDOWS
			#define ArcLoadLibrary( path, lib )			LoadLibraryA( ( LPCSTR )lib.c_str() )
			#define ArcFindLibrarySymbol( lib, name )	GetProcAddress( lib, name )
			#define ArcFreeLibrary( handle )			FreeLibrary( handle )
			#define ArcSysErrorCode()					GetLastError()
		#else
			#define ArcLoadLibrary( path, lib )			dlopen( std::string( path + "/" + lib ).c_str(), RTLD_LAZY )
			#define ArcFindLibrarySymbol( lib, name )	dlsym( lib, name )
			#define ArcFreeLibrary( handle )			dlclose( handle )
			#define ArcSysErrorCode()					dlerror()
		#endif



		// +----------------------------------------------------------------------------------------------------------+
		// | Constructor                                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		CArcPluginManager::CArcPluginManager( void ) : CArcBase()
		{
		}


		// +----------------------------------------------------------------------------------------------------------+
		// | Destructor                                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		CArcPluginManager::~CArcPluginManager( void )
		{
			for ( Plugin_t* pPlugin : m_pluginMap )
			{
				if ( pPlugin->hlib != nullptr && pPlugin->dtor != nullptr && pPlugin->pObj != nullptr )
				{
					( *pPlugin->dtor ) ( pPlugin->pObj );

					ArcFreeLibrary( pPlugin->hlib );
				}
			}

			m_pluginMap.clear();
		}


		// +----------------------------------------------------------------------------------------------------------+
		// | pluginLoaded                                                                                             |
		// +----------------------------------------------------------------------------------------------------------+
		// | Returns whether or not one or more plugins are currently loaded.                                         |
		// +----------------------------------------------------------------------------------------------------------+
		bool CArcPluginManager::pluginLoaded( void )
		{
			return !m_pluginMap.empty();
		}


		// +----------------------------------------------------------------------------------------------------------+
		// | getPluginObject                                                                                          |
		// +----------------------------------------------------------------------------------------------------------+
		// |  Returns a loaded plugin object. May return nullptr.                                                     |
		// |                                                                                                          |
		// |  <IN> -> uiIndex - The index of the plugin object to return. For a single plugin use the default value   |
		// |                   ( default = 0 ).                                                                       |
		// |                                                                                                          |
		// | Throws a std::runtime_error                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		arc::gen3::IArcPlugin* CArcPluginManager::getPluginObject( std::uint32_t uiIndex )
		{
			if ( pluginLoaded() )
			{
				return m_pluginMap.at( uiIndex )->pObj;
			}

			return nullptr;
		}


		// +----------------------------------------------------------------------------------------------------------+
		// | findPluginLibrary                                                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		// | Searches the specified path for a plugin library. Returns 'true' if a library was found, 'false'         |
		// | otherwise.                                                                                               |
		// |                                                                                                          |
		// | <IN> -> sLibPath : The path to search for libraries.                                                     |
		// |                                                                                                          |
		// | Throws a std::runtime_error                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		bool CArcPluginManager::findPlugins( const std::string& sLibPath )
		{
			std::vector<std::string> vDirs;

			#ifdef _WINDOWS
				getDirList( sLibPath + "\\*.dll", vDirs );
			#elif defined( __APPLE__ )
				// Place holder
			#else
				getDirList( sLibPath + "/*.so", vDirs );
			#endif

			if ( vDirs.size() > 0 )
			{
				for ( std::vector<std::string>::size_type i = 0; i < vDirs.size(); i++ )
				{
					loadCustomLibrary( sLibPath, vDirs.at( i ) );
				}
			}

			return pluginLoaded();
		}


		// +----------------------------------------------------------------------------------------------------------+
		// | pluginCount                                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		// | Returns the number of plugins currently under management.                                                |
		// |                                                                                                          |
		// | Throws a std::runtime_error                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		std::uint32_t CArcPluginManager::pluginCount( void )
		{
			return static_cast<std::uint32_t>( m_pluginMap.size() );
		}


		// +----------------------------------------------------------------------------------------------------------+
		// | getDirList                                                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		// | Returns the list of files within the specified directory path. Note: the "." and ".." path listings      |
		// | are excluded from the list.                                                                              |
		// |                                                                                                          |
		// | <IN>  -> sPath - The directory path to list the file for.                                                |
		// | <OUT> -> vDirs - The vector list of all files found within the specified directory path.                 |
		// |                                                                                                          |
		// | Throws a std::runtime_error                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		void CArcPluginManager::getDirList( const std::string& sPath, std::vector<std::string>& vDirs )
		{
			#ifdef _WINDOWS

				WIN32_FIND_DATAA tFindFileData;

				HANDLE hFind = FindFirstFileA( sPath.c_str(), &tFindFileData );

				if ( hFind == INVALID_HANDLE_VALUE )
				{
					THROW( "Invalid File Handle. GetLastError reports: %e", GetLastError() );
				}

				else
				{
					std::string sFile( tFindFileData.cFileName );

					if ( sFile.compare( "." ) != 0 && sFile.compare( ".." ) != 0 )
					{
						vDirs.push_back( sFile );
					}

					//  List all the other files in the directory
					// +-------------------------------------------------+
					while ( FindNextFileA( hFind, &tFindFileData ) != 0 )
					{
						std::string sFile( tFindFileData.cFileName );

						if ( sFile.compare( "." ) != 0 && sFile.compare( ".." ) != 0 )
						{
							vDirs.push_back( sFile );
						}
					}

					FindClose( hFind );
				}

			#else

				struct dirent* pDirEntry = nullptr;

				DIR	*pDir = opendir( sPath.c_str() );

				if ( pDir == nullptr )
				{
					THROW( "Failed to open dir: %s", sPath.c_str() );
				}

				else
				{
					while ( ( pDirEntry = readdir( pDir ) ) != nullptr )
					{
						std::string sDirEntry( pDirEntry->d_name );

						if ( sDirEntry.compare( "." ) != 0 && sDirEntry.compare( ".." ) != 0 )
						{
							vDirs.push_back( sDirEntry );
						}
					}

					closedir( pDir );
				}

			#endif
		}


		// +----------------------------------------------------------------------------------------------------------+
		// | loadCustomLibrary                                                                                        |
		// +----------------------------------------------------------------------------------------------------------+
		// | Loads the dynamic library ( .dll, .so ) using the specified path and file name.                          |
		// |                                                                                                          |
		// | <IN> -> sLibPath - The library path.                                                                     |
		// | <IN> -> sLibName - The library file name.                                                                |
		// |                                                                                                          |
		// | Throws a std::runtime_error                                                                              |
		// +----------------------------------------------------------------------------------------------------------+
		void CArcPluginManager::loadCustomLibrary( const std::string sLibPath, const std::string sLibName )
		{
			#ifdef _WINDOWS
				if ( !SetDllDirectoryA( sLibPath.c_str() ) )
				{
					THROW( "Failed to set library path [ %s ]!", sLibPath.c_str() );
				}
			#endif

			//  Get a handle to the DLL module
			// +--------------------------------------------------------+
			ArcPluginLib hPluginLib = ArcLoadLibrary( sLibPath, sLibName );

			if ( hPluginLib == nullptr )
			{
				THROW( "Failed to find load library [ PATH: %s, LIB: %s ]! GetLastError: %e",
						sLibPath.c_str(),
						sLibName.c_str(),
						ArcSysErrorCode() );
			}

			Plugin_t* tPlugin = createInstance( hPluginLib );

			if ( tPlugin == nullptr )
			{
				ArcFreeLibrary( hPluginLib );
			}

			bool bExists = false;

			for ( auto &rPlugin : m_pluginMap )
			{
				if ( rPlugin->hlib == hPluginLib )
				{
					bExists = true;

					break;
				}
			}

			if ( !bExists )
			{
				m_pluginMap.push_back( tPlugin );
			}
		}


		// +----------------------------------------------------------------------------------------------------------+
		// | isCustomLibrary                                                                                          |
		// +----------------------------------------------------------------------------------------------------------+
		// | Verifies that the specified library handle points to a custom deinterlace library. Returns 'true' if     |
		// | the "IS_CUSTOM_ALGORITHM" system returns 1; 'false' otherwise ( i.e. the symbol is not found ).          |
		// |                                                                                                          |
		// | <IN> -> hPlugin - A custom library handle.                                                               |
		// +----------------------------------------------------------------------------------------------------------+
		Plugin_t* CArcPluginManager::createInstance( ArcPluginLib hPluginLib )
		{
			Plugin_t* pPlugin = nullptr;

			//  If the handle is valid, try to get the function address
			// +--------------------------------------------------------+
			if ( hPluginLib != nullptr )
			{
				PluginCreate  pCtor = ( PluginCreate )ArcFindLibrarySymbol( hPluginLib, "createPlugin" );
				PluginRelease pDtor = ( PluginRelease )ArcFindLibrarySymbol( hPluginLib, "releasePlugin" );

				if ( pCtor != nullptr && pDtor != nullptr )
				{
					pPlugin = new Plugin_t;

					if ( pPlugin != nullptr )
					{
						pPlugin->hlib = hPluginLib;
						pPlugin->ctor = pCtor;
						pPlugin->dtor = pDtor;
						pPlugin->pObj = ( *pCtor ) ( );
					}
				}
			}

			return pPlugin;
		}

	}	// end gen3 namespace
}		// end arc namespace
