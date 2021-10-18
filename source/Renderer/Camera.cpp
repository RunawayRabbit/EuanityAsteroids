#include "Camera.h"

#include "../Math/EuanityMath.h"

AABB
Camera::GetCameraView() const
{
	return _CurrentView;
}

void
Camera::SetCameraView(const AABB& aabb)
{
	_CurrentView = aabb;
}

void
Camera::OffsetCamera(const Vector2& offset)
{
	_CurrentView.min += offset;
	_CurrentView.max += offset;
}


Vector2
Camera::GetFocalPoint() const
{
	return (_CurrentView.min + _CurrentView.max) * 0.5f;
}

void
Camera::SetFocalPoint(const float& x, const float& y)
{
	SetFocalPoint(Vector2(x, y));
}

Vector2
Camera::WorldToCamera(const Vector2& point) const
{
	const auto x = Math::Remap(point.x, _CurrentView.left, _CurrentView.right,
	                           0, static_cast<float>(_WindowDim.x));
	const auto y = Math::Remap(point.y, _CurrentView.top, _CurrentView.bottom,
	                           0, static_cast<float>(_WindowDim.y));

	return Vector2(x, y);
}

void
Camera::SetScale(const float& newScale)
{
	const auto invScale   = 1.0f / newScale * 0.5f;
	const auto focalPoint = GetFocalPoint();

	_CurrentView.min = focalPoint - (_WindowDim * invScale);
	_CurrentView.max = focalPoint + (_WindowDim * invScale);
}

float
Camera::GetCameraScale() const
{
	//@NOTE: Assuming that we are never squash/stretching the camera, because we really shouldn't ever do that.
	const auto cameraDimX = _CurrentView.max.x - _CurrentView.min.x;

	return _WindowDim.x / cameraDimX;
}

void
Camera::SetFocalPoint(const Vector2& focalPoint)
{
	const auto halfDim = (_CurrentView.max - _CurrentView.min) * 0.5f;;

	_CurrentView.min = focalPoint - halfDim;
	_CurrentView.max = focalPoint + halfDim;
}
