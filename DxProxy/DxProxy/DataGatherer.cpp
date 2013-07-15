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

#include "DataGatherer.h"

DataGatherer::DataGatherer() :
	m_vshadersInUse(),
	m_vshadersInUseIterator(m_vshadersInUse.begin()),
	m_allRecordedVShaders(),
	m_currentHash(0),
	m_capturingInUseShaders(false)
{
	m_shaderDumpFile.open("vertexShaderDump.csv", std::ios::out);
	m_renderTargetDumpFile.open("renderTargetDump.csv", std::ios::out);

	m_shaderDumpFile << "Shader Hash,Constant Name,ConstantType,Start Register,Register Count" << std::endl;
	m_renderTargetDumpFile << "Type,Width,Height,Format,Multisample,MultisampleQuality,IsBackBuffer,Levels,Discard" << std::endl;
}

DataGatherer::~DataGatherer()
{
	m_shaderDumpFile.close();
	m_renderTargetDumpFile.close();
}



void DataGatherer::OnCreateRT(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, bool isSwapChainBackBuffer)
{
	m_renderTargetDumpFile << "RenderTarget" << "," << Width << "," << Height << "," << Format << "," << MultiSample << "," << MultisampleQuality << "," << (isSwapChainBackBuffer ? "yes" : "no") << ","  /* Levels N/A */ << "," <<  /* Discard N/A */ std::endl;
}

void DataGatherer::OnCreateRTTexture(UINT Width, UINT Height, UINT Levels, D3DFORMAT Format)
{
	m_renderTargetDumpFile << "TextureRenderTarget" << "," << Width << "," << Height << "," << Format << "," /*<< MultiSample*/ << "," /*<< MultisampleQuality*/ << "," /*<< (isSwapChainBackBuffer ? "yes" : "no")*/ << ","  << Levels << "," <<  /* Discard N/A */ std::endl;
}

void DataGatherer::OnCreateDepthStencilSurface(UINT Width, UINT Height ,D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard)
{
	m_renderTargetDumpFile << "DepthStencil" << "," << Width << "," << Height << "," << Format << "," << MultiSample << "," << MultisampleQuality << "," /*<< (isSwapChainBackBuffer ? "yes" : "no")*/ << ","  /*<< Levels*/ << "," <<  (Discard ? "yes" : "no") << std::endl;
}

void DataGatherer::OnCreateVertexShader(D3D9ProxyVertexShader* pWrappedShader)
{
	LPD3DXCONSTANTTABLE pConstantTable = NULL;

	BYTE* pData = NULL;
	UINT pSizeOfData;
	pWrappedShader->getActual()->GetFunction(NULL, &pSizeOfData);
			
	pData = new BYTE[pSizeOfData];
	pWrappedShader->getActual()->GetFunction(pData, &pSizeOfData);

	D3DXGetShaderConstantTable(reinterpret_cast<DWORD*>(pData), &pConstantTable);
			
	if(pConstantTable != NULL) {

		D3DXCONSTANTTABLE_DESC pDesc;
		pConstantTable->GetDesc(&pDesc);

		D3DXCONSTANT_DESC pConstantDesc[512];
		UINT pConstantNum = 512;
		
		uint32_t hash = pWrappedShader->GetHash();

		// if shader not yet recorded, record shader details.
		if ((hash != 0) && m_allRecordedVShaders.insert(hash).second && m_shaderDumpFile.is_open()) {

			for(UINT i = 0; i < pDesc.Constants; i++)
			{
				D3DXHANDLE handle = pConstantTable->GetConstant(NULL,i);
				if(handle == NULL) continue;

				pConstantTable->GetConstantDesc(handle, pConstantDesc, &pConstantNum);
				if (pConstantNum >= 512) {
					OutputDebugString("Need larger constant description buffer");
				}

				
				for(UINT j = 0; j < pConstantNum; j++)
				{
					if ((pConstantDesc[j].RegisterSet == D3DXRS_FLOAT4) &&
						((pConstantDesc[j].Class == D3DXPC_VECTOR) || (pConstantDesc[j].Class == D3DXPC_MATRIX_ROWS) || (pConstantDesc[j].Class == D3DXPC_MATRIX_COLUMNS))  ) {

						m_shaderDumpFile << hash;
						//m_shaderDumpFile << "Constant";
						//m_shaderDumpFile << "Name: " << pConstantDesc[j].Name << std::endl;
						m_shaderDumpFile << "," << pConstantDesc[j].Name;
						//m_shaderDumpFile << "Type: ";

						if (pConstantDesc[j].Class == D3DXPC_VECTOR) {
							m_shaderDumpFile << ",Vector";
						}
						else if (pConstantDesc[j].Class == D3DXPC_MATRIX_ROWS) {
							m_shaderDumpFile << ",MatrixR";
						}
						else if (pConstantDesc[j].Class == D3DXPC_MATRIX_COLUMNS) {
							m_shaderDumpFile << ",MatrixC";
						}

						m_shaderDumpFile << "," << pConstantDesc[j].RegisterIndex;
						m_shaderDumpFile << "," << pConstantDesc[j].RegisterCount << std::endl;
						//m_shaderDumpFile << "Number of elements in the array:" << pConstantDesc[j].Elements << std::endl << std::endl;
					}
						
				}
			}
		}
		// else shader already recorded
	}

	_SAFE_RELEASE(pConstantTable);
	if (pData) delete[] pData;
}


