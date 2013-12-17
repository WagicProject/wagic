//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in root folder for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------
#if (!defined IOS) && (!defined QT_CONFIG)
#if (defined WIN32) && (!defined WP8)
#pragma warning(disable : 4786)
#pragma comment( lib, "giflib.lib" )
#endif

#ifdef WP8
#define _XM_NO_INTRINSICS_
#include <wrl/client.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <ppl.h>
#include <ppltasks.h>
#include <agile.h>

//using namespace DirectX;
using namespace Microsoft::WRL;
using namespace DirectX;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

_declspec(align(16)) struct MatrixBufferType
{
	DirectX::XMMATRIX mvpMatrix;
};

_declspec(align(16)) struct VertexType
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT2 texture;
};
_declspec(align(16)) struct TextureColorType
{
	bool istexture[16];
};

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch Win32 API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}

	// Function that reads from a binary file asynchronously.
	inline Concurrency::task<Platform::Array<byte>^> ReadDataAsync(Platform::String^ filename)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;
		
		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
		
		return create_task(folder->GetFileAsync(filename)).then([] (StorageFile^ file) 
		{
			return FileIO::ReadBufferAsync(file);
		}).then([] (Streams::IBuffer^ fileBuffer) -> Platform::Array<byte>^ 
		{
			auto fileData = ref new Platform::Array<byte>(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(fileData);
			return fileData;
		});
	}
}
#endif

#include <png.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XMD_H
#include <jpeglib.h>

#ifdef __cplusplus
}
#endif
#endif //IOS

#include "../../include/JGE.h"
#include "../../include/JRenderer.h"
#include "../../include/JResourceManager.h"
#include "../../include/JFileSystem.h"
#include "../../include/JAssert.h"

#if (defined WIN32) && (!defined QT_CONFIG)
#ifndef __attribute__
#define __attribute__((a))
#endif
#endif



JQuad::JQuad(JTexture *tex, float x, float y, float width, float height)
    :mTex(tex), mX(x), mY(y), mWidth(width), mHeight(height)
{
    JASSERT(tex != NULL);
    JRenderer::GetInstance()->TransferTextureToGLContext(*tex);

    mHotSpotX = 0.0f;
    mHotSpotY = 0.0f;
    //mBlend = BLEND_DEFAULT;
    for (int i=0;i<4;i++)
        mColor[i].color = 0xFFFFFFFF;

    mHFlipped = false;
    mVFlipped = false;

    SetTextureRect(x, y, width, height);
}

void JQuad::SetTextureRect(float x, float y, float w, float h)
{
    mX = x;
    mY = y;
    mWidth = w;
    mHeight = h;

    mTX0 = x/mTex->mTexWidth;
    mTY0 = y/mTex->mTexHeight;
    mTX1 = (x+w)/mTex->mTexWidth;
    mTY1 = (y+h)/mTex->mTexHeight;
}


void JQuad::GetTextureRect(float *x, float *y, float *w, float *h)
{
    *x=mX; *y=mY; *w=mWidth; *h=mHeight;
}


void JQuad::SetColor(PIXEL_TYPE color)
{
    for (int i=0;i<4;i++)
        mColor[i].color = color;
}



void JQuad::SetHotSpot(float x, float y)
{
    mHotSpotX = x;
    mHotSpotY = y;
}


//////////////////////////////////////////////////////////////////////////

JTexture::JTexture() : mBuffer(NULL)
{
    mTexId = (ID3D11ShaderResourceView*)-1;
}

JTexture::~JTexture()
{
    if (mBuffer)
    {
        delete [] mBuffer;
        mBuffer = NULL;
    }
}


void JTexture::UpdateBits(int x, int y, int width, int height, PIXEL_TYPE* bits)
{
}

//////////////////////////////////////////////////////////////////////////

JRenderer* JRenderer::mInstance = NULL;
bool JRenderer::m3DEnabled = false;

void JRenderer::Set3DFlag(bool flag) { m3DEnabled = flag; }

JRenderer* JRenderer::GetInstance()
{
    if (mInstance == NULL)
    {
        mInstance = new JRenderer();

        JASSERT(mInstance != NULL);

        mInstance->InitRenderer();
    }

    return mInstance;
}


void JRenderer::Destroy()
{
    if (mInstance)
    {
        mInstance->DestroyRenderer();
        delete mInstance;
        mInstance = NULL;
    }
}

JRenderer::JRenderer() 
	: mActualWidth(SCREEN_WIDTH_F), 
	mActualHeight(SCREEN_HEIGHT_F),
	mWindowWidth(0),
	mWindowHeight(0),
	m_Window(0),
	m_vertexShader(0),
	m_pixelShader(0),
	m_layout(0),
	m_sampleState(0),
	m_matrixBuffer(0)
{
	m_mvpMatrix = XMMatrixIdentity();
}


