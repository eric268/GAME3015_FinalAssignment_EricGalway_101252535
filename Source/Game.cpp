//***************************************************************************************
// DirectX12Application.cpp 
//
//***************************************************************************************

#include "Game.h"
#include "ResourceManager.h"
#include "TitleState.h"
#include "MenuState.h"
#include "GameState.h"
#include "PauseState.h"

const int gNumFrameResources = 3;
UINT Game::objCBIndex = 0;
Game::Game(HINSTANCE hInstance)
    : D3DApp(hInstance),
	mTheta(1.5f * XM_PI),
	mPhi(0.1f),
	mRadius(100.0f),
	mView(MathHelper::Identity4x4()),
	mProj(MathHelper::Identity4x4()),
	target(XMVectorZero()),
	//mPassCbvOffset(0),
	mCurrFrameResource(nullptr),
	mRootSignature(nullptr),
	mSrvDescriptorHeap(nullptr),
	mPlayer(Player()),
	mStateStack(this, State::Context(mPlayer))
{
	RegisterStates();
	mStateStack.pushState(States::Title);
	//mStateStack.applyPendingChanges();
}

Game::~Game()
{
    if(md3dDevice != nullptr)
        FlushCommandQueue();
}

bool Game::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildShapeGeometry();
	BuildMaterials();
	mStateStack.applyPendingChanges();
	//BuildPSOs();

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	FlushCommandQueue();

	return true;
}

void Game::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void Game::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);

	// Cycle through the circular frame resource array.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}	
	

	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
	mStateStack.update(gt);



	if (mStateStack.isEmpty())
		exit(0);
}

void Game::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.

	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::Black, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	mStateStack.draw(gt);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrFrameResource->Fence = ++mCurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void Game::OnMouseDown(WPARAM btnState, int x, int y)
{
	//Add some type of key for mouse input
}

void Game::OnMouseUp(WPARAM btnState, int x, int y)
{
	//Add some type of key for mouse input
}

void Game::OnMouseMove(WPARAM btnState, int x, int y)
{
	//Add some type of key for mouse input
}

void Game::OnKeyPressed(WPARAM key)
{
	mStateStack.handleEvent(key);

	//CommandQueue& commands = gameWorld.getCommandQueue();
	//mPlayer.handleEvent(key, commands);
}

void Game::OnKeyboardInput(const GameTimer& gt)
{

}

void Game::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for (auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}


void Game::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
	for (auto& e : mMaterials /*ResourceManager::GetInstance()->GetMaterials()*/)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
		//D3DMATRIX matTranslate;

	}
}

void Game::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mEyePos;
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();

	mMainPassCB.AmbientLight = { 0.4f, 0.4f, 0.4f, 1.0f };

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void Game::LoadTextures()
{
	mTextures.clear();

	auto desetText = std::make_unique<Texture>();
	desetText->Name = std::to_string(Textures::ID::Desert);
	desetText->Filename = L"Media/Desert.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), desetText->Filename.c_str(),
		desetText->Resource, desetText->UploadHeap));

	auto eagleText = std::make_unique<Texture>();
	eagleText->Name = std::to_string(Textures::ID::Eagle);
	eagleText->Filename = L"Media/Eagle3.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), eagleText->Filename.c_str(),
		eagleText->Resource, eagleText->UploadHeap));

	auto raptorText = std::make_unique<Texture>();
	raptorText->Name = std::to_string(Textures::ID::Raptor);
	raptorText->Filename = L"Media/Raptor.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), raptorText->Filename.c_str(),
		raptorText->Resource, raptorText->UploadHeap));

	auto title = std::make_unique<Texture>();
	title->Name = std::to_string(Textures::ID::TitleScreen);
	title->Filename = L"Media/TitleBackground.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), title->Filename.c_str(),
		title->Resource, title->UploadHeap));

	auto arrow = std::make_unique<Texture>();
	arrow->Name = std::to_string(Textures::ID::Arrow);
	arrow->Filename = L"Media/Arrow.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), arrow->Filename.c_str(),
		arrow->Resource, arrow->UploadHeap));

	auto pauseText = std::make_unique<Texture>();
	pauseText->Name = std::to_string(Textures::ID::PauseText);
	pauseText->Filename = L"Media/PauseText.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), pauseText->Filename.c_str(),
		pauseText->Resource, pauseText->UploadHeap));

	auto playText = std::make_unique<Texture>();
	playText->Name = std::to_string(Textures::ID::PlayText);
	playText->Filename = L"Media/PlayText.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), playText->Filename.c_str(),
		playText->Resource, playText->UploadHeap));

	auto quitText = std::make_unique<Texture>();
	quitText->Name = std::to_string(Textures::ID::QuitText);
	quitText->Filename = L"Media/QuitText.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), quitText->Filename.c_str(),
		quitText->Resource, quitText->UploadHeap));

	auto menuScreen = std::make_unique<Texture>();
	menuScreen->Name = std::to_string(Textures::ID::MenuScreen);
	menuScreen->Filename = L"Media/MenuBackground2.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), menuScreen->Filename.c_str(),
		menuScreen->Resource, menuScreen->UploadHeap));

	auto titleText = std::make_unique<Texture>();
	titleText->Name = std::to_string(Textures::ID::TitleText);
	titleText->Filename = L"Media/TitleText.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), titleText->Filename.c_str(),
		titleText->Resource, titleText->UploadHeap));

	


	mTextures[desetText->Name] = std::move(desetText);
	mTextures[eagleText->Name] = std::move(eagleText);
	mTextures[raptorText->Name] = std::move(raptorText);
	mTextures[title->Name] = std::move(title);
	mTextures[arrow->Name] = std::move(arrow);
	mTextures[pauseText->Name] = std::move(pauseText);
	mTextures[playText->Name] = std::move(playText);
	mTextures[quitText->Name] = std::move(quitText);
	mTextures[menuScreen->Name] = std::move(menuScreen);
	mTextures[titleText->Name] = std::move(titleText);
}

