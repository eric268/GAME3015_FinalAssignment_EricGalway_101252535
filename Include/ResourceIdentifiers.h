#pragma once
#include <map>
#include <string>
/// Namespace encapsulating Texture class
/// 
/// This is used to reference the appropriate Texture ID
namespace resourceIdentifier
{
	class Texture;
	class Font;
}

namespace Textures
{
	/// Enumeration for identifying textures
	enum ID
	{
		Eagle,
		Raptor,
		Desert,
		TitleScreen,
		Arrow,
		PauseText,
		PlayText,
		QuitText,
		MenuScreen,
		TitleText,
		NUM_TEXTURE_IDS
	};
}

namespace Fonts
{
	enum ID
	{
		Main,
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

class ResourceManager;
typedef std::map<Textures::ID, std::wstring> TextureHolder;
typedef std::map<Fonts::ID, std::wstring> FontHolder;