JRenderer::~JRenderer()
{
	if(m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}
}


void JRenderer::InitRenderer()
{
	// This flag adds support for surfaces with a different color channel ordering
	// than the API default. It is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	// If the project is in a debug build, enable debugging via SDK Layers with this flag.
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// This array defines the set of DirectX hardware feature levels this app will support.
	// Note the ordering should be preserved.
	// Don't forget to declare your application's minimum required feature level in its
	// description.  All applications are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Create the Direct3D 11 API device object and a corresponding context.
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	DX::ThrowIfFailed(
		D3D11CreateDevice(
			nullptr, // Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			creationFlags, // Set set debug and Direct2D compatibility flags.
			featureLevels, // List of feature levels this app can support.
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION, // Always set this to D3D11_SDK_VERSION for Windows Store apps.
			&device, // Returns the Direct3D device created.
			&m_featureLevel, // Returns feature level of device created.
			&context // Returns the device immediate context.
			)
		);

	// Get the Direct3D 11.1 API device and context interfaces.
	DX::ThrowIfFailed(
		device.As(&m_d3dDevice)
		);

	DX::ThrowIfFailed(
		context.As(&m_d3dContext)
		);

	// Buffer Init
	D3D11_BUFFER_DESC vertexBufferDesc;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType)*4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Now create the vertex buffer.
	DX::ThrowIfFailed(m_d3dDevice->CreateBuffer(&vertexBufferDesc, NULL, &m_vertexBuffer));

    unsigned short indices[] = {
		0, 1, 2,
		0, 2, 3,
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory( &indexBufferDesc, sizeof(indexBufferDesc) );

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	indexData.pSysMem = indices;
	DX::ThrowIfFailed(m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer));
	
	// vertex and shader mess
	// Creates shaders
	auto loadVSTask = DX::ReadDataAsync("SimpleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync("SimplePixelShader.cso");

	auto createVSTask = loadVSTask.then([this](Platform::Array<byte>^ fileData) {
		DX::ThrowIfFailed(
			m_d3dDevice->CreateVertexShader(
 				fileData->Data,
				fileData->Length,
				nullptr,
				&m_vertexShader
				)
			);

		const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_d3dDevice->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				fileData->Data,
				fileData->Length,
				&m_layout
				)
			);
	});

	auto createPSTask = loadPSTask.then([this](Platform::Array<byte>^ fileData) {
		DX::ThrowIfFailed(
			m_d3dDevice->CreatePixelShader(
				fileData->Data,
				fileData->Length,
				nullptr,
				&m_pixelShader
				)
			);

	CD3D11_BUFFER_DESC matrixBufferDesc;
	ZeroMemory( &matrixBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags  = 0;
	matrixBufferDesc.StructureByteStride = 0;
		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(
				&matrixBufferDesc,
				nullptr,
				&m_matrixBuffer
				)
			);
	});


	D3D11_BUFFER_DESC textureColorBufferDesc;
	ZeroMemory( &textureColorBufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	textureColorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	textureColorBufferDesc.ByteWidth = sizeof(TextureColorType);
	textureColorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//textureColorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureColorBufferDesc.MiscFlags = 0;
	textureColorBufferDesc.StructureByteStride = 0;
		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(&textureColorBufferDesc, NULL, &m_textureColorBuffer)
		);

	// Sample state description
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc,sizeof(D3D11_SAMPLER_DESC));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	DX::ThrowIfFailed(m_d3dDevice->CreateSamplerState(&samplerDesc, &m_sampleState));
}