//If we have 3 frame resources and n render items, then we have three 3n object constant
//buffers and 3 pass constant buffers.Hence we need 3(n + 1) constant buffer views(CBVs).
//Thus we will need to modify our CBV heap to include the additional descriptors :


void Game::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = Textures::NUM_TEXTURE_IDS;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	assert(Textures::NUM_TEXTURE_IDS > 0);

	for (int i = 0; i < Textures::NUM_TEXTURE_IDS; i++)
	{
		auto text = mTextures[std::to_string(i)]->Resource;

		if (i == 0)
		{
			srvDesc.Format = text->GetDesc().Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = -1;
			md3dDevice->CreateShaderResourceView(text.Get(), &srvDesc, hDescriptor);
		}
		else
		{
			srvDesc.Format = text->GetDesc().Format;
			md3dDevice->CreateShaderResourceView(text.Get(), &srvDesc, hDescriptor);
		}
		hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	}
}

//A root signature defines what resources need to be bound to the pipeline before issuing a draw call and
//how those resources get mapped to shader input registers. there is a limit of 64 DWORDs that can be put in a root signature.
void Game::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void Game::BuildShadersAndInputLayout()
{
	mShaders.clear();
	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}


void Game::BuildShapeGeometry()
{
	mGeometries.clear();

	GeometryGenerator geoGen;
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);
	UINT gridVertexOffset = 0;
	UINT gridIndexOffset = 0;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	auto totalVertexCount =
		grid.Vertices.size();
	std::vector<Vertex> vertices(totalVertexCount);
	UINT k = 0;

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		//vertices[k].Color = XMFLOAT4(DirectX::Colors::ForestGreen);
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].TexC = grid.Vertices[i].TexC;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));


	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;
	geo->DrawArgs["grid"] = gridSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void Game::ReAddMaterials()
{
	for (auto& e : mAllRitems)
	{
		e->material = mMaterials.find(std::to_string('0' + e->materialID))->second.get();
	}
}

void Game::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&mPSOs["transparent"])));
}

void Game::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			1, (UINT)mAllRitems.size(), (UINT)mMaterials.size()/*ResourceManager::GetInstance()->GetMaterials().size()*/));
	}
}

