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

#ifndef MATRIXSIMPLEPLUSORTHOSHIFT_H_INCLUDED
#define MATRIXSIMPLEPLUSORTHOSHIFT_H_INCLUDED


#include "d3d9.h"
#include "d3dx9.h"
#include "ShaderConstantModification.h"
#include "ShaderMatrixModification.h"

class MatrixSimplePlusOrthoShift : public ShaderMatrixModification
{
public:
	MatrixSimplePlusOrthoShift(UINT modID, std::shared_ptr<ViewAdjustment> adjustmentMatricies, bool transpose) : ShaderMatrixModification(modID, adjustmentMatricies, transpose) {};

	virtual void DoMatrixModification(D3DXMATRIX in, D3DXMATRIX& outLeft, D3DXMATRIX& outright)
	{
		if (stereoificator::AlmostSame(in[15], 1.0f, 0.00001f)) {

			outLeft = in * m_spAdjustmentMatricies->LeftShiftProjection();
			outright = in * m_spAdjustmentMatricies->RightShiftProjection();
		}
		else {
			ShaderMatrixModification::DoMatrixModification(in, outLeft, outright);
		}
	};
};


#endif