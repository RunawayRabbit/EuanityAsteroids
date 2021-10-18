#pragma once

#include "../Math/Vector2.h"
#include "../Math/AABB.h"
#include "../Math/Vector2Int.h"

class Camera
{
public:
	explicit Camera(const int& windowWidth, const int& windowHeight)
		: _WindowDim({windowWidth, windowHeight}),
		  _CurrentView(AABB(static_cast<float>(windowWidth), static_cast<float>(windowHeight)))
	{
	}

	AABB GetCameraView() const;
	void SetCameraView(const AABB& aabb);

	void OffsetCamera(const Vector2& offset);

	Vector2 GetFocalPoint() const;
	void SetFocalPoint(const Vector2& focalPoint);
	void SetFocalPoint(const float& x, const float& y);

	Vector2 WorldToCamera(const Vector2& point) const;
	void SetScale(const float& newScale);
	float GetCameraScale() const;

	void Update();

private:

	const Vector2Int _WindowDim;
	AABB _CurrentView;
	Vector2 _Velocity;

};
