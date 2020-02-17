//
// $Id: NavigationEngine.cpp,v 1.19 2004/08/11 22:09:53 clamatius Exp $

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

#include "NavigationEngine.h"
#include "BotManager.h"

const float MOVEMENT_DIFFERENCE = 100.0;
// let's kill ourselves after 90 seconds if we didn't move
const float MOVEMENT_TIMEOUT = 90.0;

Log NavigationEngine::_log(__FILE__);

NavigationEngine::NavigationEngine(Bot& bot) :
	_method(NULL),
	_nextMethod(NULL),
    _bot(bot),
    _lastPos(),
    _lastMovedTime(gpGlobals->time),
    _paused(false)
{
}


NavigationEngine::~NavigationEngine()
{
	setMethod(NULL);
}


void NavigationEngine::setMethod(NavigationMethod* method)
{
	delete _method;
	_method = method;
}


void NavigationEngine::setNextMethod(NavigationMethod* method)
{
	delete _nextMethod;
	_nextMethod = method;
}

void NavigationEngine::navigate()
{
	if (_nextMethod != NULL) {
		setMethod(_nextMethod);
		_nextMethod = NULL;
	}

	bool wasPaused = _paused;

    if (_method != NULL) {
    	NavigationMethod* newMethod = _method->navigate();
	    if (newMethod != NULL) {
    		setMethod(newMethod);
    	}
    }

	// If we just switched from paused to not paused, count that as movement for suiciding purposes
	if (wasPaused && !_paused) {
		_lastMovedTime = gpGlobals->time;
	}

    checkForMovement();
}


void NavigationEngine::pause()
{
    if (_method != NULL) {
    	_method->pause();
        _paused = true;
		_lastMovedTime = gpGlobals->time;
    }
}


void NavigationEngine::resume()
{
    if (_method != NULL) {
        _paused = false;
    	_method->resume();
		_lastMovedTime = gpGlobals->time;
    }
}


void NavigationEngine::checkForMovement()
{
    if (_bot.getEdict() != NULL && _bot.getEdict()->v.team != NO_TEAM) {
        Vector positionDifference(_bot.getEdict()->v.origin - _lastPos);

        if (positionDifference.Length() > MOVEMENT_DIFFERENCE) {
            _lastPos = _bot.getEdict()->v.origin;
            _lastMovedTime = gpGlobals->time;
            
        } else {
			if (_lastMovedTime < _bot.getProperty(Bot::kLastSpawnTime)) {
				_lastMovedTime = _bot.getProperty(Bot::kLastSpawnTime);
			}
				
            float timeout = _paused ? 5 * MOVEMENT_TIMEOUT : MOVEMENT_TIMEOUT;
            if (gpGlobals->time - _lastMovedTime > timeout) {
                // time to take this one off the ventilator, no sign of life here
                _log.Debug("Didn't move for timeout period, suiciding");
                _bot.kill();
				_lastMovedTime = gpGlobals->time;
            }
        }
    }
}


string NavigationEngine::getCurrentMethodName()
{
	return _method != NULL ? _method->getName() : "NoMovement";
}
