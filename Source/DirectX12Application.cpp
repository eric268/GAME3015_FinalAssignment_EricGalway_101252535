//***************************************************************************************
// ShapesApp.cpp 
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
    if(!D3DApp::Initialize())
        return false;

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
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
    if(mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
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


    auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    ThrowIfFailed(cmdListAlloc->Reset());

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
    if((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        // Update angles based on input to orbit camera around box.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        //mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.2 unit in the scene.
        float dx = 0.05f*static_cast<float>(x - mLastMousePos.x);
        float dy = 0.05f*static_cast<float>(y - mLastMousePos.y);

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
    if(GetAsyncKeyState('1') & 0x8000)
        mIsWireframe = true;
    else
        mIsWireframe = false;
}
 
void DirectX12Application::UpdateCamera(const GameTimer& gt)
{
    XMVECTOR right = XMVectorSet(0.01f, 0.0f, 0.0f, 1.0f);
    XMVECTOR upward = XMVectorSet(0.00f, 0.01f, 0.0f, 1.0f);
	// Convert Spherical to Cartesian coordinates.
	mEyePos.x = mRadius*sinf(mPhi)*cosf(mTheta);
	mEyePos.z = mRadius*sinf(mPhi)*sinf(mTheta);
	mEyePos.y = mRadius*cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    //Move Right and Left

    
	XMStoreFloat4x4(&mView, view);
}

void DirectX12Application::AnimateMaterials(const GameTimer& gt)
{
}



void DirectX12Application::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for(auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if(e->NumFramesDirty > 0)
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
    mMainPassCB.Lights[0].Strength = { 0.1,0.1,0.1};

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}


void DirectX12Application::BuildDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 4;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//auto desertTex = ResourceManager::GetInstance()->GetTextureHolder().get(Textures::Desert).Resource;
	//auto eagleTex = ResourceManager::GetInstance()->GetTextureHolder().get(Textures::Eagle).Resource;
	//auto raptorTex = ResourceManager::GetInstance()->GetTextureHolder().get(Textures::Raptor).Resource;

  
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//srvDesc.Format = desertTex->GetDesc().Format;
 //   md3dDevice->CreateShaderResourceView(desertTex.Get(), &srvDesc, hDescriptor);

 //   // next descriptor
 //   hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	//srvDesc.Format = eagleTex->GetDesc().Format;
	//md3dDevice->CreateShaderResourceView(eagleTex.Get(), &srvDesc, hDescriptor);

	//// next descriptor
	//hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	//srvDesc.Format = raptorTex->GetDesc().Format;
	//md3dDevice->CreateShaderResourceView(raptorTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
}

//assuming we have n renter items, we can populate the CBV heap with the following code where descriptors 0 to n-
//1 contain the object CBVs for the 0th frame resource, descriptors n to 2n−1 contains the
//object CBVs for 1st frame resource, descriptors 2n to 3n−1 contain the objects CBVs for
//the 2nd frame resource, and descriptors 3n, 3n + 1, and 3n + 2 contain the pass CBVs for the
//0th, 1st, and 2nd frame resource
//void ShapesApp::BuildConstantBufferViews()
//{
//    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//
//    UINT objCount = (UINT)mOpaqueRitems.size();
//
//    // Need a CBV descriptor for each object for each frame resource.
//    for(int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
//    {
//        auto objectCB = mFrameResources[frameIndex]->ObjectCB->Resource();
//        for(UINT i = 0; i < objCount; ++i)
//        {
//            D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();
//
//            // Offset to the ith object constant buffer in the buffer.
//            cbAddress += i*objCBByteSize;
//
//            // Offset to the object cbv in the descriptor heap.
//            int heapIndex = frameIndex*objCount + i;
//
//			//we can get a handle to the first descriptor in a heap with the ID3D12DescriptorHeap::GetCPUDescriptorHandleForHeapStart
//            auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
//
//			//our heap has more than one descriptor,we need to know the size to increment in the heap to get to the next descriptor
//			//This is hardware specific, so we have to query this information from the device, and it depends on
//			//the heap type.Recall that our D3DApp class caches this information: 	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//            handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);
//
//            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
//            cbvDesc.BufferLocation = cbAddress;
//            cbvDesc.SizeInBytes = objCBByteSize;
//
//            md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
//        }
//    }
//
//    UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
//
//    // Last three descriptors are the pass CBVs for each frame resource.
//    for(int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
//    {
//        auto passCB = mFrameResources[frameIndex]->PassCB->Resource();
//        D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();
//
//        // Offset to the pass cbv in the descriptor heap.
//        int heapIndex = mPassCbvOffset + frameIndex;
//        auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
//        handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);
//
//        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
//        cbvDesc.BufferLocation = cbAddress;
//        cbvDesc.SizeInBytes = passCBByteSize;
//        
//        md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
//    }
//}

//A root signature defines what resources need to be bound to the pipeline before issuing a draw call and
//how those resources get mapped to shader input registers. there is a limit of 64 DWORDs that can be put in a root signature.
void DirectX12Application::BuildRootSignature()
{
    //CD3DX12_DESCRIPTOR_RANGE cbvTable0;
    //cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    //
    //CD3DX12_DESCRIPTOR_RANGE cbvTable1;
    //cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    //
	//// Root parameter can be a table, root descriptor or root constants.
	//CD3DX12_ROOT_PARAMETER slotRootParameter[2];
    //
	//// Create root CBVs.
    //slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
    //slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);
    //
	//// A root signature is an array of root parameters.
	//CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr, 
    //    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    //
	//// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	//ComPtr<ID3DBlob> serializedRootSig = nullptr;
	//ComPtr<ID3DBlob> errorBlob = nullptr;
	//HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
	//	serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
    //
	//if(errorBlob != nullptr)
	//{
	//	::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	//}
	//ThrowIfFailed(hr);
    //
	//ThrowIfFailed(md3dDevice->CreateRootSignature(
	//	0,
	//	serializedRootSig->GetBufferPointer(),
	//	serializedRootSig->GetBufferSize(),
	//	IID_PPV_ARGS(mRootSignature.GetAddressOf())));

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
        //auto& p = grid.Vertices[i].Position;
        //vertices[i].Pos = p;
        //vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
        //vertices[i].Normal = GetHillsNormal(p.x, p.z);
        //vertices[i].TexC = grid.Vertices[i].TexC;
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
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3, 0.5, 0.5);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.05f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(1.0f,0.75f,1.0f,4,20);
    GeometryGenerator::MeshData cone = geoGen.CreateCone(1.0f, 1.0f, 20, 20);
    GeometryGenerator::MeshData pyramid = geoGen.CreatePyramid(1.0f, 1.0f, 20);
    GeometryGenerator::MeshData triprism = geoGen.CreateTriangularPrism(1.0f, 0.1f,1.0f, 20);
    GeometryGenerator::MeshData diamond = geoGen.CreateDiamond(3.0f, 4.0f, 4, 20);
    GeometryGenerator::MeshData wedge = geoGen.CreateWedge(1.0f, 1.0f, 1.0f, 3);
    GeometryGenerator::MeshData torus = geoGen.CreateTorus(1.0f, 0.25f, 10, 12);
    GeometryGenerator::MeshData grayBox = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3, 0.25, 1);
    GeometryGenerator::MeshData fireBox = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3, 1, 1);
    GeometryGenerator::MeshData blackCylinder = geoGen.CreateCylinder(1.0f, 1.0f, 1.0f, 14, 20);
    GeometryGenerator::MeshData roundcylinder = geoGen.CreateCylinder(1.0f, 1.0f, 1.0f, 20, 20);
    GeometryGenerator::MeshData torchHandle = geoGen.CreateCone(0.25f, 0.75f, 20, 20);
    
    


	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();
    UINT coneVertexOffset = cylinderVertexOffset + (UINT)cylinder.Vertices.size();
    UINT pyramidVertexOffset = coneVertexOffset + (UINT)cone.Vertices.size();
    UINT triprismVertexOffset = pyramidVertexOffset + (UINT)pyramid.Vertices.size();
    UINT diamondVertexOffset = triprismVertexOffset + (UINT)triprism.Vertices.size();
    UINT wedgeVertexOffset = diamondVertexOffset + (UINT)diamond.Vertices.size();
    UINT torusVertexOffset = wedgeVertexOffset + (UINT)wedge.Vertices.size();
    UINT grayBoxVertexOffset = torusVertexOffset + (UINT)torus.Vertices.size();
    UINT brownBoxVertexOffset = grayBoxVertexOffset + (UINT)grayBox.Vertices.size();
    UINT blackCylinderVertexOffset = brownBoxVertexOffset + (UINT)fireBox.Vertices.size();
    UINT roundCylinderVertexOffset = blackCylinderVertexOffset + (UINT)blackCylinder.Vertices.size();
    UINT torchHandleVertexOffset = roundCylinderVertexOffset + (UINT)roundcylinder.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();
    UINT coneIndexOffset = cylinderIndexOffset + (UINT)cylinder.Indices32.size();
    UINT pyramidIndexOffset = coneIndexOffset + (UINT)cone.Indices32.size();
    UINT triprismIndexOffset = pyramidIndexOffset + (UINT)pyramid.Indices32.size();
    UINT diamondIndexOffset = triprismIndexOffset + (UINT)triprism.Indices32.size();
    UINT wedgeIndexOffset = diamondIndexOffset + (UINT)diamond.Indices32.size();
    UINT torusIndexOffset = wedgeIndexOffset + (UINT)wedge.Indices32.size();
    UINT grayBoxIndexOffset = torusIndexOffset + (UINT)torus.Indices32.size();
    UINT brownBoxIndexOffset = grayBoxIndexOffset + (UINT)grayBox.Indices32.size();
    UINT blackCylinderIndexOffset = brownBoxIndexOffset + (UINT)fireBox.Indices32.size();
    UINT roundCylinderIndexOffset = blackCylinderIndexOffset + (UINT)blackCylinder.Indices32.size();
    UINT torchHandleIndexOffset = roundCylinderIndexOffset + (UINT)roundcylinder.Indices32.size();

    // Define the SubmeshGeometry that cover different 
    // regions of the vertex/index buffers.

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

    SubmeshGeometry coneSubmesh;
    coneSubmesh.IndexCount = (UINT)cone.Indices32.size();
    coneSubmesh.StartIndexLocation = coneIndexOffset;
    coneSubmesh.BaseVertexLocation = coneVertexOffset;

    SubmeshGeometry pyramidSubmesh;
    pyramidSubmesh.IndexCount = (UINT)pyramid.Indices32.size();
    pyramidSubmesh.StartIndexLocation = pyramidIndexOffset;
    pyramidSubmesh.BaseVertexLocation = pyramidVertexOffset;

    SubmeshGeometry triprismSubmesh;
    triprismSubmesh.IndexCount = (UINT)triprism.Indices32.size();
    triprismSubmesh.StartIndexLocation = triprismIndexOffset;
    triprismSubmesh.BaseVertexLocation = triprismVertexOffset;

    SubmeshGeometry diamondSubmesh;
    diamondSubmesh.IndexCount = (UINT)diamond.Indices32.size();
    diamondSubmesh.StartIndexLocation = diamondIndexOffset;
    diamondSubmesh.BaseVertexLocation = diamondVertexOffset;

    SubmeshGeometry wedgeSubmesh;
    wedgeSubmesh.IndexCount = (UINT)wedge.Indices32.size();
    wedgeSubmesh.StartIndexLocation = wedgeIndexOffset;
    wedgeSubmesh.BaseVertexLocation = wedgeVertexOffset;

    SubmeshGeometry torusSubmesh;
    torusSubmesh.IndexCount = (UINT)torus.Indices32.size();
    torusSubmesh.StartIndexLocation = torusIndexOffset;
    torusSubmesh.BaseVertexLocation = torusVertexOffset;

    SubmeshGeometry grayBoxSubmesh;
    grayBoxSubmesh.IndexCount = (UINT)grayBox.Indices32.size();
    grayBoxSubmesh.StartIndexLocation = grayBoxIndexOffset;
    grayBoxSubmesh.BaseVertexLocation = grayBoxVertexOffset;

    SubmeshGeometry brownBoxSubmesh;
    brownBoxSubmesh.IndexCount = (UINT)fireBox.Indices32.size();
    brownBoxSubmesh.StartIndexLocation = brownBoxIndexOffset;
    brownBoxSubmesh.BaseVertexLocation = brownBoxVertexOffset;

    SubmeshGeometry blackCylinderSubmesh;
    blackCylinderSubmesh.IndexCount = (UINT)blackCylinder.Indices32.size();
    blackCylinderSubmesh.StartIndexLocation = blackCylinderIndexOffset;
    blackCylinderSubmesh.BaseVertexLocation = blackCylinderVertexOffset;

    SubmeshGeometry roundCylinderSubmesh;
    roundCylinderSubmesh.IndexCount = (UINT)roundcylinder.Indices32.size();
    roundCylinderSubmesh.StartIndexLocation = roundCylinderIndexOffset;
    roundCylinderSubmesh.BaseVertexLocation = roundCylinderVertexOffset;

    SubmeshGeometry torchHandleSubmesh;
    torchHandleSubmesh.IndexCount = (UINT)torchHandle.Indices32.size();
    torchHandleSubmesh.StartIndexLocation = torchHandleIndexOffset;
    torchHandleSubmesh.BaseVertexLocation = torchHandleVertexOffset;


	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

    auto totalVertexCount =
        box.Vertices.size() +
        grid.Vertices.size() +
        sphere.Vertices.size() +
        cylinder.Vertices.size() +
        cone.Vertices.size() +
        pyramid.Vertices.size() +
        triprism.Vertices.size() +
        diamond.Vertices.size() +
        wedge.Vertices.size() +
        torus.Vertices.size() +
        grayBox.Vertices.size() +
        fireBox.Vertices.size() +
        blackCylinder.Vertices.size() +
        roundcylinder.Vertices.size() + 
        torchHandle.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for(size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::Wheat);
        vertices[k].Normal = box.Vertices[i].Normal;
        vertices[k].TexC = box.Vertices[i].TexC;
	}

	for(size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::ForestGreen);
        vertices[k].Normal = grid.Vertices[i].Normal;
        vertices[k].TexC = grid.Vertices[i].TexC;
	}

	for(size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::Black);
        vertices[k].Normal = sphere.Vertices[i].Normal;
        vertices[k].TexC = sphere.Vertices[i].TexC;
	}

	for(size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		//vertices[k].Color = XMFLOAT4(DirectX::Colors::Wheat);
        vertices[k].Normal = cylinder.Vertices[i].Normal;
        vertices[k].TexC = cylinder.Vertices[i].TexC;
	}

    for (size_t i = 0; i < cone.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = cone.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::MidnightBlue);
        vertices[k].Normal = cone.Vertices[i].Normal;
        vertices[k].TexC = cone.Vertices[i].TexC;
    }

    for (size_t i = 0; i < pyramid.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = pyramid.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::DarkGray);
        vertices[k].Normal = pyramid.Vertices[i].Normal;
        vertices[k].TexC = pyramid.Vertices[i].TexC;
    }

    for (size_t i = 0; i < triprism.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = triprism.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::Indigo);
        vertices[k].Normal = triprism.Vertices[i].Normal;
        vertices[k].TexC = triprism.Vertices[i].TexC;
    }

    for (size_t i = 0; i < diamond.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = diamond.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::SpringGreen);
        vertices[k].Normal = diamond.Vertices[i].Normal;
        vertices[k].TexC = diamond.Vertices[i].TexC;
    }
    for (size_t i = 0; i < wedge.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = wedge.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::SaddleBrown);
        vertices[k].Normal = wedge.Vertices[i].Normal;
        vertices[k].TexC = wedge.Vertices[i].TexC;
    }
    for (size_t i = 0; i < torus.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = torus.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::Black);
        vertices[k].Normal = torus.Vertices[i].Normal;
        vertices[k].TexC = torus.Vertices[i].TexC;
    }
    for (size_t i = 0; i < grayBox.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = grayBox.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::DarkGray);
        vertices[k].Normal = grayBox.Vertices[i].Normal;
        vertices[k].TexC = grayBox.Vertices[i].TexC;
    }
    for (size_t i = 0; i < fireBox.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = fireBox.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::BurlyWood);
        vertices[k].Normal = fireBox.Vertices[i].Normal;
        vertices[k].TexC = fireBox.Vertices[i].TexC;
    }
    for (size_t i = 0; i < blackCylinder.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = blackCylinder.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::Black);
        vertices[k].Normal = blackCylinder.Vertices[i].Normal;
        vertices[k].TexC = blackCylinder.Vertices[i].TexC;
    }
    for (size_t i = 0; i < roundcylinder.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = roundcylinder.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::BlanchedAlmond);
        vertices[k].Normal = roundcylinder.Vertices[i].Normal;
        vertices[k].TexC = roundcylinder.Vertices[i].TexC;
    }
    for (size_t i = 0; i < torchHandle.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = torchHandle.Vertices[i].Position;
        //vertices[k].Color = XMFLOAT4(DirectX::Colors::BlanchedAlmond);
        vertices[k].Normal = torchHandle.Vertices[i].Normal;
        vertices[k].TexC = torchHandle.Vertices[i].TexC;
    }

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));
    indices.insert(indices.end(), std::begin(cone.GetIndices16()), std::end(cone.GetIndices16()));
    indices.insert(indices.end(), std::begin(pyramid.GetIndices16()), std::end(pyramid.GetIndices16()));
    indices.insert(indices.end(), std::begin(triprism.GetIndices16()), std::end(triprism.GetIndices16()));
    indices.insert(indices.end(), std::begin(diamond.GetIndices16()), std::end(diamond.GetIndices16()));
    indices.insert(indices.end(), std::begin(wedge.GetIndices16()), std::end(wedge.GetIndices16()));
    indices.insert(indices.end(), std::begin(torus.GetIndices16()), std::end(torus.GetIndices16()));
    indices.insert(indices.end(), std::begin(grayBox.GetIndices16()), std::end(grayBox.GetIndices16()));
    indices.insert(indices.end(), std::begin(fireBox.GetIndices16()), std::end(fireBox.GetIndices16()));
    indices.insert(indices.end(), std::begin(blackCylinder.GetIndices16()), std::end(blackCylinder.GetIndices16()));
    indices.insert(indices.end(), std::begin(roundcylinder.GetIndices16()), std::end(roundcylinder.GetIndices16()));
    indices.insert(indices.end(), std::begin(torchHandle.GetIndices16()), std::end(torchHandle.GetIndices16()));


    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size()  * sizeof(std::uint16_t);

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

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;
    geo->DrawArgs["cone"] = coneSubmesh;
    geo->DrawArgs["pyramid"] = pyramidSubmesh;
    geo->DrawArgs["triprism"] = triprismSubmesh;
    geo->DrawArgs["diamond"] = diamondSubmesh;
    geo->DrawArgs["wedge"] = wedgeSubmesh;
    geo->DrawArgs["torus"] = torusSubmesh;
    geo->DrawArgs["grayBox"] = grayBoxSubmesh;
    geo->DrawArgs["brownBox"] = brownBoxSubmesh;
    geo->DrawArgs["blackCylinder"] = blackCylinderSubmesh;
    geo->DrawArgs["roundcylinder"] = roundCylinderSubmesh;
    geo->DrawArgs["torchHandle"] = torchHandleSubmesh;

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
    for(int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            1, (UINT)mAllRitems.size(), (UINT)mMaterials.size()));
    }
}

