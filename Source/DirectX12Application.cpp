//***************************************************************************************
// DirectX12Application.cpp 
//
// Hold down '1' key to view scene in wireframe mode.
//***************************************************************************************

#include "DirectX12Application.h"
#include "ResourceManager.h"
#include "Game.h"

// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.

const int gNumFrameResources = 3;

DirectX12Application::DirectX12Application(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
}

DirectX12Application::~DirectX12Application()
{
    if(md3dDevice != nullptr)
        FlushCommandQueue();
}

bool DirectX12Application::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	game = Game();

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildLandGeometry();
	BuildShapeGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	//BuildConstantBufferViews();
	BuildPSOs();

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}

void DirectX12Application::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void DirectX12Application::Update(const GameTimer& gt)
{
	game.Update(gt);
	OnKeyboardInput(gt);
	UpdateCamera(gt);

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
}

void DirectX12Application::Draw(const GameTimer& gt)
{
	game.Draw(gt);
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	if (mIsWireframe)
	{
		ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque_wireframe"].Get()));
	}
	else
	{
		ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));
	}

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

	// int passCbvIndex = mPassCbvOffset + mCurrFrameResourceIndex;
	// auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
	// passCbvHandle.Offset(passCbvIndex, mCbvSrvUavDescriptorSize);
	// mCommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(mCommandList.Get(), mOpaqueRitems);

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

void DirectX12Application::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void DirectX12Application::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void DirectX12Application::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		//mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.05f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.05f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		//mRadius = MathHelper::Clamp(mRadius, 5.0f, 150.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void DirectX12Application::OnKeyboardInput(const GameTimer& gt)
{
	if (GetAsyncKeyState('1') & 0x8000)
		mIsWireframe = true;
	else
		mIsWireframe = false;
}

void DirectX12Application::UpdateCamera(const GameTimer& gt)
{
	XMVECTOR right = XMVectorSet(0.01f, 0.0f, 0.0f, 1.0f);
	XMVECTOR upward = XMVectorSet(0.00f, 0.01f, 0.0f, 1.0f);
	// Convert Spherical to Cartesian coordinates.
	mEyePos.x = mRadius * sinf(mPhi) * cosf(mTheta);
	mEyePos.z = mRadius * sinf(mPhi) * sinf(mTheta);
	mEyePos.y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	//Move Right and Left


	XMStoreFloat4x4(&mView, view);
}

void DirectX12Application::AnimateMaterials(const GameTimer& gt)
{
	// Scroll the water material texture coordinates.
	auto waterMat = mMaterials["water"].get();

	float& tu = waterMat->MatTransform(3, 0);
	float& tv = waterMat->MatTransform(3, 1);

	tu += 0.1f * gt.DeltaTime();
	tv += 0.02f * gt.DeltaTime();

	if (tu >= 1.0f)
		tu -= 1.0f;

	if (tv >= 1.0f)
		tv -= 1.0f;

	waterMat->MatTransform(3, 0) = tu;
	waterMat->MatTransform(3, 1) = tv;

	// Material has changed, so need to update cbuffer.
	waterMat->NumFramesDirty = gNumFrameResources;
}

void DirectX12Application::UpdateObjectCBs(const GameTimer& gt)
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

void DirectX12Application::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
	for (auto& e : mMaterials)
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
	}
}

