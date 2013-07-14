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



#include "Stereoificator.h"


namespace stereoificator {

	bool AlmostSame(float a, float b, float epsilon)
	{
		return fabs(a - b) < epsilon;
	}

	void clamp(float* pfToClamp, float min, float max)
	{
		*pfToClamp > max ? *pfToClamp = max : (*pfToClamp < min ? *pfToClamp = min : *pfToClamp = *pfToClamp);
	}
};