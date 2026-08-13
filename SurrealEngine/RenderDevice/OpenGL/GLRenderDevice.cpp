
#include "Precomp.h"
#include "GLRenderDevice.h"
#include "GLCachedTexture.h"
#include "GLFileResource.h"
#include "Utils/UTF16.h"
#include "Utils/Logger.h"
#include "Math/halffloat.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include <surrealwidgets/core/widget.h>

GLRenderDevice::GLRenderDevice(Widget* InViewport)
{
	Viewport = InViewport;
	if (!Init(Viewport->GetNativePixelWidth(), Viewport->GetNativePixelHeight(), false))
		Exception::Throw("Could not init GLRenderDevice");
}

GLRenderDevice::~GLRenderDevice()
{
	Exit();
}

int GLRenderDevice::GetSettingsMultisample()
{
	switch (AntialiasMode)
	{
	default:
	case 0: return 0;
	case 1: return 2;
	case 2: return 4;
	}
}

bool GLRenderDevice::Init(int NewX, int NewY, bool Fullscreen)
{
#if 0
	ActiveHdr = Hdr;
	BufferCount = UseVSync ? 2 : 3;

	HDC screenDC = GetDC(0);
	DesktopResolution.Width = GetDeviceCaps(screenDC, HORZRES);
	DesktopResolution.Height = GetDeviceCaps(screenDC, VERTRES);
	ReleaseDC(0, screenDC);

	try
	{
		std::vector<D3D_FEATURE_LEVEL> featurelevels =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};

		UINT deviceFlags = GL_CREATE_DEVICE_SINGLETHREADED | GL_CREATE_DEVICE_BGRA_SUPPORT;
		if (UseDebugLayer)
			deviceFlags |= GL_CREATE_DEVICE_DEBUG;

		// First try use a more recent way of creating the device and swap chain
		HRESULT result = GLCreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			deviceFlags,
			featurelevels.data(), (UINT)featurelevels.size(),
			GL_SDK_VERSION,
			Device.TypedInitPtr(),
			&FeatureLevel,
			Context.TypedInitPtr());
		if (FAILED(result))
			LogMessage("GLDrv: Could not create a modern GL device");

		// Wonderful API you got here, Microsoft. Good job.
		ComPtr<IDXGIDevice2> dxgiDevice;
		ComPtr<IDXGIAdapter> dxgiAdapter;
		ComPtr<IDXGIFactory2> dxgiFactory;

		if (SUCCEEDED(result))
			result = Device->QueryInterface(dxgiDevice.GetIID(), dxgiDevice.InitPtr());
		else
			LogMessage("GLDrv: Could not get IDXGIDevice2 interface for the GL device");

		if (SUCCEEDED(result))
			result = dxgiDevice->GetParent(dxgiAdapter.GetIID(), dxgiAdapter.InitPtr());
		else
			LogMessage("GLDrv: Could not get IDXGIAdapter interface for the GL device");

		if (SUCCEEDED(result))
			result = dxgiAdapter->GetParent(dxgiFactory.GetIID(), dxgiFactory.InitPtr());
		else
			LogMessage("GLDrv: Could not get IDXGIFactory2 interface for the GL device");

		if (SUCCEEDED(result))
		{
			ComPtr<IDXGIFactory5> dxgiFactory5;
			result = dxgiFactory->QueryInterface(dxgiFactory5.GetIID(), dxgiFactory5.InitPtr());
			if (SUCCEEDED(result))
			{
				int support = 0;
				result = dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &support, sizeof(int));
				if (SUCCEEDED(result))
				{
					DxgiSwapChainAllowTearing = support != 0;
				}
				else
				{
					LogMessage("GLDrv: Device does not support DXGI_FEATURE_PRESENT_ALLOW_TEARING");
				}
			}
			else
			{
				LogMessage("GLDrv: Could not get IDXGIFactory5 interface for the GL device");
			}

			UINT swapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			if (DxgiSwapChainAllowTearing)
				swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

			DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
			swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapDesc.Width = NewX;
			swapDesc.Height = NewY;
			swapDesc.Format = ActiveHdr ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
			swapDesc.BufferCount = BufferCount;
			swapDesc.SampleDesc.Count = 1;
			swapDesc.Scaling = DXGI_SCALING_STRETCH;
			swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapDesc.Flags = swapChainFlags;
			swapDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
			result = dxgiFactory->CreateSwapChainForHwnd(Device, GetWindowHandle(), &swapDesc, nullptr, nullptr, SwapChain1.TypedInitPtr());
			if (SUCCEEDED(result))
			{
				dxgiFactory->MakeWindowAssociation(GetWindowHandle(), DXGI_MWA_NO_ALT_ENTER);
			}
			else
			{
				LogMessage("GLDrv: CreateSwapChainForHwnd failed");
				DxgiSwapChainAllowTearing = false;
			}
		}
		if (SUCCEEDED(result))
		{
			result = SwapChain1->QueryInterface(SwapChain.GetIID(), SwapChain.InitPtr());
			if (FAILED(result))
				SwapChain1.reset();
		}
		else
		{
			Context.reset();
			Device.reset();
		}
		dxgiFactory.reset();
		dxgiAdapter.reset();
		dxgiDevice.reset();

		// We still don't have a swap chain. Let's try the older Windows 7 API
		if (!SwapChain)
		{
			LogMessage("GLDrv: Modern GL device creation failed. Falling back to Windows 7");

			DXGI_SWAP_CHAIN_DESC swapDesc = {};
			swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapDesc.BufferDesc.Width = NewX;
			swapDesc.BufferDesc.Height = NewY;
			swapDesc.BufferDesc.Format = ActiveHdr ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
			swapDesc.BufferCount = BufferCount;
			swapDesc.SampleDesc.Count = 1;
			swapDesc.OutputWindow = GetWindowHandle();
			swapDesc.Windowed = GL_TRUE;
			swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			if (RefreshRate != 0)
			{
				swapDesc.BufferDesc.RefreshRate.Numerator = RefreshRate;
				swapDesc.BufferDesc.RefreshRate.Denominator = 1;
			}
			else
			{
				DEVMODE devmode = {};
				devmode.dmSize = sizeof(DEVMODE);
				if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devmode) && devmode.dmDisplayFrequency > 1)
				{
					swapDesc.BufferDesc.RefreshRate.Numerator = devmode.dmDisplayFrequency;
					swapDesc.BufferDesc.RefreshRate.Denominator = 1;
				}
			}

			// First try create a swap chain for Windows 8 and newer. If that fails, try the old for Windows 7
			HRESULT result = E_FAIL;
			for (DXGI_SWAP_EFFECT swapeffect : { DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_SWAP_EFFECT_DISCARD })
			{
				swapDesc.SwapEffect = swapeffect;

				result = GLCreateDeviceAndSwapChain(
					nullptr,
					D3D_DRIVER_TYPE_HARDWARE,
					0,
					deviceFlags,
					featurelevels.data(), (UINT)featurelevels.size(),
					GL_SDK_VERSION,
					&swapDesc,
					SwapChain.TypedInitPtr(),
					Device.TypedInitPtr(),
					&FeatureLevel,
					Context.TypedInitPtr());
				if (SUCCEEDED(result))
					break;

				LogMessage("GLDrv: Could not use DXGI_SWAP_EFFECT_FLIP_DISCARD. Falling back to DXGI_SWAP_EFFECT_DISCARD");
			}
			ThrowIfFailed(result, "GLCreateDeviceAndSwapChain failed");
		}

		SetDebugName(Device, "GLDrv.Device");
		SetDebugName(Context, "GLDrv.Context");

		CreateScenePass();
		CreatePresentPass();
		CreateBloomPass();

		Textures.reset(new GLTextureManager(this));
		Uploads.reset(new GLUploadManager(this));
	}
	catch (_com_error error)
	{
		LogMessage("Could not create d3d11 renderer: [_com_error] " + from_utf16(error.ErrorMessage()));
		Exit();
		return false;
	}
	catch (const std::exception& e)
	{
		LogMessage(std::string("Could not create d3d11 renderer: ") + e.what());
		Exit();
		return false;
	}

	if (!SetRes(NewX, NewY, Fullscreen))
	{
		Exit();
		return false;
	}
#endif
	return true;
}

bool GLRenderDevice::SetRes(int NewX, int NewY, bool Fullscreen)
{
#if 0
	ReleaseSwapChainResources();

	// Resize the swap chain buffers before doing the mode switch. Shouldn't really make any difference but you never know!
	if (Fullscreen)
	{
		UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		if (DxgiSwapChainAllowTearing)
			flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		SwapChain->ResizeBuffers(UseVSync ? 2 : 3, NewX, NewY, ActiveHdr ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM, flags);
	}

	HRESULT result;

	DXGI_MODE_DESC modeDesc = {};
	modeDesc.Width = NewX;
	modeDesc.Height = NewY;
	modeDesc.Format = ActiveHdr ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
	if (RefreshRate != 0)
	{
		modeDesc.RefreshRate.Numerator = RefreshRate;
		modeDesc.RefreshRate.Denominator = 1;
	}
	else
	{
		DEVMODE devmode = {};
		devmode.dmSize = sizeof(DEVMODE);
		if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devmode) && devmode.dmDisplayFrequency > 1)
		{
			modeDesc.RefreshRate.Numerator = devmode.dmDisplayFrequency;
			modeDesc.RefreshRate.Denominator = 1;
		}
	}

	if (Fullscreen)
	{
		IDXGIOutput* output = nullptr;
		result = SwapChain->GetContainingOutput(&output);
		if (SUCCEEDED(result))
		{
			DXGI_MODE_DESC modeToMatch = modeDesc;
			DXGI_MODE_DESC modeFound = {};
			result = output->FindClosestMatchingMode(&modeToMatch, &modeFound, Device);
			if (SUCCEEDED(result))
			{
				if (modeToMatch.Width == modeFound.Width && modeToMatch.Height == modeFound.Height)
				{
					modeDesc = modeFound;
				}
				else
				{
					LogMessage("FindClosestMatchingMode could not find a mode with the specified resolution");
				}
			}
			else
			{
				LogMessage("FindClosestMatchingMode failed");
			}
			NewX = modeDesc.Width;
			NewY = modeDesc.Height;
			output->Release();
		}
		else
		{
			LogMessage("GetContainingOutput failed");
		}

		result = SwapChain->SetFullscreenState(TRUE, nullptr);
		if (FAILED(result))
		{
			LogMessage("SwapChain.SetFullscreenState failed");
			// Don't fail this as it can happen if the application isn't the foreground process
		}

		result = SwapChain->ResizeTarget(&modeDesc);
		if (FAILED(result))
		{
			LogMessage("SwapChain.ResizeTarget failed");
		}
	}
	else
	{
		if (CurrentFullscreen)
		{
			result = SwapChain->SetFullscreenState(GL_FALSE, nullptr);
			if (FAILED(result))
			{
				LogMessage("SwapChain.SetFullscreenState failed");
				// Don't fail this as it can happen if the application isn't the foreground process
			}

			result = SwapChain->ResizeTarget(&modeDesc);
			if (FAILED(result))
			{
				LogMessage("SwapChain.ResizeTarget failed");
			}
		}
	}

	CurrentSizeX = NewX;
	CurrentSizeY = NewY;
	CurrentFullscreen = Fullscreen;

	BufferCount = UseVSync ? 2 : 3;
	if (!UpdateSwapChain())
		return false;

	Flush(1);
#endif
	return true;
}

void GLRenderDevice::ReleaseSwapChainResources()
{
#if 0
	BackBuffer.reset();
	BackBufferView.reset();
#endif
}

bool GLRenderDevice::UpdateSwapChain()
{
#if 0
	UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (DxgiSwapChainAllowTearing)
		flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	LogMessage("GLDrv: Updating SwapChain size to " + std::to_string(CurrentSizeX) + " x " + std::to_string(CurrentSizeY));

	HRESULT result = SwapChain->ResizeBuffers(BufferCount, CurrentSizeX, CurrentSizeY, ActiveHdr ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM, flags);
	if (FAILED(result))
	{
		return false;
	}

	if (CurrentSizeX && CurrentSizeY)
	{
		try
		{
			LogMessage("GLDrv: Resizing scene buffers to " + std::to_string(CurrentSizeX) + " x " + std::to_string(CurrentSizeY));

			ResizeSceneBuffers(CurrentSizeX, CurrentSizeY, GetSettingsMultisample());
		}
		catch (const std::exception& e)
		{
			LogMessage(std::string("Could not resize scene buffers: ") + e.what());
			return false;
		}
	}

	result = SwapChain->GetBuffer(0, __uuidof(IGLTexture2D), (void**)&BackBuffer);
	if (FAILED(result))
		return false;
	SetDebugName(BackBuffer, "BackBuffer");

	result = Device->CreateRenderTargetView(BackBuffer, nullptr, BackBufferView.TypedInitPtr());
	if (FAILED(result))
		return false;
	SetDebugName(BackBufferView, "BackBufferView");
#endif
	return true;
}

