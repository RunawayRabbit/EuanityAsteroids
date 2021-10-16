#include <algorithm> // min

#include "../ECS/EntityManager.h"
#include "../ECS/RigidbodyManager.h"
#include "../ECS/TransformManager.h"

#include "../Physics/Physics.h"

#include "../Input/InputBuffer.h"

#include "../GameObject/Create.h"

#include "../Math/Math.h" // for MoveTowards

#include "Player.h"


Player::Player(EntityManager& entityManager,
               RigidbodyManager& rigidbodyManager,
               TransformManager& transformManager,
               const Create& create,
               Physics& physics)
	: entity(Entity::Null()),
	  rigidbodyManager(rigidbodyManager),
	  physics(physics),
	  entityManager(entityManager),
	  transformManager(transformManager),
	  create(create),
	  mainThruster(Entity::Null()),
	  strafeThrusterLeft(Entity::Null()),
	  strafeThrusterRight(Entity::Null()),
	  shotTimer(0)
{
	//@NOTE: We specifically set up the player code in such a way that there IS no player until we call Spawn. We do, however,
	// have all of our initialization done at startup time.
}

void
Player::Spawn(const Vector2& startPos, const float& startRot)
{
	if(IsAlive())
		return;

	entity = create.Ship(startPos, startRot);
}

void
Player::Kill(const Entity& playerEntity)
{
	// Don't destroy if it isn't our entity being destroyed!
	if(entity != playerEntity)
		return;

	auto ship = transformManager.Get(entity);

	DestroyThruster(mainThruster);
	DestroyThruster(strafeThrusterLeft);
	DestroyThruster(strafeThrusterRight);

	entityManager.Destroy(entity);

	create.LargeExplosion(ship.value().pos);
	entity = Entity::Null();
}

void
Player::Update(const InputBuffer& inputBuffer, const float& deltaTime)
{
	// Early-out if the player's ship isn't currently spawned.
	if(!IsAlive())
		return;

	// Increment shot cooldown
	shotTimer -= deltaTime;

	// Get the rb and the transform from the respective managers
	Rigidbody* rigid;
	auto optionalTransform = transformManager.Get(entity);
	if(!optionalTransform.has_value() || !rigidbodyManager.GetMutable(entity, rigid))
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
			RenderThruster(strafeThrusterRight, Vector2(STRAFE_THRUSTER_X, STRAFE_THRUSTER_Y), 90.0f, transform, SpriteID::MUZZLE_FLASH);
		}
		else
		{
			DestroyThruster(strafeThrusterRight);
		}

		if(inputBuffer.Contains(InputToggle::StrafeRight))
		{
			newVelocity += right * (STRAFE_ACCELERATION * deltaTime);
			RenderThruster(strafeThrusterLeft, Vector2(-STRAFE_THRUSTER_X, STRAFE_THRUSTER_Y), -90.0f, transform, SpriteID::MUZZLE_FLASH);
		}
		else
		{
			DestroyThruster(strafeThrusterLeft);
		}
	}
#pragma endregion

#pragma region Accelerate
	{
		if(inputBuffer.Contains(InputToggle::MoveForward))
		{
			newVelocity += forward * (FORWARD_ACCELERATION * deltaTime);
			RenderThruster(mainThruster, Vector2(MAIN_THRUSTER_X, MAIN_THRUSTER_Y), trailRotation, transform, SpriteID::SHIP_TRAIL);
		}
		else
		{
			DestroyThruster(mainThruster);
		}
	}
#pragma endregion


#pragma region Shoot
	if(shotTimer < 0.0f && inputBuffer.Contains(InputToggle::Shoot))
	{
		shotTimer = (shotTimer > -deltaTime) ? shotTimer + SHOT_COOLDOWN : SHOT_COOLDOWN;
		// ReSharper disable once CppExpressionWithoutSideEffects
		create.Bullet(transform.pos + (forward * BULLET_SPAWN_OFFSET_Y), transform.rot, BULLET_SPEED, BULLET_LIFETIME);
	}
#pragma endregion

	// Write back our updated values.
	const auto newSpeedSq  = newVelocity.LengthSq();
	rigid->velocity        = newVelocity.SafeNormalized() * sqrt(std::min(newSpeedSq, MAX_SPEED_SQ));
	rigid->angularVelocity = std::clamp(newAngularVelocity, -MAX_ANGULAR_VELOCITY, MAX_ANGULAR_VELOCITY);
}

void
Player::RenderThruster(Entity& thruster,
                       const Vector2& thrusterOffset,
                       const float& thrusterRotation,
                       const Transform& parentTrans,
                       const SpriteID spriteID) const
{
	if(!entityManager.Exists(thruster))
	{
		thruster = create.ShipThruster(entity, thrusterOffset, thrusterRotation, spriteID);
	}

	auto OptThrusterTrans = transformManager.GetMutable(thruster);
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
	entityManager.Destroy(thruster);
	thruster = Entity::Null();
}
