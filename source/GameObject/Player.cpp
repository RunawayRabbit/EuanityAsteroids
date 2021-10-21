#include "Player.h"

#include <algorithm> // min

#include "../ECS/EntityManager.h"
#include "../ECS/RigidbodyManager.h"
#include "../ECS/TransformManager.h"
#include "../GameObject/Create.h"
#include "../Input/InputBuffer.h"
#include "../Math/EuanityMath.h"
#include "../Math/Vector2Int.h"
#include "../Physics/Physics.h"
#include "../Platform/Game.h"

Player::Player(Game& game)
	: _Game(game),
	  _Entity(Entity::Null()),
	  _MainThruster(Entity::Null()),
	  _StrafeThrusterLeft(Entity::Null()),
	  _StrafeThrusterRight(Entity::Null()),
	  _ShotTimer(0),
	  _Ship(ShipInfo::GetShip(game.GameState.PlayerShipType)),
	  _Weapon(ShipInfo::GetDefaultWeaponForShip(_Ship.Type))
{
	//@NOTE: We specifically set up the player code in such a way that there IS no player until we call Spawn. We do, however,
	// have all of our initialization done at startup time.


	_Health = _Ship.StartingHealth;
}

void
Player::Spawn(const Vector2& startPos, const float& startRot)
{
	if(IsAlive())
		return;

	_Ship = ShipInfo::GetShip(_Game.GameState.PlayerShipType);
	_Weapon = ShipInfo::GetDefaultWeaponForShip(_Ship.Type);
	_Entity = _Game.Create.Ship(_Ship, startPos, startRot);
}

void
Player::Kill(const Entity& playerEntity, const Vector2& playerVelocity)
{
	// Don't destroy if it isn't our entity being destroyed!
	if(_Entity != playerEntity)
		return;

	auto ship    = _Game.Xforms.Get(_Entity);
	auto shipPos = ship.value().pos;
	DestroyThruster(_MainThruster);
	DestroyThruster(_StrafeThrusterLeft);
	DestroyThruster(_StrafeThrusterRight);

	_Game.Entities.Destroy(_Entity);


	// ReSharper disable CppExpressionWithoutSideEffects
	for(auto i = 0; i < 3; ++i)
	{
		auto spawnDistanceFromPlayer = Math::RandomRange(10.0f, 17.0f);

		const auto offset = Vector2::Forward().RotateRad(Math::RandomRange(0.0f, Math::TAU)) * spawnDistanceFromPlayer;
		_Game.Create.LargeExplosion(shipPos + offset);
		_Game.Create.LargeExplosion(shipPos + offset);
		_Game.Create.LargeExplosion(shipPos + offset);
		_Game.Create.LargeExplosion(shipPos + offset);
	}

	for(auto i = 0; i < 12; ++i)
	{
		const auto randomVelocity = Vector2::Forward().RotateRad(Math::RandomRange(0.0f, Math::TAU)) * Math::RandomRange(10.0f, 140.0f);
		_Game.Create.SmallExplosion(shipPos, (playerVelocity * 0.1f) + randomVelocity);
	}

	for(auto i = 0; i < 12; ++i)
	{
		const auto randomVelocity = Vector2::Forward().RotateRad(Math::RandomRange(0.0f, Math::TAU)) * Math::RandomRange(60.0f, 240.0f);
		_Game.Create.TinyExplosion(shipPos, (playerVelocity * 0.1f) + randomVelocity);
	}
	// ReSharper restore CppExpressionWithoutSideEffects

	_Entity = Entity::Null();
	_Health = _Ship.StartingHealth;
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
	auto optionalTransform = _Game.Xforms.Get(_Entity);
	if(!optionalTransform.has_value() || !_Game.Rigidbodies.GetMutable(_Entity, rigid))
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
			newAngularVelocity = Math::MoveTowards(newAngularVelocity, 0, _Ship.RotateAcceleration * deltaTime);
		}
		else
		{
			// Apply torque
			if(rotatingLeft)
				trailRotation = -_Ship.RotateAcceleration * deltaTime;
			if(rotatingRight)
				trailRotation = _Ship.RotateAcceleration * deltaTime;

			newAngularVelocity += trailRotation;
		}
	}
#pragma endregion

