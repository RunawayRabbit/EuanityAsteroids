#pragma once

#include "Vector2.h"

struct Vector2Int
{
	int x;
	int y;

	Vector2 operator*(const float scalar) const
	{
		return Vector2(x * scalar, y * scalar);
	}
};
