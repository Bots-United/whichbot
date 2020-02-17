//
// $Id: NetMessageDispatcher.cpp,v 1.1 2003/09/18 01:47:27 clamatius Exp $

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


#include "message/NetMessageDispatcher.h"
#include "extern/metamod/meta_api.h"

// Static member variable declarations
bool NetMessageDispatcher::_ignoring = true;
map<int, NetMessageHandler> NetMessageDispatcher::_handlers;
NetMessage NetMessageDispatcher::_currentMessage(0, NULL);
NetMessageHandler NetMessageDispatcher::_thisHandler = NULL;
map<int, bool> NetMessageDispatcher::_ignoreList;
bool NetMessageDispatcher::_debug = false;


void NetMessageDispatcher::registerHandler(NetMessageHandler handler, const char* msgName)
{
	int size;
	int msgNum = GET_USER_MSG_ID(PLID, msgName, &size);
	if (msgNum > 0) {
		_handlers[msgNum] = handler;
	}
}


void NetMessageDispatcher::handleMessageStart(int msgTypeNum, edict_t* pEdict)
{
	_thisHandler = NULL;

	_ignoring = _ignoreList[msgTypeNum];
	if (_ignoring) {
		_currentMessage.reset(0, NULL);
	} else {
		HandlerMap::iterator found = _handlers.find(msgTypeNum);
		if (found != _handlers.end() || _debug) {
			_currentMessage.reset(msgTypeNum, pEdict);
		}
		if (found != _handlers.end()) {
			_thisHandler = found->second;
		}
	}
}


void NetMessageDispatcher::handleMessageSection(void* rock, eNetMessageType type)
{
    if ((_thisHandler != NULL || _debug) && (!_ignoring)) {
        _currentMessage.addElement(rock, type);
    }
}


void NetMessageDispatcher::handleMessageEnd()
{
    if (_thisHandler != NULL) {
        _thisHandler(_currentMessage);
    }

	if (_debug && !_ignoring && _currentMessage.getMsgType() != 23 && _currentMessage.getMsgType() != 102) {
		_currentMessage.log();
	}
}


void NetMessageDispatcher::toggleDebug()
{
    _debug = !_debug;
}


void NetMessageDispatcher::disable(int msgType)
{
	_ignoreList[msgType] = true;
}


void NetMessageDispatcher::enable(int msgType)
{
	_ignoreList[msgType] = false;
}

// NetMessageDispatcher::registerHandlers is defined in NetMessageHandlers.cpp
