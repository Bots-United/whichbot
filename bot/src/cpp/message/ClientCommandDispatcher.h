//
// $Id: ClientCommandDispatcher.h,v 1.2 2003/10/18 02:03:45 clamatius Exp $

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

#ifndef __MESSAGE_CLIENTCOMMANDDISPATCHER_H
#define __MESSAGE_CLIENTCOMMANDDISPATCHER_H

#include "BotTypedefs.h"
#include "extern/halflifesdk/extdll.h"

typedef void (*ClientCommandHandler) (edict_t* pEdict, vector<string>& args);

class ClientCommandDispatcher
{
public:

    void interceptClientCommand(edict_t* pClientEdict);

    void dispatchCommand(edict_t* pEdict, const char* args[], int numArgs);

	void registerHandler(const string& cmd, ClientCommandHandler handler);

	void registerHandlers();

protected:

	class StringComparator
	{
	public:
		bool operator () (const string& lhs, const string& rhs) const
		{
			// for whatever fricken reason (lhs < rhs) doesn't work in MS's STL implementation. sigh.
			return strcmp(lhs.c_str(), rhs.c_str()) < 0;
		}
	};

	typedef map<string, ClientCommandHandler, StringComparator> tHandlerMap;

	tHandlerMap _handlers;
};

#endif
