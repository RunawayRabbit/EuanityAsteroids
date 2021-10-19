#pragma once

#include "Vector2.h"
#include "OBB.h"

class AABB
{
public:
	AABB(const float& top, const float& bottom, const float& left, const float& right);
	AABB(const Vector2& min, const Vector2& max);
	AABB(const float& width, const float& height);

	bool Contains(const Vector2& point) const;
	bool FullyContains(const OBB& box) const;
	AABB operator+(const AABB& other) const;
	AABB operator-(const AABB& other) const;

	// DATA
	union
	{
		struct
		{
			Vector2 min;
			Vector2 max;
		};
		struct
		{
			float left;
			float top;
			float right;
			float bottom;
		};
	};
};
