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

#ifndef D3D9PROXYVOLUMETEXTURE_H_INCLUDED
#define D3D9PROXYVOLUMETEXTURE_H_INCLUDED

#include <d3d9.h>
#include <unordered_map>
#include "Direct3DVolumeTexture9.h"
#include "D3D9ProxyVolume.h"


class D3D9ProxyVolumeTexture : public BaseDirect3DVolumeTexture9
{
public:
	D3D9ProxyVolumeTexture(IDirect3DVolumeTexture9* pActualVolumeTexture, BaseDirect3DDevice9* pOwningDevice);
	virtual ~D3D9ProxyVolumeTexture();


	IDirect3DVolumeTexture9* getActual();
	
	
	// IDirect3DResource9 methods
	virtual HRESULT WINAPI GetDevice(IDirect3DDevice9** ppDevice);

	// VolumeTexture methods
	virtual HRESULT WINAPI GetVolumeLevel(UINT Level, IDirect3DVolume9** ppVolumeLevel);


   
    
	
	

protected:

	/* Wrapped Volume levels */
	std::unordered_map<UINT, D3D9ProxyVolume*> m_wrappedVolumeLevels; 
	BaseDirect3DDevice9* const m_pOwningDevice;
};

#endif