void JRenderer::OnWindowsSizeChanged(void* window, float inWidth, float inHeight)
{
	if (mWindowWidth  != inWidth ||
		mWindowHeight != inHeight 
//		|| m_orientation != DisplayProperties::CurrentOrientation
		)
	{
		mWindowWidth = inWidth;
		mWindowHeight = inHeight;
		if(window != NULL)
			m_Window = (IUnknown*)window;


		ID3D11RenderTargetView* nullViews[] = {nullptr};
		m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
		m_renderTargetView = nullptr;
		m_depthStencilView = nullptr;
		m_d3dContext->Flush();

		if(m_swapChain != nullptr)
		{
			// If the swap chain already exists, resize it.
			DX::ThrowIfFailed(
				m_swapChain->ResizeBuffers(
					2, // Double-buffered swap chain.
					static_cast<UINT>(mWindowWidth),
					static_cast<UINT>(mWindowHeight),
					DXGI_FORMAT_B8G8R8A8_UNORM,
					0
					)
				);
		}
		else
		{
			// Otherwise, create a new one using the same adapter as the existing Direct3D device.
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
			swapChainDesc.Width = static_cast<UINT>(mWindowWidth); // Match the size of the window.
			swapChainDesc.Height = static_cast<UINT>(mWindowHeight);
			swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
			swapChainDesc.Stereo = false;
			swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 2; // Use double-buffering to minimize latency.
			swapChainDesc.Scaling = DXGI_SCALING_NONE;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
			swapChainDesc.Flags = 0;

			ComPtr<IDXGIDevice1>  dxgiDevice;
			DX::ThrowIfFailed(
				m_d3dDevice.As(&dxgiDevice)
				);

			ComPtr<IDXGIAdapter> dxgiAdapter;
			DX::ThrowIfFailed(
				dxgiDevice->GetAdapter(&dxgiAdapter)
				);

			ComPtr<IDXGIFactory2> dxgiFactory;
			DX::ThrowIfFailed(
				dxgiAdapter->GetParent(
					__uuidof(IDXGIFactory2), 
					&dxgiFactory
					)
				);

//			Windows::UI::Core::CoreWindow* window = (Windows::UI::Core::CoreWindow*)window.Get();
			DX::ThrowIfFailed(
				dxgiFactory->CreateSwapChainForCoreWindow(
					m_d3dDevice.Get(),
					m_Window.Get(),
					&swapChainDesc,
					nullptr, // Allow on all displays.
					&m_swapChain
					)
				);
			
			// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
			// ensures that the application will only render after each VSync, minimizing power consumption.
			DX::ThrowIfFailed(
				dxgiDevice->SetMaximumFrameLatency(1)
				);
		}
	/*
		// Set the proper orientation for the swap chain, and generate the
		// 3D matrix transformation for rendering to the rotated swap chain.
		DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;
		switch (m_orientation)
		{
			case DisplayOrientations::Landscape:
				rotation = DXGI_MODE_ROTATION_IDENTITY;
				m_orientationTransform3D = XMFLOAT4X4( // 0-degree Z-rotation
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
					);
				break;

			case DisplayOrientations::Portrait:
				rotation = DXGI_MODE_ROTATION_ROTATE270;
				m_orientationTransform3D = XMFLOAT4X4( // 90-degree Z-rotation
					0.0f, 1.0f, 0.0f, 0.0f,
					-1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
					);
				break;

			case DisplayOrientations::LandscapeFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE180;
				m_orientationTransform3D = XMFLOAT4X4( // 180-degree Z-rotation
					-1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, -1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
					);
				break;

			case DisplayOrientations::PortraitFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE90;
				m_orientationTransform3D = XMFLOAT4X4( // 270-degree Z-rotation
					0.0f, -1.0f, 0.0f, 0.0f,
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
					);
				break;

			default:
				throw ref new Platform::FailureException();
		}

		DX::ThrowIfFailed(
			m_swapChain->SetRotation(rotation)
			);
		*/
		// Create a render target view of the swap chain back buffer.
		ComPtr<ID3D11Texture2D> backBuffer;
		DX::ThrowIfFailed(
			m_swapChain->GetBuffer(
				0,
				__uuidof(ID3D11Texture2D),
				&backBuffer
				)
			);

		DX::ThrowIfFailed(
			m_d3dDevice->CreateRenderTargetView(
				backBuffer.Get(),
				nullptr,
				&m_renderTargetView
				)
			);

		// Create a depth stencil view.
		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT, 
			static_cast<UINT>(mWindowWidth),
			static_cast<UINT>(mWindowHeight),
			1,
			1,
			D3D11_BIND_DEPTH_STENCIL
			);

		ComPtr<ID3D11Texture2D> depthStencil;
		DX::ThrowIfFailed(
			m_d3dDevice->CreateTexture2D(
				&depthStencilDesc,
				nullptr,
				&depthStencil
				)
			);

		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
		DX::ThrowIfFailed(
			m_d3dDevice->CreateDepthStencilView(
				depthStencil.Get(),
				&depthStencilViewDesc,
				&m_depthStencilView
				)
			);


 		// Setup the raster description which will determine how and what polygons will be drawn.
		D3D11_RASTERIZER_DESC rasterDesc;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;

		// Create the rasterizer state from the description we just filled out.
		DX::ThrowIfFailed(m_d3dDevice->CreateRasterizerState(&rasterDesc, &m_rasterState));

		// Now set the rasterizer state.
		m_d3dContext->RSSetState(m_rasterState.Get());


		// Set the rendering viewport to target the entire window.
		CD3D11_VIEWPORT viewport(
			0.0f,
			0.0f,
			SCREEN_WIDTH, //mWindowWidth,
			SCREEN_HEIGHT //mWindowHeight
//			-1.0f,
//			1.0f
			);

		m_d3dContext->RSSetViewports(1, &viewport);
	}
}


