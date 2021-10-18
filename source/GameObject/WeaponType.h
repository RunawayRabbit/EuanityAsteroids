#pragma once

enum class BulletType: uint8_t
{
	REGULAR_BULLET,
	BOUNCY_BULLET,
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

		weapon.BulletSpeed        = 500.0f;
		weapon.BulletLifetime     = 1.2f;
		weapon.ShotCooldown       = 0.1f;
		weapon.BulletSpawnOffsetY = 11.0f;
		weapon.BulletSpawnCount   = 1;
		weapon.BulletSpawnArcDeg  = 0.0f;

		weapon.BulletType = BulletType::REGULAR_BULLET;

		return weapon;
	}

	static WeaponType GetMediumWideWeapon()
	{
		WeaponType weapon;

		weapon.BulletSpeed        = 400.0f;
		weapon.BulletLifetime     = 2.3f;
		weapon.ShotCooldown       = 0.4f;
		weapon.BulletSpawnOffsetY = 11.0f;
		weapon.BulletSpawnCount   = 3;
		weapon.BulletSpawnArcDeg  = 15.0f;

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
		weapon.BulletSpawnCount   = 6;
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
		weapon.BulletSpawnArcDeg  = 15.0f;

		weapon.BulletType = BulletType::BOUNCY_BULLET;

		return weapon;
	}
};
