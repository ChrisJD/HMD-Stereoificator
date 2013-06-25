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

#ifndef STEREOVIEW_H_INCLUDED
#define STEREOVIEW_H_INCLUDED

#include "ProxyHelper.h"
#include "D3DProxyDevice.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <map>
#include <string.h>
#include <assert.h>

class StereoView
{
public:
	static enum StereoTypes
	{
		SIDE_BY_SIDE = 20,
		OCULUS_RIFT = 26
	};

	static enum Eyes
	{
		LEFT_EYE,
		RIGHT_EYE
	};

	StereoView(ProxyHelper::ProxyConfig& config);
	virtual ~StereoView();

	/* Must be initialised with an actual device. Not a wrapped device*/
	virtual void Init(IDirect3DDevice9* pActualDevice);
	
	//virtual void Draw();
	virtual void Draw(D3D9ProxySurface* stereoCapableSurface);
	
	//virtual void UpdateEye(int eye);
	virtual void SaveScreen();
	virtual void ReleaseEverything();
	virtual void PostReset();

	
	D3DVIEWPORT9 viewport;

	
	bool swapEyes;
	bool initialized;
	int game_type;
	int stereo_mode;

	int stereoEnabled;


	

	float DistortionScale;	// used by OculusRiftView and D3DProxyDevice


protected:
	IDirect3DDevice9* m_pActualDevice;

	virtual void SetViewEffectConstants();
	virtual void CalculateShaderVariables();

	virtual void SaveState();
	virtual void SetState();
	virtual void RestoreState();

	virtual void InitTextureBuffers();
	virtual void InitVertexBuffers();
	virtual void InitShaderEffects();



	IDirect3DTexture9* leftTexture;
	IDirect3DTexture9* rightTexture;
	IDirect3DSurface9* backBuffer;
	IDirect3DSurface9* leftSurface;
	IDirect3DSurface9* rightSurface;


	IDirect3DVertexBuffer9* screenVertexBuffer;

	DWORD tssColorOp;
	DWORD tssColorArg1;
	DWORD tssAlphaOp;
	DWORD tssAlphaArg1;
	DWORD tssConstant;

	DWORD rsAlphaEnable;
	DWORD rsZEnable;
	DWORD rsZWriteEnable;
	DWORD rsDepthBias;
	DWORD rsSlopeScaleDepthBias;
	DWORD rsSrgbEnable;

	DWORD ssSrgb;
	DWORD ssSrgb1;
	DWORD ssAddressU;
	DWORD ssAddressV;
	DWORD ssAddressW;
	DWORD ssMag0;
	DWORD ssMag1;
	DWORD ssMin0;
	DWORD ssMin1;
	DWORD ssMip0;
	DWORD ssMip1;

	IDirect3DVertexShader9* lastVertexShader;
	IDirect3DPixelShader9* lastPixelShader;
	IDirect3DBaseTexture9* lastTexture;
	IDirect3DBaseTexture9* lastTexture1;
	IDirect3DVertexDeclaration9* lastVertexDeclaration;

	IDirect3DSurface9* lastRenderTarget0;
	IDirect3DSurface9* lastRenderTarget1;

	ID3DXEffect* viewEffect;

	std::map<int, std::string> shaderEffect;

	IDirect3DStateBlock9* sb;

};

const DWORD D3DFVF_TEXVERTEX = D3DFVF_XYZRHW | D3DFVF_TEX1;

struct TEXVERTEX
{
	float x;
	float y;
	float z;
	float rhw;
	float u;
	float v;
};

#endif