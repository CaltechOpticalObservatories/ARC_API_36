// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  CArcPluginManager.h  ( Gen3 )                                                                            |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE: This file defines the standard ARC plugin manager interface.                                           |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: March 27, 2020                                                              |
// |                                                                                                                  |
// |  Copyright 2013 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+

#ifndef _GEN3_PLUGIN_MANAGER_H_
#define _GEN3_PLUGIN_MANAGER_H_

#ifdef _WINDOWS
	#pragma warning( disable: 4251 )
	#include <windows.h>
#endif

#include <vector>

#include <CArcDeinterlaceDllMain.h>
#include <IArcPlugin.h>
#include <CArcBase.h>



namespace arc
{
	namespace gen3
	{

		// +----------------------------------------------------------------------------------------------------------+
		// | Plugin factory create/release function definitions                                                       |
		// +----------------------------------------------------------------------------------------------------------+
		typedef arc::gen3::IArcPlugin* ( *PluginCreate )( );
		typedef void( *PluginRelease )( IArcPlugin* );


		/**
		 * Define OS dependent custom library handle
		 */
		#ifdef _WINDOWS
			#define ArcPluginLib	HMODULE
		#else
			#define ArcPluginLib	void*
		#endif


		/**
		 * Plugin type definition
		 */
		struct Plugin_t
		{
			ArcPluginLib	hlib;
			PluginCreate	ctor;
			PluginRelease	dtor;
			IArcPlugin*		pObj;
		};


		/** @class CArcPluginManager
		*  ARC pluginManger class.
		*  @see arc::gen3::CArcBase()
		*/
		class GEN3_CARCDEINTERLACE_API CArcPluginManager : public arc::gen3::CArcBase
		{
			public:

				/** Constructor
				 */
				CArcPluginManager( void );

				/** Destructor
				 */
				~CArcPluginManager( void );

				/** Returns whether or not one or more plugins are currently loaded.
				 *  @return <i>true</i> if at least one plugin is loaded; <i>false</i> otherwise.
				 */
				bool pluginLoaded( void );

				/** Returns a loaded plugin object.
				 *  @param uiIndex - The index of the plugin object to return. For a single plugin use the default value ( default = 0 ).
				 *  @return The plugin object at the specified index. May return nullptr.
				 */
				arc::gen3::IArcPlugin* getPluginObject( std::uint32_t uiIndex = 0 );

				/** Searches the specified directory for libraries that match the plugin interface.
				 *  @param sLibPath - The directory to search for libraries in.
				 *  @return <i>true</i> if at least one plugin was found and loaded; <i>false</i> otherwise.
				 *  @throws std::exception on error.
				 */
				bool findPlugins( const std::string& sLibPath );

				/** Returns the number of loaded plugins.
				 *  @return The loaded plugin count.
				 *  @throws std::exception on error.
				 */
				std::uint32_t pluginCount( void );

			private:

				/** Returns the list of files within the specified directory path. Note: the "." and ".." path listings are excluded from the list.
				 *  @param sPath - The directory path to list the file for.
				 *  @param vDirs - The vector list of all files found within the specified directory path.
				 *  @throws std::exception on error.
				 */
				void getDirList( const std::string& sPath, std::vector<std::string>& vDirs );

				/** Loads the dynamic library ( .dll, .so ) using the specified path and file name.
				 *  @param sLibPath - The library path.
				 *  @param sLibName - The library file name.
				 *  @return A handle to the library or nullptr on error.
				 *  @throws std::exception on error.
				 */
				void loadCustomLibrary( const std::string sLibPath, const std::string sLibName );

				/** Verifies that the specified library handle points to a custom deinterlace library.
				 *  @param hCustomLib - A custom library handle.
				 *  @return <i>true</i> if the "IS_CUSTOM_ALGORITHM" system returns 1; <i>false</i> otherwise ( i.e. the symbol is not found ).
				 */
				Plugin_t* createInstance( ArcPluginLib hPluginLib );

				/** The plugin list */
				std::vector<Plugin_t*> m_pluginMap;
		};

	}
}


#endif	// _GEN3_PLUGIN_MANAGER_H_
