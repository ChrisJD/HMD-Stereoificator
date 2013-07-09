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

#ifndef LIMITEDRANGEVALUE_H_INCLUDED
#define LIMITEDRANGEVALUE_H_INCLUDED


#include <unordered_map>
#include "Vireio.h"


class LimitedRangeValue
{
public:	
	LimitedRangeValue() :
		m_currentValue(1.0f),
		m_minValue(0.1f),
		m_maxValue(10.0f),
		m_defaultValue(1.0f)
	{}

	// if the default value is outside the min-max range the default will be set to min or max as appropriate.
	LimitedRangeValue(float defaultValue, float minValue, float maxValue) :
		m_currentValue(0.0f),
		m_minValue(0.0f),
		m_maxValue(0.0f),
		m_defaultValue(0.0f)
	{
		SetNewLimits(defaultValue, minValue, maxValue);
		m_currentValue = m_defaultValue;
	}

	virtual ~LimitedRangeValue() {}

	void  SetNewLimits(float defaultValue, float minValue, float maxValue)
	{
		m_minValue = minValue;
		m_maxValue = maxValue;
		m_defaultValue = defaultValue;
		vireio::clamp(&m_defaultValue, m_minValue, m_maxValue);
		vireio::clamp(&m_currentValue, m_minValue, m_maxValue);
	}

	// Attempts to add the provided value to the current value. If the resulting value will be clamped to the min-max range.
	// Returns the current value after the range clamp.
	float AddToValue(float toAdd)
	{
		m_currentValue += toAdd;
	
		vireio::clamp(&m_currentValue, m_minValue, m_maxValue);

		return m_currentValue;
	}

	// Attempts to set the current value to newValue. The value that is actually set will be based on the newValue clamped to the min-max range.
	// Returns the current value after the range clamp.
	float SetValue(float newValue)
	{
		m_currentValue = newValue;
	
		vireio::clamp(&m_currentValue, m_minValue, m_maxValue);

		return m_currentValue;
	}

	float Value()			{ return m_currentValue; }
	void  ResetToDefault()	{ SetValue(m_defaultValue); }
	float Max()				{ return m_maxValue; }
	float Min()				{ return m_minValue; }
	
	

private:

	float m_minValue;
	float m_maxValue;
	float m_defaultValue;
	float m_currentValue;
};



#endif