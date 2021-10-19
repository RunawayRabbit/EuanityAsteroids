#include "Camera.h"

#include <iostream>

#include "../Math/EuanityMath.h"
#include "../Platform/Game.h"

AABB
Camera::GetCameraView() const
{
	return _CurrentView;
}

Vector2
Camera::GetFocalPoint() const
{
	return (_CurrentView.min + _CurrentView.max) * 0.5f;
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

	_TargetView = AABB(focalPoint - (_WindowDim * invScale),
	                   focalPoint + (_WindowDim * invScale));
}


float
Camera::GetCameraScale() const
{
	//@NOTE: Assuming that we are never squash/stretching the camera, because we really shouldn't ever do that.
	const auto cameraDimX = _CurrentView.max.x - _CurrentView.min.x;

	return _WindowDim.x / cameraDimX;
}

void
Camera::Update(const float& deltaTime)
{
	if(!_Game->GameFieldContains(_TargetView.min))
	{
		const auto newMin = _Game->WrapToGameField(_TargetView.min);
		const auto delta  = newMin - _TargetView.min;
		const auto newMax = _TargetView.max + delta;

		_TargetView = AABB(newMin, newMax);
	}

	// Reconstruct CurrentView from deltas
	_CurrentView = _TargetView + _CurrentOffsetFromTarget;

	_CurrentView = Math::SmoothDamp(
		_CurrentView, _TargetView, _Velocity,
		MAX_CAMERA_SPEED, INTERP_SPEED, deltaTime);

	// Store deltas for next frame
	_CurrentOffsetFromTarget = _CurrentView - _TargetView;
}

void
Camera::SetFocalPoint(const Vector2& focalPoint)
{
	const auto dim     = (_TargetView.max - _TargetView.min);
	const auto halfDim = dim * 0.5f;

	const auto newMin = _Game->WrapToGameField((focalPoint - halfDim));

	_TargetView = AABB(newMin, (newMin + dim));
}

void
Camera::SetFocalPoint(const float& x, const float& y)
{
	SetFocalPoint(Vector2(x, y));
}
