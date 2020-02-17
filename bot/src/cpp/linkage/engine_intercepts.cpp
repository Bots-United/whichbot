//
// $Id: engine_intercepts.cpp,v 1.23 2005/11/30 23:07:02 clamatius Exp $

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


#include "extern/halflifesdk/extdll.h"
#include "linkage/engine_intercepts.h"
#include "extern/metamod/meta_api.h"
#include "extern/halflifesdk/eiface.h"
#include "extern/halflifesdk/usercmd.h"
#include "Bot.h"
#include "BotManager.h"
#include "strategy/HiveMind.h"
#include "linkage/version.h"
#include <strstream>

Bot* pCurrentThinkingBot = NULL;
Log _log(__FILE__);

void interceptMessageBegin(int msg_dest, int msg_type, const float* pOrigin, edict_t* pEdict)
{
    gpBotManager->getMessageDispatcher().handleMessageStart(msg_type, pEdict);
  	RETURN_META(MRES_HANDLED);
}

void interceptMessageEnd(void)
{
    gpBotManager->getMessageDispatcher().handleMessageEnd();
  	RETURN_META(MRES_HANDLED);
}

void interceptWriteByte(int iValue)
{
    byte val = (byte)iValue;
    gpBotManager->getMessageDispatcher().handleMessageSection(&val, kByte);
  	RETURN_META(MRES_HANDLED);
}

void interceptWriteChar(int iValue)
{
    char val = (char)iValue;
    gpBotManager->getMessageDispatcher().handleMessageSection(&val, kChar);
  	RETURN_META(MRES_HANDLED);
}

void interceptWriteShort(int iValue)
{
    short val = (short)iValue;

    gpBotManager->getMessageDispatcher().handleMessageSection(&val, kShort);
  	RETURN_META(MRES_HANDLED);
}

void interceptWriteLong(int iValue)
{
    long val = (long)iValue;

    gpBotManager->getMessageDispatcher().handleMessageSection(&val, kLong);
  	RETURN_META(MRES_HANDLED);
}

void interceptWriteAngle(float flValue)
{
    gpBotManager->getMessageDispatcher().handleMessageSection(&flValue, kFloat);
  	RETURN_META(MRES_HANDLED);
}

void interceptWriteCoord(float flValue)
{
    gpBotManager->getMessageDispatcher().handleMessageSection(&flValue, kFloat);
  	RETURN_META(MRES_HANDLED);
}

void interceptWriteString(const char* sz)
{
    gpBotManager->getMessageDispatcher().handleMessageSection((void*)sz, kCString);
  	RETURN_META(MRES_HANDLED);
}

void interceptWriteEntity(int iValue)
{
    gpBotManager->getMessageDispatcher().handleMessageSection(&iValue, kInt);
  	RETURN_META(MRES_HANDLED);
}

void interceptStartFrame()
{
    gpBotManager->tick();

  	RETURN_META(MRES_HANDLED);
}

int interceptClientConnect(edict_t* pClientEdict, const char* pszName, const char* pszAddress, char szRejectReason[128])
{
    CLIENT_PRINTF(pClientEdict, print_console, "\n");
    CLIENT_PRINTF(pClientEdict, print_console, "WhichBot MetaMod plugin (version ");
    CLIENT_PRINTF(pClientEdict, print_console, WHICHBOT_VERSION_STR);
    CLIENT_PRINTF(pClientEdict, print_console, ")\n");
    CLIENT_PRINTF(pClientEdict, print_console, "See whichbot.com for more details.\n");
    RETURN_META_VALUE(MRES_HANDLED, true);

    RETURN_META_VALUE(MRES_HANDLED, TRUE);
}

void interceptClientDisconnect(edict_t* pEdict)
{
    gpBotManager->clientDisconnected(pEdict);
    RETURN_META(MRES_HANDLED);
}

int interceptSpawn(edict_t* pEdict)
{
    bool handled = false;
    char* pClassname = (char*)STRING(pEdict->v.classname);

    if (strncmp(pClassname, "worldspawn", strlen("worldspawn")) == 0) {
        gpBotManager->newLevel();
        handled = true;
    }

    RETURN_META_VALUE(handled ? MRES_HANDLED : MRES_IGNORED, 0);
}


void interceptClientUserInfoChanged(edict_t* pEdict, char* infoBuffer)
{
    Bot* pBot = gpBotManager->getBot(pEdict);
    if (pBot != NULL) {
        pBot->userInfoChanged(infoBuffer);
    }
    RETURN_META(MRES_HANDLED);
}



extern char* g_argv;
vector<string> g_fakeArgs;

bool isSayCommand(const string& cmd)
{
    if (cmd == "say_team") {
        return true;

    } else if (cmd == "say") {
        return true;
    }
    return false;
}   