void JRenderer::DestroyRenderer()
{
}

void JRenderer::BeginScene()
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = 0;
	color[1] = 0;
	color[2] = 255;
	color[3] = 0;

	// Clear the back buffer.
	m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), color);
    
	// Clear the depth buffer.
	m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_d3dContext->OMSetRenderTargets(
		1,
		m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get()
		);

	XMMATRIX tmpMatrix;
	tmpMatrix = XMMatrixOrthographicOffCenterRH(0.0f, SCREEN_WIDTH_F, 0.0f, SCREEN_HEIGHT_F-1.0f, -1.0f, 1.0f);
	m_mvpMatrix = XMMatrixMultiply(tmpMatrix, XMMatrixIdentity());
}


void JRenderer::EndScene()
{
	// The application may optionally specify "dirty" or "scroll"
	// rects to improve efficiency in certain scenarios.
	DXGI_PRESENT_PARAMETERS parameters = {0};
	parameters.DirtyRectsCount = 0;
	parameters.pDirtyRects = nullptr;
	parameters.pScrollRect = nullptr;
	parameters.pScrollOffset = nullptr;
	
	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	HRESULT hr = m_swapChain->Present1(1, 0, &parameters);

	// Discard the contents of the render target.
	// This is a valid operation only when the existing contents will be entirely
	// overwritten. If dirty or scroll rects are used, this call should be removed.
//	m_d3dContext->DiscardView(m_renderTargetView.Get());

	// Discard the contents of the depth stencil.
//	m_d3dContext->DiscardView(m_depthStencilView.Get());

	// If the device was removed either by a disconnect or a driver upgrade, we 
	// must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED)
	{
		// Reset these member variables to ensure that UpdateForWindowSizeChange recreates all resources.
		float width = mWindowWidth;
		float height = mWindowHeight;

		mWindowWidth = 0;
		mWindowHeight = 0;
		m_swapChain = nullptr;

		InitRenderer();
		OnWindowsSizeChanged(NULL, width, height);
	}
	else
	{
		DX::ThrowIfFailed(hr);
	}
}

void JRenderer::BindTexture(JTexture *tex)
{
}


void JRenderer::EnableTextureFilter(bool flag)
{
    if (flag)
        mCurrentTextureFilter = TEX_FILTER_LINEAR;
    else
        mCurrentTextureFilter = TEX_FILTER_NEAREST;
}

void Swap(float *a, float *b)
{
    float n=*a;
    *a = *b;
    *b = n;
}

void JRenderer::RenderQuad(JQuad* quad, float xo, float yo, float angle, float xScale, float yScale)
{
	float width = quad->mWidth;
    float height = quad->mHeight;
    float x = -quad->mHotSpotX;
    float y = quad->mHotSpotY;

	VertexType* verticesTmp;
	// Create the vertex array.
	verticesTmp = new VertexType[4];
	if(!verticesTmp)
	{
		return ;
	}

	verticesTmp[0].position = XMFLOAT3(x, y-height, 0.0f);
	verticesTmp[1].position = XMFLOAT3(x+width, y-height, 0.0f);
	verticesTmp[2].position = XMFLOAT3(x+width, y, 0.0f);
	verticesTmp[3].position = XMFLOAT3(x, y, 0.0f);

	verticesTmp[0].texture = XMFLOAT2(quad->mTX0, quad->mTY1);
	verticesTmp[1].texture = XMFLOAT2(quad->mTX1, quad->mTY1);
	verticesTmp[2].texture = XMFLOAT2(quad->mTX0, quad->mTY0);
	verticesTmp[3].texture = XMFLOAT2(quad->mTX1, quad->mTY0);

	verticesTmp[0].color = XMFLOAT4(quad->mColor[0].r/255.0f, quad->mColor[0].g/255.0f, quad->mColor[0].b/255.0f, quad->mColor[0].a/255.0f);
	verticesTmp[1].color = XMFLOAT4(quad->mColor[1].r/255.0f, quad->mColor[1].g/255.0f, quad->mColor[1].b/255.0f, quad->mColor[1].a/255.0f);
	verticesTmp[2].color = XMFLOAT4(quad->mColor[3].r/255.0f, quad->mColor[3].g/255.0f, quad->mColor[3].b/255.0f, quad->mColor[3].a/255.0f);
	verticesTmp[3].color = XMFLOAT4(quad->mColor[2].r/255.0f, quad->mColor[2].g/255.0f, quad->mColor[2].b/255.0f, quad->mColor[2].a/255.0f);


	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;
	if(FAILED(m_d3dContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))){return ;}
	verticesPtr = (VertexType*)mappedResource.pData;
	memcpy(verticesPtr, (void*)verticesTmp, (sizeof(VertexType) * 4));
	m_d3dContext->Unmap(m_vertexBuffer, 0);

	if ( verticesTmp )
	{
		delete[] verticesTmp;
		verticesTmp = 0;
	}


	////////////////////////
	unsigned int stride;
	unsigned int offset;
	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	m_d3dContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	m_d3dContext->IASetIndexBuffer( m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);


	// Set the sampler state in the pixel shader.
	m_d3dContext->PSSetSamplers(0, 1, &m_sampleState);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	MatrixBufferType* dataPtr;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

