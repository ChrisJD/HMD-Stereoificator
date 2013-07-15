/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez

HMD-Stereoificator
Modifications Copyright (C) 2013 Chris Drain

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



#include "D3DProxyDevice.h"
#include "D3D9ProxySurface.h"
#include "StereoViewFactory.h"
#include "MotionTrackerFactory.h"
#include <typeinfo>
#include <assert.h>

#ifdef _DEBUG
#include "DxErr.h"
#endif

#define IS_RENDER_TARGET(d3dusage) ((d3dusage & D3DUSAGE_RENDERTARGET) > 0 ? true : false)

#pragma comment(lib, "d3dx9.lib")

#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)



#define SMALL_FLOAT 0.001f
#define	SLIGHTLY_LESS_THAN_ONE 0.999f



#define PI 3.141592654
#define RADIANS_TO_DEGREES(rad) ((float) rad * (float) (180.0 / PI))

D3DProxyDevice::D3DProxyDevice(IDirect3DDevice9* pDevice, BaseDirect3D9* pCreatedBy):BaseDirect3DDevice9(pDevice, pCreatedBy),
	m_activeRenderTargets (1, NULL),
	m_activeTextureStages(),
	m_activeVertexBuffers(),
	m_activeSwapChains(),
	m_keyRepeatRate(0.15f), // 150ms
	m_pDataGatherer(nullptr),
	m_pRedPixelShader(nullptr),
	m_redShaderIsActive(false)
{
	OutputDebugString("D3D ProxyDev Created\n");

	std::shared_ptr<HMDisplayInfo> defaultInfo = std::make_shared<HMDisplayInfo>(); // rift info
	m_spShaderViewAdjustment = std::make_shared<ViewAdjustment>(defaultInfo, 1.0f, false);

	m_pGameHandler = new GameHandler();
	

	// Check the maximum number of supported render targets
	D3DCAPS9 capabilities;
	BaseDirect3DDevice9::GetDeviceCaps(&capabilities);
	DWORD maxRenderTargets = capabilities.NumSimultaneousRTs;
	m_activeRenderTargets.resize(maxRenderTargets, NULL);

	D3DXMatrixIdentity(&m_leftView);
	D3DXMatrixIdentity(&m_rightView);
	D3DXMatrixIdentity(&m_leftProjection);
	D3DXMatrixIdentity(&m_rightProjection);	

	m_currentRenderingSide = vireio::Left;
	m_pCurrentMatViewTransform = &m_spShaderViewAdjustment->LeftAdjustmentMatrix(); 
	m_pCurrentView = &m_leftView;
	m_pCurrentProjection = &m_leftProjection;

	m_spManagedShaderRegisters = std::make_shared<ShaderRegisters>(capabilities.MaxVertexShaderConst, pDevice);

	m_pActiveStereoDepthStencil = NULL;
	m_pActiveIndicies = NULL;
	m_pActivePixelShader = NULL;
	m_pActiveVertexShader = NULL;
	m_pActiveVertexDeclaration = NULL;
	hudFont = NULL;
	m_bActiveViewportIsDefault = true;

	m_bViewTransformSet = false;
	m_bProjectionTransformSet = false;

	m_bInBeginEndStateBlock = false;
	m_pCapturingStateTo = NULL;

	m_isFirstBeginSceneOfFrame = true;

	yaw_mode = 0;
	pitch_mode = 0;
	translation_mode = 0;
	trackingOn = true;
	SHOCT_mode = 0;

	keyWait = false;

	// should be false for published builds
	// TODO Allow this to be turned on and off in cfg file along with vertex shader dumping and other debug/maintenance features.
	worldScaleCalculationMode = false;
}

 


D3DProxyDevice::~D3DProxyDevice()
{
	ReleaseEverything();

	if (m_pDataGatherer)
		delete m_pDataGatherer;

	m_spShaderViewAdjustment.reset();

	delete m_pGameHandler;
	m_spManagedShaderRegisters.reset();

	// always do this last
	auto it = m_activeSwapChains.begin();
	while (it != m_activeSwapChains.end()) {

		if ((*it) != NULL) {
			(*it)->Release();
			delete (*it);
		}

		it = m_activeSwapChains.erase(it);
	}
}


/* 
	This method must be called on the proxy device before the device is returned to the calling application

	Subclasses which override this method must call through to super method.
	Anything that needs to be done before the device is used by the actual application should happen here.
 */
void D3DProxyDevice::Init(ProxyHelper::ProxyConfig& cfg)
{
	OutputDebugString("D3D ProxyDev Init\n");

	if (worldScaleCalculationMode)
		cfg.separationAdjustment = 0.0f;

	config = cfg;

	eyeShutter = 1;
	saveDebugFile = false;
	trackerInitialized = false;

	char buf[64];
	LPCSTR psz = NULL;
	sprintf_s(buf, "type: %d, aspect: %f\n", config.game_type, config.aspect_multiplier);
	psz = buf;
	OutputDebugString(psz);

	m_spShaderViewAdjustment->HMDInfo()->UpdateScale(config.horizontalGameFov);
	
	m_spShaderViewAdjustment->Load(config);
	m_pGameHandler->Load(config, m_spShaderViewAdjustment);
	stereoView = StereoViewFactory::Get(config, m_spShaderViewAdjustment->HMDInfo());

	if (cfg.game_type == 11) {
		m_pDataGatherer = new DataGatherer();

		OutputDebugString("Data Gatherering Mode Active.\n");
		OutputDebugString("Data Gatherering Mode Active.\n");
		OutputDebugString("Data Gatherering Mode Active.\n");
		OutputDebugString("Data Gatherering Mode Active.\n");
	}

	OnCreateOrRestore();
}


/*
  Subclasses which override this method must call through to super method.
  Do not directly call this method in subclasses.
  This method should be used to re/create any resources that are held by the device proxy and deleted by Reset.

  The only resources used like this are going to be extra resources that are used by the proxy and are not
  part of the actual calling application. 
  
  Examples in D3DProxyDevice: The Font used in the SHOCT overlay and the stereo buffer.

  Example of something you wouldn't create here:
  Render targets in the m_activeRenderTargets collection. They need to be released to successfully Reset
  the device, but they just wrap IDirect3DSurface9 objects from the underlying application and will be
  re/created by the underlying application.

  This method will be called when the proxy device is initialised with Init (happens before device is
  passed back to actual application by CreateDevice) and after a successful device Reset.
*/
void D3DProxyDevice::OnCreateOrRestore()
{
	OutputDebugString(__FUNCTION__);
	OutputDebugString("\n");

	m_currentRenderingSide = vireio::Left;
	m_pCurrentMatViewTransform = &m_spShaderViewAdjustment->LeftAdjustmentMatrix();
	m_pCurrentView = &m_leftView;
	m_pCurrentProjection = &m_leftProjection;

	// Wrap the swap chain
	IDirect3DSwapChain9* pActualPrimarySwapChain;
	if (FAILED(BaseDirect3DDevice9::GetSwapChain(0, &pActualPrimarySwapChain))) {
		OutputDebugString("Failed to fetch swapchain.\n");
		exit(1); 
	}

	assert (m_activeSwapChains.size() == 0);
	m_activeSwapChains.push_back(new D3D9ProxySwapChain(pActualPrimarySwapChain, this, false));
	assert (m_activeSwapChains.size() == 1);

	// Set the primary rendertarget to the first stereo backbuffer
	IDirect3DSurface9* pWrappedBackBuffer;
	m_activeSwapChains[0]->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pWrappedBackBuffer);
	SetRenderTarget(0, pWrappedBackBuffer);
	pWrappedBackBuffer->Release();
	pWrappedBackBuffer = NULL;


	BaseDirect3DDevice9::GetViewport(&m_LastViewportSet);


	// If there is an initial depth stencil
	IDirect3DSurface9* pDepthStencil;
	if (SUCCEEDED(BaseDirect3DDevice9::GetDepthStencilSurface(&pDepthStencil))) { 
		
		D3DSURFACE_DESC stencilDesc;
		pDepthStencil->GetDesc(&stencilDesc);
		pDepthStencil->Release();

		IDirect3DSurface9* pTemp = NULL;
		CreateDepthStencilSurface(stencilDesc.Width, stencilDesc.Height, stencilDesc.Format, stencilDesc.MultiSampleType, stencilDesc.MultiSampleQuality, false, &pTemp, NULL);
		SetDepthStencilSurface(pTemp);
		pTemp->Release();	
	}


	if (m_pDataGatherer) {

		// Load red pixel shader
		char viewPath[512];
		ProxyHelper helper;
		helper.GetPath(viewPath, "hlsl\\MakeItRed.cso");

		std::ifstream file (viewPath, std::ios::in | std::ios::binary | std::ios::ate);

		if (file.is_open())
		{
			std::ifstream::pos_type size = file.tellg();
			char * memblock;
			memblock = new char [(unsigned int)size];
			file.seekg (0, std::ios::beg);
			file.read (memblock, size);
			file.close();

			IDirect3DPixelShader9* pPShader;
			if (FAILED(CreatePixelShader((DWORD*)memblock, &pPShader))) {

				OutputDebugString("MakeItRed, Create shader failed.\n");
				_SAFE_RELEASE(pPShader);
			}
			else {
				m_pRedPixelShader = static_cast<BaseDirect3DPixelShader9*>(pPShader);
				OutputDebugString("MakeItRed, Create shader  OK\n");
			}

			delete[] memblock;
		}
		else { 
			OutputDebugString("Unable to open MakeItRed file\n");
		}

		
	}


	SetupText();

	stereoView->Init(getActual());
	
	m_spShaderViewAdjustment->UpdateProjectionMatrices((float)stereoView->viewport.Width/(float)stereoView->viewport.Height);
	m_spShaderViewAdjustment->ComputeViewTransforms();
}