void DirectX12Application::UpdateMainPassCB(const GameTimer& gt)
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

	mMainPassCB.AmbientLight = { 0.15f, 0.15f, 0.15f, 1.0f };
	mMainPassCB.Lights[0].Direction = { -0.5, -0.5, 3.0 };
	mMainPassCB.Lights[0].Strength = { 0.1,0.1,0.1 };

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void DirectX12Application::LoadTextures()
{
	for (int i = 0; i < Textures::NUM_TEXTURE_IDS; i++)
	{
		auto temp = std::make_unique<Texture>();
		temp->Name = std::to_string(i);

		auto t = ResourceManager::GetInstance()->GetTextureHolder().find(Textures::ID(i));

		temp->Filename = t->second;
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
			mCommandList.Get(), temp->Filename.c_str(),
			temp->Resource, temp->UploadHeap));

		mTextures[temp->Name] = std::move(temp);
	}

	//auto grassTex = std::make_unique<Texture>();
	//grassTex->Name = "grassTex";
	//grassTex->Filename = L"Media/grass.dds";
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	//	mCommandList.Get(), grassTex->Filename.c_str(),
	//	grassTex->Resource, grassTex->UploadHeap));

	//auto sandbrickTex = std::make_unique<Texture>();
	//sandbrickTex->Name = "sandbrickTex";
	//sandbrickTex->Filename = L"Media/sandbrick.dds";
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	//	mCommandList.Get(), sandbrickTex->Filename.c_str(),
	//	sandbrickTex->Resource, sandbrickTex->UploadHeap));

	//auto ironTex = std::make_unique<Texture>();
	//ironTex->Name = "ironTex";
	//ironTex->Filename = L"Media/iron.dds";
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	//	mCommandList.Get(), ironTex->Filename.c_str(),
	//	ironTex->Resource, ironTex->UploadHeap));

	//auto shingleTex = std::make_unique<Texture>();
	//shingleTex->Name = "shingleTex";
	//shingleTex->Filename = L"Media/shingle.dds";
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	//	mCommandList.Get(), shingleTex->Filename.c_str(),
	//	shingleTex->Resource, shingleTex->UploadHeap));

	//auto stoneTex = std::make_unique<Texture>();
	//stoneTex->Name = "stoneTex";
	//stoneTex->Filename = L"Media/stone.dds";
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	//	mCommandList.Get(), stoneTex->Filename.c_str(),
	//	stoneTex->Resource, stoneTex->UploadHeap));

	//auto stonebrickTex = std::make_unique<Texture>();
	//stonebrickTex->Name = "stonebrickTex";
	//stonebrickTex->Filename = L"Media/stonebrick.dds";
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	//	mCommandList.Get(), stonebrickTex->Filename.c_str(),
	//	stonebrickTex->Resource, stonebrickTex->UploadHeap));

	//auto woodVTex = std::make_unique<Texture>();
	//woodVTex->Name = "woodVTex";
	//woodVTex->Filename = L"Media/woodV.dds";
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	//	mCommandList.Get(), woodVTex->Filename.c_str(),
	//	woodVTex->Resource, woodVTex->UploadHeap));

	//auto waterTex = std::make_unique<Texture>();
	//waterTex->Name = "waterTex";
	//waterTex->Filename = L"Media/water1.dds";
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	//	mCommandList.Get(), waterTex->Filename.c_str(),
	//	waterTex->Resource, waterTex->UploadHeap));

	//auto wroughtIronTex = std::make_unique<Texture>();
	//wroughtIronTex->Name = "wroughtIronTex";
	//wroughtIronTex->Filename = L"Media/wroughtIronTexture.dds";
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	//	mCommandList.Get(), wroughtIronTex->Filename.c_str(),
	//	wroughtIronTex->Resource, wroughtIronTex->UploadHeap));

	//auto flameTex = std::make_unique<Texture>();
	//flameTex->Name = "flameTex";
	//flameTex->Filename = L"Media/FlameTexture.dds";
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	//	mCommandList.Get(), flameTex->Filename.c_str(),
	//	flameTex->Resource, flameTex->UploadHeap));

	////auto crystalTex = std::make_unique<Texture>();
	////crystalTex->Name = "crystalTex";
	////crystalTex->Filename = L"../../Textures/crystal.dds";
	////ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	////    mCommandList.Get(), crystalTex->Filename.c_str(),
	////    crystalTex->Resource, crystalTex->UploadHeap));

	//mTextures[grassTex->Name] = std::move(grassTex);
	//mTextures[sandbrickTex->Name] = std::move(sandbrickTex);
	//mTextures[ironTex->Name] = std::move(ironTex);
	//mTextures[shingleTex->Name] = std::move(shingleTex);
	//mTextures[stoneTex->Name] = std::move(stoneTex);
	//mTextures[stonebrickTex->Name] = std::move(stonebrickTex);
	//mTextures[woodVTex->Name] = std::move(woodVTex);
	//mTextures[waterTex->Name] = std::move(waterTex);
	//mTextures[wroughtIronTex->Name] = std::move(wroughtIronTex);
	//mTextures[flameTex->Name] = std::move(flameTex);
	//mTextures[crystalTex->Name] = std::move(crystalTex);
}
//If we have 3 frame resources and n render items, then we have three 3n object constant
//buffers and 3 pass constant buffers.Hence we need 3(n + 1) constant buffer views(CBVs).
//Thus we will need to modify our CBV heap to include the additional descriptors :


