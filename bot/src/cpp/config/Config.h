//
// $Id: Config.h,v 1.3 2005/11/30 23:07:02 clamatius Exp $

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

#ifndef __CONFIG_CONFIG_H
#define __CONFIG_CONFIG_H

#include "BotTypedefs.h"

#include "config/EntityInfo.h"
#include "extern/getpot/GetPot.h"

class Config  
{
public:

	Config();

	bool isValid();

	EntityInfo* getEntityInfo(const string& entityName);

	void setValue(const string& entityName, const EntityInfo& val)
	{
		_influenceVals[entityName] = val;
	}

    static Config& getInstance();

	int getTweak(const string& tweakName, int defaultVal);

	float getTweak(const string& tweakName, float defaultVal);

	float getArrayTweak(const string& tweakName, int pos, float defaultVal);

	const char* getArrayTweak(const string& tweakName, int pos, const char* defaultVal);

	int getArraySize(const string& tweakName);

	double getTweak(const string& tweakName, double defaultVal);

	string getTweak(const string& tweakName, const string& defaultVal);

	tBotRole getBotRole(int botIdx);

    int getImpulse(const string& impulseName);

protected:

	GetPot* Config::parseConfigFile (string configFileName);

	string getEntityName (const string& sectionName);

    typedef hash_map<string, EntityInfo> InfluenceValMap;

	InfluenceValMap _influenceVals;
	string _configFileName;
	GetPot* _configFile;
};

#endif // __CONFIG_CONFIG_H

