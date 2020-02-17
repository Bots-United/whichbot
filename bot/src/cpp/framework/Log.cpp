//
// $Id: Log.cpp,v 1.8 2004/08/11 22:09:52 clamatius Exp $

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


#include "framework/Log.h"
#include "extern/halflifesdk/extdll.h"
#include "Bot.h"
#include "extern/halflifesdk/enginecallback.h"
#include "extern/metamod/meta_api.h"

enum LOG_MODE
{
    CONSOLE_LOG,
    FILE_LOG,
    ALL_LOGS
};

const int BUF_LEN = 512;

const LOG_MODE mode = CONSOLE_LOG;

const char* LOGFILE = "whbotlog.txt";

extern Bot* gpCurrentThinkingBot;


Log::Log(const char* name)
{
    _name = name;
}


void Log::ConsoleLog(const char* buf)
{
    if (gpCurrentThinkingBot != NULL && gpCurrentThinkingBot->getName() != NULL) {
		if (IS_DEDICATED_SERVER()) {
	        LOG_MESSAGE(PLID, "%s: %s: %s", _name, gpCurrentThinkingBot->getName()->c_str(), buf);
		} else {
			LOG_CONSOLE(PLID, "%s: %s: %s", _name, gpCurrentThinkingBot->getName()->c_str(), buf);
		}
        
    } else {
		if (IS_DEDICATED_SERVER()) {
	        LOG_MESSAGE(PLID, "%s: %s", _name, buf);
		} else {
	        LOG_CONSOLE(PLID, "%s: %s", _name, buf);
		}
    }
}


void Log::FileLog(const char* buf)
{
    FILE* fp = fopen(LOGFILE,"a");
    if (gpCurrentThinkingBot != NULL && gpCurrentThinkingBot->getName() != NULL) {
        fprintf(fp, "%s: %s: %s\n", _name, gpCurrentThinkingBot->getName(), buf);
        
    } else {
        fprintf(fp, "%s: %s\n", _name, buf);
    }
    fclose(fp);
}


void Log::Debug(const char* msg, ...)
{
    // Only debug if debug is enabled
#ifdef _DEBUG
    char buf[BUF_LEN];

    // write the message to BUF
    va_list args;
    va_start(args, msg);
    _vsnprintf(buf, BUF_LEN, msg, args);
    va_end(args);

    switch (mode) {
    case CONSOLE_LOG:
        ConsoleLog(buf);
        break;

    // default to file log
    case FILE_LOG:
        FileLog(buf);
        break;

    case ALL_LOGS:
        ConsoleLog(buf);
        FileLog(buf);
        break;


    default:
        break;
    }

#endif
}


void Log::Warn(const char* msg, ...)
{
    // Only debug if debug is enabled
    char buf[BUF_LEN];

    // write the message to BUF
    va_list args;
    va_start(args, msg);
    _vsnprintf(buf, BUF_LEN, msg, args);
    va_end(args);

    switch (mode) {
    case CONSOLE_LOG:
        LOG_ERROR(PLID, buf);
        break;

    // default to file log
    case FILE_LOG:
        FileLog(buf);
        break;

    case ALL_LOGS:
        ConsoleLog(buf);
        FileLog(buf);
        break;


    default:
        break;
    }
}
