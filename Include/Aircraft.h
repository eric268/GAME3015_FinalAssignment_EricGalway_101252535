#pragma once
#include "Entity.h"
class Aircraft : public Entity
{
public:
	enum Type
	{
		Eagle,
		Raptor
	};

public:
	Aircraft(Type type);
	Aircraft(RenderItem* renderItem);
	Aircraft(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, Type type);
	
	Textures::ID ConvertTypeToTexture(Aircraft::Type type);

	virtual void DrawCurrent(const GameTimer& gt) const;

private:
	Type mType;
};

