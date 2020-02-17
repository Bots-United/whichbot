//
// $Id: ClientCommandDispatcher.cpp,v 1.9 2004/09/11 05:33:42 clamatius Exp $

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
#include "message/ClientCommandDispatcher.h"
#include "BotManager.h"
#include "extern/metamod/meta_api.h"
#include "config/Config.h"


void ClientCommandDispatcher::dispatchCommand(edict_t* pEdict, const char* args[], int numArgs)
{
	if (numArgs > 1) {
		static int prefixLen = strlen(gpBotManager->getCommandPrefix());
		if (strncmp(gpBotManager->getCommandPrefix(), args[0], prefixLen) == 0) {

			string cmd(args[1]);
				
			tHandlerMap::iterator found = _handlers.find(cmd);
			if (found != _handlers.end()) {
				vector<string> params;
				params.push_back(cmd);
				for (int ii = 2; ii < numArgs; ii++) {
					params.push_back(string(args[ii]));
				}
				
				found->second(pEdict, params);
			}
		}
	}
}


string getNextToken(string& str, const string& delim)
{
    while (str.size() > 0 && str.find(delim) == 0) {
        str.erase(0, 1);
    }
    int delimIdx = str.find(delim);
    if (delimIdx < 0) {
        return str;
    }
    return str.substr(0, delimIdx);
}


void ClientCommandDispatcher::interceptClientCommand(edict_t* pClientEdict)
{
    int argc = CMD_ARGC();

    if (argc > 1) {
        string clientCmd = CMD_ARGV(0);
        if (clientCmd == "say" || 
			(clientCmd == "say_team" && pClientEdict != NULL && pClientEdict->v.team == ALIEN_TEAM))
		{
			string tweakNamePrefix = string("admin/") + clientCmd + string("_command_");
            string cmdParams = CMD_ARGV(1);
            vector<string> params;

            // this is the way the WON version of HL works
            if (argc == 2) {
                if (cmdParams.find("/wb ") == 0) {
                    cmdParams.erase(0, strlen("/wb "));
                    
                    string wbCmd = getNextToken(cmdParams, " ");
                    if (Config::getInstance().getTweak(tweakNamePrefix + wbCmd, 0) != 0) {
                        params.push_back(wbCmd);
                        cmdParams.erase(0, wbCmd.size());
                        
                        tHandlerMap::iterator found = _handlers.find(wbCmd);
                        if (found != _handlers.end()) {
                            while (cmdParams.size() > 0) {
                                string param = getNextToken(cmdParams, " ");
                                cmdParams.erase(0, param.size());
                                params.push_back(param);
                            }
                            
                            found->second(pClientEdict, params);
                        }
                    }
                }

            // now cope with the steam version
            } else if (argc > 2) {
                if (cmdParams.find("/wb") == 0) {
                    string wbCmd = CMD_ARGV(2);
                    if (Config::getInstance().getTweak(tweakNamePrefix + wbCmd, 0) != 0) {
                        params.push_back(wbCmd);
                        tHandlerMap::iterator found = _handlers.find(wbCmd);
                        if (found != _handlers.end()) {
                            for (int ii = 3; ii < CMD_ARGC(); ii++) {
                                params.push_back(CMD_ARGV(ii));
                            }

                            found->second(pClientEdict, params);
                        }
                    }
                }
            }
        }
    }
}


void handleClientCommands()
{
	static int prefixLen = strlen(gpBotManager->getCommandPrefix());

	int argc  = CMD_ARGC();
	if (argc > 1) {
		const char** params = new const char*[argc];
		for (int ii = 0; ii < argc; ii++) {
			params[ii] = CMD_ARGV(ii);
		}
		
		gpBotManager->getClientCommandDispatcher().dispatchCommand(NULL, params, argc);
		
		// note, we're just deleting the holding array, not the actual params (which belong to the engine DLL)
		delete [] params;
		RETURN_META(MRES_HANDLED);
	}

	RETURN_META(MRES_IGNORED);
}


void ClientCommandDispatcher::registerHandler(const string& cmd, ClientCommandHandler handler)
{
	_handlers[cmd] = handler;
	(*g_engfuncs.pfnAddServerCommand)((char*)gpBotManager->getCommandPrefix(), handleClientCommands);
}




























