void D3DProxyDevice::ReleaseEverything()
{
	// Fonts and any othe D3DX interfaces should be released first.
	_SAFE_RELEASE(hudFont);
	_SAFE_RELEASE(m_pRedPixelShader);

	
	m_spManagedShaderRegisters->ReleaseResources();

	_SAFE_RELEASE(m_pCapturingStateTo);

	// one of these will still have a count of 1 until the backbuffer is released
	for(std::vector<D3D9ProxySurface*>::size_type i = 0; i != m_activeRenderTargets.size(); i++) 
	{
		if (m_activeRenderTargets[i] != NULL) {
			m_activeRenderTargets[i]->Release();
			m_activeRenderTargets[i] = NULL;
		}
	} 


	auto it = m_activeTextureStages.begin();
	while (it != m_activeTextureStages.end()) {
		if (it->second)
			it->second->Release();

		it = m_activeTextureStages.erase(it);
	}


	auto itVB = m_activeVertexBuffers.begin();
	while (itVB != m_activeVertexBuffers.end()) {
		if (itVB->second)
			itVB->second->Release();

		itVB = m_activeVertexBuffers.erase(itVB);
	}


	_SAFE_RELEASE(m_pActiveStereoDepthStencil);
	_SAFE_RELEASE(m_pActiveIndicies);
	_SAFE_RELEASE(m_pActivePixelShader);
	_SAFE_RELEASE(m_pActiveVertexShader);
	_SAFE_RELEASE(m_pActiveVertexDeclaration);
}


/*
	Subclasses which override this method must call through to super method at the end of the subclasses
	implementation.
 */
HRESULT WINAPI D3DProxyDevice::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	//OutputDebugString(__FUNCTION__);
	//OutputDebugString("\n");

	if(stereoView)
		stereoView->ReleaseEverything();

	ReleaseEverything();

	m_bInBeginEndStateBlock = false;
	


	auto it = m_activeSwapChains.begin();
	while (it != m_activeSwapChains.end()) {

		if ((*it) != NULL)
			(*it)->Release();

		delete (*it);

		it = m_activeSwapChains.erase(it);
	}

	HRESULT hr = BaseDirect3DDevice9::Reset(pPresentationParameters);



	// if the device has been successfully reset we need to recreate any resources we created
	if (hr == D3D_OK)  {
		OnCreateOrRestore();
		stereoView->PostReset();
	}
	else {
#ifdef _DEBUG

		char buf[256];
		sprintf_s(buf, "Error: %s error description: %s\n",
				DXGetErrorString(hr), DXGetErrorDescription(hr));

		OutputDebugString(buf);
				
#endif
		OutputDebugString("Device reset failed");
	}

	return hr;
}








void D3DProxyDevice::SetupText()
{
	D3DXCreateFont( this, 22, 0, FW_BOLD, 4, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &hudFont );
}

void D3DProxyDevice::HandleControls()
{
	bool anyKeyPressed = false;
	//float keySpeed = 0.00002f;
	float seperationChange = 0.001f; // 1 mm
	//float keySpeed2 = 0.0005f;
	float mouseSpeed = 0.25f;
	float rollSpeed = 0.01f;

	
	if (worldScaleCalculationMode)
		seperationChange = 0.1f;



	static int saveWaitCount = 0; 
	saveWaitCount--;
	static bool doSaveNext = false;

	if (!keyWait) {
		
		if(KEY_DOWN(VK_NUMPAD0))
		{
			std::stringstream sstm;
			sstm << "HUD Scale: " << m_spShaderViewAdjustment->BasicAdjustmentValue(ViewAdjustment::HUD_SCALE) << std::endl;
			sstm << "HUD Distance: " << m_spShaderViewAdjustment->BasicAdjustmentValue(ViewAdjustment::HUD_DISTANCE) << std::endl;
			if (m_pDataGatherer) {
				sstm << "Selected Shader Hash: " << m_pDataGatherer->CurrentHashCode() << std::endl;
				sstm << "Vertex Shader Count: " << m_pDataGatherer->VShaderInUseCount() << std::endl;
			}
			OutputDebugString(sstm.str().c_str());

			anyKeyPressed = true;
		}

		if (m_pDataGatherer) {
			if(KEY_DOWN(VK_NUMPAD1))
			{
				std::stringstream sstm;
				sstm << "Current Shader Hash: " << m_pDataGatherer->NextShaderHash() << std::endl;
				OutputDebugString(sstm.str().c_str());

				anyKeyPressed = true;
			}

			if(KEY_DOWN(VK_NUMPAD2))
			{
				std::stringstream sstm;
				sstm << "Current Shader Hash: " << m_pDataGatherer->PreviousShaderHash() << std::endl;
				OutputDebugString(sstm.str().c_str());

				anyKeyPressed = true;
			}

			if(KEY_DOWN(VK_NUMPAD9))
			{
				
				
				if (m_pDataGatherer->CapturingInUseVShaders()) {

					m_pDataGatherer->EndInUseShaderCapture();

					std::stringstream sstm;
					sstm << "Capture ended, " << m_pDataGatherer->VShaderInUseCount() << " shaders used during capture period." << std::endl;
					OutputDebugString(sstm.str().c_str());
				}
				else {
					m_pDataGatherer->StartInUseShaderCapture();
					OutputDebugString("Capture started.");
				}


				

				anyKeyPressed = true;
			}
		}


		if(KEY_DOWN(VK_F1))
		{
			if(stereoView->initialized)
			{
				stereoView->SaveScreen();
			}
			anyKeyPressed = true;
		}

		if(KEY_DOWN(VK_F2))
		{
			if(KEY_DOWN(VK_CONTROL)) {
				seperationChange /= 10.0f;
			}
			else if(KEY_DOWN(VK_SHIFT)) {
				seperationChange *= 10.0f;
			} 

			if (worldScaleCalculationMode)
				m_spShaderViewAdjustment->ChangeBasicAdjustment(ViewAdjustment::WORLD_SCALE, -seperationChange);
			else
				m_spShaderViewAdjustment->ChangeBasicAdjustment(ViewAdjustment::SEPARATION_ADJUSTMENT, -seperationChange);
			
			saveWaitCount = 500;
			doSaveNext = true;
			anyKeyPressed = true;
		}

		if(KEY_DOWN(VK_F3))
		{
			if(KEY_DOWN(VK_CONTROL)) {
				seperationChange /= 10.0f;
			}
			else if(KEY_DOWN(VK_SHIFT))
			{
				seperationChange *= 10.0f;
			} 
			
			if (worldScaleCalculationMode)
				m_spShaderViewAdjustment->ChangeBasicAdjustment(ViewAdjustment::WORLD_SCALE, seperationChange);
			else
				m_spShaderViewAdjustment->ChangeBasicAdjustment(ViewAdjustment::SEPARATION_ADJUSTMENT, seperationChange);
			
			saveWaitCount = 500;
			doSaveNext = true;
			anyKeyPressed = true;
		}



		if(KEY_DOWN(VK_F4))
		{
			float change = 0.1f;

			if(KEY_DOWN(VK_CONTROL)) {
				m_spShaderViewAdjustment->ChangeBasicAdjustment(ViewAdjustment::HUD_SCALE, -change * m_spShaderViewAdjustment->BasicAdjustmentValue(ViewAdjustment::HUD_DISTANCE));
			}
			else {
				m_spShaderViewAdjustment->ChangeBasicAdjustment(ViewAdjustment::HUD_DISTANCE, -change);
			}

			m_spShaderViewAdjustment->RecalculateAll();
			
			saveWaitCount = 500;
			doSaveNext = true;
			anyKeyPressed = true;
		}

		if(KEY_DOWN(VK_F5))
		{
			float change = 0.1f;

			if(KEY_DOWN(VK_CONTROL)) {
				m_spShaderViewAdjustment->ChangeBasicAdjustment(ViewAdjustment::HUD_SCALE, change * m_spShaderViewAdjustment->BasicAdjustmentValue(ViewAdjustment::HUD_DISTANCE));
			}
			else {
				m_spShaderViewAdjustment->ChangeBasicAdjustment(ViewAdjustment::HUD_DISTANCE, change);
			}
			
			
			
			saveWaitCount = 500;
			doSaveNext = true;
			anyKeyPressed = true;
		}



		
		if(KEY_DOWN(VK_F6))
		{
			if(KEY_DOWN(VK_SHIFT)) {
				ProxyHelper helper;
				helper.LoadUserConfig(config, true);
				m_spShaderViewAdjustment->Load(config);
				OutputDebugString("reset defaults pressed\n");
			}
			else {
				stereoView->swapEyes = !stereoView->swapEyes;
			}
			
			
			saveWaitCount = 500;
			doSaveNext = true;
			anyKeyPressed = true;
		}

		if(KEY_DOWN(VK_F8))
		{
			if(KEY_DOWN(VK_SHIFT))
			{
				config.pitch_multiplier -= mouseSpeed;
			}  
			else if(KEY_DOWN(VK_CONTROL))
			{
				config.roll_multiplier -= rollSpeed;
			}  
			else 
			{
				config.yaw_multiplier -= mouseSpeed;
			}

			if(trackerInitialized && tracker->isAvailable())
			{
				tracker->setMultipliers(config.yaw_multiplier, config.pitch_multiplier, config.roll_multiplier);
			}

			saveWaitCount = 500;
			doSaveNext = true;
			anyKeyPressed = true;
		}
		if(KEY_DOWN(VK_F9))
		{
			if(KEY_DOWN(VK_SHIFT))
			{
				config.pitch_multiplier += mouseSpeed;
			}  
			else if(KEY_DOWN(VK_CONTROL))
			{
				config.roll_multiplier += rollSpeed;
			}  
			else 
			{
				config.yaw_multiplier += mouseSpeed;
			}

			if(trackerInitialized && tracker->isAvailable())
			{
				tracker->setMultipliers(config.yaw_multiplier, config.pitch_multiplier, config.roll_multiplier);
			}
			saveWaitCount = 500;
			doSaveNext = true;
			anyKeyPressed = true;
		}
		
		if (anyKeyPressed) {
			startTime = clock();
			keyWait = true;
		}
	}
	else {
		float elapseTimeSinceLastHandledKey = (float)(clock() - startTime) / CLOCKS_PER_SEC;
		if (elapseTimeSinceLastHandledKey >= m_keyRepeatRate) {
			keyWait = false;
		}		
	}
	
	

	if(saveDebugFile)
	{
		debugFile.close();
	}
	saveDebugFile = false;

	/*if(KEY_DOWN(VK_F12) && keyWaitCount <= 0)
	{
		// uncomment to save text debug file
		//saveDebugFile = true;
		keyWaitCount = 200;
		anyKeyPressed = true;
	}*/



	if(doSaveNext && saveWaitCount < 0)
	{
		doSaveNext = false;
		ProxyHelper* helper = new ProxyHelper();

		config.swap_eyes = stereoView->swapEyes;
		m_spShaderViewAdjustment->Save(config);

		helper->SaveConfig(config);
		
		delete helper;
	}

}

