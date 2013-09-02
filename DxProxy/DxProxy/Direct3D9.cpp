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

#include "Direct3D9.h"
#include "Direct3DDevice9.h"
#include "Main.h"
#include "D3DProxyDevice.h"

#include <DXGI.h>

BaseDirect3D9::BaseDirect3D9(IDirect3D9* pD3D) :
	m_pD3D(pD3D),
	m_nRefCount(1),
	log(LogName::D3D9Log),
	m_isRiftAdapterValid(false),
	m_riftAdapter(0)
{
	// Find rift via EnumDisplayDevices. Note that the index of the device the montior is connected to
	// doesn't seem to be guaranteed to be the same as the index of the D3D9 adapter (although in all 
	// my tests it always has been). So, in case the device and adapter index are different we match by
	// device name, for example: '\\.\DISPLAY1'
	bool riftFound = false;
	std::string riftDeviceName;

	DISPLAY_DEVICE device;
	ZeroMemory(&device, sizeof(DISPLAY_DEVICE));
	device.cb = sizeof(DISPLAY_DEVICE);

	int itDD = 0;
	while (EnumDisplayDevices(NULL, itDD, &device, 0) != 0) {

		LOG_DEBUG(log, "Device Name:" << device.DeviceName);
		LOG_DEBUG(log, "Device ID:" << device.DeviceID);
		LOG_DEBUG(log, "Device Key:" << device.DeviceKey);
		LOG_DEBUG(log, "Device String:" << device.DeviceString);

		DISPLAY_DEVICE monitorDevice;
		ZeroMemory(&monitorDevice, sizeof(DISPLAY_DEVICE));
		monitorDevice.cb = sizeof(DISPLAY_DEVICE);

		if (EnumDisplayDevices(device.DeviceName, 0, &monitorDevice, 0) != 0) {
			LOG_DEBUG(log, "Monitor Device Name:" << monitorDevice.DeviceName);
			LOG_DEBUG(log, "Monitor Device ID:" << monitorDevice.DeviceID);
			LOG_DEBUG(log, "Monitor Device Key:" << monitorDevice.DeviceKey);
			LOG_DEBUG(log, "Monitor Device String:" << monitorDevice.DeviceString);

			// Is Rift Dev Kit
			if (!riftFound && strstr(monitorDevice.DeviceID, "OVR0001")) {

				// Device rather than the attached monitor is used as the device name is what 
				// we will be needing to use with D3D
				LOG_NOTICE(log, "Rift device identified as " << device.DeviceName);
				riftDeviceName = device.DeviceName;
				riftFound = true;
			}
		}

		++itDD;
	}


	// Now that we know which device the Rift is connected to find the D3D9 adapter that device is associated with.
	if (riftFound) {
		UINT numAdapters = m_pD3D->GetAdapterCount();

		D3DADAPTER_IDENTIFIER9 adapterID;
		for (UINT i = 0; i < numAdapters; i++)
		{
			ZeroMemory(&adapterID, sizeof(D3DADAPTER_IDENTIFIER9));
			m_pD3D->GetAdapterIdentifier(i, 0, &adapterID);

			LOG_DEBUG(log, "Description:" << adapterID.Description);
			LOG_DEBUG(log, "DevName: " << adapterID.DeviceName);

			if (riftDeviceName.find(adapterID.DeviceName) != std::string::npos) {
				LOG_NOTICE(log, "Rift connected to adapter " << i);
				m_isRiftAdapterValid = true;
				m_riftAdapter = i;
				break;
			}
		}
	}

	if (!m_isRiftAdapterValid) {
		LOG_WARN(log, "Unable to determine which adapter the Rift is connected to.");
	}
}

BaseDirect3D9::~BaseDirect3D9()
{
	if(m_pD3D)
		m_pD3D->Release();
}

HRESULT WINAPI BaseDirect3D9::QueryInterface(REFIID riid, LPVOID* ppv)
{
	return m_pD3D->QueryInterface(riid, ppv);
}

ULONG WINAPI BaseDirect3D9::AddRef()
{
	return ++m_nRefCount;
}

ULONG WINAPI BaseDirect3D9::Release()
{
	if(--m_nRefCount == 0)
	{
		delete this;
		return 0;
	}
	return m_nRefCount;
}

HRESULT WINAPI BaseDirect3D9::RegisterSoftwareDevice(void* pInitializeFunction)
{
	return m_pD3D->RegisterSoftwareDevice(pInitializeFunction);
}

UINT BaseDirect3D9::GetAdapterCount()
{
	return m_pD3D->GetAdapterCount();
}

HRESULT WINAPI BaseDirect3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags,
	D3DADAPTER_IDENTIFIER9* pIdentifier)
{
	return m_pD3D->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT WINAPI BaseDirect3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
	return m_pD3D->GetAdapterModeCount(Adapter, Format);
}

HRESULT WINAPI BaseDirect3D9::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode,
	D3DDISPLAYMODE* pMode)
{
	return m_pD3D->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

HRESULT WINAPI BaseDirect3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode)
{
	return m_pD3D->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT WINAPI BaseDirect3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType,
	D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
	return m_pD3D->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
}

HRESULT WINAPI BaseDirect3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType,
	D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return m_pD3D->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType,
		CheckFormat);
}

HRESULT WINAPI BaseDirect3D9::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType,
	D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType,
	DWORD* pQualityLevels)
{
	return m_pD3D->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed,
		MultiSampleType, pQualityLevels);
}

HRESULT WINAPI BaseDirect3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType,
	D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return m_pD3D->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat,
		DepthStencilFormat);
}

HRESULT WINAPI BaseDirect3D9::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType,
	D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	return m_pD3D->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT WINAPI BaseDirect3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps)
{
	return m_pD3D->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HMONITOR WINAPI BaseDirect3D9::GetAdapterMonitor(UINT Adapter)
{
	return m_pD3D->GetAdapterMonitor(Adapter);
}

HRESULT WINAPI BaseDirect3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
	DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
	IDirect3DDevice9** ppReturnedDeviceInterface)
{
	// load configuration file
	ProxyHelper helper = ProxyHelper();
	ProxyHelper::ProxyConfig cfg;
	bool stereoificatorCfgLoaded = helper.LoadConfig(cfg);

	if (cfg.forceAdapterNumber >= (int)GetAdapterCount()) {
		LOG_ERROR(log, "forceAdapterNumber outside of valid range. Using primary Adapter instead.");
	}

	// Create real interface
	HRESULT hResult = m_pD3D->CreateDevice( (cfg.forceAdapterNumber >= (int)GetAdapterCount() || (cfg.forceAdapterNumber < 0)) ? Adapter : cfg.forceAdapterNumber, DeviceType, hFocusWindow, BehaviorFlags,
		pPresentationParameters, ppReturnedDeviceInterface);
	if(FAILED(hResult))
		return hResult;

	LOG_NOTICE(log, "Actual D3D Device created.");
	LOG_NOTICE(log, "Number of back buffers = " << pPresentationParameters->BackBufferCount);
	
	if(!stereoificatorCfgLoaded) {
		LOG_CRIT(log, "Config loading failed, config could not be loaded. Returning normal D3DDevice. Stereoificator will not be active.");
		return hResult;
	}

	LOG_NOTICE(log, "Config loading - OK");

	// Create and return proxy
	D3DProxyDevice* newDev = new D3DProxyDevice(*ppReturnedDeviceInterface, this, cfg);
	*ppReturnedDeviceInterface = newDev;

	LOG_NOTICE(log, "Stereoificator D3D device created.");

	return hResult;
}