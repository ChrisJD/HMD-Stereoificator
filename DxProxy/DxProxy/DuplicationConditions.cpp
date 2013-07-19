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

#include "DuplicationConditions.h"



DuplicationConditions::DuplicationConditions()
{	
}

DuplicationConditions::~DuplicationConditions()
{
}


//TODO implementation - For now use one set of rules for everything, at some point this is probably going to need to be reworked to allow modifications per game.
// TODO - externalise these as rules? It might be good to have default values for various rules like
// "Don't duplicate targets smaller than X pixels", where that rule could be enabled and the value changed in the cfg file for the game
// Do something similar to shader modifications?
bool DuplicationConditions::ShouldDuplicateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality,BOOL Lockable, bool isSwapChainBackBuffer)
{
	if (isSwapChainBackBuffer) {
		return true;
	}

	return true;
}

bool DuplicationConditions::ShouldDuplicateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard)
{
	return true;
}

bool DuplicationConditions::ShouldDuplicateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage, D3DFORMAT Format,D3DPOOL Pool)
{
	return IS_RENDER_TARGET(Usage);
}

bool DuplicationConditions::ShouldDuplicateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool)
{
	return IS_RENDER_TARGET(Usage);
}

