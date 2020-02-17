//
// $Id: NavigationMethod.h,v 1.3 2004/05/27 01:38:41 clamatius Exp $

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

#if !defined(AFX_NAVIGATIONMETHOD_H__E1FCC4B3_10D5_474B_82F6_D1304D9F282E__INCLUDED_)
#define AFX_NAVIGATIONMETHOD_H__E1FCC4B3_10D5_474B_82F6_D1304D9F282E__INCLUDED_

#include "BotTypedefs.h"

class NavigationMethod  
{
public:
	NavigationMethod();

	virtual ~NavigationMethod();

	virtual NavigationMethod* navigate() = 0;

	// Pause and resume navigation.
	virtual void pause() = 0;

	virtual void resume() = 0;

	virtual string getName() const = 0;

protected:

};

#endif // !defined(AFX_NAVIGATIONMETHOD_H__E1FCC4B3_10D5_474B_82F6_D1304D9F282E__INCLUDED_)
