#pragma once

#include "../Math/Vector2.h"
#include "../Math/AABB.h"

class Camera
{
public:
	Camera(const AABB startingView, const AABB worldBounds)
		: _GameField(worldBounds),
		  _CurrentView(startingView)
	{
	}

	AABB GetCameraView() const;
	void SetCameraView(const AABB& aabb);

	void OffsetCamera(const Vector2& offset);
	void SetViewSize(const int& width, const int& height);

	Vector2 GetFocalPoint() const;
	void SetFocalPoint(const Vector2& focalPoint);
	void SetFocalPoint(const float& x, const float& y);

	Vector2 WorldToCamera(const Vector2& point) const;
	void SetScale(const float& newScale);
	float GetCameraScale() const;

	void Update();

private:

	AABB _GameField;
	AABB _CurrentView;
	Vector2 _Velocity;

};