void D3DProxyDevice::HandleTracking()
{
	if(!trackingOn){
		tracker->currentRoll = 0;
		return;
	}
	if(!trackerInitialized)
	{
		OutputDebugString("Try to init Tracker\n");
		tracker = MotionTrackerFactory::Get(config);
		tracker->setMultipliers(config.yaw_multiplier, config.pitch_multiplier, config.roll_multiplier);
		trackerInitialized = true;
	}

	if(trackerInitialized && tracker->isAvailable())
	{
		tracker->updateOrientation();
	}
}



void ClearVLine(LPDIRECT3DDEVICE9 Device_Interface,int x1,int y1,int x2,int y2,int bw,D3DCOLOR Color)
{
	D3DRECT rec;
	rec.x1 = x1-bw;
	rec.y1 = y1;
	rec.x2 = x2+bw;
	rec.y2 = y2;

	Device_Interface->Clear(1,&rec,D3DCLEAR_TARGET,Color,0,0);
}

void ClearHLine(LPDIRECT3DDEVICE9 Device_Interface,int x1,int y1,int x2,int y2,int bw,D3DCOLOR Color)
{
	D3DRECT rec;
	rec.x1 = x1;
	rec.y1 = y1-bw;
	rec.x2 = x2;
	rec.y2 = y2+bw;

	Device_Interface->Clear(1,&rec,D3DCLEAR_TARGET,Color,0,0);
}


HRESULT WINAPI D3DProxyDevice::TestCooperativeLevel()
{
	HRESULT result = BaseDirect3DDevice9::TestCooperativeLevel();

	if( result == D3DERR_DEVICENOTRESET ) {

		// The calling application will start releasing resources after TestCooperativeLevel returns D3DERR_DEVICENOTRESET.
		
	}

	return result;
}


HRESULT WINAPI D3DProxyDevice::BeginScene()
{
	if (m_isFirstBeginSceneOfFrame) {
		if(saveDebugFile)
		{
			debugFile.open("d3d9_debug.txt", std::ios::out);
		}

	
		HandleControls();
		HandleTracking(); // TODO Do this as late in frame as possible (Present)? Because input for this frame would already have been handled here so 
		// injection of any mouse manipulation ?

		// TODO Doing this now gives very current roll to frame. But should it be done with handle tracking to keep latency similar?
		// How much latency does mouse enulation cause? Probably want direct roll manipulation and mouse emulation to occur with same delay
		// if possible?
		if (trackerInitialized && tracker->isAvailable() && m_spShaderViewAdjustment->RollEnabled()) {
			m_spShaderViewAdjustment->UpdateRoll(tracker->currentRoll);
		}

		m_spShaderViewAdjustment->ComputeViewTransforms();

		m_isFirstBeginSceneOfFrame = false;
	}

	return BaseDirect3DDevice9::BeginScene();
}


HRESULT WINAPI D3DProxyDevice::EndScene()
{
	return BaseDirect3DDevice9::EndScene();
}


HRESULT WINAPI D3DProxyDevice::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	IDirect3DVertexBuffer9* pActualBuffer = NULL;
	HRESULT creationResult = BaseDirect3DDevice9::CreateVertexBuffer(Length, Usage, FVF, Pool, &pActualBuffer, pSharedHandle);

	if (SUCCEEDED(creationResult))
		*ppVertexBuffer = new BaseDirect3DVertexBuffer9(pActualBuffer, this);

	return creationResult;
}

HRESULT WINAPI D3DProxyDevice::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{
	IDirect3DIndexBuffer9* pActualBuffer = NULL;
	HRESULT creationResult = BaseDirect3DDevice9::CreateIndexBuffer(Length, Usage, Format, Pool, &pActualBuffer, pSharedHandle);

	if (SUCCEEDED(creationResult))
		*ppIndexBuffer = new BaseDirect3DIndexBuffer9(pActualBuffer, this);

	return creationResult;
}



HRESULT WINAPI D3DProxyDevice::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample,
													DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle, bool isSwapChainBackBuffer)
{

	IDirect3DSurface9* pLeftRenderTarget = NULL;
	IDirect3DSurface9* pRightRenderTarget = NULL;
	HRESULT creationResult;

	// create left/mono
	if (SUCCEEDED(creationResult = BaseDirect3DDevice9::CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, &pLeftRenderTarget, pSharedHandle))) {

		if (m_pDataGatherer) {
			m_pDataGatherer->OnCreateRT(Width, Height, Format, MultiSample, MultisampleQuality, isSwapChainBackBuffer);
		}

		/* "If Needed" heuristic is the complicated part here.
		  Fixed heuristics (based on type, format, size, etc) + game specific overrides + isForcedMono + magic? */
		// TODO Should we duplicate this Render Target? Replace "true" with heuristic
		if (m_pGameHandler->ShouldDuplicateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, isSwapChainBackBuffer))
		{
			if (FAILED(BaseDirect3DDevice9::CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, &pRightRenderTarget, pSharedHandle))) {
				OutputDebugString("Failed to create right eye render target while attempting to create stereo pair, falling back to mono\n");
				pRightRenderTarget = NULL;
			}
		}
	}
	else {
		OutputDebugString("Failed to create render target\n"); 
	}


	if (SUCCEEDED(creationResult)) {
		if (!isSwapChainBackBuffer)
			*ppSurface = new D3D9ProxySurface(pLeftRenderTarget, pRightRenderTarget, this, NULL);
		else
			*ppSurface = new StereoBackBuffer(pLeftRenderTarget, pRightRenderTarget, this);
	}

	return creationResult;
}



/*
	The IDirect3DSurface9** ppSurface returned should always be a D3D9ProxySurface. Any class overloading
	this method should ensure that this remains true.
 */
HRESULT WINAPI D3DProxyDevice::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample,
													DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	//OutputDebugString(__FUNCTION__); 
	//OutputDebugString("\n"); 

	return CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, false);
}




HRESULT WINAPI D3DProxyDevice::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	//OutputDebugString(__FUNCTION__);
	//OutputDebugString("\n");

	IDirect3DSurface9* pDepthStencilSurfaceLeft = NULL;
	IDirect3DSurface9* pDepthStencilSurfaceRight = NULL;
	HRESULT creationResult;

	// create left/mono
	if (SUCCEEDED(creationResult = BaseDirect3DDevice9::CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, &pDepthStencilSurfaceLeft, pSharedHandle))) {

		if (m_pDataGatherer) {
			m_pDataGatherer->OnCreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard);
		}

		// TODO Should we always duplicated Depth stencils? I think yes, but there may be exceptions
		if (m_pGameHandler->ShouldDuplicateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard)) 
		{
			if (FAILED(BaseDirect3DDevice9::CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, &pDepthStencilSurfaceRight, pSharedHandle))) {
				OutputDebugString("Failed to create right eye Depth Stencil Surface while attempting to create stereo pair, falling back to mono\n");
				pDepthStencilSurfaceRight = NULL;
			}
		}
	}
	else {
		OutputDebugString("Failed to create Depth Stencil Surface\n"); 
	}


	if (SUCCEEDED(creationResult))
		*ppSurface = new D3D9ProxySurface(pDepthStencilSurfaceLeft, pDepthStencilSurfaceRight, this, NULL);

	return creationResult;
}





HRESULT WINAPI D3DProxyDevice::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	//OutputDebugString(__FUNCTION__); 
	//OutputDebugString("\n"); 

	// OffscreenPlainSurfaces doesn't need to be Stereo. They can't be used as render targets and they can't have rendertargets copied to them with stretch rect,
	// so don't need to be stereo capable.
	// See table at bottom of http://msdn.microsoft.com/en-us/library/windows/desktop/bb174471%28v=vs.85%29.aspx for stretch rect restrictions
	IDirect3DSurface9* pActualSurface = NULL;
	HRESULT creationResult = BaseDirect3DDevice9::CreateOffscreenPlainSurface(Width, Height, Format, Pool, &pActualSurface, pSharedHandle);

	if (SUCCEEDED(creationResult))
		*ppSurface = new D3D9ProxySurface(pActualSurface, NULL, this, NULL);

	return creationResult;
}



HRESULT WINAPI D3DProxyDevice::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
	//OutputDebugString(__FUNCTION__); 
	//OutputDebugString("\n"); 

	HRESULT creationResult;
	IDirect3DTexture9* pLeftTexture = NULL;
	IDirect3DTexture9* pRightTexture = NULL;	

	// try and create left
	if (SUCCEEDED(creationResult = BaseDirect3DDevice9::CreateTexture(Width, Height, Levels, Usage, Format, Pool, &pLeftTexture, pSharedHandle))) {

		if (m_pDataGatherer && IS_RENDER_TARGET(Usage)) {
			m_pDataGatherer->OnCreateRTTexture(Width, Height, Levels, Format);
		}
		
		// Does this Texture need duplicating?
		if (m_pGameHandler->ShouldDuplicateTexture(Width, Height, Levels, Usage, Format, Pool)) {

			if (FAILED(BaseDirect3DDevice9::CreateTexture(Width, Height, Levels, Usage, Format, Pool, &pRightTexture, pSharedHandle))) {
				OutputDebugString("Failed to create right eye texture while attempting to create stereo pair, falling back to mono\n");
				pRightTexture = NULL;
			}
		}
	}
	else {
		OutputDebugString("Failed to create texture\n"); 
	}

	if (SUCCEEDED(creationResult))
		*ppTexture = new D3D9ProxyTexture(pLeftTexture, pRightTexture, this);


	return creationResult;
}



