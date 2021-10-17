#pragma once

#include "../Math/AABB.h"

enum class ColliderType
{
	NONE,

	SHIP,

	BULLET,

	LARGE_ASTEROID,
	MEDIUM_ASTEROID,
	SMOL_ASTEROID,

	COUNT,
};

namespace ColliderRadius
{
//@TODO: Fix these numbers
constexpr static float Ship = 24.0f / 2.0f;

constexpr static float Bullet = 4.0f / 2.0f;

constexpr static float Large  = 62.31f / 2.0f;
constexpr static float Medium = 28.32f / 2.0f;
constexpr static float Small  = 8.43f / 2.0f;

inline float
GetRadiusFromType(const ColliderType& type)
{
	switch(type)
	{
		case ColliderType::SHIP: return Ship;
		case ColliderType::BULLET: return Bullet;
		case ColliderType::LARGE_ASTEROID: return Large;
		case ColliderType::MEDIUM_ASTEROID: return Medium;
		case ColliderType::SMOL_ASTEROID: return Small;
	}
	// @TODO: Might be time to write a logger?
	return 0;
}

inline AABB
GetAABB(const ColliderType& type, const Vector2& center)
{
	//@TODO: This is a bug that I'm willing to live with, due to time constraints.
	//	Refactoring colliders to be polymorphic would be way better than what I'm currently
	//	doing, but maintaining the same level of performance would be quite tough.

	if(type == ColliderType::SHIP)
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
}
