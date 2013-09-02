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

#ifndef HMDISPLAYINFO_H_INCLUDED
#define HMDISPLAYINFO_H_INCLUDED

#include "global.h"

#include "d3d9.h"
#include "d3dx9.h"

#include <math.h>
#include <utility>
#include <sstream>


#define HMD_PI 3.14159265


struct HMDisplayInfo
{
public:

#pragma warning( push )
#pragma warning( disable : 4351 ) //disable "new behaviour warning for default initialised array" for this constructor

	// All rift values from OVR_Win32_HMDDevice.cpp in LibOVR
	// Default constructing with Rift DK1 values
	HMDisplayInfo() :
		resolution(std::make_pair<UINT, UINT>(1280, 800)), // Rift dev kit 
		screenAspectRatio((float)resolution.first / (float)resolution.second),
		halfScreenAspectRatio(((float)resolution.first * 0.5f) / (float)resolution.second),
		physicalScreenSize(std::make_pair<float, float>(0.14976f, 0.0935f)), // Rift dev kit 
		eyeToScreenDistance(0.041f), // Rift dev kit
		physicalLensSeparation(0.064f), // Rift dev kit 
		distortionCoefficients(),
		log(LogName::D3D9Log)
	{
		 // Rift dev kit 
		distortionCoefficients[0] = 1.0f;
		distortionCoefficients[1] = 0.22f;
		distortionCoefficients[2] = 0.24f;
		distortionCoefficients[3] = 0.0f;

		float physicalViewCenter = physicalScreenSize.first * 0.25f; 
		float physicalOffset = physicalViewCenter - physicalLensSeparation * 0.5f;	
		// Range at this point would be -0.25 to 0.25 units. So multiply the last step by 4 to get the offset in a -1 to 1  range
		lensXCenterOffset = 4.0f * physicalOffset / physicalScreenSize.first; 

		// This scaling will ensure the source image is sampled so that the left edge of the left half of the screen is just reached
		// by the image. -1 is the left edge of the -1 to 1 range and it is adjusted for the lens center offset (note that this needs
		// adjusting if the lens is also offset vertically, See: StereoConfig::updateDistortionOffsetAndScale in LibOVR for an example
		// of how to do this)
		UpdateScale(-1);

		LOG_INFO(log, "scaleHorizontal: " << scaleHorizontal);
	}

	// Returns left lens center as a percentage of fullscreen width (use '1 - the result' for the right equivilant)
	float LeftLensCenterAsPercentage()
	{
		return ((physicalScreenSize.first / 2.0f) - (physicalLensSeparation / 2.0f)) / physicalScreenSize.first;
	}

	// Updates the scale so that the view matches the specified fov.
	// hFovInDegrees <= 0 will be treated as "scale to fill horizontal"
	void UpdateScale(float hFovInDegrees)
	{
		std::stringstream sstm;
		sstm << "hFovInDegrees: " << hFovInDegrees << std::endl;

		float leftLimitInClipCoords = -1;
		float scaleToFillHorizontal = Distort(leftLimitInClipCoords - lensXCenterOffset) / (leftLimitInClipCoords - lensXCenterOffset);
		sstm << "Initial scale (fill horizontal): " <<  scaleToFillHorizontal << std::endl;


		if (hFovInDegrees > 0) {
						
			float halfPhysicalScreenSize = physicalScreenSize.first / 2.0f;
			float halfLensSeparation = physicalLensSeparation / 2.0f;
			float lensCenterDistanceFromLeftEdge = halfPhysicalScreenSize - halfLensSeparation;
			float leftLensCenterAsPercentageOfHalfScreen = lensCenterDistanceFromLeftEdge / halfPhysicalScreenSize;
			
			// If we scale to fill just to the left edge we get this fov to the left of the lens (left half of left eye)
			double filledFovLeftOfLens = atan ((scaleToFillHorizontal * lensCenterDistanceFromLeftEdge) / eyeToScreenDistance) * (180.0f / HMD_PI);
			sstm << "filledFovLeftOfLens: " << filledFovLeftOfLens << std::endl;

			// This is the fov of the entire left view (including fov which is clipped by the centerline where left view meets right)
			double filledFovFull = filledFovLeftOfLens / leftLensCenterAsPercentageOfHalfScreen;
			sstm << "filled fov full: " << filledFovFull << std::endl;

			// The fov that is being set has this much fov left of the left lens
			double fovLeftOfLens = hFovInDegrees * leftLensCenterAsPercentageOfHalfScreen;
			sstm << "fovLeftOfLens: " << fovLeftOfLens << std::endl;

			double fovAsFractionOfFilledFov =  fovLeftOfLens / filledFovLeftOfLens;
			sstm << "fovAsFractionOfFilledFov: " << fovAsFractionOfFilledFov << std::endl;

			// Recalculate horizontal scale so that view is scaled to maintain visually correct fov.
			// Note that this is very close (could be down to precision issues) but there is still a very small difference in perceived fov and actual fov. Previous example images have been greatly improved.
			scaleHorizontal = Distort((leftLimitInClipCoords * (float)fovAsFractionOfFilledFov) - lensXCenterOffset)  / (leftLimitInClipCoords - lensXCenterOffset);
			sstm << "scaleHorizontal: " << scaleHorizontal  << std::endl;
		}
		else {
			scaleHorizontal = scaleToFillHorizontal;
		}

		LOG_INFO(log, sstm.str());
	}


#pragma warning( pop )

	// <horizontal, vertical>
	std::pair<UINT, UINT>  resolution;

	float screenAspectRatio;
	float halfScreenAspectRatio;

    // Physical characteristics are in meters
	// <horizontal, vertical> 
	std::pair<float, float> physicalScreenSize;
    float eyeToScreenDistance;
    float physicalLensSeparation;

	// The distance in a -1 to 1 range that the center of each lens is from the center of each half of the screen (center of a screen half is 0,0).
	// -1 being the far left edge of the screen half and +1 being the far right of the screen half.
	float lensXCenterOffset;

	// From Rift docs on distortion  
    //   uvResult = uvInput * (K0 + K1 * uvLength^2 + K2 * uvLength^4)
    float distortionCoefficients[4];

	float scaleHorizontal;

	// This distortion must match that being used in the shader (the distortion, not including the scaling that is included in the shader)
	virtual float Distort(float radius)
	{
        float radiusSqared = radius * radius;
        return radius * (distortionCoefficients[0] + distortionCoefficients[1] * radiusSqared + distortionCoefficients[2] * 
						radiusSqared * radiusSqared + distortionCoefficients[3] * radiusSqared * radiusSqared * radiusSqared);
	}


private:
	Log::Logger log;

};



#endif