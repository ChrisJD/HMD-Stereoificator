/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez

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

#ifndef STEREOIFICATOR_H_INCLUDED
#define STEREOIFICATOR_H_INCLUDED

#include <d3d9.h>
#include <assert.h>
#include <cmath>

// 64mm in meters
#define IPD_DEFAULT 0.064f

namespace stereoificator {

	enum RenderPosition
	{
		Center = 0,
		Left = 1,
		Right = 2
		
	};

	

	
	bool AlmostSame(float a, float b, float epsilon);
	void clamp(float* toClamp, float min, float max);

	
};

#endif