void DataGatherer::OnSetVertexShader(D3D9ProxyVertexShader* pShader)
{
	if (m_capturingInUseShaders && pShader && (std::find(m_vshadersInUse.begin(), m_vshadersInUse.end(), pShader->GetHash()) == m_vshadersInUse.end())) {
		m_vshadersInUse.push_back(pShader->GetHash());
	}
}

void DataGatherer::StartInUseShaderCapture()
{
	m_capturingInUseShaders = true;
	m_currentHash = 0;
	m_vshadersInUse.clear();
}

void DataGatherer::EndInUseShaderCapture()
{
	m_capturingInUseShaders = false;
	m_vshadersInUseIterator = m_vshadersInUse.begin();
}

bool DataGatherer::CapturingInUseVShaders()
{
	return m_capturingInUseShaders;
}


/*void DataGatherer::CheckForListChange()
{
	// iterator has been invalidated by changes to set
	if (!m_recordedShaderUpdateHandled) {
		OutputDebugString("Handling shader list change\n");
		m_recordedShaderUpdateHandled = true;
		m_vshadersInUseIterator = m_vshadersInUse.begin();

			
		if (m_currentHash != 0) {

			// move iterator back to the same hash it was on before
			bool found = false;
			while (m_vshadersInUseIterator != m_vshadersInUse.end()) {

				if (*m_vshadersInUseIterator == m_currentHash) {
					found = true;
					break;
				}

				++m_vshadersInUseIterator;
			}


			if (!found) {
				m_vshadersInUseIterator = m_vshadersInUse.begin();
			}
		}
	}
}*/


uint32_t DataGatherer::NextShaderHash()
{
	if ((m_vshadersInUse.size() == 0) || m_capturingInUseShaders) {
		m_currentHash = 0;
		return m_currentHash;
	}

	// Move to next hash. If that puts us at the end go back to the beginning
	++m_vshadersInUseIterator;
	if (m_vshadersInUseIterator == m_vshadersInUse.end()) {
		m_vshadersInUseIterator = m_vshadersInUse.begin();
		OutputDebugString("End of shader hash list\n");
	}

	m_currentHash = *m_vshadersInUseIterator;

	return m_currentHash;
}

uint32_t DataGatherer::PreviousShaderHash()
{
	if ((m_vshadersInUse.size() == 0) || m_capturingInUseShaders) {
		m_currentHash = 0;
		return m_currentHash;
	}
	

	// Move to previous hash. If that puts us at the start go to the end
	--m_vshadersInUseIterator;
	if (m_vshadersInUseIterator == m_vshadersInUse.begin()) {
		m_vshadersInUseIterator = m_vshadersInUse.end();
		OutputDebugString("Wrap around\n");
	}

	m_currentHash = *m_vshadersInUseIterator;

	return m_currentHash;
}



bool DataGatherer::ShaderMatchesCurrentHash(D3D9ProxyVertexShader* pShader)
{
	if (!pShader) {
		return false;
	}

	return pShader->GetHash() == m_currentHash;
}

uint32_t DataGatherer::CurrentHashCode()
{
	return m_currentHash;
}

UINT DataGatherer::VShaderInUseCount()
{
	return m_vshadersInUse.size();
}