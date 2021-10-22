#pragma once

#include "../Math/Vector2.h"
#include "../Math/AABB.h"
#include "../Math/Vector2Int.h"

class Game;

class Camera
{
public:
	explicit Camera(const Game* game, const int& windowWidth, const int& windowHeight);

	AABB GetCameraView() const;

	Vector2 GetFocalPoint() const;
	Vector2 GetCameraVelocity() const;

	void SetCameraView(const AABB& view);

	void SetFocalPoint(const Vector2& focalPoint);
	void SetFocalPoint(const float& x, const float& y);

	Vector2 WorldToCamera(const Vector2& point) const;
	void SetScale(const float& newScale);
	float GetCameraScale() const;

	void Update(const float& deltaTime);

private:

	const Game* _Game;

	const Vector2Int _WindowDim;

	AABB _Velocity;

	AABB _TargetView;
	AABB _PreviousTargetView;
	AABB _CurrentView;

	inline static const float MAX_CAMERA_SPEED = 800.0f;
	inline static const float INTERP_SPEED = 0.8f;
};
