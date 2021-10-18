#include <algorithm> // min

#include "../ECS/EntityManager.h"
#include "../ECS/RigidbodyManager.h"
#include "../ECS/TransformManager.h"

#include "../Physics/Physics.h"

#include "../Input/InputBuffer.h"

#include "../GameObject/Create.h"

#include "../Math/Math.h"

#include "Player.h"

#include "../Math/Vector2Int.h"


Player::Player(EntityManager& entityManager,
               RigidbodyManager& rigidbodyManager,
               TransformManager& transformManager,
               const Create& create,
               Physics& physics)
	: _RigidbodyManager(rigidbodyManager),
	  _Physics(physics),
	  _EntityManager(entityManager),
	  _TransformManager(transformManager),
	  _Create(create),
	  _Entity(Entity::Null()),
	  _MainThruster(Entity::Null()),
	  _StrafeThrusterLeft(Entity::Null()),
	  _StrafeThrusterRight(Entity::Null()),
	  _ShotTimer(0),
	  _ShipType(ShipType::GetSlowPowerfulShip())
{
	//@NOTE: We specifically set up the player code in such a way that there IS no player until we call Spawn. We do, however,
	// have all of our initialization done at startup time.
}

void
Player::Spawn(const Vector2& startPos, const float& startRot)
{
	if(IsAlive())
		return;

	_Entity = _Create.Ship(startPos, startRot);
}

void
Player::Kill(const Entity& playerEntity, const Vector2& playerVelocity)
{
	// Don't destroy if it isn't our entity being destroyed!
	if(_Entity != playerEntity)
		return;

	auto ship    = _TransformManager.Get(_Entity);
	auto shipPos = ship.value().pos;
	DestroyThruster(_MainThruster);
	DestroyThruster(_StrafeThrusterLeft);
	DestroyThruster(_StrafeThrusterRight);

	_EntityManager.Destroy(_Entity);


	// ReSharper disable CppExpressionWithoutSideEffects
	for(auto i = 0; i < 3; ++i)
	{
		auto spawnDistanceFromPlayer = Math::RandomRange(10.0f, 17.0f);

		const auto offset = Vector2::Forward().RotateRad(Math::RandomRange(0.0f, Math::TAU)) * spawnDistanceFromPlayer;
		_Create.LargeExplosion(shipPos + offset);
		_Create.LargeExplosion(shipPos + offset);
		_Create.LargeExplosion(shipPos + offset);
		_Create.LargeExplosion(shipPos + offset);
	}

	for(auto i = 0; i < 12; ++i)
	{
		const auto randomVelocity = Vector2::Forward().RotateRad(Math::RandomRange(0.0f, Math::TAU)) * Math::RandomRange(10.0f, 140.0f);
		_Create.SmallExplosion(shipPos, (playerVelocity * 0.1f) + randomVelocity);
	}

	for(auto i = 0; i < 12; ++i)
	{
		const auto randomVelocity = Vector2::Forward().RotateRad(Math::RandomRange(0.0f, Math::TAU)) * Math::RandomRange(60.0f, 240.0f);
		_Create.TinyExplosion(shipPos, (playerVelocity * 0.1f) + randomVelocity);
	}
	// ReSharper restore CppExpressionWithoutSideEffects

	_Entity = Entity::Null();
}

