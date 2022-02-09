#include "ResourceManager.h"

 ResourceManager* ResourceManager::mInstance = nullptr;

ResourceManager::ResourceManager() : mTextureHolder()
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