void GLRenderDevice::Exit()
{
#if 0
	LogMessage("GLDrv: exit called");

	UnmapVertices();

	ReleaseSwapChainResources();
	if (CurrentFullscreen && SwapChain)
		SwapChain->SetFullscreenState(GL_FALSE, nullptr);

	if (Context)
		Context->ClearState();

	Uploads.reset();
	Textures.reset();
	ReleasePresentPass();
	ReleaseBloomPass();
	ReleaseScenePass();
	ReleaseSceneBuffers();
	BackBufferView.reset();
	BackBuffer.reset();
	SwapChain.reset();
	SwapChain1.reset();
	Context.reset();

	if (DebugLayer)
	{
		DebugLayer->ReportLiveDeviceObjects(/*GL_RLDO_SUMMARY |*/ ((GL_RLDO_FLAGS)0x2)/*GL_RLDO_DETAIL*/ | ((GL_RLDO_FLAGS)0x4)/*GL_RLDO_IGNORE_INTERNAL*/);
	}

	InfoQueue.reset();
	DebugLayer.reset();

	if (Device)
	{
		Device->AddRef();
		int count = Device->Release();
		Device.reset();
		LogMessage("GLDrv: GLDrv.Device refcount is now " + std::to_string(count - 1));
	}
#endif
}

void GLRenderDevice::ResizeSceneBuffers(int width, int height, int multisample)
{
#if 0
	multisample = std::max(multisample, 1);

	if (SceneBuffers.Width == width && SceneBuffers.Height == height && multisample == SceneBuffers.Multisample && SceneBuffers.ColorBuffer && SceneBuffers.HitBuffer && SceneBuffers.PPHitBuffer && SceneBuffers.StagingHitBuffer && SceneBuffers.DepthBuffer && SceneBuffers.PPImage[0] && SceneBuffers.PPImage[1])
		return;

	SceneBuffers.ColorBufferView.reset();
	SceneBuffers.HitBufferView.reset();
	SceneBuffers.HitBufferShaderView.reset();
	SceneBuffers.PPHitBufferView.reset();
	SceneBuffers.DepthBufferView.reset();
	for (int i = 0; i < 2; i++)
	{
		SceneBuffers.PPImageShaderView[i].reset();
		SceneBuffers.PPImageView[i].reset();
		SceneBuffers.PPImage[i].reset();
	}
	SceneBuffers.ColorBuffer.reset();
	SceneBuffers.StagingHitBuffer.reset();
	SceneBuffers.PPHitBuffer.reset();
	SceneBuffers.HitBuffer.reset();
	SceneBuffers.DepthBuffer.reset();

	for (PPBlurLevel& level : SceneBuffers.BlurLevels)
	{
		level.VTexture.reset();
		level.VTextureRTV.reset();
		level.VTextureSRV.reset();
		level.HTexture.reset();
		level.HTextureRTV.reset();
		level.HTextureSRV.reset();
	}

	SceneBuffers.Width = width;
	SceneBuffers.Height = height;
	SceneBuffers.Multisample = multisample;

	GL_TEXTURE2D_DESC texDesc = {};
	texDesc.Usage = GL_USAGE_DEFAULT;
	texDesc.BindFlags = GL_BIND_RENDER_TARGET;
	texDesc.Width = SceneBuffers.Width;
	texDesc.Height = SceneBuffers.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = SceneBuffers.Multisample;
	texDesc.SampleDesc.Quality = SceneBuffers.Multisample > 1 ? GL_STANDARD_MULTISAMPLE_PATTERN : 0;
	HRESULT result = Device->CreateTexture2D(&texDesc, nullptr, SceneBuffers.ColorBuffer.TypedInitPtr());
	ThrowIfFailed(result, "CreateTexture2D(ColorBuffer) failed");
	SetDebugName(SceneBuffers.ColorBuffer, "SceneBuffers.ColorBuffer");

	texDesc = {};
	texDesc.Usage = GL_USAGE_DEFAULT;
	texDesc.BindFlags = GL_BIND_RENDER_TARGET | GL_BIND_SHADER_RESOURCE;
	texDesc.Width = SceneBuffers.Width;
	texDesc.Height = SceneBuffers.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_UINT;
	texDesc.SampleDesc.Count = SceneBuffers.Multisample;
	texDesc.SampleDesc.Quality = SceneBuffers.Multisample > 1 ? GL_STANDARD_MULTISAMPLE_PATTERN : 0;
	result = Device->CreateTexture2D(&texDesc, nullptr, SceneBuffers.HitBuffer.TypedInitPtr());
	ThrowIfFailed(result, "CreateTexture2D(HitBuffer) failed");
	SetDebugName(SceneBuffers.HitBuffer, "SceneBuffers.HitBuffer");

	texDesc = {};
	texDesc.Usage = GL_USAGE_DEFAULT;
	texDesc.BindFlags = GL_BIND_RENDER_TARGET;
	texDesc.Width = SceneBuffers.Width;
	texDesc.Height = SceneBuffers.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_UINT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	result = Device->CreateTexture2D(&texDesc, nullptr, SceneBuffers.PPHitBuffer.TypedInitPtr());
	ThrowIfFailed(result, "CreateTexture2D(PPHitBuffer) failed");
	SetDebugName(SceneBuffers.PPHitBuffer, "SceneBuffers.PPHitBuffer");

	texDesc = {};
	texDesc.Usage = GL_USAGE_STAGING;
	texDesc.BindFlags = 0;
	texDesc.Width = SceneBuffers.Width;
	texDesc.Height = SceneBuffers.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_UINT;
	texDesc.CPUAccessFlags = GL_CPU_ACCESS_READ;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	result = Device->CreateTexture2D(&texDesc, nullptr, SceneBuffers.StagingHitBuffer.TypedInitPtr());
	ThrowIfFailed(result, "CreateTexture2D(StagingHitBuffer) failed");
	SetDebugName(SceneBuffers.StagingHitBuffer, "SceneBuffers.StagingHitBuffer");

	texDesc = {};
	texDesc.Usage = GL_USAGE_DEFAULT;
	texDesc.BindFlags = GL_BIND_DEPTH_STENCIL;
	texDesc.Width = SceneBuffers.Width;
	texDesc.Height = SceneBuffers.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_D32_FLOAT;
	texDesc.SampleDesc.Count = SceneBuffers.Multisample;
	texDesc.SampleDesc.Quality = SceneBuffers.Multisample > 1 ? GL_STANDARD_MULTISAMPLE_PATTERN : 0;
	result = Device->CreateTexture2D(&texDesc, nullptr, SceneBuffers.DepthBuffer.TypedInitPtr());
	ThrowIfFailed(result, "CreateTexture2D(DepthBuffer) failed");
	SetDebugName(SceneBuffers.DepthBuffer, "SceneBuffers.DepthBuffer");

	for (int i = 0; i < 2; i++)
	{
		texDesc = {};
		texDesc.Usage = GL_USAGE_DEFAULT;
		texDesc.BindFlags = GL_BIND_RENDER_TARGET | GL_BIND_SHADER_RESOURCE;
		texDesc.Width = SceneBuffers.Width;
		texDesc.Height = SceneBuffers.Height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		result = Device->CreateTexture2D(&texDesc, nullptr, SceneBuffers.PPImage[i].TypedInitPtr());
		ThrowIfFailed(result, "CreateTexture2D(PPImage) failed");
		SetDebugName(SceneBuffers.PPImage[i], "SceneBuffers.PPImage");
	}

	result = Device->CreateRenderTargetView(SceneBuffers.ColorBuffer, nullptr, SceneBuffers.ColorBufferView.TypedInitPtr());
	ThrowIfFailed(result, "CreateRenderTargetView(ColorBuffer) failed");
	SetDebugName(SceneBuffers.ColorBufferView, "SceneBuffers.ColorBufferView");

	result = Device->CreateRenderTargetView(SceneBuffers.HitBuffer, nullptr, SceneBuffers.HitBufferView.TypedInitPtr());
	ThrowIfFailed(result, "CreateRenderTargetView(HitBuffer) failed");
	SetDebugName(SceneBuffers.HitBufferView, "SceneBuffers.HitBufferView");

	result = Device->CreateShaderResourceView(SceneBuffers.HitBuffer, nullptr, SceneBuffers.HitBufferShaderView.TypedInitPtr());
	ThrowIfFailed(result, "CreateShaderResourceView(HitBuffer) failed");
	SetDebugName(SceneBuffers.HitBufferShaderView, "SceneBuffers.HitBufferShaderView");

	result = Device->CreateRenderTargetView(SceneBuffers.PPHitBuffer, nullptr, SceneBuffers.PPHitBufferView.TypedInitPtr());
	ThrowIfFailed(result, "CreateRenderTargetView(PPHitBuffer) failed");
	SetDebugName(SceneBuffers.PPHitBufferView, "SceneBuffers.PPHitBufferView");

	result = Device->CreateDepthStencilView(SceneBuffers.DepthBuffer, nullptr, SceneBuffers.DepthBufferView.TypedInitPtr());
	ThrowIfFailed(result, "CreateDepthStencilView(DepthBuffer) failed");
	SetDebugName(SceneBuffers.DepthBufferView, "SceneBuffers.DepthBufferView");

	for (int i = 0; i < 2; i++)
	{
		result = Device->CreateRenderTargetView(SceneBuffers.PPImage[i], nullptr, SceneBuffers.PPImageView[i].TypedInitPtr());
		ThrowIfFailed(result, "CreateRenderTargetView(PPImage) failed");
		SetDebugName(SceneBuffers.PPImageView[i], "SceneBuffers.PPImageView");

		result = Device->CreateShaderResourceView(SceneBuffers.PPImage[i], nullptr, SceneBuffers.PPImageShaderView[i].TypedInitPtr());
		ThrowIfFailed(result, "CreateShaderResourceView(PPImage) failed");
		SetDebugName(SceneBuffers.PPImageShaderView[i], "SceneBuffers.PPImageShaderView");
	}

	int bloomWidth = width;
	int bloomHeight = height;
	for (PPBlurLevel& level : SceneBuffers.BlurLevels)
	{
		bloomWidth = (bloomWidth + 1) / 2;
		bloomHeight = (bloomHeight + 1) / 2;

		texDesc = {};
		texDesc.Usage = GL_USAGE_DEFAULT;
		texDesc.BindFlags = GL_BIND_RENDER_TARGET | GL_BIND_SHADER_RESOURCE;
		texDesc.Width = bloomWidth;
		texDesc.Height = bloomHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;

		result = Device->CreateTexture2D(&texDesc, nullptr, level.VTexture.TypedInitPtr());
		ThrowIfFailed(result, "CreateTexture2D(SceneBuffers.BlurLevels.VTexture) failed");
		SetDebugName(level.VTexture, "SceneBuffers.BlurLevels.VTexture");

		result = Device->CreateTexture2D(&texDesc, nullptr, level.HTexture.TypedInitPtr());
		ThrowIfFailed(result, "CreateTexture2D(SceneBuffers.BlurLevels.HTexture) failed");
		SetDebugName(level.HTexture, "SceneBuffers.BlurLevels.HTexture");

		result = Device->CreateRenderTargetView(level.VTexture, nullptr, level.VTextureRTV.TypedInitPtr());
		ThrowIfFailed(result, "CreateRenderTargetView(SceneBuffers.BlurLevels.VTextureRTV) failed");
		SetDebugName(level.VTextureRTV, "SceneBuffers.BlurLevels.VTextureRTV");

		result = Device->CreateRenderTargetView(level.HTexture, nullptr, level.HTextureRTV.TypedInitPtr());
		ThrowIfFailed(result, "CreateRenderTargetView(SceneBuffers.BlurLevels.HTextureRTV) failed");
		SetDebugName(level.HTextureRTV, "SceneBuffers.BlurLevels.HTextureRTV");

		result = Device->CreateShaderResourceView(level.VTexture, nullptr, level.VTextureSRV.TypedInitPtr());
		ThrowIfFailed(result, "CreateRenderTargetView(SceneBuffers.BlurLevels.VTextureSRV) failed");
		SetDebugName(level.VTextureSRV, "SceneBuffers.BlurLevels.VTextureSRV");

		result = Device->CreateShaderResourceView(level.HTexture, nullptr, level.HTextureSRV.TypedInitPtr());
		ThrowIfFailed(result, "CreateRenderTargetView(SceneBuffers.BlurLevels.HTextureSRV) failed");
		SetDebugName(level.HTextureSRV, "SceneBuffers.BlurLevels.HTextureSRV");

		level.Width = bloomWidth;
		level.Height = bloomHeight;
	}
#endif
}