HRESULT WINAPI D3DProxyDevice::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	//OutputDebugString(__FUNCTION__); 
	//OutputDebugString("\n"); 

	HRESULT creationResult;
	IDirect3DCubeTexture9* pLeftCubeTexture = NULL;
	IDirect3DCubeTexture9* pRightCubeTexture = NULL;	

	// try and create left
	if (SUCCEEDED(creationResult = BaseDirect3DDevice9::CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, &pLeftCubeTexture, pSharedHandle))) {

		if (m_pDataGatherer && IS_RENDER_TARGET(Usage)) {
			m_pDataGatherer->OnCreateCubeRTTexture(EdgeLength, Levels, Format);
		}
		
		// Does this Texture need duplicating?
		if (m_pGameHandler->ShouldDuplicateCubeTexture(EdgeLength, Levels, Usage, Format, Pool)) {

			if (FAILED(BaseDirect3DDevice9::CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, &pRightCubeTexture, pSharedHandle))) {
				OutputDebugString("Failed to create right eye texture while attempting to create stereo pair, falling back to mono\n");
				pRightCubeTexture = NULL;
			}
		}
	}
	else {
		OutputDebugString("Failed to create texture\n"); 
	}

	if (SUCCEEDED(creationResult))
		*ppCubeTexture = new D3D9ProxyCubeTexture(pLeftCubeTexture, pRightCubeTexture, this);

	return creationResult;
}


HRESULT WINAPI D3DProxyDevice::CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{
	//OutputDebugString(__FUNCTION__); 
	//OutputDebugString("\n"); 

	// Volumes can't be used as render targets and therefore don't need to be stereo (in DX9)
	IDirect3DVolumeTexture9* pActualTexture = NULL;
	HRESULT creationResult = BaseDirect3DDevice9::CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, &pActualTexture, pSharedHandle);

	if (SUCCEEDED(creationResult))
		*ppVolumeTexture = new D3D9ProxyVolumeTexture(pActualTexture, this);

	return creationResult;
}



HRESULT WINAPI D3DProxyDevice::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{
	IDirect3DPixelShader9* pActualPShader = NULL;
	HRESULT creationResult = BaseDirect3DDevice9::CreatePixelShader(pFunction, &pActualPShader);

	if (SUCCEEDED(creationResult))
		*ppShader = new BaseDirect3DPixelShader9(pActualPShader, this);

	return creationResult;
}

HRESULT WINAPI D3DProxyDevice::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{
	IDirect3DVertexShader9* pActualVShader = NULL;
	HRESULT creationResult = BaseDirect3DDevice9::CreateVertexShader(pFunction, &pActualVShader);

	if (SUCCEEDED(creationResult)) {
		D3D9ProxyVertexShader* pWrappedVertexShader = new D3D9ProxyVertexShader(pActualVShader, this, m_pGameHandler->GetShaderModificationRepository());
		*ppShader = pWrappedVertexShader;

		if (m_pDataGatherer) {
			m_pDataGatherer->OnCreateVertexShader(pWrappedVertexShader);
		}


	}

	return creationResult;
}

HRESULT WINAPI D3DProxyDevice::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
	IDirect3DVertexDeclaration9* pActualVertexDeclaration = NULL;
	HRESULT creationResult = BaseDirect3DDevice9::CreateVertexDeclaration(pVertexElements, &pActualVertexDeclaration );

	if (SUCCEEDED(creationResult))
		*ppDecl = new BaseDirect3DVertexDeclaration9(pActualVertexDeclaration, this);

	return creationResult;
}


HRESULT WINAPI D3DProxyDevice::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{
	// this seems a weird response to me but it's what the actual device does.
	if (!ppQuery)
		return D3D_OK;

	IDirect3DQuery9* pActualQuery = NULL;
	HRESULT creationResult = BaseDirect3DDevice9::CreateQuery(Type, &pActualQuery);

	if (SUCCEEDED(creationResult)) {
		*ppQuery = new BaseDirect3DQuery9(pActualQuery, this);
	}

	return creationResult;
}

HRESULT WINAPI D3DProxyDevice::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
{
	IDirect3DStateBlock9* pActualStateBlock = NULL;
	HRESULT creationResult = BaseDirect3DDevice9::CreateStateBlock(Type, &pActualStateBlock);

	if (SUCCEEDED(creationResult)) {

		D3D9ProxyStateBlock::CaptureType capType;

		switch (Type) {
			case D3DSBT_ALL: 
			{
				capType = D3D9ProxyStateBlock::Cap_Type_Full;
				break;
			}

			case D3DSBT_PIXELSTATE: 
			{
				capType = D3D9ProxyStateBlock::Cap_Type_Pixel;
				break;
			}

			case D3DSBT_VERTEXSTATE: 
			{
				capType = D3D9ProxyStateBlock::Cap_Type_Vertex;
				break;
			}

			default:
			{
				capType = D3D9ProxyStateBlock::Cap_Type_Full;
				break;
			}    
		}

		*ppSB = new D3D9ProxyStateBlock(pActualStateBlock, this, capType, m_currentRenderingSide == vireio::Left);
	}

	return creationResult;
}



HRESULT WINAPI D3DProxyDevice::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{

	HRESULT result;
	if (SUCCEEDED(result = BaseDirect3DDevice9::Clear(Count, pRects, Flags, Color, Z, Stencil))) {
		if (switchDrawingSide()) {

			HRESULT hr;
			if (FAILED(hr = BaseDirect3DDevice9::Clear(Count, pRects, Flags, Color, Z, Stencil))) {

#ifdef _DEBUG

				char buf[256];
				sprintf_s(buf, "Error: %s error description: %s\n",
						DXGetErrorString(hr), DXGetErrorDescription(hr));

				OutputDebugString(buf);
				OutputDebugString("Clear failed\n");
				
#endif
				
			}
		}
	}

	return result;
}



HRESULT WINAPI D3DProxyDevice::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
{	
	HRESULT result;
	if (SUCCEEDED(result = BaseDirect3DDevice9::ColorFill(pSurface, pRect, color))) {
		if (switchDrawingSide())
			BaseDirect3DDevice9::ColorFill(pSurface, pRect, color);
	}

	return result;
}


void D3DProxyDevice::BeforeDrawing()
{
	m_spManagedShaderRegisters->ApplyAllDirty(m_currentRenderingSide);

	if (m_pDataGatherer) {
		if (m_pDataGatherer->ShaderMatchesCurrentHash(m_pActiveVertexShader)) {

			// Always set pixel shader. The m_redShaderIsActive flag could be set false when a pixel shader is set in SetPixelShader
			// but as this only happens in datagathering mode I'm leaving it like this rather than spreading conditional checks around elsewhere.
			m_redShaderIsActive = true;
			getActual()->SetPixelShader(m_pRedPixelShader->getActual());
		}
		else {

			if (m_redShaderIsActive) {

				m_redShaderIsActive = false;
				getActual()->SetPixelShader((m_pActivePixelShader ? m_pActivePixelShader->getActual() : NULL));
			}
		}


		// finding what the format of te textures being used as input to this shader are
		//if (m_pActiveVertexShader && m_pActiveVertexShader->GetHash() == 671997634) {

		//	std::stringstream sstm;
		//	D3DSURFACE_DESC desc;

		//	sstm << "Begin" << std::endl;

		//	auto textureIt = m_activeTextureStages.begin();
		//	while (textureIt != m_activeTextureStages.end()) {

		//		if (textureIt->second && textureIt->second->GetType() == D3DRTYPE_TEXTURE) {

		//			D3D9ProxyTexture* pDerivedTexture = static_cast<D3D9ProxyTexture*> (textureIt->second);
		//			pDerivedTexture->GetLevelDesc(0, &desc);

		//			if (IS_RENDER_TARGET(desc.Usage)) { 
		//				sstm << "TextureRenderTarget" << "," << desc.Width << "," << desc.Height << "," << desc.Format << "," << desc.MultiSampleType << "," << desc.MultiSampleQuality << "," /*<< (isSwapChainBackBuffer ? "yes" : "no")*/ << ","  << pDerivedTexture->GetLevelCount() << "," <<  /* Discard N/A */"," /*EdgeLength*/<< std::endl;
		//			}
		//		}
		//		
		//		++textureIt;
		//	}

		//	OutputDebugString(sstm.str().c_str());
		//}
	}
}


HRESULT WINAPI D3DProxyDevice::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
	BeforeDrawing();

	HRESULT result;
	if (SUCCEEDED(result = BaseDirect3DDevice9::DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount))) {
		if (switchDrawingSide())
			BaseDirect3DDevice9::DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
	}

	return result;
}


HRESULT WINAPI D3DProxyDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
	BeforeDrawing();

	HRESULT result;
	if (SUCCEEDED(result = BaseDirect3DDevice9::DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount))) {
		if (switchDrawingSide()) {
			HRESULT result2 = BaseDirect3DDevice9::DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
			if (result != result2)
				OutputDebugString("moop\n");
		}
	}

	return result;
}


