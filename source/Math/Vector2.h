#pragma once

#include <math.h> // sqrt
#include <ostream>

#include "MathConstants.h"

class Vector2
{
public:
	Vector2(const float& x, const float& y)
	{
		this->x = x;
		this->y = y;
	};

	Vector2()
	{
		x = 0.0f;
		y = 0.0f;
	};

	float x;
	float y;

	// ARITHMETIC OVERLOADS

	inline Vector2 operator*(const float& b) const
	{
		return { x * b, y * b };
	}

	Vector2 operator+(const Vector2& b) const
	{
		Vector2 ret;
		ret.x = x + b.x;
		ret.y = y + b.y;

		return ret;
	}
	inline void operator+=(const Vector2& b)
	{
		this->x = this->x + b.x;
		this->y = this->y + b.y;
	}

	inline Vector2 operator-() const
	{
		Vector2 ret;
		ret.x = -x;
		ret.y = -y;

		return ret;
	}

	Vector2 operator-(const Vector2& b) const
	{
		return (*this) + (-b);
	}

	// Rotation
	inline Vector2 RotateRad(const float& radians) const
	{
		Vector2 ret;
		ret.x = (float)(x * cos(radians) - y * sin(radians));
		ret.y = (float)(x * sin(radians) + y * cos(radians));

		return ret;
	}
	inline Vector2 RotateDeg(const float& degrees) const
	{
		Vector2 ret;
		ret.x = (float)(x * cos(degrees * Math::DEG2RAD) - y * sin(degrees * Math::DEG2RAD));
		ret.y = (float)(x * sin(degrees * Math::DEG2RAD) + y * cos(degrees * Math::DEG2RAD));

		return ret;
	}

	inline Vector2 Rot90CCW() const
	{
		return Vector2{ y, -x };
	}
	inline Vector2 Rot90CW() const
	{
		return Vector2{ -y, x };
	}
	inline Vector2 Rot180() const
	{
		return -*this;
	}

	inline Vector2 normalized();
	inline Vector2 SafeNormalized();
	inline float Length() const;
	inline float LengthSq() const;

	inline static Vector2 zero() { return { 0.0f, 0.0f }; }

	inline static Vector2 Right() { return { 1.0f, 0.0f }; }
	inline static Vector2 Forward() { return { 0.0f, 1.0f }; }

};

inline float Dot(const Vector2& a, const Vector2& b)
{
	return a.x * b.x
		+ a.y * b.y;
}

inline Vector2 Vector2::normalized()
{
	return *this * (1.0f / sqrt(Dot(*this, *this)));
}

inline Vector2 Vector2::SafeNormalized()
{
	float dot = Dot(*this, *this);
	if (dot == 0.0f)
		return *this;
	else
		return *this * (1.0f / sqrt(dot));
}

inline std::ostream& operator<< (std::ostream& out, const Vector2& v)
{
	out << '(' << v.x << ',' << v.y << ')';
	return out;
}



inline float Vector2::LengthSq() const
{
	return Dot(*this, *this);
}

inline float Vector2::Length() const
{
	return sqrt(this->LengthSq());
}