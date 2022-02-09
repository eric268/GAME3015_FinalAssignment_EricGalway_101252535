#pragma once

namespace resourceIdentifier
{
	class Texture;
}

namespace Textures
{
	enum ID
	{
		Eagle,
		Raptor,
		Desert,
		NUM_TEXTURE_IDS
	};
}

enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	Count
};

template <typename Resource, typename Identifier>
class ResourceHolder;

typedef ResourceHolder<resourceIdentifier::Texture, Textures::ID> TextureHolder;