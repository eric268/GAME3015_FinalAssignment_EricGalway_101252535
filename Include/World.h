#pragma once
#include "ResourceIdentifiers.h"
#include "SceneNode.h"
#include "Aircraft.h"
#include <array>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class Game;
/// <summary> Class that creates game world
/// 
/// Responsible for all objects that will be created, drawn and updated in the game</summary>
class World
{
public:
	/// <summary>Default Constructor
	/// 
	/// </summary>
	World();

	/// <summary> Overloaded Constructor
	/// 
	/// Contains the width and height of the window</summary>
	/// <param name="width">Window width</param>
	/// <param name="height">Window height</param>
	World( float width, float height);

	/// <summary>Function that updates main game objects
	/// 
	/// This updates the scene graph, manages game bounds and updates the camera</summary>
	/// <param name="gt">Delta Time</param>
	void Update(const GameTimer& gt);

	/// <summary>
	/// Calls the draw function of the scene graph. 
	/// Scene graph will then call all draw functions of child nodes</summary>
	/// <param name="gt">delta time</param>
	void Draw(const GameTimer& gt);

	/// <summary>Creates all scene nodes
	/// 
	/// Initializes their respective values</summary>
	void BuildScene();

	/// <summary> 
	/// Loads individual texture from media folder
	/// </summary>
	/// <param name="id">ID which texture should be stored as for future reference</param>
	/// <param name="fileName">File path for texture resource</param>
	void AddTexture(Textures::ID id, std::wstring fileName);

	/// <summary>
	/// Loads all textures in world
	/// </summary>
	void LoadTextures();

	/// <summary>
	///  Returns the transform of the main camera
	/// </summary>
	/// <returns>XMFLOAT4X4</returns>
	XMFLOAT4X4 GetWorldView();

	/// <summary>
	/// Sets the main camera transform
	/// </summary>
	/// <param name="view">Transform to set camera transform too</param>
	void SetWorldView(XMFLOAT4X4& view);
private:
	/// <summary>
	/// Enumeration for identifying node layers
	/// </summary>
	enum Layer
	{
		Background,
		Air,
		LayerCount
	};

	/// <summary>
	/// Ensures the main aircraft and children stay within screen bounds
	/// </summary>
	void ManagePlayerPosition();

	/// <summary>
	/// Moves the main camera with scroll speed
	/// </summary>
	/// <param name="gt">Delta time</param>
	void UpdateCamera(const GameTimer& gt);

private:
	SceneNode mSceneGraph;
	std::array<SceneNode*, LayerCount>	mSceneLayers;
	Aircraft* mPlayerAircraft;
	Game* mGame;
	XMFLOAT4 mWorldBounds;
	XMFLOAT4X4 mWorldView;

	XMFLOAT3 cameraPosition;
	XMFLOAT3 mSpawnPosition;
	XMFLOAT3 worldViewPosition;

	float mScrollSpeed;
	float screenWidth;
	float screenHeight;
	float screenWidthBuffer;
	float screenToWorldRatio;
	float changeInPlayerRotation;
};


