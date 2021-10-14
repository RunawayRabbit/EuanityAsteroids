#pragma once

#include <array>

#include <SDL_rect.h>

#include "Vector2.h"

class AABB;
class Sphere;

class OBB
{
public:
	OBB(const SDL_Rect& rect, const float& rotation);
	OBB(const Vector2& center, const Vector2& ScaledXBasis, const Vector2& scaledYBasis);
	OBB(const Vector2& center, const Vector2& extents, const float& rotation);
	OBB(const Vector2& center, const float& extents, const float& rotation);
	OBB() = delete;

	float DistanceBetween(const Vector2& point) const;
	float DistanceBetweenSq(const Vector2& point) const;
	Vector2 ClosestPointTo(const Vector2& point) const;
	
	AABB Bounds() const;

	std::array<Vector2,4> GetCorners() const;

	// DATA
	Vector2 center;

private:
	Vector2 basisX;
	Vector2 basisY;
	Vector2 extents;
};