HRESULT WINAPI D3DProxyDevice::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	BeforeDrawing();

	HRESULT result;
	if (SUCCEEDED(result = BaseDirect3DDevice9::DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride))) {
		if (switchDrawingSide())
			BaseDirect3DDevice9::DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	BeforeDrawing();

	HRESULT result;
	if (SUCCEEDED(result = BaseDirect3DDevice9::DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride))) {
		if (switchDrawingSide())
			BaseDirect3DDevice9::DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	BeforeDrawing();

	HRESULT result;
	if (SUCCEEDED(result = BaseDirect3DDevice9::DrawRectPatch(Handle, pNumSegs, pRectPatchInfo))) {
		if (switchDrawingSide())
			BaseDirect3DDevice9::DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	BeforeDrawing();

	HRESULT result;
	if (SUCCEEDED(result = BaseDirect3DDevice9::DrawTriPatch(Handle, pNumSegs, pTriPatchInfo))) {
		if (switchDrawingSide())
			BaseDirect3DDevice9::DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{
	if (!pDestBuffer)
		return D3DERR_INVALIDCALL;

	BeforeDrawing();

	BaseDirect3DVertexBuffer9* pCastDestBuffer = static_cast<BaseDirect3DVertexBuffer9*>(pDestBuffer);
	BaseDirect3DVertexDeclaration9* pCastVertexDeclaration = NULL;

	HRESULT result;
	if (pVertexDecl) {
		pCastVertexDeclaration = static_cast<BaseDirect3DVertexDeclaration9*>(pVertexDecl);
		result = BaseDirect3DDevice9::ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pCastDestBuffer->getActual(), pCastVertexDeclaration->getActual(), Flags);
	}
	else {
		result = BaseDirect3DDevice9::ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pCastDestBuffer->getActual(), NULL, Flags);
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	BaseDirect3DIndexBuffer9* pWrappedNewIndexData = static_cast<BaseDirect3DIndexBuffer9*>(pIndexData);

	// Update actual index buffer
	HRESULT result;
	if (pWrappedNewIndexData)
		result = BaseDirect3DDevice9::SetIndices(pWrappedNewIndexData->getActual());
	else
		result = BaseDirect3DDevice9::SetIndices(NULL);


	
	if (SUCCEEDED(result)) {

		// If in a Begin-End StateBlock pair update the block state rather than the current proxy device state
		if (m_pCapturingStateTo) {
			m_pCapturingStateTo->SelectAndCaptureState(pWrappedNewIndexData);
		}
		else {
			// Update stored proxy index buffer
			if (m_pActiveIndicies) {
				m_pActiveIndicies->Release();
			}
		
			m_pActiveIndicies = pWrappedNewIndexData;
			if (m_pActiveIndicies) {
				m_pActiveIndicies->AddRef();
			}
		}
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	if (!m_pActiveIndicies)
		return D3DERR_INVALIDCALL;
	
	*ppIndexData = m_pActiveIndicies;
	m_pActiveIndicies->AddRef();

	return D3D_OK;
}


HRESULT WINAPI D3DProxyDevice::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{	
	BaseDirect3DVertexBuffer9* pCastStreamData = static_cast<BaseDirect3DVertexBuffer9*>(pStreamData);
	HRESULT result;
	if (pStreamData) {		
		result = BaseDirect3DDevice9::SetStreamSource(StreamNumber, pCastStreamData->getActual(), OffsetInBytes, Stride);
	}
	else {
		result = BaseDirect3DDevice9::SetStreamSource(StreamNumber, NULL, OffsetInBytes, Stride);
	}


	// Update m_activeVertexBuffers if new vertex buffer was successfully set
	if (SUCCEEDED(result)) {

		// If in a Begin-End StateBlock pair update the block state rather than the current proxy device state
		if (m_pCapturingStateTo) {
			m_pCapturingStateTo->SelectAndCaptureState(StreamNumber, pCastStreamData);
		}
		else {
			// remove existing vertex buffer that was active at StreamNumber if there is one
			if (m_activeVertexBuffers.count(StreamNumber) == 1) { 

				IDirect3DVertexBuffer9* pOldBuffer = m_activeVertexBuffers.at(StreamNumber);
				if (pOldBuffer == pStreamData)
					return result;

				if (pOldBuffer)
					pOldBuffer->Release();

				m_activeVertexBuffers.erase(StreamNumber);
			}

			// insert new vertex buffer
			if(m_activeVertexBuffers.insert(std::pair<UINT, BaseDirect3DVertexBuffer9*>(StreamNumber, pCastStreamData)).second) {
				//success
				if (pStreamData)
					pStreamData->AddRef();
			}
			else {
				OutputDebugString(__FUNCTION__);
				OutputDebugString("\n");
				OutputDebugString("Unable to store active Texture Stage.\n");
				assert(false);

				//If we get here the state of the texture tracking is fubared and an implosion is imminent.

				result = D3DERR_INVALIDCALL;
			}
		}
	}

	return result;
}

// TODO ppStreamData is marked in and out in docs. Potentially it can be a get when the stream hasn't been set before???
// Category of prolbme: Worry about it if it breaks.
HRESULT WINAPI D3DProxyDevice::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData,UINT* pOffsetInBytes,UINT* pStride)
{

	// This whole methods implementation is highly questionable. Not sure exactly how GetStreamSource works
	HRESULT result = D3DERR_INVALIDCALL;

	if (m_activeVertexBuffers.count(StreamNumber) == 1) {

		//IDirect3DVertexBuffer9* pCurrentActual = m_activeVertexBuffers[StreamNumber]->getActual();

		//IDirect3DVertexBuffer9* pActualResultBuffer = NULL;
		//HRESULT result = BaseDirect3DDevice9::GetStreamSource(StreamNumber, &pCurrentActual, pOffsetInBytes, pStride);

		
		*ppStreamData = m_activeVertexBuffers[StreamNumber];
		if ((*ppStreamData))
			(*ppStreamData)->AddRef();

		result = D3D_OK;
		
	}

	return result;
}



HRESULT WINAPI D3DProxyDevice::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	D3D9ProxySurface* newRenderTarget = static_cast<D3D9ProxySurface*>(pRenderTarget);

#ifdef _DEBUG
	if (newRenderTarget && !newRenderTarget->getActualLeft() && !newRenderTarget->getActualRight()) {
		OutputDebugString("RenderTarget is not a valid (D3D9ProxySurface) stereo capable surface\n"); 
	}
#endif
	
	//// Update actual render target ////
	HRESULT result;

	// Removing a render target
	if (newRenderTarget == NULL) {
		if (RenderTargetIndex == 0) {
			// main render target should never be set to NULL
			result = D3DERR_INVALIDCALL; 
		}		
		else {
			result = BaseDirect3DDevice9::SetRenderTarget(RenderTargetIndex, NULL);
		}
	}
	// Setting a render target
	else {
		if (m_currentRenderingSide == vireio::Left) {
			result = BaseDirect3DDevice9::SetRenderTarget(RenderTargetIndex, newRenderTarget->getActualLeft());
		}
		else {
			result = BaseDirect3DDevice9::SetRenderTarget(RenderTargetIndex, newRenderTarget->getActualRight());
		}
	}


	
	
	//// update proxy collection of stereo render targets to reflect new actual render target ////
	if (result == D3D_OK) {		
		// changing rendertarget resets viewport to fullsurface
		m_bActiveViewportIsDefault = true;

		// release old render target
		if (m_activeRenderTargets[RenderTargetIndex] != NULL)
			m_activeRenderTargets[RenderTargetIndex]->Release();

		// replace with new render target (may be NULL)
		m_activeRenderTargets[RenderTargetIndex] = newRenderTarget;
		if (m_activeRenderTargets[RenderTargetIndex] != NULL)
			m_activeRenderTargets[RenderTargetIndex]->AddRef();
	}

	return result;
}




HRESULT WINAPI D3DProxyDevice::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
{
	if ((RenderTargetIndex >= m_activeRenderTargets.capacity()) || (RenderTargetIndex < 0)) {
		return D3DERR_INVALIDCALL;
	}

	IDirect3DSurface9* targetToReturn = m_activeRenderTargets[RenderTargetIndex];
	if (!targetToReturn)
		return D3DERR_NOTFOUND;
	else {
		*ppRenderTarget = targetToReturn;
		targetToReturn->AddRef();
		return D3D_OK;
	}
}



HRESULT WINAPI D3DProxyDevice::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	D3D9ProxySurface* pNewDepthStencil = static_cast<D3D9ProxySurface*>(pNewZStencil);

	IDirect3DSurface9* pActualStencilForCurrentSide = NULL;
	if (pNewDepthStencil) {
		if (m_currentRenderingSide == vireio::Left)
			pActualStencilForCurrentSide = pNewDepthStencil->getActualLeft();
		else
			pActualStencilForCurrentSide = pNewDepthStencil->getActualRight();
	}

	// Update actual depth stencil
	HRESULT result = BaseDirect3DDevice9::SetDepthStencilSurface(pActualStencilForCurrentSide);

	// Update stored proxy depth stencil
	if (SUCCEEDED(result)) {
		if (m_pActiveStereoDepthStencil) {
			m_pActiveStereoDepthStencil->Release();
		}
		
		m_pActiveStereoDepthStencil = pNewDepthStencil;
		if (m_pActiveStereoDepthStencil) {
			m_pActiveStereoDepthStencil->AddRef();
		}
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{	
	if (!m_pActiveStereoDepthStencil)
		return D3DERR_NOTFOUND;
	
	*ppZStencilSurface = m_pActiveStereoDepthStencil;
	(*ppZStencilSurface)->AddRef();

	return D3D_OK;
}



HRESULT WINAPI D3DProxyDevice::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{	
	HRESULT result;
	if (pTexture) {
		
		IDirect3DBaseTexture9* pActualLeftTexture = NULL;
		IDirect3DBaseTexture9* pActualRightTexture = NULL;

		UnWrapTexture(pTexture, &pActualLeftTexture, &pActualRightTexture);
		
		// Try and Update the actual devices textures
		if ((pActualRightTexture == NULL) || (m_currentRenderingSide == vireio::Left)) // use left (mono) if not stereo or one left side
			result = BaseDirect3DDevice9::SetTexture(Stage, pActualLeftTexture);
		else
			result = BaseDirect3DDevice9::SetTexture(Stage, pActualRightTexture);

	}
	else {
		result = BaseDirect3DDevice9::SetTexture(Stage, NULL);
	}

	


	// Update m_activeTextureStages if new testure was successfully set
	if (SUCCEEDED(result)) {

		// If in a Begin-End StateBlock pair update the block state rather than the current proxy device state
		if (m_pCapturingStateTo) {
			m_pCapturingStateTo->SelectAndCaptureState(Stage, pTexture);
		}
		else {

			// remove existing texture that was active at Stage if there is one
			if (m_activeTextureStages.count(Stage) == 1) { 

				IDirect3DBaseTexture9* pOldTexture = m_activeTextureStages.at(Stage);
				if (pOldTexture)
					pOldTexture->Release();

				m_activeTextureStages.erase(Stage);
			}


			// insert new texture (can be a NULL pointer, this is important for StateBlock tracking)
			if(m_activeTextureStages.insert(std::pair<DWORD, IDirect3DBaseTexture9*>(Stage, pTexture)).second) {
				//success
				if (pTexture)
					pTexture->AddRef();
			}
			else {
				OutputDebugString(__FUNCTION__);
				OutputDebugString("\n");
				OutputDebugString("Unable to store active Texture Stage.\n");
				assert(false);

				//If we get here the state of the texture tracking is fubared and an implosion is imminent.

				result = D3DERR_INVALIDCALL;
			}
		}
	}

	return result;
}


HRESULT WINAPI D3DProxyDevice::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture)
{
	if (m_activeTextureStages.count(Stage) != 1)
		return D3DERR_INVALIDCALL;
	else {
		*ppTexture = m_activeTextureStages[Stage];
		if ((*ppTexture))
			(*ppTexture)->AddRef();
		return D3D_OK;
	}
}



HRESULT WINAPI D3DProxyDevice::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	BaseDirect3DPixelShader9* pWrappedPShaderData = static_cast<BaseDirect3DPixelShader9*>(pShader);

	// Update actual pixel shader
	HRESULT result;
	if (pWrappedPShaderData)
		result = BaseDirect3DDevice9::SetPixelShader(pWrappedPShaderData->getActual());
	else
		result = BaseDirect3DDevice9::SetPixelShader(NULL);

	// Update stored proxy pixel shader
	if (SUCCEEDED(result)) {

		// If in a Begin-End StateBlock pair update the block state rather than the current proxy device state
		if (m_pCapturingStateTo) {
			m_pCapturingStateTo->SelectAndCaptureState(pWrappedPShaderData);
		}
		else {

			if (m_pActivePixelShader) {
				m_pActivePixelShader->Release();
			}
		
			m_pActivePixelShader = pWrappedPShaderData;
			if (m_pActivePixelShader) {
				m_pActivePixelShader->AddRef();
			}
		}
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	if (!m_pActivePixelShader)
		return D3DERR_INVALIDCALL;
	
	*ppShader = m_pActivePixelShader;

	return D3D_OK;
}


HRESULT WINAPI D3DProxyDevice::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	D3D9ProxyVertexShader* pWrappedVShaderData = static_cast<D3D9ProxyVertexShader*>(pShader);

	// Update actual Vertex shader
	HRESULT result;
	if (pWrappedVShaderData)
		result = BaseDirect3DDevice9::SetVertexShader(pWrappedVShaderData->getActual());
	else
		result = BaseDirect3DDevice9::SetVertexShader(NULL);

	// Update stored proxy Vertex shader
	if (SUCCEEDED(result)) {

		if (m_pDataGatherer && pWrappedVShaderData) {
			m_pDataGatherer->OnSetVertexShader(pWrappedVShaderData);
		}

		// If in a Begin-End StateBlock pair update the block state rather than the current proxy device state
		if (m_pCapturingStateTo) {
			m_pCapturingStateTo->SelectAndCaptureState(pWrappedVShaderData);
		}
		else {
			if (m_pActiveVertexShader) {
				m_pActiveVertexShader->Release();
			}
		
			m_pActiveVertexShader = pWrappedVShaderData;
			if (m_pActiveVertexShader) {
				m_pActiveVertexShader->AddRef();
			}

			m_spManagedShaderRegisters->ActiveVertexShaderChanged(m_pActiveVertexShader);
		}
	}


	return result;
}