void GLRenderDevice::CreateScenePass()
{
#if 0
	std::vector<GL_INPUT_ELEMENT_DESC> elements =
	{
		{ "AttrFlags", 0, DXGI_FORMAT_R32_UINT, 0, offsetof(GLSceneVertex, Flags), GL_INPUT_PER_VERTEX_DATA, 0 },
		{ "AttrPos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GLSceneVertex, Position), GL_INPUT_PER_VERTEX_DATA, 0 },
		{ "AttrTexCoordOne", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(GLSceneVertex, TexCoord), GL_INPUT_PER_VERTEX_DATA, 0 },
		{ "AttrTexCoordTwo", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(GLSceneVertex, TexCoord2), GL_INPUT_PER_VERTEX_DATA, 0 },
		{ "AttrTexCoordThree", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(GLSceneVertex, TexCoord3), GL_INPUT_PER_VERTEX_DATA, 0 },
		{ "AttrTexCoordFour", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(GLSceneVertex, TexCoord4), GL_INPUT_PER_VERTEX_DATA, 0 },
		{ "AttrColor", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(GLSceneVertex, Color), GL_INPUT_PER_VERTEX_DATA, 0 }
	};
#endif
	ScenePass.VertexShader = CreateVertexShader("ScenePass.VertexShader", "shaders/Scene.vert");
	ScenePass.PixelShader = CreateFragmentShader("ScenePass.PixelShader", "shaders/Scene.frag");
	ScenePass.PixelShaderAlphaTest = CreateFragmentShader("ScenePass.PixelShaderAlphaTest", "shaders/Scene.frag", { "ALPHATEST" });

	CreateSceneSamplers();

	for (int i = 0; i < 2; i++)
	{
		GLRasterizerDesc rasterizerDesc = {};
		rasterizerDesc.CullEnable = false;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.DepthClipEnable = false; // Avoid clipping the weapon. The UE1 engine clips the geometry anyway.
		rasterizerDesc.MultisampleEnable = i == 1;
		ScenePass.RasterizerState[i] = CreateRasterizerState(rasterizerDesc);
	}

	for (int i = 0; i < 33; i++)
	{
		GLBlendDesc blendDesc = {};
		blendDesc.IndependentBlendEnable = true;
		blendDesc.RenderTarget[0].BlendEnable = true;
		if (i < 32)
		{
			switch (i & 3)
			{
			case 0: // PF_Translucent
				blendDesc.RenderTarget[0].BlendOp = GL_FUNC_ADD;
				blendDesc.RenderTarget[0].BlendOpAlpha = GL_FUNC_ADD;
				blendDesc.RenderTarget[0].SrcBlend = GL_ONE;
				blendDesc.RenderTarget[0].SrcBlendAlpha = GL_ONE;
				blendDesc.RenderTarget[0].DestBlend = GL_ONE_MINUS_SRC_COLOR;
				blendDesc.RenderTarget[0].DestBlendAlpha = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case 1: // PF_Modulated
				blendDesc.RenderTarget[0].BlendOp = GL_FUNC_ADD;
				blendDesc.RenderTarget[0].BlendOpAlpha = GL_FUNC_ADD;
				blendDesc.RenderTarget[0].SrcBlend = GL_DST_COLOR;
				blendDesc.RenderTarget[0].SrcBlendAlpha = GL_DST_ALPHA;
				blendDesc.RenderTarget[0].DestBlend = GL_SRC_COLOR;
				blendDesc.RenderTarget[0].DestBlendAlpha = GL_BLEND_SRC_ALPHA;
				break;
			case 2: // PF_Highlighted
				blendDesc.RenderTarget[0].BlendOp = GL_FUNC_ADD;
				blendDesc.RenderTarget[0].BlendOpAlpha = GL_FUNC_ADD;
				blendDesc.RenderTarget[0].SrcBlend = GL_ONE;
				blendDesc.RenderTarget[0].SrcBlendAlpha = GL_ONE;
				blendDesc.RenderTarget[0].DestBlend = GL_ONE_MINUS_SRC_ALPHA;
				blendDesc.RenderTarget[0].DestBlendAlpha = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case 3: // Hmm, is it faster to keep the blend mode enabled or to toggle it?
				blendDesc.RenderTarget[0].BlendOp = GL_FUNC_ADD;
				blendDesc.RenderTarget[0].BlendOpAlpha = GL_FUNC_ADD;
				blendDesc.RenderTarget[0].SrcBlend = GL_ONE;
				blendDesc.RenderTarget[0].SrcBlendAlpha = GL_ONE;
				blendDesc.RenderTarget[0].DestBlend = GL_ZERO;
				blendDesc.RenderTarget[0].DestBlendAlpha = GL_ZERO;
				break;
			}
			if (i & 4) // PF_Invisible
				blendDesc.RenderTarget[0].RenderTargetWriteMask = false;
			else
				blendDesc.RenderTarget[0].RenderTargetWriteMask = true;
		}
		else // PF_SubpixelFont
		{
			blendDesc.RenderTarget[0].BlendOp = GL_FUNC_ADD;
			blendDesc.RenderTarget[0].BlendOpAlpha = GL_FUNC_ADD;
			blendDesc.RenderTarget[0].SrcBlend = GL_CONSTANT_COLOR;
			blendDesc.RenderTarget[0].SrcBlendAlpha = GL_CONSTANT_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = GL_ONE_MINUS_SRC_COLOR;
			blendDesc.RenderTarget[0].DestBlendAlpha = GL_ONE_MINUS_SRC_ALPHA;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = true;
		}
		blendDesc.RenderTarget[1].BlendEnable = false;
		blendDesc.RenderTarget[1].RenderTargetWriteMask = true;
		ScenePass.Pipelines[i].BlendState = CreateBlendState(blendDesc);

		GLDepthStencilDesc depthStencilDesc = {};
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthFunc = GL_LEQUAL;
		if (i & 8) // PF_Occlude
			depthStencilDesc.DepthWriteMask = true;
		else
			depthStencilDesc.DepthWriteMask = false;
		ScenePass.Pipelines[i].DepthStencilState = CreateDepthStencilState(depthStencilDesc);

		if ((i & 16) || i == 32) // PF_Masked or PF_SubpixelFont
			ScenePass.Pipelines[i].PixelShader = ScenePass.PixelShaderAlphaTest.get();
		else
			ScenePass.Pipelines[i].PixelShader = ScenePass.PixelShader.get();

		ScenePass.Pipelines[i].PrimitiveTopology = GL_TRIANGLES;
	}

	// Line pipeline
	for (int i = 0; i < 2; i++)
	{
		GLBlendDesc blendDesc = {};
		blendDesc.IndependentBlendEnable = true;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].BlendOp = GL_FUNC_ADD;
		blendDesc.RenderTarget[0].BlendOpAlpha = GL_FUNC_ADD;
		blendDesc.RenderTarget[0].SrcBlend = GL_ONE;
		blendDesc.RenderTarget[0].SrcBlendAlpha = GL_ONE;
		blendDesc.RenderTarget[0].DestBlend = GL_ONE_MINUS_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlendAlpha = GL_ONE_MINUS_SRC_ALPHA;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = true;
		blendDesc.RenderTarget[1].BlendEnable = false;
		blendDesc.RenderTarget[1].RenderTargetWriteMask = true;
		ScenePass.LinePipeline[i].BlendState = CreateBlendState(blendDesc);

		GLDepthStencilDesc depthStencilDesc = {};
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthFunc = GL_LEQUAL;
		depthStencilDesc.DepthWriteMask = true;
		ScenePass.LinePipeline[i].DepthStencilState = CreateDepthStencilState(depthStencilDesc);

		ScenePass.LinePipeline[i].PixelShader = ScenePass.PixelShader.get();
		ScenePass.LinePipeline[i].PrimitiveTopology = GL_LINES;

		if (i == 0)
		{
			ScenePass.LinePipeline[i].MinDepth = 0.0f;
			ScenePass.LinePipeline[i].MaxDepth = 0.1f;
		}
	}

	// Point pipeline
	for (int i = 0; i < 2; i++)
	{
		GLBlendDesc blendDesc = {};
		blendDesc.IndependentBlendEnable = true;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].BlendOp = GL_FUNC_ADD;
		blendDesc.RenderTarget[0].BlendOpAlpha = GL_FUNC_ADD;
		blendDesc.RenderTarget[0].SrcBlend = GL_ONE;
		blendDesc.RenderTarget[0].SrcBlendAlpha = GL_ONE;
		blendDesc.RenderTarget[0].DestBlend = GL_ONE_MINUS_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlendAlpha = GL_ONE_MINUS_SRC_ALPHA;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = true;
		blendDesc.RenderTarget[1].BlendEnable = false;
		blendDesc.RenderTarget[1].RenderTargetWriteMask = true;
		ScenePass.PointPipeline[i].BlendState = CreateBlendState(blendDesc);

		GLDepthStencilDesc depthStencilDesc = {};
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthFunc = GL_LEQUAL;
		depthStencilDesc.DepthWriteMask = true;
		ScenePass.PointPipeline[i].DepthStencilState = CreateDepthStencilState(depthStencilDesc);

		ScenePass.PointPipeline[i].PixelShader = ScenePass.PixelShader.get();
		ScenePass.PointPipeline[i].PrimitiveTopology = GL_TRIANGLES;

		if (i == 0)
		{
			ScenePass.PointPipeline[i].MinDepth = 0.0f;
			ScenePass.PointPipeline[i].MaxDepth = 0.1f;
		}
	}

	ScenePass.VertexBuffer = CreateBuffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, nullptr, SceneVertexBufferSize * sizeof(GLSceneVertex), "ScenePass.VertexBuffer");
	ScenePass.IndexBuffer = CreateBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW, nullptr, SceneIndexBufferSize * sizeof(uint32_t), "ScenePass.IndexBuffer");
	ScenePass.ConstantBuffer = CreateBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, nullptr, sizeof(GLScenePushConstants), "ScenePass.ConstantBuffer");
}

void GLRenderDevice::CreateSceneSamplers()
{
	for (int i = 0; i < 16; i++)
	{
		ScenePass.Samplers[i] = std::make_shared<GLSampler>();
		SetDebugName(ScenePass.Samplers[i], "ScenePass.Samplers");

		GLuint sampler = ScenePass.Samplers[i]->Handle;

		int dummyMipmapCount = (i >> 2) & 3;
		GLint addressmode = (i & 2) ? GL_MIRROR_CLAMP_TO_EDGE : GL_MIRRORED_REPEAT;

		glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, (i & 1) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, (i & 1) ? GL_NEAREST : GL_LINEAR);
		if (i & 1)
			glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f); // To do: we should check for this extension
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, addressmode);
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, addressmode);
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, addressmode);
		glSamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, (float)dummyMipmapCount);
		glSamplerParameterf(sampler, GL_TEXTURE_LOD_BIAS, (float)dummyMipmapCount + LODBias);
	}

	ScenePass.LODBias = LODBias;
}

void GLRenderDevice::ReleaseSceneSamplers()
{
	for (auto& sampler : ScenePass.Samplers)
	{
		sampler.reset();
	}
	ScenePass.LODBias = 0.0f;
}

void GLRenderDevice::UpdateLODBias()
{
	if (ScenePass.LODBias != LODBias)
	{
		ReleaseSceneSamplers();
		CreateSceneSamplers();
	}
}

void GLRenderDevice::ReleaseScenePass()
{
	ScenePass.VertexShader.reset();
	ScenePass.InputLayout.reset();
	ScenePass.VertexBuffer.reset();
	ScenePass.IndexBuffer.reset();
	ScenePass.ConstantBuffer.reset();
	ScenePass.RasterizerState[0].reset();
	ScenePass.RasterizerState[1].reset();
	ScenePass.PixelShader.reset();
	ScenePass.PixelShaderAlphaTest.reset();
	ReleaseSceneSamplers();
	for (auto& pipeline : ScenePass.Pipelines)
	{
		pipeline.BlendState.reset();
		pipeline.DepthStencilState.reset();
	}
	for (int i = 0; i < 2; i++)
	{
		ScenePass.LinePipeline[i].BlendState.reset();
		ScenePass.LinePipeline[i].DepthStencilState.reset();
		ScenePass.PointPipeline[i].BlendState.reset();
		ScenePass.PointPipeline[i].DepthStencilState.reset();
	}
}

void GLRenderDevice::ReleaseBloomPass()
{
	BloomPass.Extract.reset();
	BloomPass.Combine.reset();
	BloomPass.BlurVertical.reset();
	BloomPass.BlurHorizontal.reset();
	BloomPass.ConstantBuffer.reset();
	BloomPass.AdditiveBlendState.reset();
}

void GLRenderDevice::ReleasePresentPass()
{
	PresentPass.PPStepLayout.reset();
	PresentPass.PPStep.reset();
	PresentPass.PPStepVertexBuffer.reset();
	PresentPass.HitResolve.reset();
	for (auto& shader : PresentPass.Present) shader.reset();
	PresentPass.PresentConstantBuffer.reset();
	PresentPass.DitherTextureView.reset();
	PresentPass.DitherTexture.reset();
	PresentPass.BlendState.reset();
	PresentPass.DepthStencilState.reset();
	PresentPass.RasterizerState.reset();
}

