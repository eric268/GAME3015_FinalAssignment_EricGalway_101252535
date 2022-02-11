#pragma once
#include <vector>
#include <map>
#include "ResourceIdentifiers.h"
#include "../D3DCommon/d3dApp.h"

/// Singleton class for accessing resources
/// 
/// The class which contains all texture and material resources
/// This class was made a Singleton so that resources can be accessed anywhere
class ResourceManager
{
public:
	/// Default constructor for Resource Manager
	ResourceManager();

	/// Returns an instance of the Resource Manager
	/// 
	/// If the instance is null it will initialize the Resource Manager Instance</summary>
	/// <returns>Instance of Resource Manager</returns>
	static ResourceManager* GetInstance();

	/// <summary>Returns Texture Container
	/// 
	/// Returns a reference to texture holder so that textures can be added or removed</summary>
	/// <returns>Map of textures ids and file loading paths</returns>
	std::map<Textures::ID, std::wstring>& GetTextureHolder();

	/// <summary>Returns Material Container
	/// 
	/// Returns a reference to material holder so that materials can be added or removed</summary>
	/// <returns>Map of texture ids and which lead to their respective materials</returns>
	std::unordered_map<Textures::ID, std::unique_ptr<Material>>& GetMaterials();

private:
	static ResourceManager* mInstance;
	std::map<Textures::ID,std::wstring> mTextureHolder;
	std::unordered_map<Textures::ID, std::unique_ptr<Material>> mMaterials;
};