// Transpose the matrices to prepare them for the shader.
	XMMATRIX tmpMatrix = XMMatrixMultiply(XMMatrixTranslation(xo,yo,0.0f),m_mvpMatrix);
	tmpMatrix = XMMatrixMultiply(XMMatrixRotationX(angle), tmpMatrix);
	tmpMatrix = XMMatrixMultiply(XMMatrixScaling(xScale, yScale, 1.0f), tmpMatrix);

		// Lock the constant buffer so it can be written to.
	DX::ThrowIfFailed(m_d3dContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->mvpMatrix = tmpMatrix;

	// Unlock the constant buffer.
	m_d3dContext->Unmap(m_matrixBuffer, 0);

	// Now set the constant buffer in the vertex shader with the updated values.
	m_d3dContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);


	TextureColorType tc;
	ZeroMemory(&tc, sizeof(tc));
	tc.istexture[0] = TRUE;
	m_d3dContext->UpdateSubresource(m_textureColorBuffer, 0, 0, &tc, 0, 0);
	m_d3dContext->PSSetConstantBuffers(0, 1, &m_textureColorBuffer);


	// Set shader texture resource in the pixel shader.
	m_d3dContext->PSSetShaderResources(0, 1, &(quad->mTex->mTexId));

	// Set the vertex input layout.
	m_d3dContext->IASetInputLayout(m_layout.Get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_d3dContext->VSSetShader(m_vertexShader.Get(), NULL, 0);
	m_d3dContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

	// Render the triangle.
	m_d3dContext->DrawIndexed( 6, 0, 0 );

}


void JRenderer::RenderQuad(JQuad* quad, VertexColor* pt)
{
}


void JRenderer::FillRect(float x, float y, float width, float height, PIXEL_TYPE color)
{

}


void JRenderer::DrawRect(float x, float y, float width, float height, PIXEL_TYPE color)
{
}


void JRenderer::FillRect(float x, float y, float width, float height, PIXEL_TYPE* colors)
{
    JColor col[4];
    for (int i=0;i<4;i++)
        col[i].color = colors[i];

    FillRect(x, y, width, height, col);
}

void JRenderer::FillRect(float x, float y, float width, float height, JColor* colors)
{
}


void JRenderer::DrawLine(float x1, float y1, float x2, float y2, PIXEL_TYPE color)
{

}


void JRenderer::PlotArray(float *x, float *y, int count, PIXEL_TYPE color)
{

}



void JRenderer::ScreenShot(const char* filename __attribute__((unused)))
{

}


static int getNextPower2(int width)
{
    int b = width;
    int n;
    for (n = 0; b != 0; n++) b >>= 1;
    b = 1 << n;
    if (b == 2 * width) b >>= 1;
    return b;
}


static void jpg_null(j_decompress_ptr cinfo __attribute__((unused)))
{
}


static boolean jpg_fill_input_buffer(j_decompress_ptr cinfo __attribute__((unused)))
{
    ////    ri.Con_Printf(PRINT_ALL, "Premature end of JPEG data\n");
    return 1;
}

static void jpg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{

    cinfo->src->next_input_byte += (size_t) num_bytes;
    cinfo->src->bytes_in_buffer -= (size_t) num_bytes;

    //// if (cinfo->src->bytes_in_buffer < 0)
    ////		ri.Con_Printf(PRINT_ALL, "Premature end of JPEG data\n");
}

static void jpeg_mem_src(j_decompress_ptr cinfo, byte *mem, int len)
{
    cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
    cinfo->src->init_source = jpg_null;
    cinfo->src->fill_input_buffer = jpg_fill_input_buffer;
    cinfo->src->skip_input_data = jpg_skip_input_data;
    cinfo->src->resync_to_restart = jpeg_resync_to_restart;
    cinfo->src->term_source = jpg_null;
    cinfo->src->bytes_in_buffer = len;
    cinfo->src->next_input_byte = mem;
}

