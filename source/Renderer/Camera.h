#pragma once

#include "../Math/Vector2.h"
#include "../Math/AABB.h"
#include "../Math/Vector2Int.h"

class Game;

class Camera
{
public:
	explicit Camera(const Game* game, const int& windowWidth, const int& windowHeight)
		: _WindowDim({ windowWidth, windowHeight }),
		  _CurrentOffsetFromTarget(0, 0, 0, 0),
		  _Velocity(AABB(0, 0, 0, 0)),
		  _TargetView(AABB(static_cast<float>(windowWidth), static_cast<float>(windowHeight))),
		  _CurrentView(AABB(static_cast<float>(windowWidth), static_cast<float>(windowHeight))),
		  _Game(game)
	{
	}

	AABB GetCameraView() const;

	Vector2 GetFocalPoint() const;
	;
	void SetFocalPoint(const Vector2& focalPoint);
	void SetFocalPoint(const float& x, const float& y);

	Vector2 WorldToCamera(const Vector2& point) const;
	void SetScale(const float& newScale);
	float GetCameraScale() const;

	void Update(const float& deltaTime);

private:

	const Vector2Int _WindowDim;
	AABB _CurrentOffsetFromTarget;
	AABB _Velocity;
	AABB _TargetView;
	AABB _CurrentView;
	const Game* _Game;

	inline static const float MAX_CAMERA_SPEED = 600.0f;
	inline static const float INTERP_SPEED = 3.9f;
};