HRESULT WINAPI D3DProxyDevice::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	if (!m_pActiveVertexShader)
		return D3DERR_INVALIDCALL;
	
	*ppShader = m_pActiveVertexShader;

	return D3D_OK;
}

HRESULT WINAPI D3DProxyDevice::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	HRESULT result = D3DERR_INVALIDCALL;

	if (m_pCapturingStateTo) {
		result = m_pCapturingStateTo->SelectAndCaptureStateVSConst(StartRegister, pConstantData, Vector4fCount);
	}
	else { 
		result = m_spManagedShaderRegisters->SetConstantRegistersF(StartRegister, pConstantData, Vector4fCount);
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::GetVertexShaderConstantF(UINT StartRegister,float* pData,UINT Vector4fCount)
{
	return m_spManagedShaderRegisters->GetConstantRegistersF(StartRegister, pData, Vector4fCount);
}




HRESULT WINAPI D3DProxyDevice::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	BaseDirect3DVertexDeclaration9* pWrappedVDeclarationData = static_cast<BaseDirect3DVertexDeclaration9*>(pDecl);

	// Update actual Vertex Declaration
	HRESULT result;
	if (pWrappedVDeclarationData)
		result = BaseDirect3DDevice9::SetVertexDeclaration(pWrappedVDeclarationData->getActual());
	else
		result = BaseDirect3DDevice9::SetVertexDeclaration(NULL);

	// Update stored proxy Vertex Declaration
	if (SUCCEEDED(result)) {

		// If in a Begin-End StateBlock pair update the block state rather than the current proxy device state
		if (m_pCapturingStateTo) {
			m_pCapturingStateTo->SelectAndCaptureState(pWrappedVDeclarationData);
		}
		else {

			if (m_pActiveVertexDeclaration) {
				m_pActiveVertexDeclaration->Release();
			}
		
			m_pActiveVertexDeclaration = pWrappedVDeclarationData;
			if (m_pActiveVertexDeclaration) {
				m_pActiveVertexDeclaration->AddRef();
			}
		}
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	if (!m_pActiveVertexDeclaration) 
		return D3DERR_INVALIDCALL; // TODO check this is the response if no declaration set
		
	*ppDecl = m_pActiveVertexDeclaration;

	return D3D_OK;
}

HRESULT WINAPI D3DProxyDevice::BeginStateBlock()
{
	HRESULT result;
	if (SUCCEEDED(result = BaseDirect3DDevice9::BeginStateBlock())) {
		m_bInBeginEndStateBlock = true;
		m_pCapturingStateTo = new D3D9ProxyStateBlock(NULL, this, D3D9ProxyStateBlock::Cap_Type_Selected, m_currentRenderingSide == vireio::Left);
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	IDirect3DStateBlock9* pActualStateBlock = NULL;
	HRESULT creationResult = BaseDirect3DDevice9::EndStateBlock(&pActualStateBlock);

	if (SUCCEEDED(creationResult)) {
		m_pCapturingStateTo->EndStateBlock(pActualStateBlock);
		*ppSB = m_pCapturingStateTo;
	}
	else {
		m_pCapturingStateTo->Release();
	}
	
	m_pCapturingStateTo = NULL;
	m_bInBeginEndStateBlock = false;

	return creationResult;
}


bool D3DProxyDevice::switchDrawingSide()
{
	bool switched = false;

	if (m_currentRenderingSide == vireio::Left) {
		switched = setDrawingSide(vireio::Right);
	}
	else if (m_currentRenderingSide == vireio::Right) {
		switched = setDrawingSide(vireio::Left);
	}
	else {
		DebugBreak();
	}

	return switched;
}


/*
	Switches rendering to which ever side is specified by side.

	Returns true if change succeeded, false if it fails. The switch will fail if you attempt to setDrawingSide(Right)
	when the current primary active render target (target 0  in m_activeRenderTargets) is not stereo.
	Attempting to switch to a side when that side is already the active side will return true without making any changes.
 */
bool D3DProxyDevice::setDrawingSide(vireio::RenderPosition side)
{
	// Already on the correct eye
	if (side == m_currentRenderingSide) {
		return true;
	}



	// should never try and render for the right eye if there is no render target for the main render targets right side
	if (!m_activeRenderTargets[0]->IsStereo() && (side == vireio::Right)) {
		return false;
	}


	// Everything hasn't changed yet but we set this first so we don't accidentally use the member instead of the local and break
	// things, as I have already managed twice.
	m_currentRenderingSide = side;


	// switch render targets to new side
	bool renderTargetChanged = false;
	HRESULT result;
	D3D9ProxySurface* pCurrentRT;
	for(std::vector<D3D9ProxySurface*>::size_type i = 0; i != m_activeRenderTargets.size(); i++) 
	{
		if ((pCurrentRT = m_activeRenderTargets[i]) != NULL) {

			if (side == vireio::Left) 
				result = BaseDirect3DDevice9::SetRenderTarget(i, pCurrentRT->getActualLeft()); 
			else 
				result = BaseDirect3DDevice9::SetRenderTarget(i, pCurrentRT->getActualRight());
				
			if (result != D3D_OK) {
				OutputDebugString("Error trying to set one of the Render Targets while switching between active eyes for drawing.\n");
			}
			else {
				renderTargetChanged = true;
			}
		}
	}

	// if a non-fullsurface viewport is active and a rendertarget changed we need to reapply the viewport
	if (renderTargetChanged && !m_bActiveViewportIsDefault) {
		BaseDirect3DDevice9::SetViewport(&m_LastViewportSet);
	}
		


	// switch depth stencil to new side
	if (m_pActiveStereoDepthStencil != NULL) { 
		if (side == vireio::Left) 
			result = BaseDirect3DDevice9::SetDepthStencilSurface(m_pActiveStereoDepthStencil->getActualLeft()); 
		else 
			result = BaseDirect3DDevice9::SetDepthStencilSurface(m_pActiveStereoDepthStencil->getActualRight());
	}


	// switch textures to new side
	IDirect3DBaseTexture9* pActualLeftTexture = NULL;
	IDirect3DBaseTexture9* pActualRightTexture = NULL;
	
	for(auto it = m_activeTextureStages.begin(); it != m_activeTextureStages.end(); ++it )
	{
		if (it->second) {
			pActualLeftTexture = NULL;
			pActualRightTexture = NULL;
			UnWrapTexture(it->second, &pActualLeftTexture, &pActualRightTexture);

			// if stereo texture
			if (pActualRightTexture != NULL) { 
				if (side == vireio::Left) 
					result = BaseDirect3DDevice9::SetTexture(it->first, pActualLeftTexture); 
				else 
					result = BaseDirect3DDevice9::SetTexture(it->first, pActualRightTexture);
			}
			// else the texture is mono and doesn't need changing. It will always be set initially and then won't need changing
				
			if (result != D3D_OK)
				OutputDebugString("Error trying to set one of the textures while switching between active eyes for drawing.\n");
		}
	}


	// update view transform for new side 
	if (m_bViewTransformSet) {

		if (side == vireio::Left) {
			m_pCurrentView = &m_leftView;
		}
		else {
			m_pCurrentView = &m_rightView;
		}

		BaseDirect3DDevice9::SetTransform(D3DTS_VIEW, m_pCurrentView);
	}

	
	// update projection transform for new side 
	if (m_bProjectionTransformSet) {

		if (side == vireio::Left) {
			m_pCurrentProjection = &m_leftProjection;
		}
		else {
			m_pCurrentProjection = &m_rightProjection;
		}

		BaseDirect3DDevice9::SetTransform(D3DTS_PROJECTION, m_pCurrentProjection);
	}


	// Updated computed view translation (used by several derived proxies - see: ComputeViewTranslation)
	if (side == vireio::Left) {
		m_pCurrentMatViewTransform = &m_spShaderViewAdjustment->LeftAdjustmentMatrix();
	}
	else {
		m_pCurrentMatViewTransform = &m_spShaderViewAdjustment->RightAdjustmentMatrix();
	}


	// Apply active stereo shader constants
	m_spManagedShaderRegisters->ApplyAllStereoConstants(side);




	return true;
}




HRESULT WINAPI D3DProxyDevice::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	IDirect3DSurface9* pWrappedBackBuffer;

	try {
		m_activeSwapChains.at(0)->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pWrappedBackBuffer);

		if (stereoView->initialized)
			stereoView->Draw(static_cast<D3D9ProxySurface*>(pWrappedBackBuffer));

		pWrappedBackBuffer->Release();
	}
	catch (std::out_of_range) {
		OutputDebugString("Present: No primary swap chain found. (Present probably called before device has been reset)");
	}

	

	m_isFirstBeginSceneOfFrame = true; // TODO this can break if device present is followed by present on another swap chain... or not work well anyway


	if (worldScaleCalculationMode) {
		// draw red lines vertically through the center of the lens/distortion.
		//TODO doesn't currently work with source based games
		int width = stereoView->viewport.Width;
		int height = stereoView->viewport.Height;
		ClearVLine(getActual(), (int)(m_spShaderViewAdjustment->HMDInfo()->LeftLensCenterAsPercentage() * width), 0, (int)(m_spShaderViewAdjustment->HMDInfo()->LeftLensCenterAsPercentage() * width) + 1, height, 1, D3DCOLOR_ARGB(255,255,0,0));
		ClearVLine(getActual(), (int)((1 - m_spShaderViewAdjustment->HMDInfo()->LeftLensCenterAsPercentage()) * width), 0, (int)((1 - m_spShaderViewAdjustment->HMDInfo()->LeftLensCenterAsPercentage()) * width) + 1, height, 1, D3DCOLOR_ARGB(255,255,0,0));
	}

	return BaseDirect3DDevice9::Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}



