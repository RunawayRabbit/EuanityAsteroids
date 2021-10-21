#pragma once
#include <cstdint>

enum class BulletType : uint8_t
{
	REGULAR_BULLET,
	BOUNCY_BULLET,
	INVISIBLE_BULLET,
};


struct WeaponType
{
	float BulletSpeed;
	float BulletLifetime;
	float ShotCooldown;
	float BulletSpawnOffsetY;
	int BulletSpawnCount;
	float BulletSpawnArcDeg;

	BulletType BulletType;

	static WeaponType GetSingleFastWeapon()
	{
		WeaponType weapon;

		weapon.BulletSpeed        = 600.0f;
		weapon.BulletLifetime     = 1.8f;
		weapon.ShotCooldown       = 0.1f;
		weapon.BulletSpawnOffsetY = 12.0f;
		weapon.BulletSpawnCount   = 2;
		weapon.BulletSpawnArcDeg  = 1.5f;

		weapon.BulletType = BulletType::REGULAR_BULLET;

		return weapon;
	}

	static WeaponType GetMediumWideWeapon()
	{
		WeaponType weapon;

		weapon.BulletSpeed        = 400.0f;
		weapon.BulletLifetime     = 2.3f;
		weapon.ShotCooldown       = 0.3f;
		weapon.BulletSpawnOffsetY = 11.0f;
		weapon.BulletSpawnCount   = 4;
		weapon.BulletSpawnArcDeg  = 18.0f;

		weapon.BulletType = BulletType::REGULAR_BULLET;

		return weapon;
	}

	static WeaponType GetSlowWideWeapon()
	{
		WeaponType weapon;

		weapon.BulletSpeed        = 200.0f;
		weapon.BulletLifetime     = 4.8f;
		weapon.ShotCooldown       = 0.6f;
		weapon.BulletSpawnOffsetY = 9.0f;
		weapon.BulletSpawnCount   = 8;
		weapon.BulletSpawnArcDeg  = 15.0f;

		weapon.BulletType = BulletType::REGULAR_BULLET;

		return weapon;
	}

	static WeaponType GetBouncyWeapon()
	{
		WeaponType weapon;

		weapon.BulletSpeed        = 400.0f;
		weapon.BulletLifetime     = 2.3f;
		weapon.ShotCooldown       = 0.4f;
		weapon.BulletSpawnOffsetY = 11.0f;
		weapon.BulletSpawnCount   = 1;
		weapon.BulletSpawnArcDeg  = 0.0f;

		weapon.BulletType = BulletType::BOUNCY_BULLET;

		return weapon;
	}

	static WeaponType GetOverpoweredTestWeapon()
	{
		WeaponType weapon;

		weapon.BulletSpeed        = 450.0f;
		weapon.BulletLifetime     = 1.9f;
		weapon.ShotCooldown       = 0.2f;
		weapon.BulletSpawnOffsetY = 11.0f;
		weapon.BulletSpawnCount   = 9;
		weapon.BulletSpawnArcDeg  = 25.0f;

		weapon.BulletType = BulletType::BOUNCY_BULLET;

		return weapon;
	}

	static WeaponType GetChaosWeapon()
	{
		WeaponType weapon;

		weapon.BulletSpeed        = 220.0f;
		weapon.BulletLifetime     = 3.7f;
		weapon.ShotCooldown       = 2.0f;
		weapon.BulletSpawnOffsetY = 11.0f;
		weapon.BulletSpawnCount   = 500;
		weapon.BulletSpawnArcDeg  = 360.0f;

		weapon.BulletType = BulletType::REGULAR_BULLET;

		return weapon;
	}
};
