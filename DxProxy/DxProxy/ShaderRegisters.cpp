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

#include "ShaderRegisters.h"
#include <assert.h>


ShaderRegisters::ShaderRegisters(DWORD maxConstantRegistersF, IDirect3DDevice9* pActualDevice) :
	m_maxConstantRegistersF(maxConstantRegistersF),
	m_registersF(maxConstantRegistersF * VECTOR_LENGTH, 0), // VECTOR_LENGTH floats per register
	m_dirtyRegistersF(),
	m_pActualDevice(pActualDevice),
	m_pActiveVertexShader(NULL),
	log(LogName::D3D9Log)
{
	assert(pActualDevice != NULL);

	m_pActualDevice->AddRef();
}

	//TODO assignment and copy - add ref to device (remove ref from old device on assign)? or prevent

ShaderRegisters::~ShaderRegisters()
{
	if (m_pActualDevice)
		m_pActualDevice->Release();

	if (m_pActiveVertexShader)
		m_pActiveVertexShader->Release();
}

void ShaderRegisters::ReleaseResources()
{
	if (m_pActiveVertexShader)
		m_pActiveVertexShader->Release();

	m_pActiveVertexShader = NULL;

	// TODO Do the constant registers get cleared on device reset? Current guess is no. Needs testing or a reference finding that says what happens.
}


HRESULT WINAPI ShaderRegisters::SetConstantRegistersF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount)
{
	if ((StartRegister >= m_maxConstantRegistersF) || ((StartRegister + Vector4fCount) >= m_maxConstantRegistersF))
		return D3DERR_INVALIDCALL;

	// Set proxy registers
	std::copy(pConstantData, pConstantData + (VECTOR_LENGTH * Vector4fCount), m_registersF.begin() + RegisterIndex(StartRegister));

	// Mark registers dirty
	for (UINT i = StartRegister; i < StartRegister + Vector4fCount; i++) {
		m_dirtyRegistersF.insert(i);
	}

	return D3D_OK;
}

HRESULT WINAPI ShaderRegisters::GetConstantRegistersF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	if ((StartRegister >= m_maxConstantRegistersF) || ((StartRegister + Vector4fCount) >= m_maxConstantRegistersF))
		return D3DERR_INVALIDCALL;

	pConstantData = &m_registersF[RegisterIndex(StartRegister)];
	
	return D3D_OK;
}

std::vector<float> ShaderRegisters::GetAllConstantRegistersF()
{
	return m_registersF;
}




void ShaderRegisters::SetFromStateBlockData(std::map<UINT, D3DXVECTOR4> * storedRegisters, D3D9ProxyVertexShader* storedShader)
{
	SAFE_RELEASE(m_pActiveVertexShader);
	m_pActiveVertexShader = storedShader;
	if (m_pActiveVertexShader)
		m_pActiveVertexShader->AddRef();

	SetFromStateBlockData(storedRegisters);
}

void ShaderRegisters::SetFromStateBlockData(std::map<UINT, D3DXVECTOR4> * storedRegisters)
{
	auto itNewRegs = storedRegisters->begin();
	while (itNewRegs != storedRegisters->end()) {

		if ((RegisterIndex(itNewRegs->first) + VECTOR_LENGTH) >= m_registersF.size())
			throw std::out_of_range("Register from stateblock is out of range, implosion imminent");

		
		//std::copy(static_cast<float*>(itNewRegs->second), static_cast<float*>(itNewRegs->second) + VECTOR_LENGTH,  &m_registersF[RegisterIndex(itNewRegs->first)]);
		// copy produces warnings, this does not.
		int regStartIndexInVector = RegisterIndex(itNewRegs->first);
		m_registersF[regStartIndexInVector	  ]	= itNewRegs->second.x;
		m_registersF[regStartIndexInVector + 1] = itNewRegs->second.y;
		m_registersF[regStartIndexInVector + 2] = itNewRegs->second.z;
		m_registersF[regStartIndexInVector + 3] = itNewRegs->second.w;
		

		// register is clean (now matches device state - unless it's stereo in which case it might not, that is handled at the end)
		m_dirtyRegistersF.erase(itNewRegs->first);
		++itNewRegs;
	}

	MarkAllStereoConstantsDirty();
}

void ShaderRegisters::MarkAllStereoConstantsDirty()
{
	if (m_pActiveVertexShader) {
		// Mark all StereoShaderConstants dirty so they are updated before drawing
		auto itStereoConstant = m_pActiveVertexShader->ModifiedConstants()->begin();
		while (itStereoConstant != m_pActiveVertexShader->ModifiedConstants()->end()) {
			
			m_dirtyRegistersF.insert(itStereoConstant->first);
			++itStereoConstant;
		}
	}
}


void ShaderRegisters::SetFromStateBlockData(std::vector<float> * storedRegisters, D3D9ProxyVertexShader* storedShader)
{
	// Full register capture should always match the size of the existing register set as size is fixed and register was captured from this
	assert(storedRegisters->size() == m_registersF.size());

	std::copy(storedRegisters->begin(), storedRegisters->end(), m_registersF.begin());

	// Data should match registers that are already on device (unless it's stereo in which case it might not, that is handled next)
	m_dirtyRegistersF.clear();

	SAFE_RELEASE(m_pActiveVertexShader);
	m_pActiveVertexShader = storedShader;
	if (m_pActiveVertexShader)
		m_pActiveVertexShader->AddRef();

	MarkAllStereoConstantsDirty();
}


bool ShaderRegisters::AnyDirty(UINT start, UINT count)
{
	auto it = m_dirtyRegistersF.lower_bound(start);

	if (it == m_dirtyRegistersF.end())
		return false;

	if (*it >= start + count) {
		return false;
	}

	return true;
}
		
