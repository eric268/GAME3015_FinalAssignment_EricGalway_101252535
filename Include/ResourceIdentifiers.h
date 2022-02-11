#pragma once

/// Namespace encapsulating Texture class
/// 
/// This is used to reference the appropriate Texture ID
namespace resourceIdentifier
{
	class Texture;
}

namespace Textures
{
	/// Enumeration for identifying textures
	enum ID
	{
		Eagle,
		Raptor,
		Desert,
		NUM_TEXTURE_IDS
	};
}

/// Enumeration for categorizing rendering type of objects
/// 
/// Important for letting the PSO know which container to categorize objects to be drawn
enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	Count
};