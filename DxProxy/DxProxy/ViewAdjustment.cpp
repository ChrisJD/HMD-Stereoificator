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
	rollEnabled(enableRoll),
	m_basicAdustments()
{
	ipd = IPD_DEFAULT;
	float maxSeparationAdjusment = 4 * (IPD_DEFAULT / 2.0f); // Max is arbitrarily 4 * default ipd separation.
	float minSeparationAdjusment = -(IPD_DEFAULT / 2.0f); // adjustment at minimum value will result in 0 separation
	
	m_basicAdustments.insert(std::pair<BasicAdjustments, LimitedRangeValue>(SEPARATION_ADJUSTMENT, LimitedRangeValue(0.0f, minSeparationAdjusment, maxSeparationAdjusment))); // meters
	m_basicAdustments.insert(std::pair<BasicAdjustments, LimitedRangeValue>(WORLD_SCALE, LimitedRangeValue(metersToWorldUnits, 0.0001f, 1000000.0f))); // multiply a value in meters by this to get world units
	m_basicAdustments.insert(std::pair<BasicAdjustments, LimitedRangeValue>(HUD_DISTANCE, LimitedRangeValue(1.0f, 0.1f, 100.0f))); // "meters" not actually meters, needs a scaling factor, probably per game, different from world factor (for hl2 the scale factor would seem to need to be be about 2 - not implemented at the moment though)
	m_basicAdustments.insert(std::pair<BasicAdjustments, LimitedRangeValue>(HUD_SCALE, LimitedRangeValue(3.1f, 0.1f, 100000.0f))); 


	n = 0.1f;
	f = 100.0f;
	l = -0.5f;
	r = 0.5f;

	
	RecalculateAll();
}

ViewAdjustment::~ViewAdjustment() 
{
	hmdInfo.reset();
}

void ViewAdjustment::Load(ProxyHelper::ProxyConfig& cfg) 
{
	rollEnabled = cfg.rollEnabled;
	m_basicAdustments[WORLD_SCALE].SetValue(cfg.worldScaleFactor);
	m_basicAdustments[SEPARATION_ADJUSTMENT].SetValue(cfg.separationAdjustment);
	m_basicAdustments[HUD_DISTANCE].SetValue(cfg.hudDistance);
	m_basicAdustments[HUD_SCALE].SetValue(cfg.hudScale);
	ipd = cfg.ipd;

	RecalculateAll();
}

void ViewAdjustment::RecalculateAll()
{
	float maxSeparationAdjusment = 4 * (IPD_DEFAULT / 2.0f); // Max is arbitrarily 4 * default ipd.
	float minSeparationAdjusment = -(ipd / 2.0f); // adjustment at minimum value will result in 0 separation
	
	m_basicAdustments[SEPARATION_ADJUSTMENT].SetNewLimits(0.0f, minSeparationAdjusment, maxSeparationAdjusment);
	
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
	D3DXMatrixIdentity(&rollMatrix);

	UpdateProjectionMatrices(hmdInfo->screenAspectRatio);
	ComputeViewTransforms();
}

void ViewAdjustment::Save(ProxyHelper::ProxyConfig& cfg) 
{
	cfg.rollEnabled = rollEnabled;
	cfg.separationAdjustment = m_basicAdustments[SEPARATION_ADJUSTMENT].Value();
	cfg.hudDistance = m_basicAdustments[HUD_DISTANCE].Value();
	cfg.hudScale = m_basicAdustments[HUD_SCALE].Value();
	
	//worldscale and ipd are not normally changed on the fly during normal operation;
	cfg.worldScaleFactor = m_basicAdustments[WORLD_SCALE].Value();
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

	D3DXMATRIX hudDistance;
	D3DXMatrixTranslation(&hudDistance, 0, 0, m_basicAdustments[HUD_DISTANCE].Value());

	D3DXMATRIX hudScale;
	float scalarHUDScale = m_basicAdustments[HUD_SCALE].Value();
	D3DXMatrixScaling(&hudScale, scalarHUDScale, scalarHUDScale, 1);

	orthoToPersViewProjTransformLeft  = matProjectionInv * hudScale * transformLeft  * hudDistance * projectLeft;
	orthoToPersViewProjTransformRight = matProjectionInv * hudScale * transformRight * hudDistance * projectRight;

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

D3DXMATRIX ViewAdjustment::LeftOrthoReproject()
{
	return orthoToPersViewProjTransformLeft;
}

D3DXMATRIX ViewAdjustment::RightOrthoReproject()
{
	return orthoToPersViewProjTransformRight;
}

D3DXMATRIX ViewAdjustment::Projection()
{
	return matProjection;
}

D3DXMATRIX ViewAdjustment::ProjectionInverse()
{
	return matProjectionInv;
}



float ViewAdjustment::ChangeBasicAdjustment(BasicAdjustments adjustment, float toAdd)
{
	return m_basicAdustments[adjustment].AddToValue(toAdd);
}

void ViewAdjustment::ResetBasicAdjustment(BasicAdjustments adjustment)
{
	m_basicAdustments[adjustment].ResetToDefault();
}

float ViewAdjustment::BasicAdjustmentValue(BasicAdjustments adjustment)
{
	return m_basicAdustments[adjustment].Value();
}



float ViewAdjustment::SeparationInWorldUnits() 
{	
	return (m_basicAdustments[SEPARATION_ADJUSTMENT].Value() + (ipd / 2.0f)) * m_basicAdustments[WORLD_SCALE].Value(); 
}

bool ViewAdjustment::RollEnabled() 
{ 
	return rollEnabled; 
}

std::shared_ptr<HMDisplayInfo> ViewAdjustment::HMDInfo()
{
	return hmdInfo;
}