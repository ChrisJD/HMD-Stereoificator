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

#ifndef SHADERCONSTANTMODIFICATIONFACTORY_H_INCLUDED
#define SHADERCONSTANTMODIFICATIONFACTORY_H_INCLUDED

#include <memory>
#include "d3d9.h"
#include "d3dx9.h"

#include "ShaderConstantModification.h"

#include "Vector4SimpleTranslate.h"

#include "ShaderMatrixModification.h"
#include "MatrixIgnoreOrtho.h"
#include "MatrixDoNothing.h"
#include "MatrixHudSquash.h"
#include "MatrixSimplePlusOrthoReproject.h"
#include "MatrixShiftProjection.h"
#include "MatrixSimplePlusOrthoShift.h"
#include "MatrixTransformHUD.h"



class ShaderConstantModificationFactory
{
public:

	enum Vector4ModificationTypes
	{
		Vec4DoNothing = 0,
		Vec4SimpleTranslate = 1
	};

	enum MatrixModificationTypes
	{
		MatDoNothing = 0,
		MatSimpleTranslate = 1, 
		MatSimpleTranslateIgnoreOrtho = 3,
		MatHudSquash = 4,
		MatSimpleTranslateOrthoReproject = 5,
		MatShiftProjection = 6,
		MatSimplePlusOrthoShift = 7,
		MatTransformHUD = 8
	};


	static std::shared_ptr<ShaderConstantModification<>> CreateVector4Modification(UINT modID, std::shared_ptr<ViewAdjustment> adjustmentMatricies)
	{
		return CreateVector4Modification(static_cast<Vector4ModificationTypes>(modID), adjustmentMatricies);
	}

	static std::shared_ptr<ShaderConstantModification<>> CreateVector4Modification(Vector4ModificationTypes mod, std::shared_ptr<ViewAdjustment> adjustmentMatricies)
	{
		switch (mod)
		{
		case Vec4SimpleTranslate:
			return std::make_shared<Vector4SimpleTranslate>(mod, adjustmentMatricies);

		default:
			OutputDebugString("Nonexistant Vec4 modification\n");
			assert(false);
			throw std::out_of_range ("Nonexistant Vec4 modification");
		}
	}

	static std::shared_ptr<ShaderConstantModification<>> CreateMatrixModification(UINT modID, std::shared_ptr<ViewAdjustment> adjustmentMatricies, bool transpose) 
	{
		return CreateMatrixModification(static_cast<MatrixModificationTypes>(modID), adjustmentMatricies, transpose);
	}

	static std::shared_ptr<ShaderConstantModification<>> CreateMatrixModification(MatrixModificationTypes mod, std::shared_ptr<ViewAdjustment> adjustmentMatricies, bool transpose)
	{
		switch (mod)
		{
		case MatDoNothing:
			return std::make_shared<MatrixDoNothing>(mod, adjustmentMatricies);

		case MatSimpleTranslate:
			return std::make_shared<ShaderMatrixModification>(mod, adjustmentMatricies, transpose);
			
		case MatSimpleTranslateIgnoreOrtho:
			return std::make_shared<MatrixIgnoreOrtho>(mod, adjustmentMatricies, transpose);

		case MatHudSquash:
			return std::make_shared<MatrixHudSquash>(mod, adjustmentMatricies, transpose);

		case MatSimpleTranslateOrthoReproject:
			return std::make_shared<MatrixSimplePlusOrthoReproject>(mod, adjustmentMatricies, transpose);

		case MatShiftProjection:
			return std::make_shared<MatrixShiftProjection>(mod, adjustmentMatricies, transpose);

		case MatSimplePlusOrthoShift:
			return std::make_shared<MatrixSimplePlusOrthoShift>(mod, adjustmentMatricies, transpose);

		case MatTransformHUD:
			return std::make_shared<MatrixTransformHUD>(mod, adjustmentMatricies, transpose);

		default:
			OutputDebugString("Nonexistant matrix modification\n");
			assert(false);
			throw std::out_of_range ("Nonexistant matrix modification");
		}
	}
};


#endif