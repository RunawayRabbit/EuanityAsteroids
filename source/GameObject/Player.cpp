


#include <algorithm> // min

#include "../ECS/EntityManager.h"
#include "../ECS/RigidbodyManager.h"
#include "../ECS/TransformManager.h"

#include "../Physics/Physics.h"

#include "../Input/InputBuffer.h"

#include "../GameObject/Create.h"

#include "../Math/Math.h" // for MoveTowards

#include "Player.h"


Player::Player(EntityManager& entityManager, RigidbodyManager& rigidbodyManager, TransformManager& transformManager, const Create& create,
	Physics& physics) :
	entityManager(entityManager),
	rigidbodyManager(rigidbodyManager),
	transformManager(transformManager),
	create(create),
	physics(physics),
	entity(Entity::Null()),
	mainThruster(Entity::Null()),
	strafeThrusterLeft(Entity::Null()),
	strafeThrusterRight(Entity::Null()),
	shotTimer(0)
{
	//@NOTE: We specifically set up the player code in such a way that there IS no player until we call Spawn. We do, however,
	// have all of our initialization done at startup time.
}

void Player::Spawn(const Vector2& startPos, const float& startRot)
{
	if (IsAlive()) return;

	entity = create.Ship(startPos, startRot);
}

void Player::Kill(const Entity& playerEntity)
{
	// Don't destroy if it isn't our entity being destroyed!
	if (entity != playerEntity) return;
	auto ship =transformManager.Get(entity);

	DestroyThruster(mainThruster);
	DestroyThruster(strafeThrusterLeft);
	DestroyThruster(strafeThrusterRight);

	entityManager.Destroy(entity);

	create.LargeExplosion(ship.value().pos);
}

void Player::Update(const InputBuffer& input, const float& deltaTime)
{
	// Early-out if the player's ship isn't currently spawned.
	if (!IsAlive()) return;

	// Increment shot cooldown
	shotTimer -= deltaTime;

	// Get the rb and the transform from the respective managers
	Rigidbody* rigid;
	auto OptionalTransform = transformManager.Get(entity);
	if (!rigidbodyManager.GetMutable(entity, rigid) || !OptionalTransform.has_value())
	{
		// If we end up here, a serious bug has occured.
		__assume(false);
		return;
	}

	auto transform = OptionalTransform.value();

	Vector2 newVelocity = rigid->velocity;
	float newAngularVelocity = rigid->angularVelocity;

	// Set up some helper vectors.
	const Vector2 forward = -Vector2::Forward().RotateDeg(transform.rot); // SDL has is "negative Y is up". Which is dumb and I hate it.
	const Vector2 right = Vector2::Right().RotateDeg(transform.rot);

	float trailRotation = 0.0f;

#pragma region Rotate
	{
		bool rotatingLeft = input.Contains(InputToggle::RotateLeft);
		bool rotatingRight = input.Contains(InputToggle::RotateRight);
		if (!rotatingLeft && !rotatingRight)
		{
			newAngularVelocity = Math::MoveTowards(newAngularVelocity, 0, rotateDeceleration * deltaTime);
		}
		else
		{
			// Apply torque
			if (rotatingLeft)
				trailRotation = -rotateAcceleration * deltaTime;
			if (rotatingRight)
				trailRotation = rotateAcceleration * deltaTime;

			newAngularVelocity += trailRotation;
		}
	}
#pragma endregion

#pragma region Strafe
	{
		if (input.Contains(InputToggle::StrafeLeft))
		{
			newVelocity += right * (-strafeAcceleration * deltaTime);
			RenderThruster(strafeThrusterRight, Vector2(strafeThrusterX, strafeThrusterY), 90.0f, transform, SpriteID::MUZZLE_FLASH);
		}
		else
		{
			DestroyThruster(strafeThrusterRight);
		}

		if (input.Contains(InputToggle::StrafeRight))
		{
			newVelocity += right * (strafeAcceleration * deltaTime);
			RenderThruster(strafeThrusterLeft, Vector2(-strafeThrusterX, strafeThrusterY), -90.0f, transform, SpriteID::MUZZLE_FLASH);
		}
		else
		{
			DestroyThruster(strafeThrusterLeft);
		}
	}
#pragma endregion

#pragma region Accelerate
	{
		if (input.Contains(InputToggle::MoveForward))
		{
			newVelocity += forward * (forwardAcceleration * deltaTime);
			RenderThruster(mainThruster, Vector2(mainThrusterX, mainThrusterY), trailRotation, transform, SpriteID::SHIP_TRAIL);
		}
		else
		{
			DestroyThruster(mainThruster);
		}
	}
#pragma endregion



#pragma region Shoot
	if (shotTimer < 0.0f && input.Contains(InputToggle::Shoot))
	{
		shotTimer = (shotTimer > -deltaTime) ? shotTimer + shotCooldown : shotCooldown;
		create.Bullet(transform.pos + (forward * bulletSpawnOffsetY), transform.rot, bulletSpeed, bulletLifetime);
	}
#pragma endregion

	// Write back our updated values.
	float newSpeedSq = newVelocity.LengthSq();
	rigid->velocity = newVelocity.SafeNormalized() * sqrt(std::min(newSpeedSq, maxSpeedSq));
	rigid->angularVelocity = std::clamp(newAngularVelocity, -maxAngularVelocity, maxAngularVelocity);
}

void Player::RenderThruster(Entity& thruster, const Vector2& thrusterOffset, const float& thrusterRotation, const Transform& parentTrans, const SpriteID spriteID)
{
	if (!entityManager.Exists(thruster))
	{
		thruster = create.ShipThruster(entity, thrusterOffset, thrusterRotation, spriteID);
	}

	auto OptThrusterTrans = transformManager.GetMutable(thruster);
	if(!OptThrusterTrans.has_value())
	{
		//@TODO: Log Error?
	}
	Transform* thrusterTrans = OptThrusterTrans.value();

	thrusterTrans->pos = parentTrans.pos + thrusterOffset.RotateDeg(parentTrans.rot);
	thrusterTrans->rot = parentTrans.rot + thrusterRotation;
}

void Player::DestroyThruster(Entity& thruster)
{
	entityManager.Destroy(thruster);
	thruster = Entity::Null();
}
