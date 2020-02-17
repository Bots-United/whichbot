// vi: set ts=4 sw=4 :
// vim: set tw=75 :

// From SDK dlls/h_export.cpp:

/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== h_export.cpp ========================================================

  Entity classes exported by Halflife.

*/

#include <extern/halflifesdk/extdll.h>
#include "extern/metamod/h_export.h"
#include "extern/metamod/meta_api.h"

#ifdef _WIN32
extern HINSTANCE h_Library;
#else
extern void* h_Library;
#endif

// From SDK dlls/h_export.cpp:

//! Holds engine functionality callbacks
enginefuncs_t g_engfuncs;
globalvars_t  *gpGlobals;
char* g_argv;

#ifdef _WIN32
extern HINSTANCE h_Library;
extern HGLOBAL h_global_argv;
#else
extern void* h_Library;
extern char h_global_argv[1024];
#endif

// Receive engine function table from engine.
// This appears to be the _first_ DLL routine called by the engine, so we
// do some setup operations here.
void WINAPI GiveFnptrsToDll( enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals )
{
    memset(&g_engfuncs, 0, sizeof(g_engfuncs));
    // Added a giant hack to omit copying the last APIs, which won't be there on all half-life versions
	memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t) - 10 * 4);
	gpGlobals = pGlobals;

 
    #ifndef __linux__
        h_global_argv = GlobalAlloc(GMEM_SHARE, 1024);
        g_argv = (char* )GlobalLock(h_global_argv);
    #else
        g_argv = (char* )h_global_argv;
    #endif

        memset(g_argv, 0, 1024);
}
