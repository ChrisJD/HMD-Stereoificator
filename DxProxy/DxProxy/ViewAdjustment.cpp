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


#include "ViewAdjustment.h"


ViewAdjustment::ViewAdjustment(std::shared_ptr<HMDisplayInfo> displayInfo, float metersToWorldUnits, bool enableRoll) :
	hmdInfo(displayInfo),
	metersToWorldMultiplier(metersToWorldUnits),
	rollEnabled(enableRoll)
{
	separationAdjustment = 0.0f;
	minSeparationAdjusment = -(IPD_DEFAULT / 2.0f);
	maxSeparationAdjusment = 4 * (IPD_DEFAULT / 2.0f);

	ipd = IPD_DEFAULT;

	n = 0.1f;					
	f = 10.0f;
	l = -0.5f;
	r = 0.5f;

	D3DXMatrixIdentity(&matProjection);
	D3DXMatrixIdentity(&matProjectionInv);
	D3DXMatrixIdentity(&leftShiftProjection);
	D3DXMatrixIdentity(&rightShiftProjection);
	D3DXMatrixIdentity(&projectLeft);
	D3DXMatrixIdentity(&projectRight);
	D3DXMatrixIdentity(&transformLeft);
	D3DXMatrixIdentity(&transformRight);
	D3DXMatrixIdentity(&matViewProjTransformRight);
	D3DXMatrixIdentity(&matViewProjTransformLeft);

	UpdateProjectionMatrices(hmdInfo->screenAspectRatio);
	D3DXMatrixIdentity(&rollMatrix);
	ComputeViewTransforms();
}

ViewAdjustment::~ViewAdjustment() 
{
	hmdInfo.reset();
}

void ViewAdjustment::Load(ProxyHelper::ProxyConfig& cfg) 
{
	rollEnabled = cfg.rollEnabled;
	metersToWorldMultiplier  = cfg.worldScaleFactor;
	separationAdjustment = cfg.separationAdjustment;
	ipd = cfg.ipd;
}

void ViewAdjustment::Save(ProxyHelper::ProxyConfig& cfg) 
{
	cfg.rollEnabled = rollEnabled;
	cfg.separationAdjustment = separationAdjustment;
	
	//worldscale and ipd are not normally edited;
	cfg.worldScaleFactor = metersToWorldMultiplier;
	cfg.ipd = ipd;
}

void ViewAdjustment::UpdateProjectionMatrices(float aspectRatio)
{
	t = 0.5f / aspectRatio;
	b = -0.5f / aspectRatio;

	D3DXMatrixPerspectiveOffCenterLH(&matProjection, l, r, b, t, n, f);
	D3DXMatrixInverse(&matProjectionInv, 0, &matProjection);

	// The lensXCenterOffset is in the same -1 to 1 space as the perspective so shift by that amount to move projection in line with the lenses
	D3DXMatrixTranslation(&leftShiftProjection, hmdInfo->lensXCenterOffset * LEFT_CONSTANT, 0, 0);
	D3DXMatrixTranslation(&rightShiftProjection, hmdInfo->lensXCenterOffset * RIGHT_CONSTANT, 0, 0);
	
	projectLeft = matProjection * leftShiftProjection;
	projectRight = matProjection * rightShiftProjection;
}


void ViewAdjustment::UpdateRoll(float roll)
{
	D3DXMatrixIdentity(&rollMatrix);
	D3DXMatrixRotationZ(&rollMatrix, roll);
}


void ViewAdjustment::ComputeViewTransforms()
{
	D3DXMatrixTranslation(&transformLeft, SeparationInWorldUnits() * LEFT_CONSTANT, 0, 0);
	D3DXMatrixTranslation(&transformRight, SeparationInWorldUnits() * RIGHT_CONSTANT, 0, 0);

	D3DXMATRIX rollTransform;
	D3DXMatrixIdentity(&rollTransform);

	if (rollEnabled) {
		D3DXMatrixMultiply(&transformLeft, &rollMatrix, &transformLeft);
		D3DXMatrixMultiply(&transformRight, &rollMatrix, &transformRight);
	}

	matViewProjTransformLeft = matProjectionInv * transformLeft * projectLeft;
	matViewProjTransformRight = matProjectionInv * transformRight * projectRight;

	D3DXMATRIX tempForward;
	D3DXMatrixTranslation(&tempForward, 0, 0, metersToWorldMultiplier * 0.1f);

	//orthoToPersViewProjTransformLeft = matProjectionInv /* transformLeft*/ * tempForward * projectLeft;
	//orthoToPersViewProjTransformRight = matProjectionInv /* transformRight*/ * tempForward * projectRight;

	D3DXMATRIX orthoLeft;
	D3DXMATRIX orthoRight;
	D3DXMatrixTranslation(&orthoLeft, hmdInfo->lensXCenterOffset * LEFT_CONSTANT, 0, 0);
	D3DXMatrixTranslation(&orthoRight, hmdInfo->lensXCenterOffset * RIGHT_CONSTANT, 0, 0);

	D3DXMATRIX squash;
	D3DXMatrixScaling(&squash, 0.5f, 0.5f, 1);
	orthoToPersViewProjTransformLeft = /*matProjectionInv * transformLeft * tempForward */ squash * orthoLeft;
	orthoToPersViewProjTransformRight = /*matProjectionInv * transformRight * tempForward */ squash * orthoRight;

}

D3DXMATRIX ViewAdjustment::LeftViewTransform()
{
	return transformLeft;
}
	
D3DXMATRIX ViewAdjustment::RightViewTransform()
{
	return transformRight;
}

D3DXMATRIX ViewAdjustment::LeftAdjustmentMatrix()
{
	return matViewProjTransformLeft;
}

D3DXMATRIX ViewAdjustment::RightAdjustmentMatrix()
{
	return matViewProjTransformRight;
}

D3DXMATRIX ViewAdjustment::LeftShiftProjection()
{
	return leftShiftProjection;
}

D3DXMATRIX ViewAdjustment::RightShiftProjection()
{
	return rightShiftProjection;
}

D3DXMATRIX ViewAdjustment::Projection()
{
	return matProjection;
}

D3DXMATRIX ViewAdjustment::ProjectionInverse()
{
	return matProjectionInv;
}

float ViewAdjustment::ChangeSeparationAdjustment(float toAdd)
{
	separationAdjustment += toAdd;
	
	vireio::clamp(&separationAdjustment, minSeparationAdjusment, maxSeparationAdjusment);

	return separationAdjustment;
}

void ViewAdjustment::ResetSeparationAdjustment()
{
	separationAdjustment = 0.0f;
}

float ViewAdjustment::ChangeWorldScale(float toAdd)
{
	metersToWorldMultiplier+= toAdd;

	vireio::clamp(&metersToWorldMultiplier, 0.01f, 1000000.0f);

	return metersToWorldMultiplier;
}

float ViewAdjustment::SeparationInWorldUnits() 
{ 
	return (separationAdjustment + (IPD_DEFAULT / 2.0f)) * metersToWorldMultiplier; 
}

float ViewAdjustment::SeparationAdjustment() 
{ 
	return separationAdjustment; 
}

bool ViewAdjustment::RollEnabled() 
{ 
	return rollEnabled; 
}

std::shared_ptr<HMDisplayInfo> ViewAdjustment::HMDInfo()
{
	return hmdInfo;
}