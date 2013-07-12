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
#include <fstream>
#include <iterator>
#include <algorithm>


class DataGatherer
{
public:
	DataGatherer();
	virtual ~DataGatherer();
	
	virtual void OnCreateVertexShader(D3D9ProxyVertexShader* pShader);
	
	uint32_t NextShaderHash();
	uint32_t PreviousShaderHash();
	bool ShaderMatchesCurrentHash(D3D9ProxyVertexShader* pShader);

	uint32_t CurrentHashCode();
	UINT VertexShaderCount();
	

private:

	void CheckForListChange();


	std::list<uint32_t> m_recordedShaders;
	std::list<uint32_t>::iterator m_recordedShaderIterator;

	std::ofstream m_shaderDumpFile;
	
	bool m_recordedShaderUpdateHandled;
	uint32_t m_currentHash;
};

#endif