void
Player::Update(const InputBuffer& inputBuffer, const float& deltaTime)
{
	// Early-out if the player's ship isn't currently spawned.
	if(!IsAlive())
		return;

	// Increment shot cooldown
	_ShotTimer -= deltaTime;

	// Get the rb and the transform from the respective managers
	Rigidbody* rigid;
	auto optionalTransform = _TransformManager.Get(_Entity);
	if(!optionalTransform.has_value() || !_RigidbodyManager.GetMutable(_Entity, rigid))
	{
		// If we end up here, a serious bug has occured.
		__assume(false);
	}

	const auto transform = optionalTransform.value();

	auto newVelocity        = rigid->velocity;
	auto newAngularVelocity = rigid->angularVelocity;

	// Set up some helper vectors.
	const auto forward = -Vector2::Forward().RotateDeg(transform.rot); // SDL has is "negative Y is up". Which is dumb and I hate it.
	const auto right   = Vector2::Right().RotateDeg(transform.rot);

	auto trailRotation = 0.0f;

#pragma region Rotate
	{
		const auto rotatingLeft  = inputBuffer.Contains(InputToggle::RotateLeft);
		const auto rotatingRight = inputBuffer.Contains(InputToggle::RotateRight);
		if(!rotatingLeft && !rotatingRight)
		{
			newAngularVelocity = Math::MoveTowards(newAngularVelocity, 0, _ShipType.RotateAcceleration * deltaTime);
		}
		else
		{
			// Apply torque
			if(rotatingLeft)
				trailRotation = -_ShipType.RotateAcceleration * deltaTime;
			if(rotatingRight)
				trailRotation = _ShipType.RotateAcceleration * deltaTime;

			newAngularVelocity += trailRotation;
		}
	}
#pragma endregion

#pragma region Strafe
	{
		if(inputBuffer.Contains(InputToggle::StrafeLeft))
		{
			newVelocity += right * (-_ShipType.StrafeAcceleration * deltaTime);
			RenderThruster(_StrafeThrusterRight, Vector2(_ShipType.StrafeThrusterX, _ShipType.StrafeThrusterY), 90.0f, transform, SpriteID::MUZZLE_FLASH);
		}
		else
		{
			DestroyThruster(_StrafeThrusterRight);
		}

		if(inputBuffer.Contains(InputToggle::StrafeRight))
		{
			newVelocity += right * (_ShipType.StrafeAcceleration * deltaTime);
			RenderThruster(_StrafeThrusterLeft, Vector2(-_ShipType.StrafeThrusterX, _ShipType.StrafeThrusterY), -90.0f, transform, SpriteID::MUZZLE_FLASH);
		}
		else
		{
			DestroyThruster(_StrafeThrusterLeft);
		}
	}
#pragma endregion

#pragma region Accelerate
	{
		if(inputBuffer.Contains(InputToggle::MoveForward))
		{
			newVelocity += forward * (_ShipType.ForwardAcceleration * deltaTime);
			RenderThruster(_MainThruster, Vector2(_ShipType.MainThrusterX, _ShipType.MainThrusterY), trailRotation, transform, SpriteID::SHIP_TRAIL);
		}
		else
		{
			DestroyThruster(_MainThruster);
		}
	}
#pragma endregion

	// Write back our updated values.
	const auto newSpeedSq  = newVelocity.LengthSq();
	rigid->velocity        = newVelocity.SafeNormalized() * sqrt(std::min(newSpeedSq, _ShipType.MaxSpeedSq));
	rigid->angularVelocity = std::clamp(newAngularVelocity, -_ShipType.MaxAngularVelocity, _ShipType.MaxAngularVelocity);

#pragma region Shoot
	if(_ShotTimer < 0.0f && inputBuffer.Contains(InputToggle::Shoot))
	{
		_ShotTimer = (_ShotTimer > -deltaTime) ? _ShotTimer + _ShipType.ShotCooldown : _ShipType.ShotCooldown;

		// ReSharper disable once CppExpressionWithoutSideEffects
		_Create.MuzzleFlash(transform.pos + forward * _ShipType.BulletSpawnOffsetY, newVelocity);

		auto bulletForward = forward.RotateDeg(-_ShipType.BulletSpawnArcDeg * 0.5f);

		const auto spawnArcIncrement = _ShipType.BulletSpawnArcDeg / (_ShipType.BulletSpawnCount - 1);
		for(auto i = 0; i < _ShipType.BulletSpawnCount; ++i)
		{
			// ReSharper disable once CppExpressionWithoutSideEffects
			_Create.Bullet(transform.pos + (bulletForward * _ShipType.BulletSpawnOffsetY), rigid->velocity + (bulletForward * _ShipType.BulletSpeed), _ShipType.BulletLifetime);
			bulletForward = bulletForward.RotateDeg(spawnArcIncrement);
		}
	}
#pragma endregion
}

Vector2Int
Player::GetPlayerPositionInt() const
{
	const auto floatPos     = GetPlayerPosition();
	const Vector2Int retVal = { static_cast<int>(floatPos.x), static_cast<int>(floatPos.y) };
	return retVal;
}

Vector2
Player::GetPlayerPosition() const
{
	auto trans     = _TransformManager.Get(_Entity);
	Vector2 retVal = { 0.0f, 0.0f };
	if(trans.has_value())
	{
		retVal = trans.value().pos;
	}
	return retVal;
}

Vector2
Player::GetPlayerVelocity() const
{
	auto rigid  = _RigidbodyManager.Get(_Entity);
	auto retVal = Vector2::zero();
	if(rigid.has_value())
	{
		retVal = rigid.value().velocity;
	}
	return retVal;
}

void
Player::RenderThruster(Entity& thruster,
                       const Vector2& thrusterOffset,
                       const float& thrusterRotation,
                       const Transform& parentTrans,
                       const SpriteID spriteID) const
{
	if(!_EntityManager.Exists(thruster))
	{
		thruster = _Create.ShipThruster(_Entity, thrusterOffset, thrusterRotation, spriteID);
	}

	auto OptThrusterTrans = _TransformManager.GetMutable(thruster);
	if(!OptThrusterTrans.has_value())
	{
		//@TODO: Log Error?
	}
	auto thrusterTrans = OptThrusterTrans.value();

	thrusterTrans->pos = parentTrans.pos + thrusterOffset.RotateDeg(parentTrans.rot);
	thrusterTrans->rot = parentTrans.rot + thrusterRotation;
}

void
Player::DestroyThruster(Entity& thruster) const
{
	_EntityManager.Destroy(thruster);
	thruster = Entity::Null();
}
