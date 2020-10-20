// +------------------------------------------------------------------------------------------------------------------+
// |  FILE:  IArcPlugin.h  ( GenIV )                                                                                  |
// +------------------------------------------------------------------------------------------------------------------+
// |  PURPOSE: This file defines the standard ARC plugin interface.                                                   |
// |                                                                                                                  |
// |  AUTHOR:  Scott Streit			DATE: November 04, 2014                                                           |
// |                                                                                                                  |
// |  Copyright 2013 Astronomical Research Cameras, Inc. All rights reserved.                                         |
// +------------------------------------------------------------------------------------------------------------------+

#ifndef _GEN3_IPLUGIN_H_
#define _GEN3_IPLUGIN_H_

#include <memory>
#include <cstdint>
#include <string>

#include <CArcDeinterlaceDllMain.h>
#include <CArcStringList.h>
#include <CArcBase.h>



//
// This interface is exported from the CArcDeinterlace4.0.dll
//
// NOTE: This class is actually a deinterlace plugin. It could be made generic by
//       changing run() to run() and removing getNameList() and getCount().
//
namespace arc
{
	namespace gen3
	{

		/** @interface IArcPlugin
		 *  Deinterlace plugin interface. Abstract class.
		 *  @see arc::gen3::CArcBase()
		 */
		class GEN3_CARCDEINTERLACE_API IArcPlugin : arc::gen3::CArcBase
		{

			public:

				/** Destructor
				 */
				virtual ~IArcPlugin( void );

				/** Executes the specified deinterlace algorithm on the specified image buffer.
				 *  @param pBuf		- The buffer to deinterlace.
				 *  @param uiCols	- The number of columns in the image.
				 *  @param uiRows	- The number of rows in the image.
				 *  @param uiBpp		- The bits-per-pixel for the buffer data.
				 *  @param sAlg		- The deinterlace algorithm name. One of the strings returned from the getNameList() method.
				 *  @param uiArg		- Optional algorithm argument ( default = 0 ).
				 *  @throws std::runtime_error on error
				 */
				virtual void run( void* pBuf, std::uint32_t uiCols, std::uint32_t uiRows, const std::uint32_t uiBpp, const std::string& sAlg, std::uint32_t uiArg = 0 ) = 0;

				/** Returns the list of algorithms supported by the plugin.
				 *  @return A string list of algorithm names.
				 *  @see arc::gen3::CArcStringList in CArcBase4.0.dll
				 *  @throws std::runtime_error on error
				 */
				virtual arc::gen3::CArcStringList* getNameList( void );

				/** Returns the number of algorithms supported by the plugin.
				 *  @return The number of algorithms supported by the plugin.
				 *  @throws std::runtime_error on error
				 */
				virtual std::uint32_t getCount( void );

			protected:

				/** Constructor */
				IArcPlugin( void );

				/** Algorithm name list */
				std::unique_ptr<arc::gen3::CArcStringList> m_pList;
		};

	}		// end gen3 namespace
}			// end arc namespace


#endif	// _GEN3_IPLUGIN_H_
