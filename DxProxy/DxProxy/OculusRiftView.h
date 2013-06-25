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

#ifndef OCULUSRIFTVIEW_H_INCLUDED
#define OCULUSRIFTVIEW_H_INCLUDED

#include "StereoView.h"
#include "HMDisplayInfo.h"

class OculusRiftView : public StereoView
{
public:

	OculusRiftView(ProxyHelper::ProxyConfig& config, std::shared_ptr<HMDisplayInfo> hmd);
	virtual ~OculusRiftView();
	virtual void InitShaderEffects();

	virtual void SetViewEffectConstants();
	virtual void CalculateShaderVariables( );

private:
	float LensCenter[2];
	float Scale[2];
	float ScaleIn[2];

	std::shared_ptr<HMDisplayInfo> hmdInfo;
};

#endif