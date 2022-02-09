#include "ResourceManager.h"

 ResourceManager* ResourceManager::mInstance = nullptr;

ResourceManager::ResourceManager() : mTextureHolder(), mMaterials()
{
}

ResourceManager* ResourceManager::GetInstance()
{
	if (mInstance == nullptr)
	{
		mInstance = new ResourceManager();
	}
	return mInstance;
}

std::map<Textures::ID, std::wstring>& ResourceManager::GetTextureHolder()
{
	return mTextureHolder;
}

std::unordered_map<Textures::ID, std::unique_ptr<Material>>& ResourceManager::GetMaterials()
{
	return mMaterials;
}




