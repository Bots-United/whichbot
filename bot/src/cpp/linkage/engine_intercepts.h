//
// $Id: engine_intercepts.h,v 1.9 2004/03/30 02:54:26 clamatius Exp $

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


#ifndef __LINKAGE_ENGINE_INTERCEPTS_H
#define __LINKAGE_ENGINE_INTERCEPTS_H

#include "extern/halflifesdk/extdll.h"

unsigned int interceptGetWonID(edict_t* pEdict);

const char* interceptGetAuthId(edict_t* pEdict);

void interceptMessageBegin(int msg_dest, int msg_type, const float* pOrigin, edict_t* ed);

void interceptMessageEnd(void);

void interceptWriteByte(int iValue);

void interceptWriteChar(int iValue);

void interceptWriteShort(int iValue);

void interceptWriteLong(int iValue);

void interceptWriteAngle(float flValue);

void interceptWriteCoord(float flValue);

void interceptWriteString(const char* sz);

void interceptWriteEntity(int iValue);

void interceptStartFrame();

void interceptClientDisconnect(edict_t* pEdict);

int interceptSpawn(edict_t* pEdict);

void interceptCommandStart(const edict_t* pEdict, const usercmd_s* pCmd, unsigned int seed);

void interceptSetOrigin(edict_t* pEdict, const float *rgflOrigin);

const char* interceptCmdArgs();

const char* interceptCmdArgv(int idx);

int interceptCmdArgc();

void interceptClientUserInfoChanged(edict_t* pEdict, char* infobuffer);

void interceptRemoveEntity(edict_t* pEdict);

void interceptClientCommand(edict_t* pClientEdict);

int interceptClientConnect(edict_t* pClientEdict, const char* pszName, const char* pszAddress, char szRejectReason[128]);

void interceptSetClientMaxspeed(const edict_t* pEdict, float newMaxSpeed);

#endif