void DirectX12Application::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 10;
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
	}
}

//A root signature defines what resources need to be bound to the pipeline before issuing a draw call and
//how those resources get mapped to shader input registers. there is a limit of 64 DWORDs that can be put in a root signature.
void DirectX12Application::BuildRootSignature()
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

void DirectX12Application::BuildShadersAndInputLayout()
{
	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void DirectX12Application::BuildLandGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  In addition, color the vertices based on their height so we have
	// sandy looking beaches, grassy low hills, and snow mountain peaks.
	//

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		auto& p = grid.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Pos.y = 0;
		vertices[i].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[i].TexC = grid.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = grid.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";

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

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["landGeo"] = std::move(geo);
}

void DirectX12Application::BuildShapeGeometry()
{
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

void DirectX12Application::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
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


	//
	// PSO for opaque wireframe objects.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&mPSOs["opaque_wireframe"])));
}

void DirectX12Application::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			1, (UINT)mAllRitems.size(), (UINT)mMaterials.size()));
	}
}

void DirectX12Application::BuildMaterials()
{
	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->MatCBIndex = 0;
	grass->DiffuseSrvHeapIndex = 0;
	grass->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	grass->Roughness = 0.25f;

	auto sandbrick = std::make_unique<Material>();
	sandbrick->Name = "sandbrick";
	sandbrick->MatCBIndex = 1;
	sandbrick->DiffuseSrvHeapIndex = 1;
	sandbrick->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sandbrick->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	sandbrick->Roughness = 1.0f;

	auto iron = std::make_unique<Material>();
	iron->Name = "iron";
	iron->MatCBIndex = 2;
	iron->DiffuseSrvHeapIndex = 2;
	iron->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	iron->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	iron->Roughness = 0.7f;

	auto shingle = std::make_unique<Material>();
	shingle->Name = "shingle";
	shingle->MatCBIndex = 3;
	shingle->DiffuseSrvHeapIndex = 3;
	shingle->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	shingle->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	shingle->Roughness = 1.0f;

	auto stone = std::make_unique<Material>();
	stone->Name = "stone";
	stone->MatCBIndex = 4;
	stone->DiffuseSrvHeapIndex = 4;
	stone->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	stone->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	stone->Roughness = 1.0f;

	auto stonebrick = std::make_unique<Material>();
	stonebrick->Name = "stonebrick";
	stonebrick->MatCBIndex = 5;
	stonebrick->DiffuseSrvHeapIndex = 5;
	stonebrick->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	stonebrick->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	stonebrick->Roughness = 1.0f;

	auto woodV = std::make_unique<Material>();
	woodV->Name = "woodV";
	woodV->MatCBIndex = 6;
	woodV->DiffuseSrvHeapIndex = 6;
	woodV->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	woodV->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	woodV->Roughness = 0.9f;

	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->MatCBIndex = 7;
	water->DiffuseSrvHeapIndex = 7;
	//step 6: what happens if you change the alpha to 1.0? 100% water and no blending?
	water->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.6f);
	water->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->Roughness = 0.0f;

	auto wroughtIron = std::make_unique<Material>();
	wroughtIron->Name = "wroughtIron";
	wroughtIron->MatCBIndex = 8;
	wroughtIron->DiffuseSrvHeapIndex = 8;
	//step 6: what happens if you change the alpha to 1.0? 100% water and no blending?
	wroughtIron->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.6f);
	wroughtIron->FresnelR0 = XMFLOAT3(0.9f, 0.9f, 0.9f);
	wroughtIron->Roughness = 1.0f;

	auto flames = std::make_unique<Material>();
	flames->Name = "flames";
	flames->MatCBIndex = 9;
	flames->DiffuseSrvHeapIndex = 9;
	//step 6: what happens if you change the alpha to 1.0? 100% water and no blending?
	flames->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.6f);
	flames->FresnelR0 = XMFLOAT3(0.3f, 0.3f, 0.3f);
	flames->Roughness = 0.2f;

	mMaterials["grass"] = std::move(grass);
	mMaterials["sandbrick"] = std::move(sandbrick);
	mMaterials["iron"] = std::move(iron);
	mMaterials["shingle"] = std::move(shingle);
	mMaterials["stone"] = std::move(stone);
	mMaterials["stonebrick"] = std::move(stonebrick);
	mMaterials["woodV"] = std::move(woodV);
	mMaterials["water"] = std::move(water);
	mMaterials["wroughtIron"] = std::move(wroughtIron);
	mMaterials["flames"] = std::move(flames);
}

