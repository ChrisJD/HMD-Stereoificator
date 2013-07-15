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

#ifndef DATAGATHERER_H_INCLUDED
#define DATAGATHERER_H_INCLUDED

#include "MurmurHash3.h"
#include "D3D9ProxyVertexShader.h"

#include <d3dx9.h>
#include <list>
#include <unordered_set>
#include <fstream>
#include <iterator>
#include <algorithm>


class DataGatherer
{
public:
	DataGatherer();
	virtual ~DataGatherer();
	
	void OnCreateRT(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, bool isSwapChainBackBuffer);
	void OnCreateRTTexture(UINT Width, UINT Height, UINT Levels, D3DFORMAT Format);
	void OnCreateDepthStencilSurface(UINT Width, UINT Height ,D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard);
	void OnCreateVertexShader(D3D9ProxyVertexShader* pShader);
	void OnSetVertexShader(D3D9ProxyVertexShader* pShader);
	
	uint32_t NextShaderHash();
	uint32_t PreviousShaderHash();
	bool ShaderMatchesCurrentHash(D3D9ProxyVertexShader* pShader);

	uint32_t CurrentHashCode();
	UINT VShaderInUseCount();

	void StartInUseShaderCapture();
	void EndInUseShaderCapture();
	bool CapturingInUseVShaders();
	

private:

	void CheckForListChange();


	std::unordered_set<uint32_t> m_allRecordedVShaders;

	// Shaders currently is use.
	std::list<uint32_t> m_vshadersInUse;
	std::list<uint32_t>::iterator m_vshadersInUseIterator;
	bool m_capturingInUseShaders;

	std::ofstream m_shaderDumpFile;
	std::ofstream m_renderTargetDumpFile;
	
	uint32_t m_currentHash;
};

#endif