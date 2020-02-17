//
// $Id: linkfunc.h,v 1.3 2003/09/21 22:57:54 clamatius Exp $

// Copyright (c) 2003, WhichBot Project
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the WhichBot Project nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef __LINKAGE_LINKFUNC_H
#define __LINKAGE_LINKFUNC_H

#ifdef _WIN32
extern HINSTANCE h_Library;
#else
extern void* h_Library;
#endif

#ifndef __linux__

typedef int (FAR* GETENTITYAPI)(DLL_FUNCTIONS* , int);
typedef int (FAR* GETNEWDLLFUNCTIONS)(NEW_DLL_FUNCTIONS* , int* );
typedef void (DLLEXPORT* GIVEFNPTRSTODLL)(enginefuncs_t* , globalvars_t* );
typedef void (FAR* LINK_ENTITY_FUNC)(entvars_t* );

#else

#include <dlfcn.h>
#define GetProcAddress dlsym

typedef int BOOL;

typedef int (*GETENTITYAPI)(DLL_FUNCTIONS* , int);
typedef int (*GETNEWDLLFUNCTIONS)(NEW_DLL_FUNCTIONS* , int* );
typedef void (*GIVEFNPTRSTODLL)(enginefuncs_t* , globalvars_t* );
typedef void (*LINK_ENTITY_FUNC)(entvars_t* );

#endif

#define LINK_ENTITY_TO_FUNC(mapClassName) \
    extern "C" EXPORT void mapClassName( entvars_t* pev ); \
    void mapClassName( entvars_t* pev ) { \
    static LINK_ENTITY_FUNC otherClassName = NULL; \
    static int skip_this = 0; \
    if (skip_this) return; \
    if (otherClassName == NULL) \
    otherClassName = (LINK_ENTITY_FUNC)GetProcAddress(h_Library, #mapClassName); \
    if (otherClassName == NULL) { \
    skip_this = 1; return; \
    } \
(*otherClassName)(pev); }

#endif // __LINKAGE_LINKFUNC_H
