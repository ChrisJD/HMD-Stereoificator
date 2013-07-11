/********************************************************************
Stereoificator
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

#include "StereoCursor.h"


StereoCursor::StereoCursor(D3DProxyDevice* pWrappedDevice) :
	m_pWrappedDevice(pWrappedDevice),
	m_pCursorTexture(nullptr),
	m_pSprite(nullptr)
{
	ProxyHelper helper;
	char fullCursorPath[512];
	helper.GetPath(fullCursorPath, "img\\cursor.png");

	D3DXCreateTextureFromFile(pWrappedDevice, fullCursorPath, &m_pCursorTexture);
	//m_pCursorTexture->GetSurfaceLevel(0, &m_pCursorSurface);
}
	
StereoCursor::~StereoCursor()
{
	if (m_pCursorTexture) {
		m_pCursorTexture->Release();
		m_pCursorTexture = NULL;
	}

	if (m_pSprite) {
		m_pSprite->Release();
		m_pSprite = NULL;
	}
}

void SetCursorSurface(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
	//TODO
	// D3DXLoadSurfaceFromSurface 
}

void StereoCursor::SetPosition(int x,int y)
{
	m_x = x;
	m_y = y;
}

void StereoCursor::DrawCursor()
{
	m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
	
	D3DXMATRIX mat;
	D3DXMatrixTranslation (&mat, (float)m_x, (float)m_y, 0.0f);
	m_pSprite->SetTransform(&mat);

	m_pSprite->Draw(m_pCursorTexture, NULL, NULL, NULL, 0xFFFFFFFF);

	m_pSprite->End();
}