void DirectX12Application::BuildMaterials()
{
    auto grass = std::make_unique<Material>();
    grass->Name = "desert";
    grass->MatCBIndex = 0;
    grass->DiffuseSrvHeapIndex = 0;
    grass->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    grass->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
    grass->Roughness = 0.25f;

    auto sandbrick = std::make_unique<Material>();
    sandbrick->Name = "eagle";
    sandbrick->MatCBIndex = 1;
    sandbrick->DiffuseSrvHeapIndex = 1;
    sandbrick->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    sandbrick->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
    sandbrick->Roughness = 1.0f;

    auto iron = std::make_unique<Material>();
    iron->Name = "raptor";
    iron->MatCBIndex = 2;
    iron->DiffuseSrvHeapIndex = 2;
    iron->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    iron->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
    iron->Roughness = 0.7f;

    mMaterials["desert"] = std::move(grass);
    mMaterials["eagle"] = std::move(sandbrick);
    mMaterials["raptor"] = std::move(iron);
}

//This function allows for rotation on all axis
void DirectX12Application::createShapeInWorld(UINT& objIndex, XMFLOAT3 scaling, XMFLOAT3 translation, XMFLOAT3 angle, std::string shapeName, std::string materialName)
{
    auto temp = std::make_unique<RenderItem>();
    XMStoreFloat4x4(&temp->World, XMMatrixScaling(scaling.x, scaling.y, scaling.z) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(angle.x), 
        XMConvertToRadians(angle.y), XMConvertToRadians(angle.z))* XMMatrixTranslation(translation.x, translation.y + (0.5 * scaling.y), translation.z));

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
    //gridRitem->World = MathHelper::Identity4x4();
    //XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
    //gridRitem->ObjCBIndex = objCBIndex++;
    //gridRitem->Mat = mMaterials["grass"].get();
    //gridRitem->Geo = mGeometries["shapeGeo"].get();
    //gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    //gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
    //gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
    //gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
    //mAllRitems.push_back(std::move(gridRitem));

	//auto boxRitem = std::make_unique<RenderItem>();
	//XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(2.0f, 2.0f, 2.0f)*XMMatrixTranslation(0.0f, 0.5f, 0.0f));
	//boxRitem->ObjCBIndex = objCBIndex++;
    //boxRitem->Mat = mMaterials["grass"].get();
	//boxRitem->Geo = mGeometries["shapeGeo"].get();
	//boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
	//boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
	//boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
	//mAllRitems.push_back(std::move(boxRitem));

    auto gridRitem = std::make_unique<RenderItem>();
    XMStoreFloat4x4(&gridRitem->World, XMMatrixTranslation(-10.0f, -1.5f, 0.0f));
    XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
    gridRitem->ObjCBIndex = objCBIndex++;
    gridRitem->Mat = mMaterials["grass"].get();
    gridRitem->Geo = mGeometries["landGeo"].get();
    gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
    gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
    gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
    mAllRitems.push_back(std::move(gridRitem));
    
    //Test shape for lighting
    createShapeInWorld(objCBIndex, XMFLOAT3(5.0f, 5.0f, 5.0f), XMFLOAT3(0.0, 10.0, -20.0), XMFLOAT3(0.0f, 0.0f, 0.0f), "box", "woodV");

	// All the render items are opaque.
	for(auto& e : mAllRitems)
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
    for(size_t i = 0; i < ritems.size(); ++i)
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
