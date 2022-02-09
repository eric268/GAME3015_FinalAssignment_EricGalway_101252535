#pragma once
#include <vector>
#include "ResourceHolder.h"
#include "ResourceIdentifiers.h"
#include "../D3DCommon/d3dApp.h"

class ResourceManager
{
public:
	ResourceManager();

	static ResourceManager* GetInstance();
	std::map<Textures::ID, std::wstring>& GetTextureHolder();

private:
	static ResourceManager* mInstance;

	std::map<Textures::ID,std::wstring> mTextureHolder;
};

