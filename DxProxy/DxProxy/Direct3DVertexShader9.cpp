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

#include "Direct3DVertexShader9.h"
#include <assert.h>

BaseDirect3DVertexShader9::BaseDirect3DVertexShader9(IDirect3DVertexShader9* pActualVertexShader, IDirect3DDevice9 *pOwningDevice) :
	m_pActualVertexShader(pActualVertexShader),
	m_pOwningDevice(pOwningDevice),
	m_nRefCount(1)
{
	assert (pActualVertexShader != NULL);
	assert (pOwningDevice != NULL);



	pOwningDevice->AddRef();
}

BaseDirect3DVertexShader9::~BaseDirect3DVertexShader9()
{
	if(m_pActualVertexShader) 
		m_pActualVertexShader->Release();

	if (m_pOwningDevice)
		m_pOwningDevice->Release();

}

HRESULT WINAPI BaseDirect3DVertexShader9::QueryInterface(REFIID riid, LPVOID* ppv)
{
	return m_pActualVertexShader->QueryInterface(riid, ppv);
}

ULONG WINAPI BaseDirect3DVertexShader9::AddRef()
{
	return ++m_nRefCount;
}

ULONG WINAPI BaseDirect3DVertexShader9::Release()
{
	if(--m_nRefCount == 0)
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}

IDirect3DVertexShader9* BaseDirect3DVertexShader9::getActual()
{
	return m_pActualVertexShader;
}


HRESULT WINAPI BaseDirect3DVertexShader9::GetDevice(IDirect3DDevice9** ppDevice)
{
	if (!m_pOwningDevice)
		return D3DERR_INVALIDCALL;
	else {
		*ppDevice = m_pOwningDevice;
		//m_pOwningDevice->AddRef(); //TODO Test this. Docs don't have the notice that is usually there about a refcount increase
		return D3D_OK;
	}
}

HRESULT WINAPI BaseDirect3DVertexShader9::GetFunction(void *pDate, UINT *pSizeOfData)
{
	return m_pActualVertexShader->GetFunction(pDate, pSizeOfData);
}