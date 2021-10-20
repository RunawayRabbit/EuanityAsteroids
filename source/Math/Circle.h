#pragma once

#include "Vector2.h"

class Circle
{
public:
	Circle(const Vector2& center, const float& radius)
		: Center(center),
		  Radius(radius)
	{
	}

	bool Overlaps(const Circle& other) const
	{
		const auto diff        = this->Center - other.Center;
		const auto distanceSq  = Dot(diff, diff);
		const auto radiusSumSq =
			(this->Radius + other.Radius) *
			(this->Radius + other.Radius);

		return distanceSq < radiusSumSq;
	}


	//DATA

	Vector2 Center;
	float Radius;
};
