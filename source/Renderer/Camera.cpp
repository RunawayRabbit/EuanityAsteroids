#include "Camera.h"

AABB
Camera::GetScreenAABB() const
{
	return _CurrentView;
}
