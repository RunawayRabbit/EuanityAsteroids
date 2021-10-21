#pragma once
#include "WeaponType.h"
#include "../Physics/ColliderType.h"
#include "../Renderer/SpriteID.h"

class ShipInfo
{
public:
	enum class ShipType : uint8_t
	{
		FastWeak,
		Normal,
		SlowPowerful,
	};

	ShipType Type;

	ColliderType ColliderType;
	SpriteID SpriteID;

	float MainThrusterX;
	float MainThrusterY;
	float StrafeThrusterX;
	float StrafeThrusterY;

	float MaxSpeed;
	float ForwardAcceleration;
	float StrafeAcceleration;

	float MaxAngularVelocity;
	float RotateAcceleration;
	float RotateDeceleration;

	float MaxSpeedSq;

	int StartingHealth;

	static ShipInfo GetFastAgileShip()
	{
		ShipInfo ship;

		ship.Type = ShipType::FastWeak;

		ship.ColliderType = ColliderType::SHIP_1;
		ship.SpriteID     = SpriteID::SHIP_1;

		ship.MainThrusterX   = 0.0f;
		ship.MainThrusterY   = 18.0f;
		ship.StrafeThrusterX = 4.0f;
		ship.StrafeThrusterY = 13.0f;

		ship.MaxSpeed            = 450.0f;
		ship.ForwardAcceleration = ship.MaxSpeed / 0.3f;
		ship.StrafeAcceleration  = ship.MaxSpeed / 0.8f;

		ship.MaxAngularVelocity = 420.0f;
		ship.RotateAcceleration = ship.MaxAngularVelocity / 0.4f;
		ship.RotateDeceleration = ship.RotateAcceleration * 2.0f;

		ship.MaxSpeedSq = ship.MaxSpeed * ship.MaxSpeed;

		ship.StartingHealth = 3;

		return ship;
	}

	static ShipInfo GetNormalShip()
	{
		ShipInfo ship;

		ship.Type            = ShipType::Normal;

		ship.ColliderType = ColliderType::SHIP_2;
		ship.SpriteID     = SpriteID::SHIP_2;

		ship.MainThrusterX   = 0.0f;
		ship.MainThrusterY   = 18.0f;
		ship.StrafeThrusterX = 4.0f;
		ship.StrafeThrusterY = 13.0f;

		ship.MaxSpeed            = 210.0f;
		ship.ForwardAcceleration = ship.MaxSpeed / 0.6f;
		ship.StrafeAcceleration  = ship.MaxSpeed / 1.5f;

		ship.MaxAngularVelocity = 320.0f;
		ship.RotateAcceleration = ship.MaxAngularVelocity / 0.4f;
		ship.RotateDeceleration = ship.RotateAcceleration * 2.0f;

		ship.MaxSpeedSq = ship.MaxSpeed * ship.MaxSpeed;

		ship.StartingHealth = 5;

		return ship;
	}

	static ShipInfo GetSlowPowerfulShip()
	{
		ShipInfo ship;
		ship.Type            = ShipType::SlowPowerful;

		ship.ColliderType = ColliderType::SHIP_3;
		ship.SpriteID     = SpriteID::SHIP_3;

		ship.MainThrusterX   = 0.0f;
		ship.MainThrusterY   = 18.0f;
		ship.StrafeThrusterX = 4.0f;
		ship.StrafeThrusterY = 13.0f;

		ship.MaxSpeed            = 120.0f;
		ship.ForwardAcceleration = ship.MaxSpeed / 0.9f;
		ship.StrafeAcceleration  = ship.MaxSpeed / 1.5f;

		ship.MaxAngularVelocity = 210.0f;
		ship.RotateAcceleration = ship.MaxAngularVelocity / 0.8f;
		ship.RotateDeceleration = ship.RotateAcceleration * 2.0f;

		ship.MaxSpeedSq = ship.MaxSpeed * ship.MaxSpeed;

		ship.StartingHealth = 10;

		return ship;
	}

	static WeaponType GetDefaultWeaponForShip(const ShipType& ship)
	{
		switch(ship)
		{
			case ShipType::FastWeak:
			{
				return WeaponType::GetSingleFastWeapon();
			}
			case ShipType::Normal:
			{
				return WeaponType::GetMediumWideWeapon();
			}
			case ShipType::SlowPowerful:
			{
				return WeaponType::GetSlowWideWeapon();
			}
			default:
			{
				return WeaponType::GetMediumWideWeapon();
			}
		}
	}

	static ShipInfo GetShip(const ShipType& ship)
	{
		switch(ship)
		{
			case ShipType::FastWeak:
			{
				return GetFastAgileShip();
			}
			case ShipType::Normal:
			{
				return GetNormalShip();
			}
			case ShipType::SlowPowerful:
			{
				return GetSlowPowerfulShip();
			}
			default:
			{
				return GetNormalShip();
			}
		}
	}
};
