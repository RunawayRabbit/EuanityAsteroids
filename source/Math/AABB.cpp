#include "AABB.h"
#include "OBB.h"
#include "Circle.h"

AABB::AABB(const float& top, const float& bottom, const float& left, const float& right)
{
	this->top    = top;
	this->bottom = bottom;
	this->left   = left;
	this->right  = right;
}

AABB::AABB(const Vector2& min, const Vector2& max)
{
	this->min = min;
	this->max = max;
}

AABB::AABB(const float& width, const float& height)
{
	this->top    = 0;
	this->bottom = height;
	this->left   = 0;
	this->right  = width;
}

bool
AABB::Contains(const Vector2& point) const
{
	return point.x > left && point.x < right &&
		point.y > top && point.y < bottom;
}

bool
AABB::FullyContains(const OBB& oriented) const
{
	for(Vector2& corner : oriented.GetCorners())
	{
		if(!this->Contains(corner))
			return false;
	}
	return true;
}

AABB
AABB::operator+(const AABB& other) const
{
	return AABB(
		this->top + other.top,
		this->bottom + other.bottom,
		this->left + other.left,
		this->right + other.right);
}

AABB
AABB::operator-(const AABB& other) const
{
	return AABB(
    this->top - other.top,
    this->bottom - other.bottom,
    this->left - other.left,
    this->right - other.right);
}