HRESULT WINAPI D3DProxyDevice::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{
	HRESULT result;
	try {
		result = m_activeSwapChains.at(iSwapChain)->GetBackBuffer(iBackBuffer, D3DBACKBUFFER_TYPE_MONO, ppBackBuffer);
		// ref count increase happens in the swapchain GetBackBuffer so we don't add another ref here as we are just passing the value through
	}
	catch (std::out_of_range) {
		OutputDebugString("GetBackBuffer: out of range getting swap chain");
		result = D3DERR_INVALIDCALL;
	}

	return result;
}




HRESULT WINAPI D3DProxyDevice::GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
{
	try {
		*pSwapChain = m_activeSwapChains.at(iSwapChain); 
		//Device->GetSwapChain increases ref count on the chain (docs don't say this)
		(*pSwapChain)->AddRef();
	}
	catch (std::out_of_range) {
		OutputDebugString("GetSwapChain: out of range fetching swap chain");
		return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

HRESULT WINAPI D3DProxyDevice::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)
{
	IDirect3DSwapChain9* pActualSwapChain;
	HRESULT result = BaseDirect3DDevice9::CreateAdditionalSwapChain(pPresentationParameters, &pActualSwapChain);

	if (SUCCEEDED(result)) {
		D3D9ProxySwapChain* wrappedSwapChain = new D3D9ProxySwapChain(pActualSwapChain, this, true);
		*pSwapChain = wrappedSwapChain;
		m_activeSwapChains.push_back(wrappedSwapChain);
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{ 
	HRESULT result;
	try {
		result = m_activeSwapChains.at(iSwapChain)->GetFrontBufferData(pDestSurface);
	}
	catch (std::out_of_range) {
		OutputDebugString("GetFrontBufferData: out of range fetching swap chain");
		result = D3DERR_INVALIDCALL;
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
{
	if ((pDestSurface == NULL) || (pRenderTarget == NULL))
		return D3DERR_INVALIDCALL;

	D3D9ProxySurface* pWrappedRenderTarget = static_cast<D3D9ProxySurface*>(pRenderTarget);
	D3D9ProxySurface* pWrappedDest = static_cast<D3D9ProxySurface*>(pDestSurface);

	IDirect3DSurface9* pRenderTargetLeft = pWrappedRenderTarget->getActualLeft();
	IDirect3DSurface9* pRenderTargetRight = pWrappedRenderTarget->getActualRight();
	IDirect3DSurface9* pDestSurfaceLeft = pWrappedDest->getActualLeft();
	IDirect3DSurface9* pDestSurfaceRight = pWrappedDest->getActualRight();

	HRESULT result = BaseDirect3DDevice9::GetRenderTargetData(pRenderTargetLeft, pDestSurfaceLeft);

	if (SUCCEEDED(result)) {
		if (!pRenderTargetRight && pDestSurfaceRight) {
			//OutputDebugString("INFO: GetRenderTargetData - Source is not stereo, destination is stereo. Copying source to both sides of destination.\n");

			if (FAILED(BaseDirect3DDevice9::GetRenderTargetData(pRenderTargetLeft, pDestSurfaceRight))) {
				OutputDebugString("ERROR: GetRenderTargetData - Failed to copy source left to destination right.\n");
			}
		} 
		else if (pRenderTargetRight && !pDestSurfaceRight) {
			//OutputDebugString("INFO: GetRenderTargetData - Source is stereo, destination is not stereo. Copied Left side only.\n");
		}
		else if (pRenderTargetRight && pDestSurfaceRight)	{
			if (FAILED(BaseDirect3DDevice9::GetRenderTargetData(pRenderTargetRight, pDestSurfaceRight))) {
				OutputDebugString("ERROR: GetRenderTargetData - Failed to copy source right to destination right.\n");
			}
		}
	}
	
	return result;
}

HRESULT WINAPI D3DProxyDevice::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
	if (!pCursorBitmap)
		return BaseDirect3DDevice9::SetCursorProperties(XHotSpot, YHotSpot, NULL);

	return BaseDirect3DDevice9::SetCursorProperties(XHotSpot, YHotSpot, static_cast<D3D9ProxySurface*>(pCursorBitmap)->getActualLeft());
}


HRESULT WINAPI D3DProxyDevice::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{
	if (!pSourceSurface || !pDestSurface)
		 return D3DERR_INVALIDCALL;


	D3D9ProxySurface* pWrappedSource = static_cast<D3D9ProxySurface*>(pSourceSurface);
	D3D9ProxySurface* pWrappedDest = static_cast<D3D9ProxySurface*>(pDestSurface);


	IDirect3DSurface9* pSourceSurfaceLeft = pWrappedSource->getActualLeft();
	IDirect3DSurface9* pSourceSurfaceRight = pWrappedSource->getActualRight();
	IDirect3DSurface9* pDestSurfaceLeft = pWrappedDest->getActualLeft();
	IDirect3DSurface9* pDestSurfaceRight = pWrappedDest->getActualRight();

	HRESULT result = BaseDirect3DDevice9::StretchRect(pSourceSurfaceLeft, pSourceRect, pDestSurfaceLeft, pDestRect, Filter);

	if (SUCCEEDED(result)) {
		if (!pSourceSurfaceRight && pDestSurfaceRight) {
			//OutputDebugString("INFO: StretchRect - Source is not stereo, destination is stereo. Copying source to both sides of destination.\n");

			if (FAILED(BaseDirect3DDevice9::StretchRect(pSourceSurfaceLeft, pSourceRect, pDestSurfaceRight, pDestRect, Filter))) {
				OutputDebugString("ERROR: StretchRect - Failed to copy source left to destination right.\n");
			}
		} 
		else if (pSourceSurfaceRight && !pDestSurfaceRight) {
			//OutputDebugString("INFO: StretchRect - Source is stereo, destination is not stereo. Copied Left side only.\n");
		}
		else if (pSourceSurfaceRight && pDestSurfaceRight)	{
			if (FAILED(BaseDirect3DDevice9::StretchRect(pSourceSurfaceRight, pSourceRect, pDestSurfaceRight, pDestRect, Filter))) {
				OutputDebugString("ERROR: StretchRect - Failed to copy source right to destination right.\n");
			}
		}
	}

	return result;
}


HRESULT WINAPI D3DProxyDevice::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
{
	if (!pSourceSurface || !pDestinationSurface)
		 return D3DERR_INVALIDCALL;

	IDirect3DSurface9* pSourceSurfaceLeft = static_cast<D3D9ProxySurface*>(pSourceSurface)->getActualLeft();
	IDirect3DSurface9* pSourceSurfaceRight = static_cast<D3D9ProxySurface*>(pSourceSurface)->getActualRight();
	IDirect3DSurface9* pDestSurfaceLeft = static_cast<D3D9ProxySurface*>(pDestinationSurface)->getActualLeft();
	IDirect3DSurface9* pDestSurfaceRight = static_cast<D3D9ProxySurface*>(pDestinationSurface)->getActualRight();

	HRESULT result = BaseDirect3DDevice9::UpdateSurface(pSourceSurfaceLeft, pSourceRect, pDestSurfaceLeft, pDestPoint);

	if (SUCCEEDED(result)) {
		if (!pSourceSurfaceRight && pDestSurfaceRight) {
			//OutputDebugString("INFO: UpdateSurface - Source is not stereo, destination is stereo. Copying source to both sides of destination.\n");

			if (FAILED(BaseDirect3DDevice9::UpdateSurface(pSourceSurfaceLeft, pSourceRect, pDestSurfaceRight, pDestPoint))) {
				OutputDebugString("ERROR: UpdateSurface - Failed to copy source left to destination right.\n");
			}
		} 
		else if (pSourceSurfaceRight && !pDestSurfaceRight) {
			//OutputDebugString("INFO: UpdateSurface - Source is stereo, destination is not stereo. Copied Left side only.\n");
		}
		else if (pSourceSurfaceRight && pDestSurfaceRight)	{
			if (FAILED(BaseDirect3DDevice9::UpdateSurface(pSourceSurfaceRight, pSourceRect, pDestSurfaceRight, pDestPoint))) {
				OutputDebugString("ERROR: UpdateSurface - Failed to copy source right to destination right.\n");
			}
		}
	}

	return result;
}

HRESULT WINAPI D3DProxyDevice::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
{
	if (!pSourceTexture || !pDestinationTexture)
		 return D3DERR_INVALIDCALL;


	IDirect3DBaseTexture9* pSourceTextureLeft = NULL;
	IDirect3DBaseTexture9* pSourceTextureRight = NULL;
	IDirect3DBaseTexture9* pDestTextureLeft = NULL;
	IDirect3DBaseTexture9* pDestTextureRight = NULL;

	UnWrapTexture(pSourceTexture, &pSourceTextureLeft, &pSourceTextureRight);
	UnWrapTexture(pDestinationTexture, &pDestTextureLeft, &pDestTextureRight);


	HRESULT result = BaseDirect3DDevice9::UpdateTexture(pSourceTextureLeft, pDestTextureLeft);

	if (SUCCEEDED(result)) {
		if (!pSourceTextureRight && pDestTextureRight) {
			//OutputDebugString("INFO: UpdateTexture - Source is not stereo, destination is stereo. Copying source to both sides of destination.\n");

			if (FAILED(BaseDirect3DDevice9::UpdateTexture(pSourceTextureLeft, pDestTextureRight))) {
				OutputDebugString("ERROR: UpdateTexture - Failed to copy source left to destination right.\n");
			}
		} 
		else if (pSourceTextureRight && !pDestTextureRight) {
			//OutputDebugString("INFO: UpdateTexture - Source is stereo, destination is not stereo. Copied Left side only.\n");
		}
		else if (pSourceTextureRight && pDestTextureRight)	{
			if (FAILED(BaseDirect3DDevice9::UpdateTexture(pSourceTextureRight, pDestTextureRight))) {
				OutputDebugString("ERROR: UpdateTexture - Failed to copy source right to destination right.\n");
			}
		}
	}

	return result;
}


HRESULT D3DProxyDevice::SetStereoViewTransform(D3DXMATRIX pLeftMatrix, D3DXMATRIX pRightMatrix, bool apply)
{
	if (D3DXMatrixIsIdentity(&pLeftMatrix) && D3DXMatrixIsIdentity(&pRightMatrix)) {
		m_bViewTransformSet = false;
	}
	else {
		m_bViewTransformSet = true;
	}
	
	m_leftView = pLeftMatrix;
	m_rightView = pRightMatrix;

	if (m_currentRenderingSide == vireio::Left) {
		m_pCurrentView = &m_leftView;
	}
	else {
		m_pCurrentView = &m_rightView;
	}

	if (apply)
		return BaseDirect3DDevice9::SetTransform(D3DTS_VIEW, m_pCurrentView);
	else
		return D3D_OK;
}


HRESULT D3DProxyDevice::SetStereoProjectionTransform(D3DXMATRIX pLeftMatrix, D3DXMATRIX pRightMatrix, bool apply)
{
	if (D3DXMatrixIsIdentity(&pLeftMatrix) && D3DXMatrixIsIdentity(&pRightMatrix)) {
		m_bProjectionTransformSet = false;
	}
	else {
		m_bProjectionTransformSet = true;
	}
	
	m_leftProjection = pLeftMatrix;
	m_rightProjection = pRightMatrix;

	if (m_currentRenderingSide == vireio::Left) {
		m_pCurrentProjection = &m_leftProjection;
	}
	else {
		m_pCurrentProjection = &m_rightProjection;
	}

	if (apply)
		return BaseDirect3DDevice9::SetTransform(D3DTS_PROJECTION, m_pCurrentProjection);
	else
		return D3D_OK;
}


HRESULT WINAPI D3DProxyDevice::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	if(State == D3DTS_VIEW)
	{
		D3DXMATRIX tempLeft;
		D3DXMATRIX tempRight;
		D3DXMATRIX* pViewToSet = NULL;
		bool tempIsTransformSet = false;

		if (!pMatrix) {
			D3DXMatrixIdentity(&tempLeft);
			D3DXMatrixIdentity(&tempRight);
		}
		else {

			D3DXMATRIX sourceMatrix(*pMatrix);

			// If the view is set to the identity then we don't need to perform any adjustments
			if (D3DXMatrixIsIdentity(&sourceMatrix)) {

				D3DXMatrixIdentity(&tempLeft);
				D3DXMatrixIdentity(&tempRight);
			}
			else {
				// If the view matrix is modified we need to apply left/right adjustments (for stereo rendering)
				tempLeft = sourceMatrix * m_spShaderViewAdjustment->LeftViewTransform();
				tempRight = sourceMatrix * m_spShaderViewAdjustment->RightViewTransform();

				tempIsTransformSet = true;
			}
		}


		// If capturing state block capture without updating proxy device
		if (m_pCapturingStateTo) {
			m_pCapturingStateTo->SelectAndCaptureViewTransform(tempLeft, tempRight);
			if (m_currentRenderingSide == vireio::Left) {
				pViewToSet = &tempLeft;
			}
			else {
				pViewToSet = &tempRight;
			}
		}
		else { // otherwise update proxy device

			m_bViewTransformSet = tempIsTransformSet;
			m_leftView = tempLeft;
			m_rightView = tempRight;

			if (m_currentRenderingSide == vireio::Left) {
				m_pCurrentView = &m_leftView;
			}
			else {
				m_pCurrentView = &m_rightView;
			}

			pViewToSet = m_pCurrentView;
		}

		return BaseDirect3DDevice9::SetTransform(State, pViewToSet);
		
	}
	else if(State == D3DTS_PROJECTION)
	{

		D3DXMATRIX tempLeft;
		D3DXMATRIX tempRight;
		D3DXMATRIX* pProjectionToSet = NULL;
		bool tempIsTransformSet = false;

		if (!pMatrix) {
			
			D3DXMatrixIdentity(&tempLeft);
			D3DXMatrixIdentity(&tempRight);
		}
		else {
			D3DXMATRIX sourceMatrix(*pMatrix);

			// If the view is set to the identity then we don't need to perform any adjustments
		
			if (D3DXMatrixIsIdentity(&sourceMatrix)) {

				D3DXMatrixIdentity(&tempLeft);
				D3DXMatrixIdentity(&tempRight);
			}
			else {
				

				tempLeft = sourceMatrix * m_spShaderViewAdjustment->LeftShiftProjection();
				tempRight = sourceMatrix * m_spShaderViewAdjustment->RightShiftProjection();

				tempIsTransformSet = true;
			}


			
		}


		// If capturing state block capture without updating proxy device
		if (m_pCapturingStateTo) {

			m_pCapturingStateTo->SelectAndCaptureProjectionTransform(tempLeft, tempRight);
			if (m_currentRenderingSide == vireio::Left) {
				pProjectionToSet = &tempLeft;
			}
			else {
				pProjectionToSet = &tempRight;
			}
		}
		else { // otherwise update proxy device

			m_bProjectionTransformSet = tempIsTransformSet;
			m_leftProjection = tempLeft;
			m_rightProjection = tempRight;

			if (m_currentRenderingSide == vireio::Left) {
				m_pCurrentProjection = &m_leftProjection;
			}
			else {
				m_pCurrentProjection = &m_rightProjection;
			}

			pProjectionToSet = m_pCurrentProjection;
		}

		return BaseDirect3DDevice9::SetTransform(State, pProjectionToSet);
	}

	return BaseDirect3DDevice9::SetTransform(State, pMatrix);
}


HRESULT WINAPI D3DProxyDevice::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
	OutputDebugString(__FUNCTION__); 
	OutputDebugString("\n"); 
	OutputDebugString("Not implemented - Fix Me! (if i need fixing)\n"); 

	return BaseDirect3DDevice9::MultiplyTransform(State, pMatrix);
}


HRESULT WINAPI D3DProxyDevice::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	// try and set, if success save viewport
	// if viewport width and height match primary render target size and zmin is 0 and zmax 1 
	// set m_bActiveViewportIsDefault flag true.

	HRESULT result = BaseDirect3DDevice9::SetViewport(pViewport);

	if (SUCCEEDED(result)) {

		// If in a Begin-End StateBlock pair update the block state rather than the current proxy device state
		if (m_pCapturingStateTo) {
			m_pCapturingStateTo->SelectAndCaptureState(*pViewport);
		}
		else {
			m_bActiveViewportIsDefault = isViewportDefaultForMainRT(pViewport);
			m_LastViewportSet = *pViewport;
		}
	}
	
	return result;
}

/* Comparison made against active primary render target */
bool D3DProxyDevice::isViewportDefaultForMainRT(CONST D3DVIEWPORT9* pViewport)
{
	D3D9ProxySurface* pPrimaryRenderTarget = m_activeRenderTargets[0];
	D3DSURFACE_DESC pRTDesc;
	pPrimaryRenderTarget->GetDesc(&pRTDesc);

	return  ((pViewport->Height == pRTDesc.Height) && (pViewport->Width == pRTDesc.Width) &&
			(pViewport->MinZ <= SMALL_FLOAT) && (pViewport->MaxZ >= SLIGHTLY_LESS_THAN_ONE));
}


// Get actual textures from the various wrapper texture types
void D3DProxyDevice::UnWrapTexture(IDirect3DBaseTexture9* pWrappedTexture, IDirect3DBaseTexture9** ppActualLeftTexture, IDirect3DBaseTexture9** ppActualRightTexture)
{
	if (!pWrappedTexture)
		assert (false);

	D3DRESOURCETYPE type = pWrappedTexture->GetType();

	*ppActualLeftTexture = NULL;
	*ppActualRightTexture = NULL;
	
	switch (type)
	{
		case D3DRTYPE_TEXTURE:
		{
			D3D9ProxyTexture* pDerivedTexture = static_cast<D3D9ProxyTexture*> (pWrappedTexture);
			*ppActualLeftTexture = pDerivedTexture->getActualLeft();
			*ppActualRightTexture = pDerivedTexture->getActualRight();

			break;
		}
		case D3DRTYPE_VOLUMETEXTURE:
		{
			D3D9ProxyVolumeTexture* pDerivedTexture = static_cast<D3D9ProxyVolumeTexture*> (pWrappedTexture);
			*ppActualLeftTexture = pDerivedTexture->getActual();
			break;
		}
		case D3DRTYPE_CUBETEXTURE:
		{
			D3D9ProxyCubeTexture* pDerivedTexture = static_cast<D3D9ProxyCubeTexture*> (pWrappedTexture);
			*ppActualLeftTexture = pDerivedTexture->getActualLeft();
			*ppActualRightTexture = pDerivedTexture->getActualRight();
			break;
		}

		default:
			OutputDebugString("Unhandled texture type in SetTexture\n");
			break;
	}

	if ((*ppActualLeftTexture) == NULL) {
		OutputDebugString("No left texture? Unpossible!\n");
		assert (false);
	}
}