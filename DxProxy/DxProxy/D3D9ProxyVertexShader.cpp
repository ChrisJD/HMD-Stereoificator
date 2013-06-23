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

#include "D3D9ProxyVertexShader.h"

D3D9ProxyVertexShader::D3D9ProxyVertexShader(IDirect3DVertexShader9* pActualVertexShader, D3DProxyDevice *pOwningDevice, ShaderModificationRepository* pModLoader) :
	BaseDirect3DVertexShader9(pActualVertexShader, pOwningDevice),
	m_pActualDevice(pOwningDevice->getActual()),
	m_modifiedConstants()
{
	if (pModLoader)
		m_modifiedConstants = pModLoader->GetModifiedConstantsF(pActualVertexShader);
}

D3D9ProxyVertexShader::~D3D9ProxyVertexShader()
{}

std::map<UINT, StereoShaderConstant<float>>* D3D9ProxyVertexShader::ModifiedConstants()
{
	return &m_modifiedConstants;
}