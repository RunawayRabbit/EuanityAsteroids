#pragma once

#include <cmath> // for fmod
#include <algorithm> // for clamp
#include <random> // for mersenne twister

#include "Vector2.h"

#include "AABB.h"
#include "OBB.h"
#include "Circle.h"

#include "MathConstants.h"

namespace Math
{
	// Angle Deltas

	static float AngleDelta(float a, float b)
	{
		return fmod((b - a + PI), TAU) - PI;
	}
	static float AngleDeltaDeg(float a, float b)
	{
		return fmod((b - a + 180.0f), 360.0f) - 180.0f;
	}

	// Angle Lerps

	static float LerpAngle(float a, float b, float t)
	{
		float delta = AngleDelta(a, b);
		if (delta > PI) delta -= TAU;

		return a + delta * t;
	}
	static float LerpAngleDeg(float a, float b, float t)
	{
		float delta = AngleDeltaDeg(a, b);
		if (delta > 180.0f) delta -= 360.0f;

		return a + delta * t;
	}

	// Clamped Angle Lerps

	static float LerpAngleClamped(float a, float b, float t)
	{
		float delta = AngleDelta(a, b);
		if (delta > PI) delta -= TAU;

		return a + delta * std::clamp(t, 0.0f, 1.0f);
	}
	static float LerpAngleDegClamped(float a, float b, float t)
	{
		float delta = AngleDeltaDeg(a, b);
		if (delta > 180.0f) delta -= 360.0f;

		return a + delta * std::clamp(t, 0.0f, 1.0f);
	}

	// RNG

	static float RandomRange(float min, float max)
	{
		static std::random_device device;
		static std::mt19937 generator(device());
		std::uniform_real<float> range(min, max);
		return range(generator);
	}

	static int RandomRange(int min, int max)
	{
		static std::random_device device;
		static std::mt19937 generator(device());
		std::uniform_int range(min, max);
		return range(generator);
	}

	static Vector2 RandomOnUnitCircle()
	{
		return Vector2::Forward().RotateRad(RandomRange(0.0f, TAU));
	}


	// Random Numbery Stuff
	static float Repeat(float t, float length)
	{
		return std::clamp(t - floor(t / length) * length, 0.0f, length);
	}

	static float SignOf(float t)
	{
		return (t >= 0.0f) ? 1.0f : -1.0f;
	}

	static float MoveTowards(float current, float target, float maxDelta)
	{
		if (std::abs(target - current) <= maxDelta)
			return target;
		return current + Math::SignOf(target - current) * maxDelta;
	}
}
