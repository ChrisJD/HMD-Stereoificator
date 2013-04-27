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

#include "D3DProxyDeviceFrameDumper.h"


#define logMethodName() if (logMethods) std::clog << __FUNCTION__ << std::endl
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? true : false)


D3DProxyDeviceFrameDumper::D3DProxyDeviceFrameDumper(IDirect3DDevice9* pDevice):D3DProxyDevice(pDevice)
{
	OutputDebugString("D3D ProxyDev Frame Dumper Created\n");

	logFile = NULL;
	logMethods = false;
	logKeyDownLastFrame = false;
	numFramesCapped = 0;
}


D3DProxyDeviceFrameDumper::~D3DProxyDeviceFrameDumper()
{
	if (logFile != NULL)
		std::fclose(logFile);
}





HRESULT WINAPI D3DProxyDeviceFrameDumper::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	if (logMethods) {
		numFramesCapped++;

		if (numFramesCapped >= NUM_FRAMES_TO_CAP) {
			logMethods = false;
			if (logFile != NULL)
				fclose(logFile);
		}
	}

	bool logKeyDown = KEY_DOWN(VK_PRIOR); //page up

	if (!logKeyDownLastFrame && logKeyDown && !logMethods) {
		logMethods = true;
		numFramesCapped = 0;
		logFile = freopen ("d3dMethodLog.txt", "w", stderr);
	}

	logKeyDownLastFrame = logKeyDown;


	return BaseDirect3DDevice9::Present( pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}



HRESULT WINAPI D3DProxyDeviceFrameDumper::BeginScene()
{
	logMethodName();
	return BaseDirect3DDevice9::BeginScene();
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::EndScene()
{
	logMethodName();
	return BaseDirect3DDevice9::EndScene();
}


HRESULT WINAPI D3DProxyDeviceFrameDumper::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	logMethodName();
	return BaseDirect3DDevice9::Reset(pPresentationParameters);
}




HRESULT WINAPI D3DProxyDeviceFrameDumper::TestCooperativeLevel()
{
	logMethodName();
	return BaseDirect3DDevice9::TestCooperativeLevel();
}