/*
==============
LoadJPG
==============
*/
void JRenderer::LoadJPG(TextureInfo &textureInfo, const char *filename, int mode __attribute__((unused)), int TextureFormat __attribute__((unused)))
{
    textureInfo.mBits = NULL;

    struct jpeg_decompress_struct	cinfo;
    struct jpeg_error_mgr jerr;
    BYTE *rawdata, *rgbadata, *scanline, *p, *q;
    int	rawsize, i;

    JFileSystem* fileSystem = JFileSystem::GetInstance();
    if (!fileSystem->OpenFile(filename)) return;

    rawsize = fileSystem->GetFileSize();

    rawdata = new BYTE[rawsize];

    if (!rawdata)
    {
        fileSystem->CloseFile();
        return;
    }

    fileSystem->ReadFile(rawdata, rawsize);
    fileSystem->CloseFile();

    // Initialize libJpeg Object
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_mem_src(&cinfo, rawdata, rawsize);

    // Process JPEG header
    jpeg_read_header(&cinfo, true);



    // Start Decompression
    jpeg_start_decompress(&cinfo);

    // Check Colour Components
    if(cinfo.output_components != 3 && cinfo.output_components != 4)
    {
        ////		ri.Con_Printf(PRINT_ALL, "Invalid JPEG colour components\n");
        jpeg_destroy_decompress(&cinfo);
        ////		ri.FS_FreeFile(rawdata);
        return;
    }

    int tw = getNextPower2(cinfo.output_width);
    int th = getNextPower2(cinfo.output_height);


    // Allocate Memory for decompressed image
    rgbadata = new BYTE[tw * th * 4];
    if(!rgbadata)
    {
        ////		ri.Con_Printf(PRINT_ALL, "Insufficient RAM for JPEG buffer\n");
        jpeg_destroy_decompress(&cinfo);
        ////		ri.FS_FreeFile(rawdata);
        delete [] rgbadata;
        return;
    }


    // Pass sizes to output

    // Allocate Scanline buffer
    scanline = (byte *)malloc(cinfo.output_width * 3);
    if(!scanline)
    {
        ////		ri.Con_Printf(PRINT_ALL, "Insufficient RAM for JPEG scanline buffer\n");

        jpeg_destroy_decompress(&cinfo);
        ////		ri.FS_FreeFile(rawdata);

        delete [] rgbadata;
        return;
    }

    // Read Scanlines, and expand from RGB to RGBA
    BYTE* currRow = rgbadata;

    while(cinfo.output_scanline < cinfo.output_height)
    {
        p = scanline;
        jpeg_read_scanlines(&cinfo, &scanline, 1);

        q = currRow;
        for(i=0; i<(int)cinfo.output_width; i++)
        {
            q[0] = p[0];
            q[1] = p[1];
            q[2] = p[2];
            q[3] = 255;

            p+=3; q+=4;
        }
        currRow += tw*4;
    }

    // Free the scanline buffer
    free(scanline);

    textureInfo.mBits = rgbadata;
    textureInfo.mWidth = cinfo.output_width;
    textureInfo.mHeight = cinfo.output_height;
    textureInfo.mTexWidth = tw;
    textureInfo.mTexHeight = th;

    // Finish Decompression
    try {
        jpeg_finish_decompress(&cinfo);
    } catch(...) {}

    // Destroy JPEG object
    jpeg_destroy_decompress(&cinfo);

    delete[] rawdata;
}


static void PNGCustomWarningFn(png_structp png_ptr __attribute__((unused)), png_const_charp warning_msg __attribute__((unused)))
{
    // ignore PNG warnings
}


static void PNGCustomReadDataFn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    png_size_t check;

    JFileSystem *fileSystem = (JFileSystem*)png_ptr->io_ptr;

    check = fileSystem->ReadFile(data, length);

    if (check != length)
    {
        png_error(png_ptr, "Read Error!");
    }
}

