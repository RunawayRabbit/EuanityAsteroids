#pragma once

#include "Vector2.h"

class Circle
{
public:
	Circle(const Vector2& center, const float& radius) :
		center(center),
		radius(radius) {};

	inline bool Overlaps(const Circle& other)
	{
		Vector2 diff = this->center - other.center;
		float distanceSq = Dot(diff, diff);
		float radiusSumSq = (this->radius + other.radius) *
			(this->radius + other.radius);

		return distanceSq < radiusSumSq;
	}


	//DATA

	Vector2 center;
	float radius;
};