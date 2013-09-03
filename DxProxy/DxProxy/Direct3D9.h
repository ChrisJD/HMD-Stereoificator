/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez

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

#ifndef DIRECT3D9_H_INCLUDED
#define DIRECT3D9_H_INCLUDED

#pragma comment(lib, "dxgi.lib")

#include "ProxyHelper.h"
#include "global.h"

#include <windows.h>
#include <d3d9.h>
#include <cstring>

class __declspec(dllexport) BaseDirect3D9 : public IDirect3D9
{
public:
	BaseDirect3D9(IDirect3D9* pD3D);
	virtual ~BaseDirect3D9();

	virtual HRESULT WINAPI QueryInterface(REFIID riid, LPVOID* ppv);
	virtual ULONG WINAPI AddRef();
	virtual ULONG WINAPI Release();

	virtual HRESULT WINAPI RegisterSoftwareDevice(void* pInitializeFunction);
	virtual UINT WINAPI GetAdapterCount();
	virtual HRESULT WINAPI GetAdapterIdentifier(UINT Adapter, DWORD Flags,
		D3DADAPTER_IDENTIFIER9* pIdentifier);
	virtual UINT WINAPI GetAdapterModeCount(UINT Adapter, D3DFORMAT Format);
	virtual HRESULT WINAPI EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode,
		D3DDISPLAYMODE* pMode);
	virtual HRESULT WINAPI GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode);
	virtual HRESULT WINAPI CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType,
		D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
	virtual HRESULT WINAPI CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType,
		D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
	virtual HRESULT WINAPI CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType,
		D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType,
		DWORD* pQualityLevels);
	virtual HRESULT WINAPI CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType,
		D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
	virtual HRESULT WINAPI CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType,
		D3DFORMAT SourceFormat, D3DFORMAT TargetFormat);
	virtual HRESULT WINAPI GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps);
	virtual HMONITOR WINAPI GetAdapterMonitor(UINT Adapter);
	virtual HRESULT WINAPI CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
		DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
		IDirect3DDevice9** ppReturnedDeviceInterface);

	

	/* It is possible that the specified adapter from the client app and the adapter the Rift is connected
		to may have different capabilities. So instead of just overriding the adapter when the device is
		created we override all of the Direct3D behaviour when forcing the Rift so that only one device
		is reported as being available to the client app. As only one adapter will appear to be available
		it will be reported as adapter 0, so the Rift adapters capabilities need to be reported for this
		adapter and the rift adapter will be be mapped to 0 as far as the client can tell. 
		
		Changes are also needed in GetCreationParameters and GetDeviceCaps in Device9 when adapter forced
		*/
	bool AreForcingRift() { return m_forceDisplayOnRift && m_isRiftAdapterValid; }

private:

	IDirect3D9* m_pD3D;
	ULONG m_nRefCount;

	bool m_forceDisplayOnRift;
	bool m_isRiftAdapterValid;
	UINT m_riftAdapter; 

#pragma warning( push )
#pragma warning( disable : 4251 ) //class 'Log::Logger' needs to have dll-interface to be used by clients of class 'BaseDirect3D9'. Logger is never exposed to callers so ignoring warning
	Log::Logger log;
	ProxyHelper::ProxyConfig cfg;
#pragma warning( pop ) 
};


#endif
