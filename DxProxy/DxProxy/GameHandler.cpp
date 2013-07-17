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

#include "GameHandler.h"

#define IS_RENDER_TARGET(d3dusage) ((d3dusage & D3DUSAGE_RENDERTARGET) > 0 ? true : false)
#define IS_POOL_DEFAULT(d3dpool) ((d3dpool & D3DPOOL_DEFAULT) > 0 ? true : false)


GameHandler::GameHandler()
{	
}

GameHandler::~GameHandler()
{
}


//TODO implementation - For now use one set of rules for everything, at some point this is probably going to need to be reworked to allow modifications per game.
// TODO - externalise these as rules? It might be good to have default values for various rules like
// "Don't duplicate targets smaller than X pixels", where that rule could be enabled and the value changed in the cfg file for the game
// Do something similar to shader modifications?
// Extending class and overriding these methods would end up with a similar problem with duplicate code to original shader handling. Try and avoid (or at least find approach that avoids the duplication)
bool GameHandler::ShouldDuplicateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality,BOOL Lockable, bool isSwapChainBackBuffer)
{
	if (isSwapChainBackBuffer) {
		return true;
	}

	//return !((Width == Height) || (Width <= 1024)); // Trying some random things out - this one fixes guy on screens in hl2 (but makes him left shifted - his shaders would need a non-stereo value or a modification that returns unmodified in place of left)
	// enabling the line above breaks reflections in f1 2010
	//TODO implementation
	//return true;
	return Width != Height;
}

bool GameHandler::ShouldDuplicateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard)
{
	//TODO implementation
	//return MultiSample == 4;
	//return Width != Height;
	return Width != Height;
}

bool GameHandler::ShouldDuplicateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage, D3DFORMAT Format,D3DPOOL Pool)
{
	//TODO implementation
	// IF render target then check render target rules?
	//return false;
	return IS_RENDER_TARGET(Usage) && (Width != Height);
}

bool GameHandler::ShouldDuplicateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool)
{
	//TODO implementation
	// IF render target then check render target rules?
	return false;
	//return IS_RENDER_TARGET(Usage);
}

