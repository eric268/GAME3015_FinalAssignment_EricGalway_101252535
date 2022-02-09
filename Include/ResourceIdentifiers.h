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

template <typename Resource, typename Identifier>
class ResourceHolder;

typedef ResourceHolder<resourceIdentifier::Texture, Textures::ID> TextureHolder;