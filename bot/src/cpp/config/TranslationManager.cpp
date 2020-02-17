//
// $Id: TranslationManager.cpp,v 1.4 2004/05/27 06:10:33 clamatius Exp $

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

#include "config/TranslationManager.h"
#include "extern/getpot/GetPot.h"
#include "extern/metamod/meta_api.h"

Log TranslationManager::_log(__FILE__);
hash_map<string, string> TranslationManager::_translations;

const string UNKNOWN_VAL = "{unknown}";


const string translationConfigFilename("addons/whichbot/conf/wb_translate.pot");
const string translationConfigFilenamePrefix("addons/whichbot/conf/wb_tr_");

void TranslationManager::loadDefaultTranslations()
{
    loadTranslations(translationConfigFilename);
}


void TranslationManager::loadMapTranslations(const string& mapname)
{
    loadTranslations(translationConfigFilenamePrefix + mapname + ".pot");
}


void TranslationManager::loadTranslations(const string& filename)
{
    string gameDir(GET_GAME_INFO(PLID, GINFO_GAMEDIR));
    string fullPathname = gameDir + "/" + filename;

    GetPot* configFile = new GetPot(fullPathname.c_str());

    if (configFile != NULL) {
        vector<string> variables = configFile->get_variable_names();

        for (vector<string>::iterator ii = variables.begin(); ii != variables.end(); ii++) {
            int endDelimPos = ii->find_last_of("/");
            if (endDelimPos < 0) {
                endDelimPos = 0;
            }

            string sourceName(ii->substr(endDelimPos+1, ii->length() - endDelimPos + 1));
            _translations[sourceName] = (*configFile)(ii->c_str(), UNKNOWN_VAL.c_str());
        }

    } else {
        _log.Warn("Failed to open translation config file %s", fullPathname.c_str());
    }

    delete configFile;
}

string TranslationManager::getTranslation(const string& source)
{
    hash_map<string, string>::iterator found = _translations.find(source);

    if (found != _translations.end()) {
        return found->second;

    } else {
        return source;
    }
}
