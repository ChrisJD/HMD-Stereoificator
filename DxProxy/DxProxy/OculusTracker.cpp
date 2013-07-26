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

#include "OculusTracker.h"

OculusTracker::OculusTracker()
{
	pManager = NULL;
	pHMD = NULL;
	pSensor = NULL;
	
	System::Init(); // start LibOVR

	pManager = *DeviceManager::Create();

	pHMD = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();

	if (pHMD) {
		pSensor = *pHMD->GetSensor();
	
		if (pSensor)
			SFusion.AttachToSensor(pSensor);

		OutputDebugString("OculusTracker Created\n");
	}
	else {
		OutputDebugString("OculusTracker Not created, is the Rift's USB cable connected and the dev box powered?\n");
	}
}

OculusTracker::~OculusTracker()
{
	if (pSensor)
		pSensor.Clear();
	
	pManager.Clear();
	System::Destroy();  // shutdown LibOVR
}


int OculusTracker::getOrientationFromDevice(float* yaw, float* pitch, float* roll) 
{
	//OutputDebugString("OculusTracker getOrient\n");

	if(SFusion.IsAttachedToSensor() == false)
		return 1;												// error no sensor

	// all orintations are in degrees
	hmdOrient = SFusion.GetOrientation();
	hmdOrient.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(yaw, pitch, roll);
	*yaw = -RadToDegree(*yaw);
	*pitch = RadToDegree(*pitch);
	*roll = -RadToDegree(*roll);

	return 0; 
}

bool OculusTracker::isAvailable()
{
	return SFusion.IsAttachedToSensor();
}
