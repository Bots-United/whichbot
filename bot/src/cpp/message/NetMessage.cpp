//
// $Id: NetMessage.cpp,v 1.3 2005/12/14 22:28:30 clamatius Exp $

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

#include "NetMessage.h"
#include <sstream>

Log NetMessage::_log(__FILE__);


NetMessage::NetMessage(int msgType, edict_t* pEdict) :
    _pEdict(pEdict),
    _msgType(msgType)
{
}


NetMessage::~NetMessage()
{
    reset(0, NULL);
}


void NetMessage::reset(int msgType, edict_t* pEdict)
{
    _msgType = msgType;
    _pEdict = pEdict;
    
    for (vector<NetMessageElement*>::iterator ii = _elements.begin(); ii != _elements.end(); ii++) {
        delete *ii;
    }
    _elements.clear();
}


void NetMessage::addElement(void* rock, eNetMessageType type)
{
    NetMessageElement* element = new NetMessageElement(rock, type);
    _elements.push_back(element);
}


int NetMessage::size() const
{
    return _elements.size();
}


const bool NetMessage::isEmpty() const
{
	return ((_msgType == 0) && (size() == 0));
}


const NetMessageElement* NetMessage::getElementAt(int ii) const
{
    if (ii < 0 || ii > size()) {
        return NULL;
        
    } else {
        return _elements[ii];
    }
}


byte NetMessage::getByteAt(int ii) const
{
    assert(ii >= 0 && ii < size());
    assert(_elements[ii]->type == kByte);
    
    return *(byte*)_elements[ii]->rock;
}


int NetMessage::getIntAt(int ii) const
{
    assert(ii >= 0 && ii < size());
    assert(_elements[ii]->type == kInt);
    
    return *(int*)_elements[ii]->rock;
}


long NetMessage::getLongAt(int ii) const
{
    assert(ii >= 0 && ii < size());
    assert(_elements[ii]->type == kLong);
    
    return *(long*)_elements[ii]->rock;
}


short NetMessage::getShortAt(int ii) const
{
    assert(ii >= 0 && ii < size());
    assert(_elements[ii]->type == kShort);
    
    return *(short*)_elements[ii]->rock;
}


float NetMessage::getFloatAt(int ii) const
{
    assert(ii >= 0 && ii < size());
    assert(_elements[ii]->type == kFloat);
    
    return *(short*)_elements[ii]->rock;
}


Vector NetMessage::getVectorAt(int ii) const
{
    assert(ii >= 0 && ii+2 < size());
    assert(_elements[ii]->type == kFloat);
    assert(_elements[ii+1]->type == kFloat);
    assert(_elements[ii+2]->type == kFloat);
    
    float xPos = *(float*)_elements[ii]->rock;
    float yPos = *(float*)_elements[ii+1]->rock;
    float zPos = *(float*)_elements[ii+2]->rock;
    return Vector(xPos, yPos, zPos);
}


const char* NetMessage::getCStringAt(int ii) const
{
    assert(ii >= 0 && ii < size());
    assert(_elements[ii]->type = kCString);
    
    return (const char*)_elements[ii]->rock;
}


string NetMessage::toString() const
{
    ostringstream oss;
    for (int ii = 0; ii < size(); ++ii) {
        oss << "[" << ii << "]" << getElementAt(ii)->toString().c_str();
    }
    return oss.str();
}


eNetMessageType NetMessage::getTypeAt(int ii) const
{
    if (ii >= 0 && ii < size()) {
        return _elements[ii]->type;
    }
    return kUnknownType;
}


edict_t* NetMessage::getEdict() const
{
    return _pEdict;
}


void NetMessage::log()
{
	const char* className = _pEdict ? STRING(_pEdict->v.classname) : "No Entity";
	_log.Debug("Starting message type %d for edict 0x%x (%s)", _msgType, _pEdict, className);
	for (vector<NetMessageElement*>::iterator ii = _elements.begin(); ii != _elements.end(); ii++) {
		_log.FileLog((*ii)->toString().c_str());
	}
	_log.Debug("Ending message type %d for edict 0x%x (%s)", _msgType, _pEdict, className);
}

