#pragma once

struct ShipType
{
	float MainThrusterX;
	float MainThrusterY;
	float StrafeThrusterX;
	float StrafeThrusterY;

	float MaxSpeed;
	float ForwardAcceleration;
	float StrafeAcceleration;

	float BulletSpeed;
	float BulletLifetime;
	float ShotCooldown;
	float BulletSpawnOffsetY;
	int BulletSpawnCount;
	float BulletSpawnArcDeg;

	float MaxAngularVelocity;
	float RotateAcceleration;
	float RotateDeceleration;

	float MaxSpeedSq;

	static ShipType GetFastAgileShip()
	{
		ShipType ship;
		ship.MainThrusterX       = 0.0f;
		ship.MainThrusterY       = 18.0f;
		ship.StrafeThrusterX     = 4.0f;
		ship.StrafeThrusterY     = 13.0f;
		ship.MaxSpeed            = 450.0f;
		ship.ForwardAcceleration = ship.MaxSpeed / 0.3f;
		ship.StrafeAcceleration  = ship.MaxSpeed / 0.8f;
		ship.BulletSpeed         = 500.0f;
		ship.BulletLifetime      = 1.2f;
		ship.ShotCooldown        = 0.1f;
		ship.BulletSpawnOffsetY  = 11.0f;
		ship.BulletSpawnCount    = 1;
		ship.BulletSpawnArcDeg   = 0.0f;
		ship.MaxAngularVelocity  = 420.0f;
		ship.RotateAcceleration  = ship.MaxAngularVelocity / 0.4f;
		ship.RotateDeceleration  = ship.RotateAcceleration * 2.0f;

		ship.MaxSpeedSq = ship.MaxSpeed * ship.MaxSpeed;

		return ship;
	}

	static ShipType GetNormalShip()
	{
		ShipType ship;
		ship.MainThrusterX       = 0.0f;
		ship.MainThrusterY       = 18.0f;
		ship.StrafeThrusterX     = 4.0f;
		ship.StrafeThrusterY     = 13.0f;
		ship.MaxSpeed            = 210.0f;
		ship.ForwardAcceleration = ship.MaxSpeed / 0.6f;
		ship.StrafeAcceleration  = ship.MaxSpeed / 1.5f;
		ship.BulletSpeed         = 400.0f;
		ship.BulletLifetime      = 2.3f;
		ship.ShotCooldown        = 0.4f;
		ship.BulletSpawnOffsetY  = 11.0f;
		ship.BulletSpawnCount    = 3;
		ship.BulletSpawnArcDeg   = 15.0f;
		ship.MaxAngularVelocity  = 320.0f;
		ship.RotateAcceleration  = ship.MaxAngularVelocity / 0.4f;
		ship.RotateDeceleration  = ship.RotateAcceleration * 2.0f;

		ship.MaxSpeedSq = ship.MaxSpeed * ship.MaxSpeed;

		return ship;
	}

	static ShipType GetSlowPowerfulShip()
	{
		ShipType ship;
		ship.MainThrusterX       = 0.0f;
		ship.MainThrusterY       = 18.0f;
		ship.StrafeThrusterX     = 4.0f;
		ship.StrafeThrusterY     = 13.0f;
		ship.MaxSpeed            = 120.0f;
		ship.ForwardAcceleration = ship.MaxSpeed / 0.9f;
		ship.StrafeAcceleration  = ship.MaxSpeed / 1.5f;
		ship.BulletSpeed         = 200.0f;
		ship.BulletLifetime      = 4.8f;
		ship.ShotCooldown        = 0.6f;
		ship.BulletSpawnOffsetY  = 9.0f;
		ship.BulletSpawnCount    = 6;
		ship.BulletSpawnArcDeg   = 15.0f;
		ship.MaxAngularVelocity  = 210.0f;
		ship.RotateAcceleration  = ship.MaxAngularVelocity / 0.8f;
		ship.RotateDeceleration  = ship.RotateAcceleration * 2.0f;

		ship.MaxSpeedSq = ship.MaxSpeed * ship.MaxSpeed;

		return ship;
	}
};
