#pragma once

#include "../Math/Vector2.h"
#include "../Math/AABB.h"

class Camera
{
public:
	Camera(const AABB startingView, const AABB worldBounds)
		: _WorldBounds(worldBounds),
		  _CurrentView(startingView)
	{
	}

	AABB WorldToCamera(const AABB& rect);


	void Update();

	Vector2 GetFocalPoint();
	AABB GetScreenAABB() const;
	void SetScale();
	void DebugMoveCamera(int x, int y);

private:

	AABB _WorldBounds;
	AABB _CurrentView;
	Vector2 _Velocity;

};