JTexture* JRenderer::LoadTexture(const char* filename, int mode, int TextureFormat __attribute__((unused)))
{
    TextureInfo textureInfo;

    textureInfo.mBits = NULL;

    if (strstr(filename, ".jpg")!=NULL || strstr(filename, ".JPG")!=NULL)
        LoadJPG(textureInfo, filename);
#if (!defined IOS) && (!defined QT_CONFIG) && (!defined SDL_CONFIG) && (!defined WP8)
    else if(strstr(filename, ".gif")!=NULL || strstr(filename, ".GIF")!=NULL)
        LoadGIF(textureInfo,filename);
#endif
    else if(strstr(filename, ".png")!=NULL || strstr(filename, ".PNG")!=NULL)
        LoadPNG(textureInfo, filename);

    if (textureInfo.mBits == NULL) {
        printf("Texture %s failed to load\n", filename);
        return NULL;
    }

    bool ret = false;

    JTexture *tex = new JTexture();

    if (tex)
    {
        if (mImageFilter != NULL)
            mImageFilter->ProcessImage((PIXEL_TYPE*)textureInfo.mBits, textureInfo.mWidth, textureInfo.mHeight);

        tex->mFilter = TEX_FILTER_LINEAR;
        tex->mWidth = textureInfo.mWidth;
        tex->mHeight = textureInfo.mHeight;
        tex->mTexWidth = textureInfo.mTexWidth;
        tex->mTexHeight = textureInfo.mTexHeight;

        tex->mBuffer = textureInfo.mBits;
    }

    return tex;
}

int JRenderer::LoadPNG(TextureInfo &textureInfo, const char *filename, int mode __attribute__((unused)), int TextureFormat __attribute__((unused)))
{
    textureInfo.mBits = NULL;

    DWORD* p32;

    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    png_uint_32 width, height, tw, th;
    int bit_depth, color_type, interlace_type, x, y;
    DWORD* line;

    JFileSystem* fileSystem = JFileSystem::GetInstance();
    if (!fileSystem->OpenFile(filename))
        return JGE_ERR_CANT_OPEN_FILE;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
    {
        fileSystem->CloseFile();

        return JGE_ERR_PNG;
    }

    png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, PNGCustomWarningFn);
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        //fclose(fp);
        fileSystem->CloseFile();

        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);

        return JGE_ERR_PNG;
    }
    png_init_io(png_ptr, NULL);
    png_set_read_fn(png_ptr, (png_voidp)fileSystem, PNGCustomReadDataFn);

    png_set_sig_bytes(png_ptr, sig_read);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
    png_set_strip_16(png_ptr);
    png_set_packing(png_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

    line = (DWORD*) malloc(width * 4);
    if (!line)
    {
        //fclose(fp);
        fileSystem->CloseFile();

        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return JGE_ERR_MALLOC_FAILED;
    }


    tw = getNextPower2(width);
    th = getNextPower2(height);

    int size = tw * th * 4;			// RGBA

    BYTE* buffer = new BYTE[size];

    //JTexture *tex = new JTexture();

    if (buffer)
    {


        p32 = (DWORD*) buffer;

        for (y = 0; y < (int)height; y++)
        {
            png_read_row(png_ptr, (BYTE*) line, png_bytep_NULL);
            for (x = 0; x < (int)width; x++)
            {
                DWORD color32 = line[x];
                int a = (color32 >> 24) & 0xff;
                int r = color32 & 0xff;
                int g = (color32 >> 8) & 0xff;
                int b = (color32 >> 16) & 0xff;

                color32 = r | (g << 8) | (b << 16) | (a << 24);
                *(p32+x) = color32;

            }
            p32 += tw;

        }

    }


    free (line);

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    fileSystem->CloseFile();


    textureInfo.mBits = buffer;
    textureInfo.mWidth = width;
    textureInfo.mHeight = height;
    textureInfo.mTexWidth = tw;
    textureInfo.mTexHeight = th;

    return 1;
    //return textureInfo;

}




void JRenderer::TransferTextureToGLContext(JTexture& inTexture)
{
    if (inTexture.mBuffer != NULL)
    {
		D3D11_TEXTURE2D_DESC desc;
		HRESULT hr = E_FAIL;

		desc.Width = static_cast<UINT>( inTexture.mTexWidth );
		desc.Height = static_cast<UINT>( inTexture.mTexHeight );
		desc.MipLevels = static_cast<UINT>( 1 );
		desc.ArraySize = static_cast<UINT>( 1 );
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = inTexture.mBuffer;
		initData.SysMemPitch = inTexture.mTexWidth*4;
		initData.SysMemSlicePitch = inTexture.mTexHeight*inTexture.mTexWidth*4;

		ID3D11Texture2D* tex2D = nullptr;
		hr = m_d3dDevice->CreateTexture2D( &desc, &initData, &tex2D);
		if( S_OK != hr) 
			return;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		D3D11_TEXTURE2D_DESC desc1;
		// Get a texture description to determine the texture
		// format of the loaded texture.
		tex2D->GetDesc( &desc1 );

		// Fill in the D3D11_SHADER_RESOURCE_VIEW_DESC structure.
		srvDesc.Format = desc1.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = desc1.MipLevels;

		// Create the shader resource view.
		m_d3dDevice->CreateShaderResourceView( tex2D, &srvDesc, &inTexture.mTexId);
		
		delete [] inTexture.mBuffer;
        inTexture.mBuffer = NULL;
    }
}

