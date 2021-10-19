#pragma once

enum class InputOneShot
{
	NONE = 0,

	MouseUp,
	MouseDown,

	START_DEBUG,

    DEBUG_Camera
};

enum class InputToggle
{
	NONE = 0,

	MoveForward,
	RotateLeft,
	RotateRight,

	StrafeLeft,
	StrafeRight,

	Shoot,

	START_DEBUG,
};
