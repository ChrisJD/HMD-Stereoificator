/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez

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


#include "SharedMemoryTracker.h"

#include <string>
#include <iostream>

TCHAR szName[]=TEXT("VireioSMTrack");

SharedMemoryTracker::SharedMemoryTracker():MotionTracker()
{
	OutputDebugString("Socket Tracker Created\n");
	hMapFile = NULL;
	pTrackBuf = NULL;
	sharedMemoryAvailable = openSharedMemory();
}

SharedMemoryTracker::~SharedMemoryTracker()
{
	UnmapViewOfFile(pTrackBuf);
	CloseHandle(hMapFile);
}


int SharedMemoryTracker::getOrientationFromDevice(float* yaw, float* pitch, float* roll) 
{
	OutputDebugString("Socket Tracker getOrient\n");

	if(pTrackBuf == NULL)
		return 1;												// error no buffer

	// all orintations are in degrees
	*yaw = pTrackBuf->Yaw;
	*pitch = pTrackBuf->Pitch;
	*roll = pTrackBuf->Roll;

	return 0; 
}

bool SharedMemoryTracker::isAvailable()
{
	return sharedMemoryAvailable;
}


bool SharedMemoryTracker::openSharedMemory()
{
	hMapFile = CreateFileMapping(	INVALID_HANDLE_VALUE,	// use paging file
									NULL,					// default security
									PAGE_READWRITE,			// read/write access
									0,						// maximum object size (high-order DWORD)
									sizeof(TrackData),		// maximum object size (low-order DWORD)
									szName);				// name of mapping object

	if (hMapFile == NULL)										// Could not create file mapping object
		return false;
		
	pTrackBuf = (TrackData*) MapViewOfFile(hMapFile,			// handle to map object
		FILE_MAP_ALL_ACCESS,									// read/write permission
		0,
		0,
		sizeof(TrackData));

	if (pTrackBuf == NULL)										// Could not map view of file
	{
		CloseHandle(hMapFile);
		return false;
	}

	return true;
}