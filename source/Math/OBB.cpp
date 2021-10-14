
#include "AABB.h"
#include "OBB.h"

#include "Vector2.h"


OBB::OBB(const SDL_Rect& rect, const float& rotationDeg)
{
	this->center = Vector2((float)rect.x + (rect.w / 2.0f), (float)rect.y + (rect.h / 2.0f));
	this->basisX = Vector2::Right().RotateDeg(rotationDeg);
	this->basisY = Vector2::Forward().RotateDeg(rotationDeg);
	this->extents.x = (float)rect.w / 2.0f;
	this->extents.y = (float)rect.h / 2.0f;
}

OBB::OBB(const Vector2& center, const Vector2& extents, const float& rotationDeg)
{
	this->center = center;
	this->basisX = Vector2::Right().RotateDeg(rotationDeg);
	this->basisY = Vector2::Forward().RotateDeg(rotationDeg);
	this->extents.x = extents.x;
	this->extents.y = extents.y;
}

OBB::OBB(const Vector2& center, const float& extents, const float& rotationDeg)
{
	this->center = center;
	this->basisX = Vector2{ 1.0f, 0.0f }.RotateDeg(rotationDeg);
	this->basisY = Vector2{ 0.0f, 1.0f }.RotateDeg(rotationDeg);
	this->extents.x = extents;
	this->extents.y = extents;
}


AABB OBB::Bounds() const
{
	// @TODO: Very likely simd-able
	AABB result = AABB(center, center);
	for (auto& corner : GetCorners())
	{
		result.min.x = corner.x < result.min.x ? corner.x : result.min.x;
		result.min.y = corner.y < result.min.y ? corner.y : result.min.y;
		result.max.x = corner.x > result.max.x ? corner.x : result.max.x;
		result.max.y = corner.y > result.max.y ? corner.y : result.max.y;
	}
	return result;
}

Vector2 OBB::ClosestPointTo(const Vector2& point) const
{
	Vector2 distToCenter = point - this->center;

	// Position in "box space"
	float localX = Dot(distToCenter, this->basisX);
	float localY = Dot(distToCenter, this->basisY);

	// Clamp to the range of -extents to extents
	if (localX < -extents.x)
		localX = -extents.x;
	if (localX > extents.x)
		localX = extents.x;

	if (localY < -extents.y)
		localY = -extents.y;
	if (localY > extents.y)
		localY = extents.y;

	// Offset by our clamped ranges along X and Y
	return this->center + (this->basisX * localX) + (this->basisY * localY);
}

float OBB::DistanceBetweenSq(const Vector2& point) const
{
	Vector2 closestPoint = this->ClosestPointTo(point);
	return Dot(closestPoint - point, closestPoint - point);
}

float OBB::DistanceBetween(const Vector2& point) const
{
	return sqrt(this->DistanceBetweenSq(point));
}

std::array<Vector2, 4> OBB::GetCorners() const
{
	Vector2 basisXExtents = basisX * extents.x;
	Vector2 basisYExtents = basisY * extents.y;
	return {
		(center - basisXExtents - basisYExtents),
		(center - basisXExtents + basisYExtents),
		(center + basisXExtents - basisYExtents),
		(center + basisXExtents + basisYExtents),
	};
}