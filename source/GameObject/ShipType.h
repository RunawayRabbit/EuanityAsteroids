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

	float MaxAngularVelocity;
	float RotateAcceleration;
	float RotateDeceleration;

	float MaxSpeedSq;

	static ShipType GetFastAgileShip()
	{
		ShipType ship;
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

		return ship;
	}

	static ShipType GetNormalShip()
	{
		ShipType ship;
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

		return ship;
	}

	static ShipType GetSlowPowerfulShip()
	{
		ShipType ship;
		ship.MainThrusterX   = 0.0f;
		ship.MainThrusterY   = 18.0f;
		ship.StrafeThrusterX = 4.0f;
		ship.StrafeThrusterY = 13.0f;

		ship.MaxSpeed            = 120.0f;
		ship.ForwardAcceleration = ship.MaxSpeed / 0.9f;
		ship.StrafeAcceleration  = ship.MaxSpeed / 1.5f;

		ship.MaxAngularVelocity  = 210.0f;
		ship.RotateAcceleration = ship.MaxAngularVelocity / 0.8f;
		ship.RotateDeceleration = ship.RotateAcceleration * 2.0f;

		ship.MaxSpeedSq = ship.MaxSpeed * ship.MaxSpeed;

		return ship;
	}
};
