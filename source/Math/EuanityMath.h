#pragma once

#include <cmath> // for fmod
#include <algorithm> // for clamp
#include <cassert>
#include <random> // for mersenne twister

#include "Vector2.h"

#include "AABB.h"
#include "OBB.h"
#include "Circle.h"

#include "MathConstants.h"

namespace Math
{
// Angle Deltas

static float
AngleDelta(float a, float b)
{
	return fmod((b - a + PI), TAU) - PI;
}

static float
AngleDeltaDeg(float a, float b)
{
	return fmod((b - a + 180.0f), 360.0f) - 180.0f;
}

// Angle Lerps

static float
LerpAngle(float a, float b, float t)
{
	float delta = AngleDelta(a, b);
	if(delta > PI)
		delta -= TAU;

	return a + delta * t;
}

static float
LerpAngleDeg(float a, float b, float t)
{
	float delta = AngleDeltaDeg(a, b);
	if(delta > 180.0f)
		delta -= 360.0f;

	return a + delta * t;
}

// Clamped Angle Lerps

static float
LerpAngleClamped(float a, float b, float t)
{
	float delta = AngleDelta(a, b);
	if(delta > PI)
		delta -= TAU;

	return a + delta * std::clamp(t, 0.0f, 1.0f);
}

static float
LerpAngleDegClamped(float a, float b, float t)
{
	float delta = AngleDeltaDeg(a, b);
	if(delta > 180.0f)
		delta -= 360.0f;

	return a + delta * std::clamp(t, 0.0f, 1.0f);
}

// RNG

static float
RandomRange(float min, float max)
{
	static std::random_device device;
	static std::mt19937 generator(device());
	std::uniform_real<float> range(min, max);
	return range(generator);
}

static int
RandomRange(int min, int max)
{
	static std::random_device device;
	static std::mt19937 generator(device());
	std::uniform_int range(min, max);
	return range(generator);
}

static Vector2
RandomOnUnitCircle()
{
	return Vector2::Forward().RotateRad(RandomRange(0.0f, TAU));
}


// Random Numbery Stuff
static float
Repeat(float t, float length)
{
	return std::clamp(t - floor(t / length) * length, 0.0f, length);
}

static float
RepeatNeg(float t, float length)
{
	while(t < 0)
		t += length;
	return std::clamp(t - floor(t / length) * length, 0.0f, length);
}

static float
SignOf(float t)
{
	return (t >= 0.0f) ? 1.0f : -1.0f;
}

static float
MoveTowards(float current, float target, float maxDelta)
{
	if(std::abs(target - current) <= maxDelta)
		return target;
	return current + Math::SignOf(target - current) * maxDelta;
}

static float
Lerp(const float& t, const float& a, const float& b)
{
	return (1.0f - t) * a + b * t;
}

static float
InvLerp(const float& t, const float& a, const float& b)
{
	return (t - a) / (b - a);
}


static float
Remap(const float& t,
      const float& inputMin,
      const float& inputMax,
      const float& outputMin,
      const float& outputMax)
{
	const auto inputT = InvLerp(t, inputMin, inputMax);
	return Lerp(inputT, outputMin, outputMax);
}

static float
RemapClamped(const float& t,
             const float& inputMin,
             const float& inputMax,
             const float& outputMin,
             const float& outputMax)
{
	const auto min = outputMin < outputMax ? outputMin : outputMax;
	const auto max = outputMin > outputMax ? outputMin : outputMax;
	return std::clamp(Remap(t, inputMin, inputMax,
	                        outputMin, outputMax),
	                  min, max);
}

static float
SmoothDamp(const float from,
           const float to,
           float& velocity,
           const float maxSpeed,
           const float smoothTime,
           const float deltaTime)
{
	// Ripped shamelessly from Game Programming Gems 4, credit to Noel Llopis.
	assert(smoothTime > 0.001f);

	const auto omega    = 2.0f / smoothTime;
	const auto x        = omega * deltaTime;
	const auto exp      = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
	const auto maxDelta = maxSpeed * smoothTime;

	const auto delta = std::clamp(from - to, -maxDelta, maxDelta);
	const auto temp  = (velocity + omega * delta) * deltaTime;

	auto result = from - delta + (delta + temp) * exp;
	velocity    = (velocity - omega * temp) * exp;

	// Prevent overshooting
	if(to - from > 0.0f == result > to)
	{
		result   = to;
		velocity = 0.0f;
	}

	return result;
}

static Vector2
SmoothDamp(const Vector2& from,
           const Vector2& to,
           Vector2& velocity,
           const float& maxSpeed,
           const float& smoothTime,
           const float& deltaTime)
{
	return Vector2(
		SmoothDamp(from.x, to.x, velocity.x, maxSpeed, smoothTime, deltaTime),
		SmoothDamp(from.y, to.y, velocity.y, maxSpeed, smoothTime, deltaTime));
}

static AABB
SmoothDamp(const AABB& from,
           const AABB& to,
           AABB& velocity,
           const float& maxSpeed,
           const float& smoothTime,
           const float& deltaTime)
{
	return AABB(SmoothDamp(from.min, to.min, velocity.min, maxSpeed, smoothTime, deltaTime),
	            SmoothDamp(from.max, to.max, velocity.max, maxSpeed, smoothTime, deltaTime));
}

inline int
Mod(const int num, const int divisor)
{
	return (divisor + (num % divisor)) % divisor;
}
}
