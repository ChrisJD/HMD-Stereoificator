/********************************************************************
HMD-Stereoificator
Copyright (C) 2013 Chris Drain

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED


#include "src/Logger.h"


#define SAFE_RELEASE(x) if(x) { x->Release(); x = NULL; } 

// 6 is all logging enabled, 0 will strip all logging mesages from build
#ifdef _DEBUG
#define LOGGING_LEVEL 6
#else
#define LOGGING_LEVEL 4
#endif


// Use these macros rather than using the .debug(), etc methods directly to allow easy stripping of logs
// logger is a Log::Logger
#if LOGGING_LEVEL >= 6
#define LOG_DEBUG(logger, logMsg) do { logger.debug() << logMsg; } while(0)
#else
#define LOG_DEBUG(logger, logMsg) do { } while(0)
#endif

#if LOGGING_LEVEL >= 5
#define LOG_INFO(logger, logMsg) do { logger.info() << logMsg; } while(0)
#else
#define LOG_INFO(logger, logMsg) do { } while(0)
#endif

#if LOGGING_LEVEL >= 4
#define LOG_NOTICE(logger, logMsg) do { logger.notice() << logMsg; } while(0)
#else
#define LOG_NOTICE(logger, logMsg) do { } while(0)
#endif

#if LOGGING_LEVEL >= 3
#define LOG_WARN(logger, logMsg) do { logger.warning() << logMsg; } while(0)
#else
#define LOG_WARN(logger, logMsg) do { } while(0)
#endif

#if LOGGING_LEVEL >= 2
#define LOG_ERROR(logger, logMsg) do { logger.error() << logMsg; } while(0)
#else
#define LOG_ERROR(logger, logMsg) do { } while(0)
#endif

#if LOGGING_LEVEL >= 1
#define LOG_CRIT(logger, logMsg) do { logger.critic() << logMsg; } while(0)
#else
#define LOG_CRIT(logger, logMsg) do { } while(0)
#endif



namespace LogName {
	const char * const D3D9Log = "Stereoificator.D3D9";
}



//extern const char* SteroificatorD3D9Log;
//extern Log::Logger logs;



#endif