void Game::BuildMaterials()
{
	//mMaterials.clear();

	auto mat0 = std::make_unique<Material>();
	mat0->Name = std::to_string('0');
	mat0->MatCBIndex = 0;
	mat0->DiffuseSrvHeapIndex = 0;
	mat0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat0->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat0->Roughness = 1.0f;
	mMaterials[std::to_string('0')] = std::move(mat0);

	auto mat1 = std::make_unique<Material>();
	mat1->Name = std::to_string('1');
	mat1->MatCBIndex = 1;
	mat1->DiffuseSrvHeapIndex = 1;
	mat1->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat1->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat1->Roughness = 1.0f;
	mMaterials[std::to_string('1')] = std::move(mat1);

	auto mat2 = std::make_unique<Material>();
	mat2->Name = std::to_string('2');
	mat2->MatCBIndex = 2;
	mat2->DiffuseSrvHeapIndex = 2;
	mat2->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat2->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat2->Roughness = 1.0f;
	mMaterials[std::to_string('2')] = std::move(mat2);

	auto mat3 = std::make_unique<Material>();
	mat3->Name = std::to_string('3');
	mat3->MatCBIndex = 3;
	mat3->DiffuseSrvHeapIndex = 3;
	mat3->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat3->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat3->Roughness = 1.0f;
	mMaterials[std::to_string('3')] = std::move(mat3);

	auto mat4 = std::make_unique<Material>();
	mat4->Name = std::to_string('4');
	mat4->MatCBIndex = 4;
	mat4->DiffuseSrvHeapIndex = 4;
	mat4->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat4->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat4->Roughness = 1.0f;
	mMaterials[std::to_string('4')] = std::move(mat4);

	auto mat5 = std::make_unique<Material>();
	mat5->Name = std::to_string('5');
	mat5->MatCBIndex = 5;
	mat5->DiffuseSrvHeapIndex = 5;
	mat5->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat5->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat5->Roughness = 1.0f;
	mMaterials[std::to_string('5')] = std::move(mat5);

	auto mat6 = std::make_unique<Material>();
	mat6->Name = std::to_string('6');
	mat6->MatCBIndex = 6;
	mat6->DiffuseSrvHeapIndex = 6;
	mat6->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat6->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat6->Roughness = 1.0f;
	mMaterials[std::to_string('6')] = std::move(mat6);

	auto mat7 = std::make_unique<Material>();
	mat7->Name = std::to_string('7');
	mat7->MatCBIndex = 7;
	mat7->DiffuseSrvHeapIndex = 7;
	mat7->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat7->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat7->Roughness = 1.0f;
	mMaterials[std::to_string('7')] = std::move(mat7);

	auto mat8 = std::make_unique<Material>();
	mat8->Name = std::to_string('8');
	mat8->MatCBIndex = 8;
	mat8->DiffuseSrvHeapIndex = 8;
	mat8->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat8->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat8->Roughness = 1.0f;
	mMaterials[std::to_string('8')] = std::move(mat8);

	auto mat9 = std::make_unique<Material>();
	mat9->Name = std::to_string('9');
	mat9->MatCBIndex = 9;
	mat9->DiffuseSrvHeapIndex = 9;
	mat9->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat9->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat9->Roughness = 1.0f;
	mMaterials[std::to_string('9')] = std::move(mat9);
}

bool Game::GetKeyIsPressed()
{
	return keyIsPressed;
}

WPARAM Game::GetKeyPressed()
{
	return keyPressed;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> Game::GetCMDList()
{
	return mCommandList;
}

std::vector<std::unique_ptr<RenderItem>>& Game::GetRenderItems()
{
	return mAllRitems;
}

float Game::GetClientWidth()
{
	return mClientWidth;
}

float Game::GetClientHeight()
{
	return mClientHeight;
}

//void Game::AddTexture(Textures::ID id, std::wstring fileName)
//{
//	//ResourceManager::GetInstance()->GetTextureHolder().insert(std::pair<Textures::ID, std::wstring>(id, fileName));
//}

void Game::BuildRenderItems()
{
	for (auto& e : mAllRitems)
		mOpaqueRitems.push_back(e.get());
}

void Game::AddRenderItem(RenderItem* renderItems)
{
	mOpaqueRitems.push_back(renderItems);
}

void Game::InitalizeCamera(float radius, float theta, float phi)
{
	mEyePos.x = radius * sinf(phi) * cosf(theta);
	mEyePos.z = radius * sinf(phi) * sinf(theta);
	mEyePos.y = radius * cosf(phi);
	view = XMMatrixLookAtLH(XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f), target, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMStoreFloat4x4(&mView, view);
}

void Game::InitalizeState()
{
}

void Game::RegisterStates()
{
	mStateStack.registerState<TitleState>(States::Title);
	mStateStack.registerState<MenuState>(States::Menu);
	mStateStack.registerState<GameState>(States::Game);
	mStateStack.registerState<PauseState>(States::Pause);
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Game::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

