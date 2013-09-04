/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez
Modifications 2013 Chris Drain

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

#include "OculusRiftView.h"
//#include "D3DProxyDevice.h"


OculusRiftView::OculusRiftView(ProxyHelper::ProxyConfig& config, std::shared_ptr<HMDisplayInfo> hmd) : StereoView(config),
	hmdInfo(hmd)
{
	LOG_INFO(logs, __FUNCTION__);

	shaderFileName = "HMDBarrel.fx";
}

OculusRiftView::~OculusRiftView()
{
	hmdInfo.reset();
}




void OculusRiftView::SetViewEffectConstants() 
{
	viewEffect->SetFloatArray("LensCenter", LensCenter, 2);
	viewEffect->SetFloatArray("Scale", Scale, 2);
	viewEffect->SetFloatArray("ScaleIn", ScaleIn, 2);
	viewEffect->SetFloatArray("HmdWarpParam", hmdInfo->distortionCoefficients, 4);
}



void OculusRiftView::CalculateShaderVariables()
{
	// Center of half screen is 0.25 in x (halfscreen x input in 0 to 0.5 range)
	// Lens offset is in a -1 to 1 range. Using in shader with a 0 to 0.5 range so use 25% of the value.
	LensCenter[0] = 0.25f + (hmdInfo->lensXCenterOffset * 0.25f);
	// Center of halfscreen range is 0.5 in y (halfscreen y input in 0 to 1 range)
	LensCenter[1] = 0.5f; // lens is assumed to be vertically centered with respect to the screen.
	
	D3DSURFACE_DESC eyeTextureDescriptor;
	leftSurface->GetDesc(&eyeTextureDescriptor);

	float inputTextureAspectRatio = (float)eyeTextureDescriptor.Width / (float)eyeTextureDescriptor.Height;
	
	// Note: The range is shifted using the LensCenter in the shader before the scale is applied so you actually end up with a -1 to 1 range
	// in the distortion function rather than the 0 to 2 I mention below.
	// Input texture scaling to sample the 0 to 0.5 x range of the half screen area in the correct aspect ratio in the distortion function
	// x is changed from 0 to 0.5 to 0 to 2.
	ScaleIn[0] = 4.0f;
	// y is changed from 0 to 1 to 0 to 2 and scaled to account for aspect ratio
	ScaleIn[1] = 2.0f / (inputTextureAspectRatio * 0.5f); // 1/2 aspect ratio for differing input ranges
	
	float scaleFactor = 1.0f / hmdInfo->scaleHorizontal;

	// Scale from 0 to 2 to 0 to 1  for x and y 
	// Then use scaleFactor to fill horizontal space in line with the lens and adjust for aspect ratio for y.
	Scale[0] = (1.0f / 4.0f) * scaleFactor;
	Scale[1] = (1.0f / 2.0f) * scaleFactor * inputTextureAspectRatio;
}

