#pragma once

#include <cmath> // sqrt
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

	Vector2 operator*(const float& b) const
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
	void operator+=(const Vector2& b)
	{
		this->x = this->x + b.x;
		this->y = this->y + b.y;
	}

	Vector2 operator-() const
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

	bool operator==(const Vector2& b) const
	{
		return ( x == b.x ) && (y == b.y);
	}
	bool operator!=(const Vector2& b) const
	{
		return !(*this==b);
	}

	// Rotation
	Vector2 RotateRad(const float& radians) const
	{
		Vector2 ret;
		ret.x = (x * cos(radians) - y * sin(radians));
		ret.y = (x * sin(radians) + y * cos(radians));

		return ret;
	}
	Vector2 RotateDeg(const float& degrees) const
	{
		Vector2 ret;
		ret.x = (x * cos(degrees * Math::DEG2RAD) - y * sin(degrees * Math::DEG2RAD));
		ret.y = (x * sin(degrees * Math::DEG2RAD) + y * cos(degrees * Math::DEG2RAD));

		return ret;
	}

	Vector2 Rot90CCW() const
	{
		return Vector2{ y, -x };
	}
	Vector2 Rot90CW() const
	{
		return Vector2{ -y, x };
	}
	Vector2 Rot180() const
	{
		return -*this;
	}

	inline Vector2 Normalized() const;
	inline Vector2 SafeNormalized() const;
	inline float Length() const;
	inline float LengthSq() const;
	float GetAngleRadFromVector() const
	{
		return atan2(y,x);
	}
	float GetAngleDegFromVector() const
	{
		return GetAngleRadFromVector() * Math::RAD2DEG;
	}

	static Vector2 Zero() { return { 0.0f, 0.0f }; }
	static Vector2 One() { return { 1.0f, 1.0f }; }

	static Vector2 Right() { return { 1.0f, 0.0f }; }
	static Vector2 Forward() { return { 0.0f, 1.0f }; }

};

inline float Dot(const Vector2& a, const Vector2& b)
{
	return a.x * b.x
		+ a.y * b.y;
}

inline Vector2 Vector2::Normalized() const
{
	return *this * (1.0f / sqrt(Dot(*this, *this)));
}

inline Vector2 Vector2::SafeNormalized() const
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