JTexture* JRenderer::CreateTexture(int width, int height, int mode __attribute__((unused)))
{
    JTexture *tex = new JTexture();
	HRESULT hr = E_FAIL;

    if (tex)
    {
        int size = width * height * sizeof(PIXEL_TYPE);			// RGBA
        BYTE* buffer = new BYTE[size];
        if (buffer)
        {
            tex->mFilter = TEX_FILTER_LINEAR;
            tex->mWidth = width;
            tex->mHeight = height;
            tex->mTexWidth = width;
            tex->mTexHeight = height;
			tex->mBuffer = buffer;

			TransferTextureToGLContext(*tex);
        }
        else
        {
            delete tex;
            tex = NULL;
        }
    }

    return tex;
}


void JRenderer::EnableVSync(bool flag __attribute__((unused)))
{
    //	if (flag)
    //		hge->System_SetState(HGE_FPS, 60);	// HGEFPS_VSYNC
    //	else
    //		hge->System_SetState(HGE_FPS, HGEFPS_UNLIMITED);
}


void JRenderer::ClearScreen(PIXEL_TYPE color)
{

}


void JRenderer::SetTexBlend(int src, int dest)
{

}


void JRenderer::SetTexBlendSrc(int src)
{// NOT USED

}


void JRenderer::SetTexBlendDest(int dest)
{// NOT USED

}


void JRenderer::Enable2D()
{// NOT USED
    if (mCurrentRenderMode == MODE_2D)
        return;

    mCurrentRenderMode = MODE_2D;


}


void JRenderer::Enable3D()
{ /* NOT USED */
}


void JRenderer::SetClip(int, int, int, int)
{// NOT USED
}

void JRenderer::LoadIdentity()
{// NOT USED
}

void JRenderer::Translate(float, float, float)
{// NOT USED
}

void JRenderer::RotateX(float)
{// NOT USED
}

void JRenderer::RotateY(float)
{// NOT USED
}

void JRenderer::RotateZ(float)
{// NOT USED
}


void JRenderer::PushMatrix()
{// NOT USED
}

void JRenderer::PopMatrix()
{// NOT USED
}

void JRenderer::RenderTriangles(JTexture* texture, Vertex3D *vertices, int start, int count)
{// NOT USED

}


void JRenderer::SetFOV(float fov)
{// NOT USED
    mFOV = fov;
}


void JRenderer::FillPolygon(float* x, float* y, int count, PIXEL_TYPE color)
{// NOT USED

}


void JRenderer::DrawPolygon(float* x, float* y, int count, PIXEL_TYPE color)
{// NOT USED
}


void JRenderer::DrawLine(float x1, float y1, float x2, float y2, float lineWidth, PIXEL_TYPE color)
{// NOT USED
    float dy=y2-y1;
    float dx=x2-x1;
    if(dy==0 && dx==0)
        return;

    float l=(float)hypot(dx,dy);

    float x[4];
    float y[4];

    x[0]=x1+lineWidth*(y2-y1)/l;
    y[0]=y1-lineWidth*(x2-x1)/l;

    x[1]=x1-lineWidth*(y2-y1)/l;
    y[1]=y1+lineWidth*(x2-x1)/l;

    x[2]=x2-lineWidth*(y2-y1)/l;
    y[2]=y2+lineWidth*(x2-x1)/l;

    x[3]=x2+lineWidth*(y2-y1)/l;
    y[3]=y2-lineWidth*(x2-x1)/l;

    FillPolygon(x, y, 4, color);
}


void JRenderer::DrawCircle(float x, float y, float radius, PIXEL_TYPE color)
{// NOT USED
}

void JRenderer::FillCircle(float x, float y, float radius, PIXEL_TYPE color)
{// NOT USED
}


void JRenderer::DrawPolygon(float x, float y, float size, int count, float startingAngle, PIXEL_TYPE color)
{// NOT USED
}


void JRenderer::FillPolygon(float x, float y, float size, int count, float startingAngle, PIXEL_TYPE color)
{// NOT USED
}


void JRenderer::SetImageFilter(JImageFilter* imageFilter)
{
    mImageFilter = imageFilter;
}



void JRenderer::DrawRoundRect(float x, float y, float w, float h, float radius, PIXEL_TYPE color)
{// NOT USED
}



void JRenderer::FillRoundRect(float x, float y, float w, float h, float radius, PIXEL_TYPE color)
{// NOT USED
}

