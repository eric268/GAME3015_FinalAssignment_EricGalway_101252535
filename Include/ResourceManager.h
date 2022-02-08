#pragma once
#include "ResourceHolder.h"
#include "ResourceIdentifiers.h"
#include "../D3DCommon/d3dApp.h"

class ResourceManager
{
public:
	ResourceManager();

	static ResourceManager* GetInstance();
	ResourceHolder <Texture, Textures::ID>& GetTextureHolder();

private:
	static ResourceManager* mInstance;

	ResourceHolder <Texture, Textures::ID> mTextureHolder;
};

