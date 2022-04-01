#pragma once

/// <summary>
/// A namespace encapsulating an enum used for differentiating between different types of 
/// of Scene Nodes
/// </summary>
namespace Category
{
	enum Type
	{
		//each category has one bit to set to 1 except None- this allows you to have multiple categories
		// size_t anyaircraft = Category::PlayerAircraft || Category::AlliedAircraft || Category::EnemyAircraft

		None = 0,
		Scene = 1 << 0,
		PlayerAircraft = 1 << 1,
		AlliedAircraft = 1 << 2,
		EnemyAircraft = 1 << 3,
	};
}