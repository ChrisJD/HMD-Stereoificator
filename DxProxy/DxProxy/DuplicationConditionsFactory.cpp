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


#include "DuplicationConditionsFactory.h"


DuplicationConditions* DuplicationConditionsFactory::Create(DuplicationConditionNames name)
{
	DuplicationConditions* handler = NULL;

	switch (name) 
	{
	case DC_SOURCE_DEFAULT:
		handler = new DCSourceDefault();
		break;

	case DC_UE3_DEFAULT:

		//break;

	case DC_EGO_DEFAULT:

		//break;

	default:
		handler = new DuplicationConditions();
		break;
	}

	return handler;
}