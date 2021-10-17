#include <algorithm> // min

#include "../ECS/EntityManager.h"
#include "../ECS/RigidbodyManager.h"
#include "../ECS/TransformManager.h"

#include "../Physics/Physics.h"

#include "../Input/InputBuffer.h"

#include "../GameObject/Create.h"

#include "../Math/Math.h" // for MoveTowards

#include "Player.h"

#include "../Math/Vector2Int.h"


Player::Player(EntityManager& entityManager,
               RigidbodyManager& rigidbodyManager,
               TransformManager& transformManager,
               const Create& create,
               Physics& physics)
	: _Entity(Entity::Null()),
	  _RigidbodyManager(rigidbodyManager),
	  _Physics(physics),
	  _EntityManager(entityManager),
	  _TransformManager(transformManager),
	  _Create(create),
	  _MainThruster(Entity::Null()),
	  _StrafeThrusterLeft(Entity::Null()),
	  _StrafeThrusterRight(Entity::Null()),
	  _ShotTimer(0)
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
Player::Kill(const Entity& playerEntity)
{
	// Don't destroy if it isn't our entity being destroyed!
	if(_Entity != playerEntity)
		return;

	auto ship = _TransformManager.Get(_Entity);

	DestroyThruster(_MainThruster);
	DestroyThruster(_StrafeThrusterLeft);
	DestroyThruster(_StrafeThrusterRight);

	_EntityManager.Destroy(_Entity);

	_Create.LargeExplosion(ship.value().pos, GetPlayerVelocity() * -0.4f);
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
			newAngularVelocity = Math::MoveTowards(newAngularVelocity, 0, ROTATE_DECELERATION * deltaTime);
		}
		else
		{
			// Apply torque
			if(rotatingLeft)
				trailRotation = -ROTATE_ACCELERATION * deltaTime;
			if(rotatingRight)
				trailRotation = ROTATE_ACCELERATION * deltaTime;

			newAngularVelocity += trailRotation;
		}
	}
#pragma endregion

#pragma region Strafe
	{
		if(inputBuffer.Contains(InputToggle::StrafeLeft))
		{
			newVelocity += right * (-STRAFE_ACCELERATION * deltaTime);
			RenderThruster(_StrafeThrusterRight, Vector2(STRAFE_THRUSTER_X, STRAFE_THRUSTER_Y), 90.0f, transform, SpriteID::MUZZLE_FLASH);
		}
		else
		{
			DestroyThruster(_StrafeThrusterRight);
		}

		if(inputBuffer.Contains(InputToggle::StrafeRight))
		{
			newVelocity += right * (STRAFE_ACCELERATION * deltaTime);
			RenderThruster(_StrafeThrusterLeft, Vector2(-STRAFE_THRUSTER_X, STRAFE_THRUSTER_Y), -90.0f, transform, SpriteID::MUZZLE_FLASH);
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
			newVelocity += forward * (FORWARD_ACCELERATION * deltaTime);
			RenderThruster(_MainThruster, Vector2(MAIN_THRUSTER_X, MAIN_THRUSTER_Y), trailRotation, transform, SpriteID::SHIP_TRAIL);
		}
		else
		{
			DestroyThruster(_MainThruster);
		}
	}
#pragma endregion


#pragma region Shoot
	if(_ShotTimer < 0.0f && inputBuffer.Contains(InputToggle::Shoot))
	{
		_ShotTimer = (_ShotTimer > -deltaTime) ? _ShotTimer + SHOT_COOLDOWN : SHOT_COOLDOWN;
		// ReSharper disable once CppExpressionWithoutSideEffects
		_Create.Bullet(transform.pos + (forward * BULLET_SPAWN_OFFSET_Y), transform.rot, BULLET_SPEED, BULLET_LIFETIME);
	}
#pragma endregion

	// Write back our updated values.
	const auto newSpeedSq  = newVelocity.LengthSq();
	rigid->velocity        = newVelocity.SafeNormalized() * sqrt(std::min(newSpeedSq, MAX_SPEED_SQ));
	rigid->angularVelocity = std::clamp(newAngularVelocity, -MAX_ANGULAR_VELOCITY, MAX_ANGULAR_VELOCITY);
}

Vector2Int
Player::GetPlayerPositionInt() const
{
	const auto floatPos     = GetPlayerPosition();
	const Vector2Int retVal = {static_cast<int>(floatPos.x), static_cast<int>(floatPos.y)};
	return retVal;
}

Vector2
Player::GetPlayerPosition() const
{
	auto trans = _TransformManager.Get(_Entity);
	Vector2 retVal = {0.0f,0.0f};
	if(trans.has_value())
	{
		retVal = trans.value().pos;

	}
	return retVal;
}

Vector2
Player::GetPlayerVelocity() const
{
	auto rigid = _RigidbodyManager.Get(_Entity);
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
