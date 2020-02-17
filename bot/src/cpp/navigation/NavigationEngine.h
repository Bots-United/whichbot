//
// $Id: NavigationEngine.h,v 1.9 2004/05/27 01:38:41 clamatius Exp $

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

#ifndef __NAVIGATION_NAVIGATIONENGINE_H
#define __NAVIGATION_NAVIGATIONENGINE_H

#include "NavigationMethod.h"
#include "BotTypedefs.h"
#include "extern/halflifesdk/extdll.h"
#include "framework/Log.h"
class Bot;
 
class NavigationEngine  
{
public:

	NavigationEngine(Bot& bot);

	virtual ~NavigationEngine();

	void navigate();
	void pause();
	void resume();

	void setMethod(NavigationMethod* method);

	void setNextMethod(NavigationMethod* method);

	string getCurrentMethodName();

protected:

    void checkForMovement();

	NavigationMethod* _method;
	NavigationMethod* _nextMethod;
    Bot& _bot;

    Vector _lastPos;
    float _lastMovedTime;
	bool _paused;

    static Log _log;
};

#endif // __NAVIGATION_NAVIGATIONENGINE_H