void GLRenderDevice::ReleaseSceneBuffers()
{
	SceneBuffers.ColorBufferView.reset();
	SceneBuffers.HitBufferView.reset();
	SceneBuffers.HitBufferShaderView.reset();
	SceneBuffers.PPHitBufferView.reset();
	SceneBuffers.DepthBufferView.reset();
	for (int i = 0; i < 2; i++)
	{
		SceneBuffers.PPImageShaderView[i].reset();
		SceneBuffers.PPImageView[i].reset();
		SceneBuffers.PPImage[i].reset();
	}
	SceneBuffers.ColorBuffer.reset();
	SceneBuffers.StagingHitBuffer.reset();
	SceneBuffers.PPHitBuffer.reset();
	SceneBuffers.HitBuffer.reset();
	SceneBuffers.DepthBuffer.reset();
	for (PPBlurLevel& level : SceneBuffers.BlurLevels)
	{
		level.VTexture.reset();
		level.VTextureRTV.reset();
		level.VTextureSRV.reset();
		level.HTexture.reset();
		level.HTextureRTV.reset();
		level.HTextureSRV.reset();
	}
}

GLRenderDevice::ScenePipelineState* GLRenderDevice::GetPipeline(uint32_t PolyFlags)
{
	int index;
	if (PolyFlags & PF_Translucent)
	{
		index = 0;
	}
	else if (PolyFlags & PF_Modulated)
	{
		index = 1;
	}
	else if (PolyFlags & PF_Highlighted)
	{
		index = 2;
	}
	else
	{
		index = 3;
	}

	if (PolyFlags & PF_Invisible)
	{
		index |= 4;
	}
	if (PolyFlags & PF_Occlude)
	{
		index |= 8;
	}
	if (PolyFlags & PF_Masked)
	{
		index |= 16;
	}

	if (PolyFlags == PF_SubpixelFont)
	{
		index = 32;
	}

	return &ScenePass.Pipelines[index];
}

void GLRenderDevice::RunBloomPass()
{
#if 0
	GLRenderTargetView* rtvs[1] = {};
	GLShaderResourceView* srvs[1] = {};

	float blurAmount = 0.6f + BloomAmount * (1.9f / 255.0f);
	GLBloomPushConstants pushconstants;
	ComputeBlurSamples(7, blurAmount, pushconstants.SampleWeights);

	GLBuffer* vertexBuffers[1] = { PresentPass.PPStepVertexBuffer.get() };
	GLBuffer* cbs[1] = { BloomPass.ConstantBuffer.get() };
	int stride = sizeof(vec2);
	int offset = 0;
	Context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
	Context->IASetInputLayout(PresentPass.PPStepLayout);
	Context->IASetPrimitiveTopology(GL_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->VSSetShader(PresentPass.PPStep, nullptr, 0);
	Context->RSSetState(PresentPass.RasterizerState);
	Context->PSSetConstantBuffers(0, 1, cbs);
	Context->OMSetDepthStencilState(PresentPass.DepthStencilState, 0);
	Context->OMSetBlendState(PresentPass.BlendState, nullptr, 0xffffffff);
	SetBufferData(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, BloomPass.ConstantBuffer.get(), &pushconstants, sizeof(GLBloomPushConstants));

	GLViewport viewport = {};
	viewport.MaxDepth = 1.0f;

	// Extract overbright pixels that we want to bloom:
	viewport.Width = (float)SceneBuffers.BlurLevels[0].Width;
	viewport.Height = (float)SceneBuffers.BlurLevels[0].Height;
	rtvs[0] = SceneBuffers.BlurLevels[0].VTextureRTV.get();
	srvs[0] = SceneBuffers.PPImageShaderView[0].get();
	Context->OMSetRenderTargets(1, rtvs, nullptr);
	SetViewport(viewport);
	Context->PSSetShader(BloomPass.Extract, nullptr, 0);
	Context->PSSetShaderResources(0, 1, srvs);
	Context->Draw(6, 0);

	// Blur and downscale:
	for (int i = 0; i < SceneBuffers.NumBloomLevels - 1; i++)
	{
		auto& blevel = SceneBuffers.BlurLevels[i];
		auto& next = SceneBuffers.BlurLevels[i + 1];

		viewport.Width = (float)blevel.Width;
		viewport.Height = (float)blevel.Height;
		SetViewport(viewport);
		BlurStep(blevel.VTextureSRV, blevel.HTextureRTV, false);
		BlurStep(blevel.HTextureSRV, blevel.VTextureRTV, true);

		// Linear downscale:
		viewport.Width = (float)next.Width;
		viewport.Height = (float)next.Height;
		rtvs[0] = next.VTextureRTV.get();
		srvs[0] = blevel.VTextureSRV.get();
		Context->OMSetRenderTargets(1, rtvs, nullptr);
		SetViewport(viewport);
		Context->PSSetShader(BloomPass.Combine, nullptr, 0);
		Context->PSSetShaderResources(0, 1, srvs);
		Context->Draw(6, 0);
	}

	// Blur and upscale:
	for (int i = SceneBuffers.NumBloomLevels - 1; i > 0; i--)
	{
		auto& blevel = SceneBuffers.BlurLevels[i];
		auto& next = SceneBuffers.BlurLevels[i - 1];

		viewport.Width = (float)blevel.Width;
		viewport.Height = (float)blevel.Height;
		SetViewport(viewport);
		BlurStep(blevel.VTextureSRV.get(), blevel.HTextureRTV.get(), false);
		BlurStep(blevel.HTextureSRV.get(), blevel.VTextureRTV.get(), true);

		// Linear upscale:
		viewport.Width = (float)next.Width;
		viewport.Height = (float)next.Height;
		rtvs[0] = next.VTextureRTV.get();
		srvs[0] = blevel.VTextureSRV.get();
		Context->OMSetRenderTargets(1, rtvs, nullptr);
		SetViewport(viewport);
		Context->PSSetShader(BloomPass.Combine, nullptr, 0);
		Context->PSSetShaderResources(0, 1, srvs);
		Context->Draw(6, 0);
	}

	viewport.Width = (float)SceneBuffers.BlurLevels[0].Width;
	viewport.Height = (float)SceneBuffers.BlurLevels[0].Height;
	SetViewport(viewport);
	BlurStep(SceneBuffers.BlurLevels[0].VTextureSRV.get(), SceneBuffers.BlurLevels[0].HTextureRTV.get(), false);
	BlurStep(SceneBuffers.BlurLevels[0].HTextureSRV.get(), SceneBuffers.BlurLevels[0].VTextureRTV.get(), true);

	// Add bloom back to scene post process texture:
	viewport.Width = (float)SceneBuffers.Width;
	viewport.Height = (float)SceneBuffers.Height;
	rtvs[0] = SceneBuffers.PPImageView[0].get();
	srvs[0] = SceneBuffers.BlurLevels[0].VTextureSRV.get();
	Context->OMSetRenderTargets(1, rtvs, nullptr);
	Context->OMSetBlendState(BloomPass.AdditiveBlendState, nullptr, 0xffffffff);
	SetViewport(viewport);
	Context->PSSetShader(BloomPass.Combine, nullptr, 0);
	Context->PSSetShaderResources(0, 1, srvs);
	Context->Draw(6, 0);
#endif
}

void GLRenderDevice::BlurStep(GLShaderResourceView* input, GLRenderTargetView* output, bool vertical)
{
#if 0
	Context->OMSetRenderTargets(1, &output, nullptr);
	Context->PSSetShader(vertical ? BloomPass.BlurVertical : BloomPass.BlurHorizontal, nullptr, 0);
	Context->PSSetShaderResources(0, 1, &input);
	Context->Draw(6, 0);
#endif
}

float GLRenderDevice::ComputeBlurGaussian(float n, float theta) // theta = Blur Amount
{
	return (float)((1.0f / std::sqrtf(2 * 3.14159265359f * theta)) * std::expf(-(n * n) / (2.0f * theta * theta)));
}

void GLRenderDevice::ComputeBlurSamples(int sampleCount, float blurAmount, float* sampleWeights)
{
	sampleWeights[0] = ComputeBlurGaussian(0, blurAmount);

	float totalWeights = sampleWeights[0];

	for (int i = 0; i < sampleCount / 2; i++)
	{
		float weight = ComputeBlurGaussian(i + 1.0f, blurAmount);

		sampleWeights[i * 2 + 1] = weight;
		sampleWeights[i * 2 + 2] = weight;

		totalWeights += weight * 2;
	}

	for (int i = 0; i < sampleCount; i++)
	{
		sampleWeights[i] /= totalWeights;
	}
}

void GLRenderDevice::CreateBloomPass()
{
	BloomPass.Extract = CreateFragmentShader("BloomPass.Extract", "shaders/BloomExtract.frag");
	BloomPass.Combine = CreateFragmentShader("BloomPass.Combine", "shaders/BloomCombine.frag");
	BloomPass.BlurVertical = CreateFragmentShader("BloomPass.BlurVertical", "shaders/Blur.frag", { "BLUR_VERTICAL" });
	BloomPass.BlurHorizontal = CreateFragmentShader("BloomPass.BlurHorizontal", "shaders/Blur.frag", { "BLUR_HORIZONTAL" });

	BloomPass.ConstantBuffer = CreateBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, nullptr, sizeof(GLBloomPushConstants), "BloomPass.ConstantBuffer");

	GLBlendDesc blendDesc = {};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = true;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].BlendOp = GL_FUNC_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = GL_FUNC_ADD;
	blendDesc.RenderTarget[0].SrcBlend = GL_ONE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = GL_ONE;
	blendDesc.RenderTarget[0].DestBlend = GL_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = GL_ONE;
	BloomPass.AdditiveBlendState = CreateBlendState(blendDesc);
}

void GLRenderDevice::CreatePresentPass()
{
	std::vector<vec2> positions =
	{
		vec2(-1.0, -1.0),
		vec2( 1.0, -1.0),
		vec2(-1.0,  1.0),
		vec2(-1.0,  1.0),
		vec2( 1.0, -1.0),
		vec2( 1.0,  1.0)
	};

	PresentPass.PPStepVertexBuffer = CreateBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, positions.data(), positions.size() * sizeof(vec2), "PresentPass.PPStepVertexBuffer");
	PresentPass.PresentConstantBuffer = CreateBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, nullptr, sizeof(GLPresentPushConstants), "PresentPass.PresentConstantBuffer");

#if 0
	std::vector<GL_INPUT_ELEMENT_DESC> elements =
	{
		{ "AttrPos", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, GL_INPUT_PER_VERTEX_DATA, 0 }
	};
#endif

	PresentPass.PPStep = CreateVertexShader("PresentPass.PPStep", "shaders/PPStep.vert");

	static const char* transferFunctions[2] = { nullptr, "HDR_MODE" };
	static const char* gammaModes[2] = { "GAMMA_MODE_D3D9", "GAMMA_MODE_XOPENGL" };
	static const char* colorModes[4] = { nullptr, "COLOR_CORRECT_MODE0", "COLOR_CORRECT_MODE1", "COLOR_CORRECT_MODE2" };
	for (int i = 0; i < 16; i++)
	{
		std::vector<std::string> defines;
		if (transferFunctions[i & 1]) defines.push_back(transferFunctions[i & 1]);
		if (gammaModes[(i >> 1) & 1]) defines.push_back(gammaModes[(i >> 1) & 1]);
		if (colorModes[(i >> 2) & 3]) defines.push_back(colorModes[(i >> 2) & 3]);

		PresentPass.Present[i] = CreateFragmentShader("PresentPass.Present", "shaders/Present.frag", defines);
	}

	PresentPass.HitResolve = CreateFragmentShader("PresentPass.HitResolve", "shaders/HitResolve.frag");

	static const float ditherdata[64] =
	{
		.0078125, .2578125, .1328125, .3828125, .0234375, .2734375, .1484375, .3984375,
		.7578125, .5078125, .8828125, .6328125, .7734375, .5234375, .8984375, .6484375,
		.0703125, .3203125, .1953125, .4453125, .0859375, .3359375, .2109375, .4609375,
		.8203125, .5703125, .9453125, .6953125, .8359375, .5859375, .9609375, .7109375,
		.0390625, .2890625, .1640625, .4140625, .0546875, .3046875, .1796875, .4296875,
		.7890625, .5390625, .9140625, .6640625, .8046875, .5546875, .9296875, .6796875,
		.1015625, .3515625, .2265625, .4765625, .1171875, .3671875, .2421875, .4921875,
		.8515625, .6015625, .9765625, .7265625, .8671875, .6171875, .9921875, .7421875
	};

	PresentPass.DitherTexture = std::make_shared<GLTexture>();
	glBindTexture(GL_TEXTURE_2D, PresentPass.DitherTexture->Handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 8, 8, 0, GL_RED, GL_FLOAT, ditherdata);
	ThrowIfGLError("CreateTexture2D(DitherTexture) failed");
	SetDebugName(PresentPass.DitherTexture, "PresentPass.DitherTexture");

	PresentPass.DitherTextureView = PresentPass.DitherTexture;

	GLBlendDesc blendDesc = {};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = true;
	PresentPass.BlendState = CreateBlendState(blendDesc);

	GLDepthStencilDesc depthStencilDesc = {};
	depthStencilDesc.DepthFunc = GL_ALWAYS;
	PresentPass.DepthStencilState = CreateDepthStencilState(depthStencilDesc);

	GLRasterizerDesc rasterizerDesc = {};
	rasterizerDesc.CullEnable = false;
	PresentPass.RasterizerState = CreateRasterizerState(rasterizerDesc);
}

