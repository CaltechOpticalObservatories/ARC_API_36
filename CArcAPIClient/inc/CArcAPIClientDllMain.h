#ifndef _ARC_CARCAPICLIENT_DLLMAIN_H_
#define _ARC_CARCAPICLIENT_DLLMAIN_H_


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DLLTEST_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DLLTEST_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WIN32
	#ifdef CARCAPICLIENT_EXPORTS
		#define CARCAPICLIENT_API __declspec( dllexport )
	#else
		#define CARCAPICLIENT_API __declspec( dllimport )
	#endif
#else
	#define CARCAPICLIENT_API
#endif


#endif	// _ARC_CARCAPICLIENT_DLLMAIN_H_
