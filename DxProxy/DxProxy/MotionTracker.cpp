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

#include "MotionTracker.h"

MotionTracker::MotionTracker()
{	
	currentYaw = 0.0f;
	currentPitch = 0.0f;
	currentRoll = 0.0f;

	leftoverYaw = 0.0f;
	leftoverPitch = 0.0f;

	multiplierYaw = 25.0f;
	multiplierPitch = 25.0f;
	multiplierRoll = 1.0f;

	mouseData.type = INPUT_MOUSE;
	mouseData.mi.dx = 0;
	mouseData.mi.dy = 0;
	mouseData.mi.mouseData = 0;
	mouseData.mi.dwFlags = MOUSEEVENTF_MOVE;
	mouseData.mi.time = 0;
	mouseData.mi.dwExtraInfo = 0;

	
	OutputDebugString("Motion Tracker Created\n");
}

MotionTracker::~MotionTracker()
{
}


int MotionTracker::getOrientationFromDevice(float* yaw, float* pitch, float* roll) 
{
	*yaw = 0.0f;
	*pitch = 0.0f;
	*roll = 0.0f;

	return -1;
}

bool MotionTracker::isAvailable()
{
	return false;
}

void MotionTracker::updateOrientation()
{
	float newYaw, newPitch, newRoll;

	if(getOrientationFromDevice(&newYaw, &newPitch, &newRoll) == 0)
	{
		newYaw = fmodf(newYaw, 360.0f);
		newPitch = -fmodf(newPitch, 360.0f);

		
		float yawChange = newYaw - currentYaw + leftoverYaw;
		/// http://stackoverflow.com/questions/1878907/the-smallest-difference-between-2-angles
		/// Change in angle as shortest signed angle
		yawChange = fmodf((yawChange + 180.0f), 360.0f);
		if (yawChange < 0)
			yawChange = 360.0f - yawChange; // Handling negative numbers as fmodf not the correct type of modulus
		yawChange -= 180.0f;
		/// End Change in angle as shortest signed angle 

		yawChange *= multiplierYaw;
		leftoverYaw = fmodf(yawChange, 1.0f); // left over that will remain after multiplier is applied and we have rounded to a whole number (mouse dx/dy are longs)
		mouseData.mi.dx = round(yawChange - leftoverYaw); // amount to move (the leftover would be lost anyway so subtract it so we know exactly what is gonig on with rounding)
		leftoverYaw /= multiplierYaw; // The left over to carry to the next update needs to be un-multiplied so divide by multiplier to get back to 


		float pitchChange = newPitch - currentPitch + leftoverPitch;
		pitchChange = fmodf((pitchChange + 180.0f), 360.0f);
		if (pitchChange < 0)
			pitchChange = 360.0f - pitchChange;
		pitchChange -= 180.0f;

		pitchChange *= multiplierPitch;
		leftoverPitch = fmodf(pitchChange, 1.0f);
		mouseData.mi.dy = round(pitchChange - leftoverPitch);
		leftoverPitch /= multiplierPitch;
		
		


		//mouseData.mi.dx = (long) yawChange;
		//mouseData.mi.dy = (long) pitchChange;

		// Keep fractional difference so it's added to the next update.
		//leftoverYaw = ((float) mouseData.mi.dx) / multiplierYaw;
		//leftoverPitch = ((float) mouseData.mi.dy) / multiplierPitch;
		
		//OutputDebugString("Motion Tracker SendInput\n");
		SendInput(1, &mouseData, sizeof(INPUT));

		currentYaw = newYaw;
		currentPitch = newPitch;
		currentRoll = (float)( newRoll * (PI/180.0) * multiplierRoll);			// convert from deg to radians then apply mutiplier
	}
}

void MotionTracker::setMultipliers(float yaw, float pitch, float roll)
{
	multiplierYaw = yaw;
	multiplierPitch = pitch;
	multiplierRoll = roll;
	currentYaw = 0.0f;
	currentPitch = 0.0f;
	currentRoll = 0.0f;
}