void GLRenderDevice::Flush(bool AllowPrecache)
{
	DrawBatches();
	ClearTextureCache();

	if (AllowPrecache && UsePrecache)
		PrecacheOnFlip = 1;
}

void GLRenderDevice::MapVertices(bool nextBuffer)
{
	// To do: OpenGL doesn't have D3D11_MAP_WRITE_NO_OVERWRITE.
	// This will ruin the performance when nextBuffer is false.
	// It _does_ have persistent buffers that we could utilize.

	if (!SceneVertices)
	{
		glBindBuffer(GL_ARRAY_BUFFER, ScenePass.VertexBuffer->Handle);
		if (nextBuffer)
			glBufferData(GL_ARRAY_BUFFER, SceneVertexBufferSize * sizeof(GLSceneVertex), nullptr, GL_DYNAMIC_DRAW);
		SceneVertices = (GLSceneVertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}

	if (!SceneIndexes)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ScenePass.IndexBuffer->Handle);
		if (nextBuffer)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, SceneIndexBufferSize * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
		SceneIndexes = (uint32_t*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
}

void GLRenderDevice::UnmapVertices()
{
	if (SceneVertices)
	{
		glBindBuffer(GL_ARRAY_BUFFER, ScenePass.VertexBuffer->Handle);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		SceneVertices = nullptr;
	}

	if (SceneIndexes)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ScenePass.IndexBuffer->Handle);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		SceneIndexes = nullptr;
	}
}

