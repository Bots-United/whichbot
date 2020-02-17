//
// $Id: AreaManager.cpp,v 1.1 2003/09/18 01:47:28 clamatius Exp $

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

#include "AreaManager.h"

vector<AreaInfo*> AreaManager::_areas;

void AreaManager::addArea(AreaInfo& info)
{
	AreaInfo* newInfo = new AreaInfo(info);
	_areas.push_back(newInfo);
}

bool vecBetween(const Vector& point, const Vector& bottomLeft, const Vector& topRight)
{
	return
		((point.x >= bottomLeft.x && point.x <= topRight.x) || (point.x >= topRight.x && point.x <= bottomLeft.x)) &&
		((point.y >= bottomLeft.y && point.y <= topRight.x) || (point.y >= topRight.y && point.y <= bottomLeft.y)) &&
        ((point.z >= bottomLeft.z && point.z <= topRight.z) || (point.z >= topRight.z && point.z <= bottomLeft.z));
}

string unknownAreaName("[unknown]");

const string& AreaManager::getAreaName(const Vector& location)
{
	// ahh, the efficient joy of a linear scan.  TODO - some better way to do this
	for (vector<AreaInfo*>::iterator ii = _areas.begin(); ii != _areas.end(); ii++) {
		if (vecBetween(location, (*ii)->getBottomLeft(), (*ii)->getTopRight())) {
			return (*ii)->getName();
		}
	}
	return unknownAreaName;
}

bool AreaManager::exists(const string& areaName)
{
	// ahh, the efficient joy of a linear scan.  TODO - some better way to do this
	for (vector<AreaInfo*>::iterator ii = _areas.begin(); ii != _areas.end(); ii++) {
		if ((*ii)->getName() == areaName) {
			return true;
		}
	}
	return false;
}

void AreaManager::reset ()
{
    while (_areas.size() > 0) {
        delete _areas.back();
        _areas.pop_back();
    }
}
