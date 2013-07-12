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
	m_recordedShaders(),
	m_recordedShaderIterator(m_recordedShaders.begin()),
	m_currentHash(0),
	m_recordedShaderUpdateHandled(true)
{
	m_shaderDumpFile.open("vertexShaderDump.csv", std::ios::out);

	m_shaderDumpFile << "Shader Hash,Constant Name,ConstantType,Start Register,Register Count" << std::endl;
}

DataGatherer::~DataGatherer()
{
	m_shaderDumpFile.close();
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

		if ((hash != 0) && m_recordedShaders.insert(hash).second && m_shaderDumpFile.is_open()) {
			// insertion succeeded therefore shader not recorded yet so record shader details.

			m_recordedShaderUpdateHandled = false;

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


uint32_t DataGatherer::NextShaderHash()
{
	if (m_recordedShaders.size() == 0) {
		m_currentHash = 0;
		return;
	}


	// iterator has been invalidated by changes to set
	if (!m_recordedShaderUpdateHandled) {
		OutputDebugString("Handling shader list change\n");
		m_recordedShaderUpdateHandled = true;
		m_recordedShaderIterator = m_recordedShaders.begin();

			
		if (m_currentHash != 0) {

			// move iterator back to the same hash it was on before
			bool found = false;
			while (m_recordedShaderIterator != m_recordedShaders.end()) {

				if (*m_recordedShaderIterator == m_currentHash) {
					found = true;
					break;
				}

				++m_recordedShaderIterator;
			}


			if (!found) {
				m_recordedShaderIterator = m_recordedShaders.begin();
			}
		}
	}

	// Move to next hash. If that puts us at the end go back to the beginning
	++m_recordedShaderIterator;
	if (m_recordedShaderIterator == m_recordedShaders.end()) {
		m_recordedShaderIterator = m_recordedShaders.begin();
		OutputDebugString("End of shader hash list\n");
	}

	m_currentHash = *m_recordedShaderIterator;

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