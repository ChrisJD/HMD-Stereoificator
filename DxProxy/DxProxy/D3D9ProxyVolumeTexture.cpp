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

#include "D3D9ProxyVolumeTexture.h"
#include <assert.h>


D3D9ProxyVolumeTexture::D3D9ProxyVolumeTexture(IDirect3DVolumeTexture9* pActualVolumeTexture, BaseDirect3DDevice9* pOwningDevice) :
	BaseDirect3DVolumeTexture9(pActualVolumeTexture),
	m_wrappedVolumeLevels(),
	m_pOwningDevice(pOwningDevice),
	log(LogName::D3D9Log)
{
	assert (pOwningDevice != NULL);

	m_pOwningDevice->AddRef();
}

D3D9ProxyVolumeTexture::~D3D9ProxyVolumeTexture()
{
	// delete all surfaces in m_levels
	auto it = m_wrappedVolumeLevels.begin();
	while (it != m_wrappedVolumeLevels.end()) {
		// we have to explicitly delete the Volume here as the Release behaviour of the volume would get stuck in a loop
		// calling back to the container Release.
		delete it->second;
		it = m_wrappedVolumeLevels.erase(it);
	}

	if (m_pOwningDevice)
		m_pOwningDevice->Release();
}




IDirect3DVolumeTexture9* D3D9ProxyVolumeTexture::getActual()
{
	return m_pActualTexture;
}



HRESULT WINAPI D3D9ProxyVolumeTexture::GetDevice(IDirect3DDevice9** ppDevice)
{
	if (!m_pOwningDevice)
		return D3DERR_INVALIDCALL;
	else {
		*ppDevice = m_pOwningDevice;
		m_pOwningDevice->AddRef();
		return D3D_OK;
	}
}



HRESULT WINAPI D3D9ProxyVolumeTexture::GetVolumeLevel(UINT Level, IDirect3DVolume9** ppVolumeLevel)
{
	HRESULT finalResult;

	// Have we already got a Proxy for this surface level?
	if (m_wrappedVolumeLevels.count(Level) == 1) { // yes

		// TODO Should we call through to underlying texture and make sure the result of doing this operation on the 
		// underlying texture would still be a success? (not if we don't have to, will see if it becomes a problem)

		*ppVolumeLevel = m_wrappedVolumeLevels[Level];
		(*ppVolumeLevel)->AddRef();

		finalResult = D3D_OK;
	}
	else {
		// Get underlying surfaces (stereo pair made from the surfaces at the same level in the left and right textues), 
		//  wrap, then store in m_wrappedSurfaceLevels and return the wrapped surface
		IDirect3DVolume9* pActualVolumeLevel = NULL;

		HRESULT result = m_pActualTexture->GetVolumeLevel(Level, &pActualVolumeLevel);

		if (SUCCEEDED(result)) {

			D3D9ProxyVolume* pWrappedVolumeLevel = new D3D9ProxyVolume(pActualVolumeLevel, m_pOwningDevice, this);

			if(m_wrappedVolumeLevels.insert(std::pair<ULONG, D3D9ProxyVolume*>(Level, pWrappedVolumeLevel)).second) {
				// insertion of wrapped surface level into m_wrappedSurfaceLevels succeeded
				*ppVolumeLevel = pWrappedVolumeLevel;
				(*ppVolumeLevel)->AddRef();
				finalResult = D3D_OK;
			}
			else {
				// Failure to insert should not be possible. In this case we could still return the wrapped surface,
				// however, if we did and it was requested again a new wrapped instance will be returned and things would explode
				// at some point. Better to fail fast.
				LOG_CRIT(log, __FUNCTION__);
				LOG_CRIT(log, "Unable to store surface level.");
				assert(false);

				finalResult = D3DERR_INVALIDCALL;
			}
		}
		else { 
			LOG_ERROR(log, __FUNCTION__);
			LOG_ERROR(log, "Error fetching actual surface level.");
			finalResult = result;
		}
	}

	return finalResult;
}