//This function allows for rotation on all axis
void DirectX12Application::createShapeInWorld(UINT& objIndex, XMFLOAT3 scaling, XMFLOAT3 translation, XMFLOAT3 angle, std::string shapeName, std::string materialName)
{
	auto temp = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&temp->World, XMMatrixScaling(scaling.x, scaling.y, scaling.z) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(angle.x),
		XMConvertToRadians(angle.y), XMConvertToRadians(angle.z)) * XMMatrixTranslation(translation.x, translation.y + (0.5 * scaling.y), translation.z));

	temp->ObjCBIndex = objIndex++;
	temp->Geo = mGeometries["shapeGeo"].get();
	temp->Mat = mMaterials[materialName].get();
	temp->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	temp->IndexCount = temp->Geo->DrawArgs[shapeName].IndexCount;
	temp->StartIndexLocation = temp->Geo->DrawArgs[shapeName].StartIndexLocation;
	temp->BaseVertexLocation = temp->Geo->DrawArgs[shapeName].BaseVertexLocation;
	mAllRitems.push_back(std::move(temp));
}

void DirectX12Application::BuildRenderItems()
{
	UINT objCBIndex = 0;

	//auto gridRitem = std::make_unique<RenderItem>();
	//XMStoreFloat4x4(&gridRitem->World, XMMatrixTranslation(-10.0f, -1.5f, 0.0f));
	//XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	//gridRitem->ObjCBIndex = objCBIndex++;
	//gridRitem->Mat = mMaterials["grass"].get();
	//gridRitem->Geo = mGeometries["landGeo"].get();
	//gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
	//gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	//gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
	//mAllRitems.push_back(std::move(gridRitem));

	//Test shape for lighting
	createShapeInWorld(objCBIndex, XMFLOAT3(5.0f, 5.0f, 5.0f), XMFLOAT3(0.0, 10.0, -20.0), XMFLOAT3(), "grid", "grass");



	// All the render items are opaque.
	for (auto& e : mAllRitems)
		mOpaqueRitems.push_back(e.get());
}


//The DrawRenderItems method is invoked in the main Draw call:
void DirectX12Application::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		// // Offset to the CBV in the descriptor heap for this object and for this frame resource.
		// UINT cbvIndex = mCurrFrameResourceIndex*(UINT)mOpaqueRitems.size() + ri->ObjCBIndex;
		// auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
		// cbvHandle.Offset(cbvIndex, mCbvSrvUavDescriptorSize);
		//
		// cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);
		//
		// cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}



std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> DirectX12Application::GetStaticSamplers()
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