UINT WINAPI D3DProxyDeviceFrameDumper::GetAvailableTextureMem()
{
	logMethodName();
	return BaseDirect3DDevice9::GetAvailableTextureMem();
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::EvictManagedResources()
{
	logMethodName();
	return BaseDirect3DDevice9::EvictManagedResources();
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetDirect3D(IDirect3D9** ppD3D9)
{
	logMethodName();
	return BaseDirect3DDevice9::GetDirect3D(ppD3D9);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetDeviceCaps(D3DCAPS9* pCaps)
{
	logMethodName();
	return BaseDirect3DDevice9::GetDeviceCaps(pCaps);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode)
{
	logMethodName();
	return BaseDirect3DDevice9::GetDisplayMode(iSwapChain, pMode);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	logMethodName();
	return BaseDirect3DDevice9::GetCreationParameters(pParameters);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
{
	logMethodName();
	return BaseDirect3DDevice9::SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void WINAPI D3DProxyDeviceFrameDumper::SetCursorPosition(int X,int Y,DWORD Flags)
{
	logMethodName();
	return BaseDirect3DDevice9::SetCursorPosition(X, Y, Flags);
}

BOOL WINAPI D3DProxyDeviceFrameDumper::ShowCursor(BOOL bShow)
{
	logMethodName();
	return BaseDirect3DDevice9::ShowCursor(bShow);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
{
	logMethodName();
	return BaseDirect3DDevice9::GetSwapChain(iSwapChain, pSwapChain);
}

UINT WINAPI D3DProxyDeviceFrameDumper::GetNumberOfSwapChains()
{
	logMethodName();
	return BaseDirect3DDevice9::GetNumberOfSwapChains();
}



HRESULT WINAPI D3DProxyDeviceFrameDumper::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{
	logMethodName();
	return BaseDirect3DDevice9::GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetRasterStatus(UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
{
	logMethodName();
	return BaseDirect3DDevice9::GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetDialogBoxMode(BOOL bEnableDialogs)
{
	logMethodName();
	return BaseDirect3DDevice9::SetDialogBoxMode(bEnableDialogs);
}

void WINAPI D3DProxyDeviceFrameDumper::SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{
	logMethodName();
	return BaseDirect3DDevice9::SetGammaRamp(iSwapChain, Flags, pRamp);
}

void WINAPI D3DProxyDeviceFrameDumper::GetGammaRamp(UINT iSwapChain,D3DGAMMARAMP* pRamp)
{
	logMethodName();
	return BaseDirect3DDevice9::GetGammaRamp(iSwapChain, pRamp);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
{
	logMethodName();
	return BaseDirect3DDevice9::UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
{
	logMethodName();
	return BaseDirect3DDevice9::UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
{
	logMethodName();
	return BaseDirect3DDevice9::GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface)
{
	logMethodName();
	return BaseDirect3DDevice9::GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{
	logMethodName();
	return BaseDirect3DDevice9::StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
{
	logMethodName();
	return BaseDirect3DDevice9::ColorFill(pSurface, pRect, color);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
	logMethodName();
	return BaseDirect3DDevice9::SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
{
	logMethodName();
	return BaseDirect3DDevice9::GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	logMethodName();
	return BaseDirect3DDevice9::SetDepthStencilSurface(pNewZStencil);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	logMethodName();
	return BaseDirect3DDevice9::GetDepthStencilSurface(ppZStencilSurface);
}



HRESULT WINAPI D3DProxyDeviceFrameDumper::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{
	logMethodName();
	return BaseDirect3DDevice9::Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
	logMethodName();
	return BaseDirect3DDevice9::SetTransform(State, pMatrix);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
{
	logMethodName();
	return BaseDirect3DDevice9::GetTransform(State, pMatrix);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
	logMethodName();
	return BaseDirect3DDevice9::MultiplyTransform(State, pMatrix);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	logMethodName();
	return BaseDirect3DDevice9::SetViewport(pViewport);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetViewport(D3DVIEWPORT9* pViewport)
{
	logMethodName();
	return BaseDirect3DDevice9::GetViewport(pViewport);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	logMethodName();
	return BaseDirect3DDevice9::SetMaterial(pMaterial);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetMaterial(D3DMATERIAL9* pMaterial)
{
	logMethodName();
	return BaseDirect3DDevice9::GetMaterial(pMaterial);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetLight(DWORD Index,CONST D3DLIGHT9* pLight)
{
	logMethodName();
	return BaseDirect3DDevice9::SetLight(Index, pLight);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetLight(DWORD Index,D3DLIGHT9* pLight)
{
	logMethodName();
	return BaseDirect3DDevice9::GetLight(Index, pLight);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::LightEnable(DWORD Index,BOOL Enable)
{
	logMethodName();
	return BaseDirect3DDevice9::LightEnable(Index, Enable);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetLightEnable(DWORD Index,BOOL* pEnable)
{
	logMethodName();
	return BaseDirect3DDevice9::GetLightEnable(Index, pEnable);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetClipPlane(DWORD Index,CONST float* pPlane)
{
	logMethodName();
	return BaseDirect3DDevice9::SetClipPlane(Index, pPlane);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetClipPlane(DWORD Index,float* pPlane)
{
	logMethodName();
	return BaseDirect3DDevice9::GetClipPlane(Index, pPlane);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{
	logMethodName();
	return BaseDirect3DDevice9::SetRenderState(State, Value);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue)
{
	logMethodName();
	return BaseDirect3DDevice9::GetRenderState(State, pValue);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateStateBlock(Type, ppSB);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::BeginStateBlock()
{
	logMethodName();
	return BaseDirect3DDevice9::BeginStateBlock();
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	logMethodName();
	return BaseDirect3DDevice9::EndStateBlock(ppSB);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
	logMethodName();
	return BaseDirect3DDevice9::SetClipStatus(pClipStatus);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
	logMethodName();
	return BaseDirect3DDevice9::GetClipStatus(pClipStatus);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture)
{
	logMethodName();
	return BaseDirect3DDevice9::GetTexture(Stage, ppTexture);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{
	logMethodName();
	return BaseDirect3DDevice9::SetTexture(Stage, pTexture);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{
	logMethodName();
	return BaseDirect3DDevice9::GetTextureStageState(Stage, Type, pValue);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{
	logMethodName();
	return BaseDirect3DDevice9::SetTextureStageState(Stage, Type, Value);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{
	logMethodName();
	return BaseDirect3DDevice9::GetSamplerState(Sampler, Type, pValue);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{
	logMethodName();
	return BaseDirect3DDevice9::SetSamplerState(Sampler, Type, Value);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::ValidateDevice(DWORD* pNumPasses)
{
	logMethodName();
	return BaseDirect3DDevice9::ValidateDevice(pNumPasses);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{
	logMethodName();
	return BaseDirect3DDevice9::SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries)
{
	logMethodName();
	return BaseDirect3DDevice9::GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetCurrentTexturePalette(UINT PaletteNumber)
{
	logMethodName();
	return BaseDirect3DDevice9::SetCurrentTexturePalette(PaletteNumber);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetCurrentTexturePalette(UINT *PaletteNumber)
{
	logMethodName();
	return BaseDirect3DDevice9::GetCurrentTexturePalette(PaletteNumber);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetScissorRect(CONST RECT* pRect)
{
	logMethodName();
	return BaseDirect3DDevice9::SetScissorRect(pRect);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetScissorRect(RECT* pRect)
{
	logMethodName();
	return BaseDirect3DDevice9::GetScissorRect(pRect);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	logMethodName();
	return BaseDirect3DDevice9::SetSoftwareVertexProcessing(bSoftware);
}

BOOL WINAPI D3DProxyDeviceFrameDumper::GetSoftwareVertexProcessing()
{
	logMethodName();
	return BaseDirect3DDevice9::GetSoftwareVertexProcessing();
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetNPatchMode(float nSegments)
{
	logMethodName();
	return BaseDirect3DDevice9::SetNPatchMode(nSegments);
}

float WINAPI D3DProxyDeviceFrameDumper::GetNPatchMode()
{
	logMethodName();
	return BaseDirect3DDevice9::GetNPatchMode();
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
	logMethodName();
	return BaseDirect3DDevice9::DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
	logMethodName();
	return BaseDirect3DDevice9::DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	logMethodName();
	return BaseDirect3DDevice9::DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	logMethodName();
	return BaseDirect3DDevice9::DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{
	logMethodName();
	return BaseDirect3DDevice9::ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	logMethodName();
	return BaseDirect3DDevice9::SetVertexDeclaration(pDecl);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	logMethodName();
	return BaseDirect3DDevice9::GetVertexDeclaration(ppDecl);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetFVF(DWORD FVF)
{
	logMethodName();
	return BaseDirect3DDevice9::SetFVF(FVF);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetFVF(DWORD* pFVF)
{
	logMethodName();
	return BaseDirect3DDevice9::GetFVF(pFVF);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateVertexShader(pFunction, ppShader);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	logMethodName();
	return BaseDirect3DDevice9::SetVertexShader(pShader);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	logMethodName();
	return BaseDirect3DDevice9::GetVertexShader(ppShader);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	logMethodName();
	return BaseDirect3DDevice9::SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
	logMethodName();
	return BaseDirect3DDevice9::GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	logMethodName();
	return BaseDirect3DDevice9::SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
	logMethodName();
	return BaseDirect3DDevice9::GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
	logMethodName();
	return BaseDirect3DDevice9::SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
	logMethodName();
	return BaseDirect3DDevice9::GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
{
	logMethodName();
	return BaseDirect3DDevice9::SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* pOffsetInBytes,UINT* pStride)
{
	logMethodName();
	return BaseDirect3DDevice9::GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetStreamSourceFreq(UINT StreamNumber,UINT Setting)
{
	logMethodName();
	return BaseDirect3DDevice9::SetStreamSourceFreq(StreamNumber, Setting);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetStreamSourceFreq(UINT StreamNumber,UINT* pSetting)
{
	logMethodName();
	return BaseDirect3DDevice9::GetStreamSourceFreq(StreamNumber, pSetting);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	logMethodName();
	return BaseDirect3DDevice9::SetIndices(pIndexData);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	logMethodName();
	return BaseDirect3DDevice9::GetIndices(ppIndexData);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{
	logMethodName();
	return BaseDirect3DDevice9::CreatePixelShader(pFunction, ppShader);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	logMethodName();
	return BaseDirect3DDevice9::SetPixelShader(pShader);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	logMethodName();
	return BaseDirect3DDevice9::GetPixelShader(ppShader);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	logMethodName();
	return BaseDirect3DDevice9::SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
	logMethodName();
	return BaseDirect3DDevice9::GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	logMethodName();
	return BaseDirect3DDevice9::SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
	logMethodName();
	return BaseDirect3DDevice9::GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
	logMethodName();
	return BaseDirect3DDevice9::SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
	logMethodName();
	return BaseDirect3DDevice9::GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	logMethodName();
	return BaseDirect3DDevice9::DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	logMethodName();
	return BaseDirect3DDevice9::DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::DeletePatch(UINT Handle)
{
	logMethodName();
	return BaseDirect3DDevice9::DeletePatch(Handle);
}

HRESULT WINAPI D3DProxyDeviceFrameDumper::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{
	logMethodName();
	return BaseDirect3DDevice9::CreateQuery(Type, ppQuery);
}