const char* interceptCmdArgs()
{
    if (g_fakeArgs.size() != 0) {
        memset(g_argv, 0, 1024);

        int idx = 0;

        vector<string>::iterator ii = g_fakeArgs.begin();
        // Giant hack to get round a bug in the HL engine, where "say hello" causes the message to be "say hello".
        if (isSayCommand(*ii)) {
            ii++;
        }

        for (; ii != g_fakeArgs.end(); ii++) {
            if (idx < 1023) {
                strcpy(&g_argv[idx], ii->c_str());
                idx += ii->length();
                strcpy(&g_argv[idx], " ");
                idx++;
            }
        }
        RETURN_META_VALUE(MRES_SUPERCEDE, g_argv);

    } else {
        RETURN_META_VALUE(MRES_IGNORED, NULL);
    }
}


const char* interceptCmdArgv(int idx)
{
    if (g_fakeArgs.size() > 0 && idx < (int)g_fakeArgs.size()) {
        memset(g_argv, 0, 1024);

        strcpy(g_argv, g_fakeArgs[idx].c_str());
        RETURN_META_VALUE(MRES_SUPERCEDE, g_argv);

    } else {
        RETURN_META_VALUE(MRES_IGNORED, NULL);
    }
}


int interceptCmdArgc()
{
    if (g_fakeArgs.size() > 0) {
        int argc = g_fakeArgs.size();

        RETURN_META_VALUE(MRES_SUPERCEDE, argc);

    } else {
        RETURN_META_VALUE(MRES_IGNORED, 0);
    }
}


void interceptRemoveEntity(edict_t* pEdict)
{
	if ((pEdict->v.team == MARINE_TEAM) || (pEdict->v.team == ALIEN_TEAM)) {
		HiveMind::entityDestroyed(pEdict);
		RETURN_META(MRES_HANDLED);

	} else {
		RETURN_META(MRES_IGNORED);
	}
}


void ClientPrint(edict_t *pEntity, int msg_dest, const char *msg_name)
{
   MESSAGE_BEGIN( MSG_ONE, REG_USER_MSG("TextMsg", -1), NULL, pEntity);
   WRITE_BYTE( msg_dest );
   WRITE_STRING( msg_name );
   MESSAGE_END();
}


const float LEAP_VELOCITY = 750.0;
const float LEAP_PITCH_ANGLE = -10.0;

void doLeap(edict_t* pEdict)
{
    vec3_t leapAngles = pEdict->v.angles;
    leapAngles.x += LEAP_PITCH_ANGLE;

    UTIL_MakeVectors(leapAngles);
    pEdict->v.velocity = gpGlobals->v_forward * LEAP_VELOCITY;
}

void doBlink(edict_t* pEdict)
{
	Bot* pBot = gpBotManager->getBot(pEdict);
	if (pBot != NULL) {
		pBot->getMovement()->doBlink();
	}
}

void interceptCommandStart(const edict_t* pPlayer, const struct usercmd_s* pCmd, unsigned int seed)
{
    if (pCmd != NULL) {
        edict_t* pEdict = (edict_t*)pPlayer;
        if (gpBotManager->getBot(pEdict) != NULL && !Bot::isEvolving(pEdict)) {
            if (pCmd->impulse == Config::getInstance().getImpulse(IMPULSE_LEAP)) {
                doLeap(pEdict);
            } else if (pCmd->impulse == Config::getInstance().getImpulse(IMPULSE_BLINK)) {
                doBlink(pEdict);
            }
        }
    }
    RETURN_META(MRES_IGNORED);
}


void interceptClientCommand(edict_t* pClientEdict)
{
    gpBotManager->getClientCommandDispatcher().interceptClientCommand(pClientEdict);

    RETURN_META(MRES_HANDLED);
}


string authReturnVal;
const char* interceptGetAuthId(edict_t* pEdict)
{
    Bot* pBot = gpBotManager->getBot(pEdict);
    if (pBot != NULL) {
        int botIdx = gpBotManager->getBotIndex(*pBot);
        strstream str;
        str << "00000000:";
        str.width(12);
        str.fill('0');
        str << (botIdx + 1);
        str << ":0";
        str >> authReturnVal;

        RETURN_META_VALUE(MRES_SUPERCEDE, authReturnVal.c_str());

    } else {
        RETURN_META_VALUE(MRES_IGNORED, NULL);
    }
}


void interceptSetClientMaxspeed(const edict_t* pEdict, float newMaxSpeed)
{
    _log.Debug("edict %x maxspeed %f", pEdict, newMaxSpeed);
    RETURN_META(MRES_IGNORED);
}

unsigned int interceptGetWonID(edict_t* pEdict)
{
    Bot* pBot = gpBotManager->getBot(pEdict);
    if (pBot != NULL) {
        int botIdx = gpBotManager->getBotIndex(*pBot);
        RETURN_META_VALUE(MRES_SUPERCEDE, botIdx);

    } else {
        RETURN_META_VALUE(MRES_IGNORED, 0);
    }
}
