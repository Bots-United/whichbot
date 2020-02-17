//
// $Id: HiveManager.h,v 1.3 2004/02/08 20:38:26 clamatius Exp $

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


#ifndef __WORLDSTATE_HIVEMANAGER_H
#define __WORLDSTATE_HIVEMANAGER_H

#include "BotTypedefs.h"
#include "HiveInfo.h"

class HiveManager  
{
public:

	static void addHive (HiveInfo& info);

	/**
	 * getHive() - Fetch the hive info for the specified eHiveId
	 * @param hiveId The hive we're interested in
	 * @return The request HiveInfo instance.  NULL if no info is available.
	 **/
	static HiveInfo* getHive (const int hiveId);

	static int getActiveHiveCount ();

    static inline int getHiveCount () { return _hives.size(); }

	static void reset ();

    static void resetTraits();

    static void addTraitLevel(int traitId);

    static int getTraitLevel(int traitId);

    static inline bool isTraitAvailable(int traitId) { return getTraitLevel(traitId) > 0; }

    static inline bool isTraitMaxed(int traitId) { return getTraitLevel(traitId) == 3; }

	static float distanceToNearestHive(Vector& fromPoint);

    static HiveInfo* getNearestHive(Vector& fromPoint);

    static HiveInfo* getNearestActiveHive(Vector& fromPoint);

protected:

	static vector<HiveInfo*> _hives;

    static map<int, int> _traits;
};

#endif
