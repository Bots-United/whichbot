//
// $Id: NetMessage.h,v 1.3 2005/12/14 22:28:30 clamatius Exp $

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

#ifndef __MESSAGE_NETMESSAGE_H
#define  __MESSAGE_NETMESSAGE_H

#include "BotTypedefs.h"
#include "NetMessageElement.h"
#include "Bot.h"
#include "framework/Log.h"

class NetMessage  
{
public:

    NetMessage(int msgType, edict_t* pForEdict);

    ~NetMessage();

    void addElement(void* pRock, eNetMessageType type);

	inline int getMsgType() const { return _msgType; }
	
    int size() const;

	const bool isEmpty() const;

    const NetMessageElement* getElementAt(int ii) const;

    byte getByteAt(int ii) const;

    int getIntAt(int ii) const;

    long getLongAt(int ii) const;

    short getShortAt(int ii) const;

    Vector getVectorAt(int ii) const;

	float getFloatAt(int ii) const;

    const char* getCStringAt(int ii) const;

    eNetMessageType getTypeAt(int ii) const;

    void reset(int msgType, edict_t* pEdict);

	string toString () const;

    edict_t* getEdict() const;

    void log();

protected:

	edict_t* _pEdict;
    int _msgType;
    vector<NetMessageElement*> _elements;

    static Log _log;
};

#endif //  __MESSAGE_NETMESSAGE_H
