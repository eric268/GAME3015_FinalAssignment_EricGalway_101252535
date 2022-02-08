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

ResourceHolder<Texture, Textures::ID>& ResourceManager::GetTextureHolder()
{
	return mTextureHolder;
}