#pragma region Strafe
	{
		if(inputBuffer.Contains(InputToggle::StrafeLeft))
		{
			newVelocity += right * (-_Ship.StrafeAcceleration * deltaTime);
			RenderThruster(_StrafeThrusterRight, Vector2(_Ship.StrafeThrusterX, _Ship.StrafeThrusterY), 90.0f, transform,
			               SpriteID::MUZZLE_FLASH);
		}
		else
		{
			DestroyThruster(_StrafeThrusterRight);
		}

		if(inputBuffer.Contains(InputToggle::StrafeRight))
		{
			newVelocity += right * (_Ship.StrafeAcceleration * deltaTime);
			RenderThruster(_StrafeThrusterLeft, Vector2(-_Ship.StrafeThrusterX, _Ship.StrafeThrusterY), -90.0f, transform,
			               SpriteID::MUZZLE_FLASH);
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
			newVelocity += forward * (_Ship.ForwardAcceleration * deltaTime);
			RenderThruster(_MainThruster, Vector2(_Ship.MainThrusterX, _Ship.MainThrusterY), trailRotation, transform,
			               SpriteID::SHIP_TRAIL);
		}
		else
		{
			DestroyThruster(_MainThruster);
		}
	}
#pragma endregion

	// Write back our updated values.
	const auto newSpeedSq  = newVelocity.LengthSq();
	rigid->velocity        = newVelocity.SafeNormalized() * sqrt(std::min(newSpeedSq, _Ship.MaxSpeedSq));
	rigid->angularVelocity = std::clamp(newAngularVelocity, -_Ship.MaxAngularVelocity, _Ship.MaxAngularVelocity);

#pragma region Shoot
	if(_ShotTimer < 0.0f && inputBuffer.Contains(InputToggle::Shoot))
	{
		_ShotTimer = (_ShotTimer > -deltaTime) ? _ShotTimer + _Weapon.ShotCooldown : _Weapon.ShotCooldown;

		// ReSharper disable once CppExpressionWithoutSideEffects
		_Game.Create.MuzzleFlash(transform.pos + forward * _Weapon.BulletSpawnOffsetY, newVelocity);

		auto bulletForward = forward.RotateDeg(-_Weapon.BulletSpawnArcDeg * 0.5f);

		const auto spawnArcIncrement = _Weapon.BulletSpawnArcDeg / (_Weapon.BulletSpawnCount - 1);
		for(auto i = 0; i < _Weapon.BulletSpawnCount; ++i)
		{
			// ReSharper disable once CppExpressionWithoutSideEffects
			_Game.Create.Bullet(_Weapon.BulletType, transform.pos + (bulletForward * _Weapon.BulletSpawnOffsetY),
			               rigid->velocity + (bulletForward * _Weapon.BulletSpeed), _Weapon.BulletLifetime);
			bulletForward = bulletForward.RotateDeg(spawnArcIncrement);
		}
	}
#pragma endregion
}

bool
Player::IsAlive() const
{
	return _Game.Entities.Exists(_Entity);
}

Vector2Int
Player::GetPlayerPositionInt() const
{
	const auto floatPos     = GetPlayerPosition();
	const Vector2Int retVal = { static_cast<int>(floatPos.x), static_cast<int>(floatPos.y) };
	return retVal;
}

Vector2
Player::GetPlayerForward() const
{
	auto trans     = _Game.Xforms.Get(_Entity);
	Vector2 retVal = { 0.0f, 0.0f };
	if(trans.has_value())
	{
		retVal = Vector2::Forward().RotateDeg(trans.value().rot);
	}
	return retVal;
}

Vector2
Player::GetPlayerPosition() const
{
	auto trans     = _Game.Xforms.Get(_Entity);
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
	auto rigid  = _Game.Rigidbodies.Get(_Entity);
	auto retVal = Vector2::Zero();
	if(rigid.has_value())
	{
		retVal = rigid.value().velocity;
	}
	return retVal;
}

void
Player::TakeDamage(const int damage)
{
	_Health -= damage;
	if(_Health <= 0)
	{
		Kill(_Entity, Vector2::Zero());
	}
}

ColliderType
Player::GetShipColliderType() const
{
	return _Ship.ColliderType;
}

void
Player::RenderThruster(Entity& thruster,
                       const Vector2& thrusterOffset,
                       const float& thrusterRotation,
                       const Transform& parentTrans,
                       const SpriteID spriteID) const
{
	if(!_Game.Entities.Exists(thruster))
	{
		thruster = _Game.Create.ShipThruster(_Entity, thrusterOffset, thrusterRotation, spriteID);
	}

	auto OptThrusterTrans = _Game.Xforms.GetMutable(thruster);
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
	_Game.Entities.Destroy(thruster);
	thruster = Entity::Null();
}
