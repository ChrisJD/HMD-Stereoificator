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

#ifndef STEREOSHADERCONSTANT_H_INCLUDED
#define STEREOSHADERCONSTANT_H_INCLUDED

#include <vector>
#include <string>
#include <memory>
#include "d3d9.h"
#include "d3dx9.h"
#include "ShaderConstantModification.h"

//  Number of T in a Register (L) = (4 for float/int, 1 for bool)
template <class T=float, UINT L=4>
class StereoShaderConstant
{
public:	
	StereoShaderConstant(UINT StartReg, const T* pData, UINT dataCount, std::shared_ptr<ShaderConstantModification<T>> modification) :
		m_StartRegister(StartReg),
		m_Count(dataCount),
		m_DataOriginal(pData, pData + (dataCount * L)),
		m_DataLeft(),
		m_DataRight(),
		m_modification(modification)
	{
		m_DataLeft.resize(dataCount * L);
		m_DataRight.resize(dataCount * L);
		Update(pData);
	}

	virtual ~StereoShaderConstant() 
	{
		m_modification.reset();
	}

	// pointer to new data. Verify data dimensions match this constant _before_ calling if needed
	void Update(const T* pData) 
	{
		m_DataOriginal.assign(pData, pData + (m_Count * L));
		m_modification->ApplyModification(pData, &m_DataLeft, &m_DataRight);
	}

	/* Return true if this constant represents the same constant as other  (contents of the registers does not need to match) but the registers must be the same and use the same modification */
	bool SameConstantAs(const StereoShaderConstant<T> & other)
	{
		return ((other.m_StartRegister == m_StartRegister) &&
			(other.m_modification->m_ModificationID == m_modification->m_ModificationID));
	}
		
	T* DataLeftPointer() 
	{
		return &m_DataLeft[0];
	}

	T* DataRightPointer() 
	{
		return &m_DataRight[0];
	}

	T* DataOriginalPointer() 
	{
		return &m_DataOriginal[0];
	}

	UINT StartRegister() { return m_StartRegister; }
	UINT Count() { return m_Count; }
	

private:
	std::vector<T> m_DataOriginal;
	std::vector<T> m_DataLeft;
	std::vector<T> m_DataRight;

	std::shared_ptr<ShaderConstantModification<T>> m_modification;

	UINT m_StartRegister;
	UINT m_Count;
	
};



#endif