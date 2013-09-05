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

#include "Main.h"

void stereoificator::InitStereoificatorD3D9()
{
	++gInitCallCount;

	if (gIsInitialized) {
		Log::Logger tempLog(LogName::D3D9Log);
		LOG_INFO(tempLog, "InitStereoificatorD3D9 called when lib has already been initialized."); 
		LOG_INFO(tempLog, "InitStereoificatorD3D9 called " << gInitCallCount << " times.");
	}

	gIsInitialized = true;

	// Log setup
	Log::Logger logs(LogName::D3D9Log); // use this name in other classes to get access to the same log channel (from the log manager or by specifying the same name in constructor)
	
	Log::Config::Vector configList;
    Log::Config::addOutput(configList, "OutputFile");
    Log::Config::setOption(configList, "filename",          "Stereoificator.D3D9.log");
    Log::Config::setOption(configList, "filename_old",      "Stereoificator.D3D9.previous.log");
    Log::Config::setOption(configList, "max_startup_size",  "0");				// always start new file at startup
    Log::Config::setOption(configList, "max_size",          "100000");			// 10MB
	
#ifdef _DEBUG
	Log::Config::addOutput(configList, "OutputConsole");
	Log::Config::addOutput(configList, "OutputDebug");
	logs.setLevel(Log::Log::eDebug);
#else
	logs.setLevel(Log::Log::eInfo);
#endif
	
	bool logConfiguredOK = true;
	try
    {
        // Configure the Log Manager (create the Output objects)
        Log::Manager::configure(configList);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what();
		OutputDebugString("Stereoificator log configuration failed.");
		logConfiguredOK = false;
    }

	if (logConfiguredOK) {
		LOG_NOTICE(logs, "Stereoificator logging successfully initialized.");
	}






	LOG_NOTICE(logs, "Stereoificator D3D9 library successfully initialized.");
}