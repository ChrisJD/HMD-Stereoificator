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

#ifndef DIRECT3DVERTEXDECLARATION9_H_INCLUDED
#define DIRECT3DVERTEXDECLARATION9_H_INCLUDED

#include <d3d9.h>
#include "Direct3DDevice9.h"

class BaseDirect3DVertexDeclaration9 : public IDirect3DVertexDeclaration9
{
public:
	BaseDirect3DVertexDeclaration9(IDirect3DVertexDeclaration9* pActualVertexDeclaration, IDirect3DDevice9* pOwningDevice);
	virtual ~BaseDirect3DVertexDeclaration9();

	// IUnknown
	virtual HRESULT WINAPI QueryInterface(REFIID riid, LPVOID* ppv);
	virtual ULONG WINAPI AddRef();
	virtual ULONG WINAPI Release();

	IDirect3DVertexDeclaration9* getActual();

	// IDirect3DVertexDeclaration9
	virtual HRESULT WINAPI GetDevice(IDirect3DDevice9 **ppDevice);
	virtual HRESULT WINAPI GetDeclaration(D3DVERTEXELEMENT9 *pDecl, UINT *pNumElements);

protected:

	IDirect3DVertexDeclaration9* const m_pActualVertexDeclaration;
	IDirect3DDevice9* m_pOwningDevice;
	ULONG m_nRefCount;
};

#endif
