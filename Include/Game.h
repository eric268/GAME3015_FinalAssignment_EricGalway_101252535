#pragma once

#include "../D3DCommon/d3dApp.h"
#include "../D3DCommon/MathHelper.h"
#include "../D3DCommon/UploadBuffer.h"
#include "../D3DCommon/GeometryGenerator.h"
#include "../D3DCommon/FrameResource.h"
#include "../D3DCommon/RenderItem.h"
#include "World.h"
#include "StateStack.h"
#include "Player.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;


/// Game class that acts as application framework 
/// 
/// This class is responsible for creating the game world, handling user input and iterating through main game loop
/// Only one instance of this class is able to be created at a time
class Game : public D3DApp
{
public:
	Game(HINSTANCE hInstance);
	Game(const Game& rhs) = delete;
	Game& operator=(const Game& rhs) = delete;
	~Game();

	virtual bool Initialize()override;

	void BuildRenderItems();
	void AddRenderItem(RenderItem* renderItems);

	virtual bool GetKeyIsPressed() override;
	virtual WPARAM GetKeyPressed() override;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCMDList();

	std::vector<std::unique_ptr<RenderItem>>& GetRenderItems();

	float GetClientWidth();
	float GetClientHeight();

	void AddTexture(Textures::ID id, std::wstring fileName);
	void BuildFrameResources();
	void BuildMaterials();
	void BuildPSOs();
	void InitalizeCamera();
	void InitalizeState();
	void LoadTextures();
	void LoadText();

	bool doOnce = false;


public:
	static UINT objCBIndex;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	FrameResource* mCurrFrameResource;
	UINT mCbvSrvDescriptorSize = 0;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	std::vector<RenderItem*> mOpaqueRitems;
	int mCurrFrameResourceIndex = 0;
private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
	virtual void OnKeyPressed(WPARAM key) override;

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	

	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();

	void BuildShapeGeometry();


	void RegisterStates();

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;

	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];


private:
	//World gameWorld;
	Player mPlayer;
	StateStack mStateStack;



	ComPtr<ID3D12RootSignature> mRootSignature;

	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	PassConstants mMainPassCB;

	UINT mPassCbvOffset;

	//Camera Variables
	XMFLOAT3  mEyePos;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	float mTheta;
	float mPhi;
	float mRadius;
	XMVECTOR target;
	XMMATRIX view;

};