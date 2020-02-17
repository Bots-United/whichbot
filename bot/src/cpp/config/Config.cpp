//
// $Id: Config.cpp,v 1.9 2005/12/14 19:20:35 clamatius Exp $

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

#include "BotTypedefs.h"
#include "config/Config.h"
#include "framework/Log.h"
#include <memory>
#include <errno.h>
#include "extern/halflifesdk/extdll.h"
#include "extern/metamod/meta_api.h"
#include "BotManager.h"

Log configMsgLog(__FILE__);


static auto_ptr<Config> _singleton(NULL);


Config::Config()
{
    // Get the game directory from Metamod
    string gameDir(GET_GAME_INFO(PLID, GINFO_GAMEDIR));
    // and append our relative path to the config file
    _configFileName = gameDir + "/addons/whichbot/conf/whichbot.txt";

	_configFile = parseConfigFile(_configFileName);
}

bool Config::isValid()
{
	return _configFile != NULL;
}

EntityInfo* Config::getEntityInfo(const string& entityName)
{
    InfluenceValMap::iterator found = _influenceVals.find(entityName);
    if (found != _influenceVals.end()) {
        return &found->second;
        
    } else {
        return NULL;
    }
}


float Config::getTweak(const string& tweakName, float defaultVal)
{
	return _configFile ? (float)(*_configFile)(tweakName.c_str(), defaultVal) : defaultVal;
}


float Config::getArrayTweak(const string& tweakName, int pos, float defaultVal)
{
	return _configFile ? (float)(*_configFile)(tweakName.c_str(), defaultVal, pos) : defaultVal;
}


const char* Config::getArrayTweak(const string& tweakName, int pos, const char* defaultVal)
{
	return _configFile ? (*_configFile)(tweakName.c_str(), defaultVal, pos) : defaultVal;
}


double Config::getTweak(const string& tweakName, double defaultVal)
{
	return _configFile ? (*_configFile)(tweakName.c_str(), defaultVal) : defaultVal;
}


int Config::getTweak(const string& tweakName, int defaultVal)
{
	return _configFile ? (*_configFile)(tweakName.c_str(), defaultVal) : defaultVal;
}


int Config::getArraySize(const string& tweakName)
{
	return _configFile ? _configFile->vector_variable_size(tweakName.c_str()) : 0;
}


string Config::getTweak(const string& tweakName, const string& defaultVal)
{
	return _configFile ? (*_configFile)(tweakName.c_str(), defaultVal.c_str()) : defaultVal;
}


Config& Config::getInstance()
{
	if (_singleton.get() == NULL) {
	    auto_ptr<Config> newInstance(new Config());
	    _singleton = newInstance;
	}
	return *_singleton;
}


string Config::getEntityName (const string& sectionName)
{
	int startDelimPos;
	if ((startDelimPos = sectionName.find_first_of("/", 0)) < 0) {
		return string("");
	}
	startDelimPos++;

	int endDelimPos;
	if ((endDelimPos = sectionName.find_last_of("/")) < 0) {
		return string("");
	}

	return sectionName.substr(startDelimPos, endDelimPos-startDelimPos);
}

static const string entitySection = "entities";
GetPot* Config::parseConfigFile (string configFileName)
{	
    errno = 0;
	GetPot* configFile = new GetPot(configFileName.c_str());
	if (configFile == NULL) {
        LOG_CONSOLE(PLID, "Failed to load config file %s: %s (error code %d)", 
                    configFileName.c_str(), strerror(errno), errno);
        LOG_ERROR(PLID, "Failed to load config file %s: %s (error code %d)", 
                  configFileName.c_str(), strerror(errno), errno);

		configMsgLog.Debug("Failed to instantiate GetPot for file %s", configFileName.c_str());

	} else {
        LOG_CONSOLE(PLID, "Loading WhichBot config file %s", 
                    configFileName.c_str());
		configMsgLog.Debug("Parsing configuration file %s", configFileName.c_str());
		vector<string> sections = configFile->get_section_names();
        int numSectionsFound = 0;
		for (vector<string>::iterator ii = sections.begin(); ii != sections.end(); ++ii)
		{
            numSectionsFound++;
			if ((*ii).compare(0, entitySection.length(), entitySection) == 0) {
				string entityName = getEntityName(*ii);
				if (entityName.length() > 0) {
					// Setup the variable names that we're interested in.
					string typeVarName = *ii + "type";
					string teamVarName = *ii + "team";
					string classNameVarName = *ii + "class_name";
					string defaultInfluenceVarName = *ii + "default_influence";
					string isBuildingVarName = *ii + "is_building";
					string attackValueVarName = *ii + "attack_value";

					// Get the values from the file.
					eEntityType entityType = (eEntityType)(*configFile)(typeVarName.c_str(), kInvalidEntity);
					int team = (*configFile)(teamVarName.c_str(), NO_TEAM);
					const char* className = (*configFile)(classNameVarName.c_str(), "");
					int defaultInfluence = (*configFile)(defaultInfluenceVarName.c_str(), 0);
					bool isBuilding = ((*configFile)(isBuildingVarName.c_str(), 1) != 0);
					int attackValue = (*configFile)(attackValueVarName.c_str(), 0);
					configMsgLog.Debug("Read entity=%s, type=%d, team=%d, className=%s, defaultInfluence=%d, "
						"isBuilding=%d, attackValue=%d", className, entityType, team, className, defaultInfluence,
						isBuilding, attackValue);
					setValue(className, EntityInfo(entityType, team, className, defaultInfluence, isBuilding, attackValue));
				}
			}
        }

        if (numSectionsFound == 0) {
            LOG_CONSOLE(PLID, "Failed to parse config file %s", configFileName.c_str());
            LOG_ERROR(PLID, "Failed to parse config file %s", configFileName.c_str());
        }
	}
	return configFile;
}


tBotRole Config::getBotRole(int botIdx)
{
	string configKeyName = gpBotManager->inCombatMode() ? "strategy/roles/combat_bot_roles" : "strategy/roles/classic_bot_roles";
	tBotRole role = kScoutRole;
	int numConfiguredRoles = getArraySize(configKeyName);
	if (botIdx >= 0 && botIdx < numConfiguredRoles) {
		string botRoleName = getArrayTweak(configKeyName, botIdx, "-");
		if (botRoleName.compare("G") == 0) {
			role = kGorgeRole;

		} else if (botRoleName.compare("S") == 0) {
			role = kScoutRole;

		} else {
			int packNumber = atoi(botRoleName.c_str());
			if (packNumber > 0 && packNumber <= MAX_PACKS) {
				role = (tBotRole)(kPack1Role + packNumber - 1);
			}
		}
	}
	return role;
}


int Config::getImpulse(const string& impulseName)
{
    const string prefix("game/impulses/");
    int impulse = getTweak(prefix + impulseName, -1);
    if (impulse < 0) {
        LOG_CONSOLE(PLID, "Impulse config missing for %s", impulseName.c_str());
        LOG_ERROR(PLID, "Impulse config missing for %s", impulseName.c_str());
    }
    return impulse;
}
