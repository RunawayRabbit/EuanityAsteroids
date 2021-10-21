#pragma once

#include "../Math/AABB.h"

enum class ColliderType
{
	NONE,

	SHIP_1,
	SHIP_2,
	SHIP_3,

	SHIP_END,

	BULLET,
	BOUNCY_BULLET,

	LARGE_ASTEROID,
	MEDIUM_ASTEROID,
	SMOL_ASTEROID,

	LARGEST_POSSIBLE_COLLIDER,

	COUNT,
};

namespace ColliderUtils
{
//@TODO: Fix these numbers
constexpr static float Ship1X = 16.0f;;
constexpr static float Ship1Y = 28.0f;;
constexpr static float Ship2X = 25.0f;;
constexpr static float Ship2Y = 23.0f;;
constexpr static float Ship3X = 22.0f;;
constexpr static float Ship3Y = 31.0f;;

constexpr static float Bullet = 4.0f / 2.0f;

constexpr static float Large  = 62.31f / 2.0f;
constexpr static float Medium = 28.32f / 2.0f;
constexpr static float Small  = 8.43f / 2.0f;

inline bool
IsPlayerShip(const ColliderType& type)
{
	return (type >= ColliderType::SHIP_1 && type < ColliderType::SHIP_END);
}


inline float
GetRadiusFromType(const ColliderType& type)
{
	switch(type)
	{
		case ColliderType::SHIP_1: return std::max(Ship1X, Ship1Y);
		case ColliderType::SHIP_2: return std::max(Ship2X, Ship2Y);
		case ColliderType::SHIP_3: return std::max(Ship3X, Ship3Y);
		case ColliderType::BULLET:
		case ColliderType::BOUNCY_BULLET: return Bullet;
		case ColliderType::LARGE_ASTEROID: return Large;
		case ColliderType::MEDIUM_ASTEROID: return Medium;
		case ColliderType::SMOL_ASTEROID: return Small;
		case ColliderType::LARGEST_POSSIBLE_COLLIDER: return Large;
	}
	// @TODO: Might be time to write a logger?
	return 0;
}

inline AABB
GetAABB(const ColliderType& type, const Vector2& center)
{
	//	Refactoring colliders to be polymorphic would be way better than what I'm currently
	//	doing, but maintaining the same level of performance would be quite tough.

	if(type >= ColliderType::SHIP_1 && type <= ColliderType::SHIP_3)
	{
		// @BUG:
		//	This will return the WRONG AABB for the Ship collider type, because it is an OBB, and
		//	we are assuming that it is square and axis-alligned.

		_assume(false);
	}

	const auto radius = GetRadiusFromType(type);
	const auto min    = Vector2(center.x - radius, center.y - radius);
	const auto max    = Vector2(center.x + radius, center.y + radius);
	return AABB(min, max);
}

inline Vector2
GetDimFromType(ColliderType collider)
{
	switch(collider)
	{
		case ColliderType::SHIP_1: return Vector2(Ship1X, Ship1Y);
		case ColliderType::SHIP_2: return Vector2(Ship2X, Ship2Y);
		case ColliderType::SHIP_3: return Vector2(Ship3X, Ship3Y);
		default: return Vector2::Zero();
	}
}
}