void GLRenderDevice::Lock(vec4 InFlashScale, vec4 InFlashFog, vec4 ScreenClear, uint8_t* InHitData, int* InHitSize)
{
#if 0
	if (Viewport->GetNativePixelWidth() != CurrentSizeX || Viewport->GetNativePixelHeight() != CurrentSizeY)
	{
		if (!SetRes(Viewport->GetNativePixelWidth(), Viewport->GetNativePixelHeight(), CurrentFullscreen))
		{
			Exception::Throw("Could not resize GLRenderDevice");
		}
	}

	nulltex = Textures->GetNullTexture();

	int wantedBufferCount = UseVSync ? 2 : 3;
	if (BufferCount != wantedBufferCount)
	{
		BufferCount = wantedBufferCount;
		ReleaseSwapChainResources();
		UpdateSwapChain();
	}

	if (CurrentSizeX && CurrentSizeY)
	{
		try
		{
			ResizeSceneBuffers(CurrentSizeX, CurrentSizeY, GetSettingsMultisample());
		}
		catch (const std::exception& e)
		{
			LogMessage(std::string("Could not resize scene buffers: ") + e.what());
			return;
		}
	}

	HitData = InHitData;
	HitSize = InHitSize;

	FlashScale = InFlashScale;
	FlashFog = InFlashFog;

	FLOAT color[4] = { ScreenClear.x, ScreenClear.y, ScreenClear.z, ScreenClear.w };
	FLOAT zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLRenderTargetView* views[2] = { SceneBuffers.ColorBufferView.get(), SceneBuffers.HitBufferView.get() };
	Context->ClearRenderTargetView(SceneBuffers.ColorBufferView, color);
	Context->ClearRenderTargetView(SceneBuffers.HitBufferView, zero);
	Context->ClearDepthStencilView(SceneBuffers.DepthBufferView, GL_CLEAR_DEPTH, 1.0f, 0);
	Context->OMSetRenderTargets(2, views, SceneBuffers.DepthBufferView);

	UINT stride = sizeof(GLSceneVertex);
	UINT offset = 0;
	GLBuffer* vertexBuffers[1] = { ScenePass.VertexBuffer.get() };
	GLBuffer* cbs[1] = { ScenePass.ConstantBuffer.get() };
	Context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
	Context->IASetIndexBuffer(ScenePass.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	Context->IASetInputLayout(ScenePass.InputLayout);
	Context->VSSetShader(ScenePass.VertexShader, nullptr, 0);
	Context->VSSetConstantBuffers(0, 1, cbs);
	Context->RSSetState(ScenePass.RasterizerState[SceneBuffers.Multisample > 1]);

	GL_RECT box = {};
	box.right = CurrentSizeX;
	box.bottom = CurrentSizeY;
	Context->RSSetScissorRects(1, &box);

	MapVertices(true);

	SceneConstants.HitIndex = 0;
	ForceHitIndex = -1;

	IsLocked = true;
#endif
}

GLPresentPushConstants GLRenderDevice::GetGLPresentPushConstants()
{
	GLPresentPushConstants pushconstants;
	pushconstants.HdrScale = 0.8f + HdrScale * (3.0f / 255.0f);
	if (IsOrtho)
	{
		pushconstants.GammaCorrection = { 1.0f };
		pushconstants.Contrast = 1.0f;
		pushconstants.Saturation = 1.0f;
		pushconstants.Brightness = 0.0f;
	}
	else
	{
		float brightness = clamp(Brightness * 2.0f, 0.05f, 2.99f);

		if (GammaMode == 0)
		{
			float invGammaRed = 1.0f / std::max(brightness + GammaOffset + GammaOffsetRed, 0.001f);
			float invGammaGreen = 1.0f / std::max(brightness + GammaOffset + GammaOffsetGreen, 0.001f);
			float invGammaBlue = 1.0f / std::max(brightness + GammaOffset + GammaOffsetBlue, 0.001f);
			pushconstants.GammaCorrection = vec4(invGammaRed, invGammaGreen, invGammaBlue, 0.0f);
		}
		else
		{
			float invGammaRed = (GammaOffset + GammaOffsetRed + 2.0f) > 0.0f ? 1.0f / (GammaOffset + GammaOffsetRed + 1.0f) : 1.0f;
			float invGammaGreen = (GammaOffset + GammaOffsetGreen + 2.0f) > 0.0f ? 1.0f / (GammaOffset + GammaOffsetGreen + 1.0f) : 1.0f;
			float invGammaBlue = (GammaOffset + GammaOffsetBlue + 2.0f) > 0.0f ? 1.0f / (GammaOffset + GammaOffsetBlue + 1.0f) : 1.0f;
			pushconstants.GammaCorrection = vec4(invGammaRed, invGammaGreen, invGammaBlue, brightness);
		}

		// pushconstants.Contrast = clamp(Contrast, 0.1f, 3.f);
		if (Contrast >= 128)
		{
			pushconstants.Contrast = 1.0f + (Contrast - 128) / 127.0f * 3.0f;
		}
		else
		{
			pushconstants.Contrast = std::max(Contrast / 128.0f, 0.1f);
		}

		// pushconstants.Saturation = clamp(Saturation, -1.0f, 1.0f);
		pushconstants.Saturation = 1.0f - 2.0f * (255 - Saturation) / 255.0f;

		// pushconstants.Brightness = clamp(LinearBrightness, -1.8f, 1.8f);
		if (LinearBrightness >= 128)
		{
			pushconstants.Brightness = (LinearBrightness - 128) / 127.0f * 1.8f;
		}
		else
		{
			pushconstants.Brightness = (128 - LinearBrightness) / 128.0f * -1.8f;
		}
	}
	return pushconstants;
}

void GLRenderDevice::Unlock(bool Blit)
{
#if 0
	if (!IsLocked) // Don't trust the engine.
		return;

	DrawBatches();
	UnmapVertices();

	Batch.SceneIndexStart = 0;
	GLSceneVertexPos = 0;
	SceneIndexPos = 0;

	if (Blit)
	{
		if (SceneBuffers.Multisample > 1)
		{
			Context->ResolveSubresource(SceneBuffers.PPImage[0], 0, SceneBuffers.ColorBuffer, 0, DXGI_FORMAT_R16G16B16A16_FLOAT);
		}
		else
		{
			Context->CopyResource(SceneBuffers.PPImage[0], SceneBuffers.ColorBuffer);
		}

		if (Bloom)
		{
			RunBloomPass();
		}

		GLRenderTargetView* rtvs[1] = { BackBufferView.get() };
		Context->OMSetRenderTargets(1, rtvs, nullptr);

		GLViewport viewport = {};
		viewport.Width = (float)CurrentSizeX;
		viewport.Height = (float)CurrentSizeY;
		viewport.MaxDepth = 1.0f;
		SetViewport(viewport);

		GLPresentPushConstants pushconstants = GetGLPresentPushConstants();

		// Select present shader based on what the user is actually using
		int presentShader = 0;
		if (ActiveHdr) presentShader |= 1;
		if (GammaMode == 1) presentShader |= 2;
		if (pushconstants.Brightness != 0.0f || pushconstants.Contrast != 1.0f || pushconstants.Saturation != 1.0f) presentShader |= (clamp(GrayFormula, 0, 2) + 1) << 2;

		UINT stride = sizeof(vec2);
		UINT offset = 0;
		GLBuffer* vertexBuffers[1] = { PresentPass.PPStepVertexBuffer.get()};
		GLShaderResourceView* psResources[] = { SceneBuffers.PPImageShaderView[0].get(), PresentPass.DitherTextureView.get() };
		GLBuffer* cbs[1] = { PresentPass.PresentConstantBuffer.get() };
		Context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
		Context->IASetInputLayout(PresentPass.PPStepLayout);
		Context->IASetPrimitiveTopology(GL_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Context->VSSetShader(PresentPass.PPStep, nullptr, 0);
		Context->RSSetState(PresentPass.RasterizerState);
		Context->PSSetShader(PresentPass.Present[presentShader], nullptr, 0);
		Context->PSSetConstantBuffers(0, 1, cbs);
		Context->PSSetShaderResources(0, 2, psResources);
		Context->OMSetDepthStencilState(PresentPass.DepthStencilState, 0);
		Context->OMSetBlendState(PresentPass.BlendState, nullptr, 0xffffffff);
		SetBufferData(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, PresentPass.PresentConstantBuffer.get(), &pushconstants, sizeof(GLPresentPushConstants));
		Context->Draw(6, 0);

		if (SwapChain1)
		{
			UINT flags = 0;
			if (!UseVSync && !CurrentFullscreen && DxgiSwapChainAllowTearing)
				flags |= DXGI_PRESENT_ALLOW_TEARING;

			DXGI_PRESENT_PARAMETERS presentParams = {};
			SwapChain1->Present1(UseVSync ? 1 : 0, flags, &presentParams);
		}
		else
		{
			SwapChain->Present(UseVSync ? 1 : 0, 0);
		}

		Batch.Pipeline = nullptr;
		Batch.Tex = nullptr;
		Batch.Lightmap = nullptr;
		Batch.Detailtex = nullptr;
		Batch.Macrotex = nullptr;
		Batch.SceneIndexStart = 0;

		UpdateLODBias();
	}

	if (HitData)
	{
		GL_BOX box = {};
		box.left = HitX;
		box.right = HitX + HitWidth;
		box.top = SceneBuffers.Height - HitY - HitHeight;
		box.bottom = SceneBuffers.Height - HitY;
		box.front = 0;
		box.back = 1;

		// Resolve multisampling
		if (SceneBuffers.Multisample > 1)
		{
			GLRenderTargetView* rtvs[1] = { SceneBuffers.PPHitBufferView.get() };
			Context->OMSetRenderTargets(1, rtvs, nullptr);

			GLViewport viewport = {};
			viewport.TopLeftX = (float)box.left;
			viewport.TopLeftY = (float)box.top;
			viewport.Width = (float)(box.right - box.left);
			viewport.Height = (float)(box.bottom - box.top);
			viewport.MaxDepth = 1.0f;
			SetViewport(viewport);

			UINT stride = sizeof(vec2);
			UINT offset = 0;
			GLBuffer* vertexBuffers[1] = { PresentPass.PPStepVertexBuffer.get() };
			GLShaderResourceView* srvs[1] = { SceneBuffers.HitBufferShaderView.get() };
			Context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
			Context->IASetInputLayout(PresentPass.PPStepLayout);
			Context->IASetPrimitiveTopology(GL_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			Context->VSSetShader(PresentPass.PPStep, nullptr, 0);
			Context->RSSetState(PresentPass.RasterizerState);
			Context->PSSetShader(PresentPass.HitResolve, nullptr, 0);
			Context->PSSetShaderResources(0, 1, srvs);
			Context->OMSetDepthStencilState(PresentPass.DepthStencilState, 0);
			Context->OMSetBlendState(PresentPass.BlendState, nullptr, 0xffffffff);

			Context->Draw(6, 0);
		}
		else
		{
			Context->CopySubresourceRegion(SceneBuffers.PPHitBuffer, 0, box.left, box.top, 0, SceneBuffers.HitBuffer, 0, &box);
		}

		// Copy the hit buffer to a mappable texture, but only the part we want to examine
		Context->CopySubresourceRegion(SceneBuffers.StagingHitBuffer, 0, 0, 0, 0, SceneBuffers.PPHitBuffer, 0, &box);

		// Lock the buffer and look for the last hit
		int hit = 0;
		GL_MAPPED_SUBRESOURCE mapping = {};
		HRESULT result = Context->Map(SceneBuffers.StagingHitBuffer, 0, GL_MAP_READ, 0, &mapping);
		if (SUCCEEDED(result))
		{
			int width = HitWidth;
			int height = HitHeight;
			for (int y = 0; y < height; y++)
			{
				const int* line = (const int*)(((const char*)mapping.pData) + y * mapping.RowPitch);
				for (int x = 0; x < width; x++)
				{
					hit = std::max(hit, line[x]);
				}
			}
			Context->Unmap(SceneBuffers.StagingHitBuffer, 0);
		}
		hit--;

		hit = std::max(hit, ForceHitIndex);

		if (hit >= 0 && hit < (int)HitQueries.size())
		{
			const HitQuery& query = HitQueries[hit];
			memcpy(HitData, HitBuffer.data() + query.Start, query.Count);
			*HitSize = query.Count;
		}
		else
		{
			*HitSize = 0;
		}
	}

	Context->OMSetRenderTargets(0, nullptr, nullptr);

	HitQueryStack.clear();
	HitQueries.clear();
	HitBuffer.clear();
	HitData = nullptr;
	HitSize = nullptr;

	IsLocked = false;
#endif
}

void GLRenderDevice::PushHit(const uint8_t* Data, int Count)
{
	if (Count <= 0) return;
	HitQueryStack.insert(HitQueryStack.end(), Data, Data + Count);

	SetHitLocation();
}

void GLRenderDevice::PopHit(int Count, bool bForce)
{
	if (bForce) // Force hit what we are popping
		ForceHitIndex = (int)(HitQueries.size() - 1);

	HitQueryStack.resize(HitQueryStack.size() - Count);

	SetHitLocation();
}

void GLRenderDevice::SetHitLocation()
{
	DrawBatches();

	if (!HitQueryStack.empty())
	{
		int index = (int)HitQueries.size();

		HitQuery query;
		query.Start = (int)HitBuffer.size();
		query.Count = (int)HitQueryStack.size();
		HitQueries.push_back(query);

		HitBuffer.insert(HitBuffer.end(), HitQueryStack.begin(), HitQueryStack.end());

		SceneConstants.HitIndex = index + 1;
	}
	else
	{
		SceneConstants.HitIndex = 0;
	}

	SetBufferData(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, ScenePass.ConstantBuffer.get(), &SceneConstants, sizeof(GLScenePushConstants));
}

void GLRenderDevice::DrawComplexSurface(SceneNode* Frame, SurfaceInfo& Surface, SurfaceFacet& Facet)
{
	uint32_t PolyFlags = ApplyPrecedenceRules(Surface.PolyFlags);

	ComplexSurfaceInfo info;
	info.facet = &Facet;
	info.tex = Textures->GetTexture(Surface.Texture, !!(PolyFlags & PF_Masked));
	info.lightmap = Textures->GetTexture(Surface.LightMap, false);
	info.macrotex = Textures->GetTexture(Surface.MacroTexture, false);
	info.detailtex = Textures->GetTexture(Surface.DetailTexture, false);
	info.fogmap = (Surface.FogMap && Surface.FogMap->NumMips > 0 && !Surface.FogMap->Mips[0].Data.empty()) ?
		Textures->GetTexture(Surface.FogMap, false) : nulltex;
	info.editorcolor = nullptr;

	if (Surface.DetailTexture && Surface.FogMap) info.detailtex = nulltex;

	if (info.fogmap != nulltex)
		info.detailtex = info.fogmap;

	SetPipeline(PolyFlags);
	SetDescriptorSet(PolyFlags, info);

	DrawComplexSurfaceFaces(info);

	Stats.ComplexSurfaces++;
}

void GLRenderDevice::DrawComplexSurfaceFaces(const ComplexSurfaceInfo& info)
{
	if (info.facet->VertexCount < 3)
		return;

	uint32_t flags = 0;
	if (info.lightmap != nulltex) flags |= 1;
	if (info.macrotex != nulltex) flags |= 2;
	if (info.detailtex != nulltex && info.fogmap == nulltex) flags |= 4;
	if (info.fogmap != nulltex) flags |= 8;
	if (LightMode == 1) flags |= 64;

	vec3 xaxis = info.facet->MapCoords.XAxis;
	vec3 yaxis = info.facet->MapCoords.YAxis;
	float UDot = dot(xaxis, info.facet->MapCoords.Origin);
	float VDot = dot(yaxis, info.facet->MapCoords.Origin);

	float UPan = UDot + info.tex->PanX;
	float VPan = VDot + info.tex->PanY;
	float LMUPan = UDot + info.lightmap->PanX - 0.5f * info.lightmap->UScale;
	float LMVPan = VDot + info.lightmap->PanY - 0.5f * info.lightmap->VScale;
	float MacroUPan = UDot + info.macrotex->PanX;
	float MacroVPan = VDot + info.macrotex->PanY;
	float DetailUPan = UDot + (info.fogmap == nulltex ? info.detailtex->PanX : info.fogmap->PanX - 0.5f * info.fogmap->UScale);
	float DetailVPan = VDot + (info.fogmap == nulltex ? info.detailtex->PanY : info.fogmap->PanY - 0.5f * info.fogmap->VScale);

	float UMult = info.tex->UMult;
	float VMult = info.tex->VMult;
	float LMUMult = info.lightmap->UMult;
	float LMVMult = info.lightmap->VMult;
	float MacroUMult = info.macrotex->UMult;
	float MacroVMult = info.macrotex->VMult;
	float DetailUMult = info.fogmap == nulltex ? info.detailtex->UMult : info.fogmap->UMult;
	float DetailVMult = info.fogmap == nulltex ? info.detailtex->VMult : info.fogmap->VMult;

	vec4 color = info.editorcolor ? *info.editorcolor : vec4(1.0f);

	auto pts = info.facet->Vertices;
	uint32_t vcount = info.facet->VertexCount;

	uint32_t icount = (vcount - 2) * 3;
	auto alloc = ReserveVertices(vcount, icount);
	if (alloc.vptr)
	{
		GLSceneVertex* vptr = alloc.vptr;
		uint32_t* iptr = alloc.iptr;
		uint32_t vpos = alloc.vpos;

		for (uint32_t i = 0; i < vcount; i++)
		{
			vec3 point = pts[i];
			float u = dot(xaxis, point);
			float v = dot(yaxis, point);

			vptr->Flags = flags;
			vptr->Position.x = point.x;
			vptr->Position.y = point.y;
			vptr->Position.z = point.z;
			vptr->TexCoord.s = (u - UPan) * UMult;
			vptr->TexCoord.t = (v - VPan) * VMult;
			vptr->TexCoord2.s = (u - LMUPan) * LMUMult;
			vptr->TexCoord2.t = (v - LMVPan) * LMVMult;
			vptr->TexCoord3.s = (u - MacroUPan) * MacroUMult;
			vptr->TexCoord3.t = (v - MacroVPan) * MacroVMult;
			vptr->TexCoord4.s = (u - DetailUPan) * DetailUMult;
			vptr->TexCoord4.t = (v - DetailVPan) * DetailVMult;
			vptr->Color = color;
			vptr++;
		}

		for (uint32_t i = vpos + 2; i < vpos + vcount; i++)
		{
			*(iptr++) = vpos;
			*(iptr++) = i - 1;
			*(iptr++) = i;
		}

		UseVertices(vcount, icount);
	}
}

void GLRenderDevice::DrawGouraudPolygon(SceneNode* Frame, TextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags)
{
	if (NumPts < 3) return; // This can apparently happen!!

	PolyFlags = ApplyPrecedenceRules(PolyFlags);

	GLCachedTexture* tex = Textures->GetTexture(&Info, !!(PolyFlags & PF_Masked));

	SetPipeline(PolyFlags);
	SetDescriptorSet(PolyFlags, tex);

	float UMult = tex->UMult;
	float VMult = tex->VMult;

	int flags = (PolyFlags & (PF_RenderFog | PF_Translucent | PF_Modulated)) == PF_RenderFog ? 16 : 0;
	if ((PolyFlags & (PF_Translucent | PF_Modulated)) == 0 && LightMode == 2) flags |= 32;

	auto alloc = ReserveVertices(NumPts, (NumPts - 2) * 3);
	if (alloc.vptr)
	{
		GLSceneVertex* vptr = alloc.vptr;
		uint32_t* iptr = alloc.iptr;
		uint32_t vpos = alloc.vpos;

		if (PolyFlags & PF_Modulated)
		{
			GLSceneVertex* vertex = vptr;

			for (int i = 0; i < NumPts; i++)
			{
				const GouraudVertex* P = Pts + i;
				vertex->Flags = flags;
				vertex->Position.x = P->Point.x;
				vertex->Position.y = P->Point.y;
				vertex->Position.z = P->Point.z;
				vertex->TexCoord.s = P->UV.x * UMult;
				vertex->TexCoord.t = P->UV.y * VMult;
				vertex->TexCoord2.s = P->Fog.x;
				vertex->TexCoord2.t = P->Fog.y;
				vertex->TexCoord3.s = P->Fog.z;
				vertex->TexCoord3.t = P->Fog.w;
				vertex->TexCoord4.s = 0.0f;
				vertex->TexCoord4.t = 0.0f;
				vertex->Color.r = 1.0f;
				vertex->Color.g = 1.0f;
				vertex->Color.b = 1.0f;
				vertex->Color.a = 1.0f;
				vertex++;
			}
		}
		else
		{
			GLSceneVertex* vertex = vptr;
			for (int i = 0; i < NumPts; i++)
			{
				const GouraudVertex* P = Pts + i;
				vertex->Flags = flags;
				vertex->Position.x = P->Point.x;
				vertex->Position.y = P->Point.y;
				vertex->Position.z = P->Point.z;
				vertex->TexCoord.s = P->UV.x * UMult;
				vertex->TexCoord.t = P->UV.y * VMult;
				vertex->TexCoord2.s = P->Fog.x;
				vertex->TexCoord2.t = P->Fog.y;
				vertex->TexCoord3.s = P->Fog.z;
				vertex->TexCoord3.t = P->Fog.w;
				vertex->TexCoord4.s = 0.0f;
				vertex->TexCoord4.t = 0.0f;
				vertex->Color.r = P->Light.x;
				vertex->Color.g = P->Light.y;
				vertex->Color.b = P->Light.z;
				vertex->Color.a = 1.0f;
				vertex++;
			}
		}

		uint32_t vstart = vpos;
		uint32_t vcount = NumPts;
		for (uint32_t i = vstart + 2; i < vstart + vcount; i++)
		{
			*(iptr++) = vstart;
			*(iptr++) = i - 1;
			*(iptr++) = i;
		}

		UseVertices(NumPts, (NumPts - 2) * 3);
	}

	Stats.GouraudPolygons++;
}

void GLRenderDevice::DrawTile(SceneNode* Frame, TextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags)
{
	PolyFlags = ApplyPrecedenceRules(PolyFlags);

	if (PolyFlags == PF_SubpixelFont)
	{
		AddDrawBatch();
		Batch.BlendConstants[0] = Color.x;
		Batch.BlendConstants[1] = Color.y;
		Batch.BlendConstants[2] = Color.z;
		Batch.BlendConstants[3] = Color.w;
		Color = vec4(1.0f);
	}

	GLCachedTexture* tex = Textures->GetTexture(&Info, !!(PolyFlags & PF_Masked));
	float UMult = tex->UMult;
	float VMult = tex->VMult;
	float u0 = U * UMult;
	float v0 = V * VMult;
	float u1 = (U + UL) * UMult;
	float v1 = (V + VL) * VMult;
	bool clamp = (u0 >= 0.0f && u1 <= 1.00001f && v0 >= 0.0f && v1 <= 1.00001f);

	SetPipeline(PolyFlags);
	SetDescriptorSet(PolyFlags, tex, clamp);

	if (SceneBuffers.Multisample > 1)
	{
		XL = std::floor(X + XL + 0.5f);
		YL = std::floor(Y + YL + 0.5f);
		X = std::floor(X + 0.5f);
		Y = std::floor(Y + 0.5f);
		XL = XL - X;
		YL = YL - Y;
	}

	auto alloc = ReserveVertices(4, 6);
	if (alloc.vptr)
	{
		GLSceneVertex* vptr = alloc.vptr;
		uint32_t* iptr = alloc.iptr;
		uint32_t vpos = alloc.vpos;

		float r, g, b, a;
		if (PolyFlags & PF_Modulated)
		{
			r = 1.0f;
			g = 1.0f;
			b = 1.0f;
		}
		else
		{
			r = Color.x;
			g = Color.y;
			b = Color.z;
		}
		a = 1.0f;

		float rfx2z = RFX2 * Z;
		float rfy2z = RFY2 * Z;
		X -= Frame->FX2;
		Y -= Frame->FY2;
		XL += X;
		YL += Y;
		U *= UMult;
		UL = U + UL * UMult;
		V *= VMult;
		VL = V + VL * VMult;

		vptr[0].Flags = 0;
		vptr[0].Position.x = rfx2z * X;
		vptr[0].Position.y = rfy2z * Y;
		vptr[0].Position.z = Z;
		vptr[0].TexCoord.s = U;
		vptr[0].TexCoord.t = V;
		vptr[0].TexCoord2.s = 0.0f;
		vptr[0].TexCoord2.t = 0.0f;
		vptr[0].TexCoord3.s = 0.0f;
		vptr[0].TexCoord3.t = 0.0f;
		vptr[0].TexCoord4.s = 0.0f;
		vptr[0].TexCoord4.t = 0.0f;
		vptr[0].Color.r = r;
		vptr[0].Color.g = g;
		vptr[0].Color.b = b;
		vptr[0].Color.a = a;

		vptr[1].Flags = 0;
		vptr[1].Position.x = rfx2z * XL;
		vptr[1].Position.y = rfy2z * Y;
		vptr[1].Position.z = Z;
		vptr[1].TexCoord.s = UL;
		vptr[1].TexCoord.t = V;
		vptr[1].TexCoord2.s = 0.0f;
		vptr[1].TexCoord2.t = 0.0f;
		vptr[1].TexCoord3.s = 0.0f;
		vptr[1].TexCoord3.t = 0.0f;
		vptr[1].TexCoord4.s = 0.0f;
		vptr[1].TexCoord4.t = 0.0f;
		vptr[1].Color.r = r;
		vptr[1].Color.g = g;
		vptr[1].Color.b = b;
		vptr[1].Color.a = a;

		vptr[2].Flags = 0;
		vptr[2].Position.x = rfx2z * XL;
		vptr[2].Position.y = rfy2z * YL;
		vptr[2].Position.z = Z;
		vptr[2].TexCoord.s = UL;
		vptr[2].TexCoord.t = VL;
		vptr[2].TexCoord2.s = 0.0f;
		vptr[2].TexCoord2.t = 0.0f;
		vptr[2].TexCoord3.s = 0.0f;
		vptr[2].TexCoord3.t = 0.0f;
		vptr[2].TexCoord4.s = 0.0f;
		vptr[2].TexCoord4.t = 0.0f;
		vptr[2].Color.r = r;
		vptr[2].Color.g = g;
		vptr[2].Color.b = b;
		vptr[2].Color.a = a;

		vptr[3].Flags = 0;
		vptr[3].Position.x = rfx2z * X;
		vptr[3].Position.y = rfy2z * YL;
		vptr[3].Position.z = Z;
		vptr[3].TexCoord.s = U;
		vptr[3].TexCoord.t = VL;
		vptr[3].TexCoord2.s = 0.0f;
		vptr[3].TexCoord2.t = 0.0f;
		vptr[3].TexCoord3.s = 0.0f;
		vptr[3].TexCoord3.t = 0.0f;
		vptr[3].TexCoord4.s = 0.0f;
		vptr[3].TexCoord4.t = 0.0f;
		vptr[3].Color.r = r;
		vptr[3].Color.g = g;
		vptr[3].Color.b = b;
		vptr[3].Color.a = a;

		iptr[0] = vpos;
		iptr[1] = vpos + 1;
		iptr[2] = vpos + 2;
		iptr[3] = vpos;
		iptr[4] = vpos + 2;
		iptr[5] = vpos + 3;

		UseVertices(4, 6);
	}

	Stats.Tiles++;
}

vec4 GLRenderDevice::ApplyInverseGamma(vec4 color)
{
	if (IsOrtho)
		return color;
	float brightness = clamp(Brightness * 2.0f, 0.05f, 2.99f);
	float gammaRed = std::max(brightness + GammaOffset + GammaOffsetRed, 0.001f);
	float gammaGreen = std::max(brightness + GammaOffset + GammaOffsetGreen, 0.001f);
	float gammaBlue = std::max(brightness + GammaOffset + GammaOffsetBlue, 0.001f);
	return vec4(pow(color.r, gammaRed), pow(color.g, gammaGreen), pow(color.b, gammaBlue), color.a);
}

void GLRenderDevice::Draw3DLine(SceneNode* Frame, vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2)
{
	// P1 = P1.TransformPointBy(Frame->Coords);
	// P2 = P2.TransformPointBy(Frame->Coords);
	if (IsOrtho)
	{
		P1.x = (P1.x) / Frame->Zoom + Frame->FX2;
		P1.y = (P1.y) / Frame->Zoom + Frame->FY2;
		P1.z = 1;
		P2.x = (P2.x) / Frame->Zoom + Frame->FX2;
		P2.y = (P2.y) / Frame->Zoom + Frame->FY2;
		P2.z = 1;

		if (std::abs(P2.x - P1.x) + std::abs(P2.y - P1.y) >= 0.2)
		{
			Draw2DLine(Frame, Color, LineFlags, P1, P2);
		}
		else if (IsOrthoLowDetail)
		{
			Draw2DPoint(Frame, Color, LINE_None, P1.x - 1, P1.y - 1, P1.x + 1, P1.y + 1, P1.z);
		}
	}
	else
	{
		bool occlude = !!(LineFlags & LINE_DepthCued);
		SetPipeline(&ScenePass.LinePipeline[occlude]);
		SetDescriptorSet(PF_Highlighted);
		vec4 color = ApplyInverseGamma(vec4(Color.x, Color.y, Color.z, 1.0f));

		auto alloc = ReserveVertices(2, 2);
		if (alloc.vptr)
		{
			GLSceneVertex* vptr = alloc.vptr;
			uint32_t* iptr = alloc.iptr;
			uint32_t vpos = alloc.vpos;

			vptr[0] = { 0, vec3(P1.x, P1.y, P1.z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color };
			vptr[1] = { 0, vec3(P2.x, P2.y, P2.z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color };

			iptr[0] = vpos;
			iptr[1] = vpos + 1;

			UseVertices(2, 2);
		}
	}
}

void GLRenderDevice::Draw2DLine(SceneNode* Frame, vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2)
{
	bool occlude = !!(LineFlags & LINE_DepthCued);
	SetPipeline(&ScenePass.LinePipeline[occlude]);
	SetDescriptorSet(PF_Highlighted);
	vec4 color = ApplyInverseGamma(vec4(Color.x, Color.y, Color.z, 1.0f));

	auto alloc = ReserveVertices(2, 2);
	if (alloc.vptr)
	{
		GLSceneVertex* vptr = alloc.vptr;
		uint32_t* iptr = alloc.iptr;
		uint32_t vpos = alloc.vpos;

		vptr[0] = { 0, vec3(RFX2 * P1.z * (P1.x - Frame->FX2), RFY2 * P1.z * (P1.y - Frame->FY2), P1.z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color };
		vptr[1] = { 0, vec3(RFX2 * P2.z * (P2.x - Frame->FX2), RFY2 * P2.z * (P2.y - Frame->FY2), P2.z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color };

		iptr[0] = vpos;
		iptr[1] = vpos + 1;

		UseVertices(2, 2);
	}
}

void GLRenderDevice::Draw2DPoint(SceneNode* Frame, vec4 Color, uint32_t LineFlags, float X1, float Y1, float X2, float Y2, float Z)
{
	bool occlude = !!(LineFlags & LINE_DepthCued);
	SetPipeline(&ScenePass.PointPipeline[occlude]);
	SetDescriptorSet(PF_Highlighted);
	vec4 color = ApplyInverseGamma(vec4(Color.x, Color.y, Color.z, 1.0f));

	auto alloc = ReserveVertices(4, 6);
	if (alloc.vptr)
	{
		GLSceneVertex* vptr = alloc.vptr;
		uint32_t* iptr = alloc.iptr;
		uint32_t vpos = alloc.vpos;

		vptr[0] = { 0, vec3(RFX2 * Z * (X1 - Frame->FX2 - 0.5f), RFY2 * Z * (Y1 - Frame->FY2 - 0.5f), Z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color };
		vptr[1] = { 0, vec3(RFX2 * Z * (X2 - Frame->FX2 + 0.5f), RFY2 * Z * (Y1 - Frame->FY2 - 0.5f), Z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color };
		vptr[2] = { 0, vec3(RFX2 * Z * (X2 - Frame->FX2 + 0.5f), RFY2 * Z * (Y2 - Frame->FY2 + 0.5f), Z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color };
		vptr[3] = { 0, vec3(RFX2 * Z * (X1 - Frame->FX2 - 0.5f), RFY2 * Z * (Y2 - Frame->FY2 + 0.5f), Z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color };

		iptr[0] = vpos;
		iptr[1] = vpos + 1;
		iptr[2] = vpos + 2;
		iptr[3] = vpos;
		iptr[4] = vpos + 2;
		iptr[5] = vpos + 3;

		UseVertices(4, 6);
	}
}

void GLRenderDevice::ClearZ()
{
	DrawBatches();

	// SceneBuffers.DepthBufferView
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void GLRenderDevice::ReadPixels(TextureColor* Pixels)
{
#if 0
	UnmapVertices();

	GLTexture2D* stagingTexture = nullptr;

	GL_TEXTURE2D_DESC texDesc = {};
	texDesc.Usage = GL_USAGE_STAGING;
	texDesc.BindFlags = 0;
	texDesc.Width = SceneBuffers.Width;
	texDesc.Height = SceneBuffers.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.CPUAccessFlags = GL_CPU_ACCESS_READ;
	HRESULT result = Device->CreateTexture2D(&texDesc, nullptr, &stagingTexture);
	if (FAILED(result))
		return;
	SetDebugName(stagingTexture, "ReadPixels.StagingTexture");

	if (GammaCorrectScreenshots)
	{
		GLRenderTargetView* rtvs[1] = { SceneBuffers.PPImageView[1].get() };
		Context->OMSetRenderTargets(1, rtvs, nullptr);

		GLViewport viewport = {};
		viewport.Width = (float)CurrentSizeX;
		viewport.Height = (float)CurrentSizeY;
		viewport.MaxDepth = 1.0f;
		SetViewport(viewport);

		GLPresentPushConstants pushconstants = GetGLPresentPushConstants();

		// Select present shader based on what the user is actually using
		int presentShader = 0;
		if (ActiveHdr) presentShader |= 1;
		if (GammaMode == 1) presentShader |= 2;
		if (pushconstants.Brightness != 0.0f || pushconstants.Contrast != 1.0f || pushconstants.Saturation != 1.0f) presentShader |= (clamp(GrayFormula, 0, 2) + 1) << 2;

		UINT stride = sizeof(vec2);
		UINT offset = 0;
		GLBuffer* vertexBuffers[1] = { PresentPass.PPStepVertexBuffer.get() };
		GLBuffer* cbs[1] = { PresentPass.PresentConstantBuffer.get() };
		GLShaderResourceView* psResources[] = { SceneBuffers.PPImageShaderView[0].get(), PresentPass.DitherTextureView.get() };
		Context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
		Context->IASetInputLayout(PresentPass.PPStepLayout);
		Context->IASetPrimitiveTopology(GL_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Context->VSSetShader(PresentPass.PPStep, nullptr, 0);
		Context->RSSetState(PresentPass.RasterizerState);
		Context->PSSetShader(PresentPass.Present[presentShader], nullptr, 0);
		Context->PSSetConstantBuffers(0, 1, cbs);
		Context->PSSetShaderResources(0, 2, psResources);
		Context->OMSetDepthStencilState(PresentPass.DepthStencilState, 0);
		Context->OMSetBlendState(PresentPass.BlendState, nullptr, 0xffffffff);
		SetBufferData(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, PresentPass.PresentConstantBuffer.get(), &pushconstants, sizeof(GLPresentPushConstants));
		Context->Draw(6, 0);

		Context->CopyResource(stagingTexture, SceneBuffers.PPImage[1]);
	}
	else
	{
		Context->CopyResource(stagingTexture, SceneBuffers.PPImage[0]);
	}

	GL_MAPPED_SUBRESOURCE mapped = {};
	result = Context->Map(stagingTexture, 0, GL_MAP_READ, 0, &mapped);
	if (SUCCEEDED(result))
	{
		uint8_t* srcpixels = (uint8_t*)mapped.pData;
		int w = CurrentSizeX;
		int h = CurrentSizeY;
		void* data = Pixels;

		for (int y = 0; y < h; y++)
		{
			int desty = GammaCorrectScreenshots ? y : (h - y - 1);
			uint8_t* dest = (uint8_t*)data + desty * w * 4;
			uint16_t* src = (uint16_t*)(srcpixels + y * mapped.RowPitch);
			for (int x = 0; x < w; x++)
			{
				float red = halfToFloatSimple(*(src++));
				float green = halfToFloatSimple(*(src++));
				float blue = halfToFloatSimple(*(src++));
				float alpha = halfToFloatSimple(*(src++));

				dest[0] = (int)clamp(std::round(blue * 255.0f), 0.0f, 255.0f);
				dest[1] = (int)clamp(std::round(green * 255.0f), 0.0f, 255.0f);
				dest[2] = (int)clamp(std::round(red * 255.0f), 0.0f, 255.0f);
				dest[3] = (int)clamp(std::round(alpha * 255.0f), 0.0f, 255.0f);
				dest += 4;
			}
		}

		Context->Unmap(stagingTexture, 0);
	}

	stagingTexture->Release();

	if (IsLocked)
		MapVertices(false);
#endif
}

void GLRenderDevice::EndFlash()
{
	if (FlashScale != vec4(0.5f, 0.5f, 0.5f, 0.0f) || FlashFog != vec4(0.0f, 0.0f, 0.0f, 0.0f))
	{
		DrawBatches();

		vec4 color(FlashFog.x, FlashFog.y, FlashFog.z, 1.0f - std::min(FlashScale.x * 2.0f, 1.0f));
		vec2 zero2(0.0f);

		SceneConstants.ObjectToProjection = mat4::identity();
		SceneConstants.ObjectToView = mat4::identity();
		SceneConstants.NearClip = vec4(0.0f, 0.0f, 0.0f, 0.0f);

		SetBufferData(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, ScenePass.ConstantBuffer.get(), &SceneConstants, sizeof(GLScenePushConstants));

		SetPipeline(PF_Highlighted);
		SetDescriptorSet(0);

		auto alloc = ReserveVertices(4, 6);
		if (alloc.vptr)
		{
			GLSceneVertex* vptr = alloc.vptr;
			uint32_t* iptr = alloc.iptr;
			uint32_t vpos = alloc.vpos;

			vptr[0] = { 0, vec3(-1.0f, -1.0f, 0.0f), zero2, zero2, zero2, zero2, color };
			vptr[1] = { 0, vec3(1.0f, -1.0f, 0.0f), zero2, zero2, zero2, zero2, color };
			vptr[2] = { 0, vec3(1.0f,  1.0f, 0.0f), zero2, zero2, zero2, zero2, color };
			vptr[3] = { 0, vec3(-1.0f,  1.0f, 0.0f), zero2, zero2, zero2, zero2, color };

			iptr[0] = vpos;
			iptr[1] = vpos + 1;
			iptr[2] = vpos + 2;
			iptr[3] = vpos;
			iptr[4] = vpos + 2;
			iptr[5] = vpos + 3;

			UseVertices(4, 6);
		}

		DrawBatches();
		if (CurrentFrame)
			SetSceneNode(CurrentFrame);
	}
}

void GLRenderDevice::SetSceneNode(SceneNode* Frame)
{
	DrawBatches();

	CurrentFrame = Frame;
	Aspect = Frame->FY / Frame->FX;
	RProjZ = (float)std::tan(radians(Frame->FovAngle) * 0.5);
	RFX2 = 2.0f * RProjZ / Frame->FX;
	RFY2 = 2.0f * RProjZ * Aspect / Frame->FY;

	SceneViewport = {};
	SceneViewport.TopLeftX = (float)Frame->XB;
	SceneViewport.TopLeftY = (float)(SceneBuffers.Height - Frame->YB - Frame->Y);
	SceneViewport.Width = (float)Frame->X;
	SceneViewport.Height = (float)Frame->Y;
	SceneViewport.MinDepth = 0.1f;
	SceneViewport.MaxDepth = 1.0f;
	SetViewport(SceneViewport);

	SceneConstants.ObjectToProjection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);

	// TBD; do this or do like UE1 does and do the transform on the CPU?
	// maybe optionally do one or the other? transform on CPU can be super slow --Xaleros
	SceneConstants.ObjectToProjection = SceneConstants.ObjectToProjection * Frame->WorldToView * Frame->ObjectToWorld;

	SceneConstants.ObjectToView = Frame->WorldToView * Frame->ObjectToWorld;
	SceneConstants.NearClip = Frame->NearClip;

	SetBufferData(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, ScenePass.ConstantBuffer.get(), &SceneConstants, sizeof(GLScenePushConstants));
}

void GLRenderDevice::SetBufferData(GLenum target, GLenum usage, GLBuffer* buffer, const void* data, size_t size)
{
	glBindBuffer(target, ScenePass.ConstantBuffer->Handle);
	glBufferData(target, size, data, usage);
}

void GLRenderDevice::SetViewport(const GLViewport& viewport)
{
	// To do: deal with OpenGL viewports using lower left origin
	glViewport((GLint)std::round(viewport.TopLeftX), (GLint)std::round(viewport.TopLeftY), (GLsizei)std::round(viewport.Width), (GLsizei)std::round(viewport.Height));
	glDepthRangef(viewport.MinDepth, viewport.MaxDepth);
}

void GLRenderDevice::PrecacheTexture(TextureInfo& Info, uint32_t PolyFlags)
{
	PolyFlags = ApplyPrecedenceRules(PolyFlags);
	Textures->GetTexture(&Info, !!(PolyFlags & PF_Masked));
}

void GLRenderDevice::ClearTextureCache()
{
	Textures->ClearCache();
}

bool GLRenderDevice::SupportsTextureFormat(TextureFormat Format)
{
	return Uploads->SupportsTextureFormat(Format);
}

void GLRenderDevice::UpdateTextureRect(TextureInfo& Info, int U, int V, int UL, int VL)
{
	Textures->UpdateTextureRect(&Info, U, V, UL, VL);
}

void GLRenderDevice::AddDrawBatch()
{
	if (Batch.SceneIndexStart != SceneIndexPos)
	{
		Batch.SceneIndexEnd = SceneIndexPos;
		QueuedBatches.push_back(Batch);
		Batch.SceneIndexStart = SceneIndexPos;
	}
}

void GLRenderDevice::DrawBatches(bool nextBuffer)
{
	AddDrawBatch();

	UnmapVertices();

	for (const DrawBatchEntry& entry : QueuedBatches)
		DrawEntry(entry);
	QueuedBatches.clear();

	MapVertices(nextBuffer);

	if (nextBuffer)
	{
		GLSceneVertexPos = 0;
		SceneIndexPos = 0;
		Stats.BuffersUsed++;
	}

	Batch.SceneIndexStart = SceneIndexPos;
}

void GLRenderDevice::DrawEntry(const DrawBatchEntry& entry)
{
#if 0
	size_t icount = entry.SceneIndexEnd - entry.SceneIndexStart;

	GLShaderResourceView* views[4] =
	{
		entry.Tex->Texture.get(),
		entry.Lightmap->Texture.get(),
		entry.Macrotex->Texture.get(),
		entry.Detailtex->Texture.get()
	};

	GLSampler* samplers[4] =
	{
		ScenePass.Samplers[entry.TexSamplerMode].get(),
		ScenePass.Samplers[0].get(),
		ScenePass.Samplers[entry.MacrotexSamplerMode].get(),
		ScenePass.Samplers[entry.DetailtexSamplerMode].get()
	};

	if (SceneViewport.MinDepth != entry.Pipeline->MinDepth || SceneViewport.MaxDepth != entry.Pipeline->MaxDepth)
	{
		SceneViewport.MinDepth = entry.Pipeline->MinDepth;
		SceneViewport.MaxDepth = entry.Pipeline->MaxDepth;
		SetViewport(SceneViewport);
	}

	Context->PSSetSamplers(0, 4, samplers);
	Context->PSSetShaderResources(0, 4, views);
	Context->PSSetShader(entry.Pipeline->PixelShader, nullptr, 0);

	Context->OMSetBlendState(entry.Pipeline->BlendState, entry.BlendConstants, 0xffffffff);
	Context->OMSetDepthStencilState(entry.Pipeline->DepthStencilState, 0);

	Context->IASetPrimitiveTopology(entry.Pipeline->PrimitiveTopology);

	Context->DrawIndexed((UINT)icount, (UINT)entry.SceneIndexStart, 0);
	Stats.DrawCalls++;
#endif
}

void GLRenderDevice::SetDebugName(GLenum type, GLuint handle, const char* name)
{
	// Note: only exists in OpenGL 4.3 or if GL_KHR_debug extension is reported
	if (UseDebugLayer && glObjectLabel)
		glObjectLabel(type, handle, -1, name);
}

std::shared_ptr<GLBlendState> GLRenderDevice::CreateBlendState(const GLBlendDesc& desc)
{
	return std::make_shared<GLBlendState>(desc);
}

std::shared_ptr<GLRasterizerState> GLRenderDevice::CreateRasterizerState(const GLRasterizerDesc& desc)
{
	return std::make_shared<GLRasterizerState>(desc);
}

std::shared_ptr<GLDepthStencilState> GLRenderDevice::CreateDepthStencilState(const GLDepthStencilDesc& desc)
{
	return std::make_shared<GLDepthStencilState>(desc);
}

std::shared_ptr<GLBuffer> GLRenderDevice::CreateBuffer(GLenum target, GLenum usage, const void* data, size_t size, const char* debugName)
{
	auto buffer = std::make_shared<GLBuffer>();
	SetDebugName(buffer, debugName);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer->Handle);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GLScenePushConstants), nullptr, GL_DYNAMIC_DRAW);
	ThrowIfGLError((std::string("CreateBuffer failed for ") + debugName).c_str());
	return buffer;
}

std::shared_ptr<GLProgram> GLRenderDevice::CreateProgram(const std::string& programName, std::shared_ptr<GLVertexShader> vertexShader, std::shared_ptr<GLFragmentShader> fragmentShader)
{
	auto program = std::make_shared<GLProgram>();
	SetDebugName(program, programName.c_str());
	glAttachShader(program->Handle, vertexShader->Handle);
	glAttachShader(program->Handle, fragmentShader->Handle);
	glLinkProgram(program->Handle);
	GLint status = GL_FALSE;
	glGetProgramiv(program->Handle, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length = 0;
		glGetProgramiv(program->Handle, GL_INFO_LOG_LENGTH, &length);
		std::string log;
		log.resize(length);
		glGetProgramInfoLog(program->Handle, length, &length, log.data());
		log.resize(length);
		throw std::runtime_error("Could not link program'" + programName + "':" + log);
	}
	return program;
}

std::shared_ptr<GLVertexShader> GLRenderDevice::CreateVertexShader(const std::string& shaderName, const std::string& filename, const std::vector<std::string> defines)
{
	auto shader = std::make_shared<GLVertexShader>();
	SetDebugName(shader, shaderName.c_str());
	CompileGlsl(shader.get(), filename, defines);
	return shader;
}

std::shared_ptr<GLFragmentShader> GLRenderDevice::CreateFragmentShader(const std::string& shaderName, const std::string& filename, const std::vector<std::string> defines)
{
	auto shader = std::make_shared<GLFragmentShader>();
	SetDebugName(shader, shaderName.c_str());
	CompileGlsl(shader.get(), filename, defines);
	return shader;
}

void GLRenderDevice::CompileGlsl(GLShader* shader, const std::string& filename, const std::vector<std::string> defines)
{
	std::string code = "#version 330\r\n";
	for (const std::string& define : defines)
	{
		code += "#define ";
		code += define;
		code += "\r\n";
	}
	code += "#line 1\r\n";
	code += GLFileResource::readAllText(filename);

	GLchar* source = (GLchar*)code.data();
	GLint length = (GLint)code.size();
	glShaderSource(shader->Handle, 1, &source, &length);
	glCompileShader(shader->Handle);

	GLint status = GL_FALSE;
	glGetShaderiv(shader->Handle, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length = 0;
		glGetShaderiv(shader->Handle, GL_INFO_LOG_LENGTH, &length);
		std::string log;
		log.resize(length);
		glGetShaderInfoLog(shader->Handle, length, &length, log.data());
		log.resize(length);
		throw std::runtime_error("Could not compile shader '" + filename + "':" + log);
	}
}
