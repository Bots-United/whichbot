//
// $Id: NetMessageElement.cpp,v 1.5 2004/05/26 22:41:52 clamatius Exp $

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

#include "NetMessageElement.h"
#include <iomanip>
#include <sstream>

Log NetMessageElement::_log(__FILE__);

NetMessageElement::NetMessageElement(void* newRock, eNetMessageType newType) :
type(newType)
{
	if (newRock == NULL) {
		_log.Warn("Warning: null network message intercepted, game DLL may be broken [msg type=%d]", newType);
	}

	int strLength = 0;
    switch (newType) {
    case kByte:
        rock = (byte*)new byte;
        *((byte*)rock) = *(byte*)newRock;
        break;
    case kChar:
        rock = (char*)new char;
        *((char*)rock) = *(char*)newRock;
        break;
    case kShort:
        rock = (short*)new short;
        *((short*)rock) = *(short*)newRock;
        break;
    case kInt:
        rock = (int*)new int;
        *((int*)rock) = *(int*)newRock;
        break;
    case kLong:
        rock = (long*)new long;
        *((long*)rock) = *(long*)newRock;
        break;
    case kFloat:
        rock = (float*)new float;
        *((float*)rock) = *(float*)newRock;
        break;
    case kCString:
		// add 1 to the strlen so we copy the null byte too
		if (newRock != NULL) {
			strLength = min(255, strlen((char*)newRock) + 1);
			rock = (char*)new char[strLength];
			strncpy((char*)rock, (const char*)newRock, strLength);

		} else {
			// This shouldn't be happening
			rock = (char*)new char[1];
			((char*)rock)[0] = '\0';
		}
        break;
    default:
        assert(0);
    }
}

NetMessageElement::~NetMessageElement()
{
    switch (type) {
    case kByte:
        delete (byte*)rock;
        break;
    case kChar:
        delete (char*)rock;
        break;
    case kShort:
        delete (short*)rock;
        break;
    case kInt:
        delete (int*)rock;
        break;
    case kLong:
        delete (long*)rock;
        break;
    case kFloat:
        delete (float*)rock;
        break;
    case kCString:
        delete [] (char*)rock;
        break;
    default:
        assert(0);
    }
}

string NetMessageElement::toString () const
{
    ostringstream oss;
    switch (type) {
    case kByte:
        // VC++ ostringstream doesn't like byte/unsigned char
        oss << "kByte=0x" << hex << internal << setfill('0') << setw(2) << (short)*(byte*)rock;
        break;
    case kChar:
        oss << "kChar=" << *((char*)rock);
        break;
    case kShort:
        oss << "kShort=" << (short)*((short*)rock);
        break;
    case kInt:
        oss << "kInt=" << (int)*((int*)rock);
        break;
    case kLong:
        oss << "kLong=" << (long)*((long*)rock);
        break;
    case kFloat:
        oss << "kFloat=" << fixed << *((float*)rock);
        break;
    case kCString:
        oss << "kCString=" << "{" << (const char*)rock << "}";
        break;
    default:
        assert(0);
        oss << "unknowntype " << type;
        
    }
    
    return oss.str();
}