void ShaderRegisters::ApplyAllDirty(stereoificator::RenderPosition currentSide) 
{	
	
	if (m_dirtyRegistersF.size() == 0)
		return;

	if (m_pActiveVertexShader) {
		ApplyStereoConstants(currentSide, true);
	}


	// Apply all remaining dirty registers (should just be non-stereo that remain dirty) to device
	auto it = m_dirtyRegistersF.begin();

	if (it != m_dirtyRegistersF.end()) {

		int startReg = *it; // can't dererefence this if it might be end
		int lastReg = startReg;

		while (it != m_dirtyRegistersF.end()) {

		
			auto itNext = std::next(it);
			if ((itNext != m_dirtyRegistersF.end()) && (*itNext == lastReg + 1)) {
				// skip through until we reach the end of a continuous series of dirty registers
				lastReg = *itNext;
			}
			else {
				// set this series of registers
				m_pActualDevice->SetVertexShaderConstantF(startReg, &m_registersF[RegisterIndex(startReg)], lastReg - startReg + 1);

				// If there are more dirty registers left the next register will be the new startReg
				if (itNext != m_dirtyRegistersF.end()) {
					startReg = *itNext;
					lastReg = startReg;
				}
			}

			++it;
		}
	}

	m_dirtyRegistersF.clear();
}

void ShaderRegisters::ApplyAllStereoConstants(stereoificator::RenderPosition currentSide)
{
	ApplyStereoConstants(currentSide, false);
}

void ShaderRegisters::ApplyStereoConstants(stereoificator::RenderPosition currentSide, const bool dirtyOnly)
{
	if (!m_pActiveVertexShader)
		return;

	auto itStereoConstant = m_pActiveVertexShader->ModifiedConstants()->begin();
	while (itStereoConstant != m_pActiveVertexShader->ModifiedConstants()->end()) {

		// if any of the registers that make up this constant are dirty update before setting
		if ( AnyDirty(itStereoConstant->second.StartRegister(), itStereoConstant->second.Count())) { // Should we do this or make this method just switch sides without checking for updated data? 

			itStereoConstant->second.Update(&m_registersF[RegisterIndex(itStereoConstant->second.StartRegister())]);

			if (dirtyOnly) {
				// Apply this dirty constant to device
				float* data;
				switch (currentSide) {

				case stereoificator::Left:
					data = itStereoConstant->second.DataLeftPointer();
					break;

				case stereoificator::Right:
					data = itStereoConstant->second.DataRightPointer();
					break;

				case stereoificator::Center:
					data = itStereoConstant->second.DataOriginalPointer();
					break;

				default:
					LOG_ERROR(log, "SetSide - Unknown rendering position.");
					break;
				}
				m_pActualDevice->SetVertexShaderConstantF(itStereoConstant->second.StartRegister(), data, itStereoConstant->second.Count());
			}

			// These registers are no longer dirty
			for (UINT i = itStereoConstant->second.StartRegister(); i < itStereoConstant->second.StartRegister() + itStereoConstant->second.Count(); i++)
				m_dirtyRegistersF.erase(i);
		}

		if (!dirtyOnly) {
			// Apply this constant to device
			float* data;
			switch (currentSide) {

			case stereoificator::Left:
				data = itStereoConstant->second.DataLeftPointer();
				break;

			case stereoificator::Right:
				data = itStereoConstant->second.DataRightPointer();
				break;

			case stereoificator::Center:
				data = itStereoConstant->second.DataOriginalPointer();
				break;

			default:
				LOG_ERROR(log, "SetSide - Unknown rendering position.");
				break;
			}
			m_pActualDevice->SetVertexShaderConstantF(itStereoConstant->second.StartRegister(), data, itStereoConstant->second.Count());
		}

		++itStereoConstant;
	}
}


void ShaderRegisters::ActiveVertexShaderChanged(D3D9ProxyVertexShader* pNewVertexShader)
{
	if (m_pActiveVertexShader == pNewVertexShader)
		return;

	if (pNewVertexShader) {

		std::map<UINT, StereoShaderConstant<float>>* pNewShaderModConstants = pNewVertexShader->ModifiedConstants();

		std::map<UINT, StereoShaderConstant<float>>* pOldShaderModConstants = NULL;
		if (m_pActiveVertexShader)
			pOldShaderModConstants = m_pActiveVertexShader->ModifiedConstants();

		// Update the data in new shader constants with data from matching constants from last shader.)
		auto itNewConstants = pNewShaderModConstants->begin();
		while (itNewConstants != pNewShaderModConstants->end()) {

			bool mightBeDirty = true;

			if (pOldShaderModConstants) {
				// No idea if this is saving any time or if it would be better to just mark all the registers dirty and re-apply the constants on first draw
				if (m_pActiveVertexShader->ModifiedConstants()->count(itNewConstants->first) == 1) {
					if (pOldShaderModConstants->at(itNewConstants->first).SameConstantAs(itNewConstants->second)) {
						(*pNewShaderModConstants).at(itNewConstants->first) = (*pOldShaderModConstants).at(itNewConstants->first);
						mightBeDirty = false;
					}
				}
			}

			// If there isn't a corresponding old modification then this modified constant will need updating
			if (mightBeDirty) {
				m_dirtyRegistersF.insert(itNewConstants->first);
			}

			++itNewConstants;
		}


	}
	
	SAFE_RELEASE(m_pActiveVertexShader);
	m_pActiveVertexShader = pNewVertexShader;
	if (m_pActiveVertexShader)
		m_pActiveVertexShader->AddRef();
}
	