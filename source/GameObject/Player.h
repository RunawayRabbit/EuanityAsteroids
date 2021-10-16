#pragma once

#include "..\ECS\Entity.h"
#include "..\ECS\EntityManager.h"
#include "..\Physics\Physics.h"

class RigidbodyManager;
class EntityManager;
class TransformManager;
class Create;

struct InputBuffer;

class Player
{
public:
	Player(EntityManager& entityManager, RigidbodyManager& rigidbodyManager,
		TransformManager& transformManager, const Create& create, Physics& physics);

	void Spawn(const Vector2& startPos, const float& startRot);
	void Kill(const Entity& playerEntity);

	void Update(const InputBuffer& inputBuffer, const float& deltaTime);
	inline bool IsAlive()
	{
		return entityManager.Exists(entity);
	}

	Entity entity;

private:

	void RenderThruster(Entity& thruster, const Vector2& thrusterOffset, const float& thrusterRotation, const Transform& parentTrans, const SpriteID spriteID) const;
	void DestroyThruster(Entity& thruster) const;

	RigidbodyManager& rigidbodyManager;
	Physics& physics;

	EntityManager& entityManager;
	TransformManager& transformManager;
	const Create& create;

	Entity mainThruster;
	Entity strafeThrusterLeft;
	Entity strafeThrusterRight;

	float shotTimer;

	// All of the tweakables live here!

	// @TODO: Pull all of these out into a struct that we can use, so that we
	//  can implement multiple ship types!
	static constexpr float mainThrusterX = 0.0f;
	static constexpr float mainThrusterY = 18.0f;

	static constexpr float strafeThrusterX = 4.0f;
	static constexpr float strafeThrusterY = 13.0f;

	static constexpr float maxSpeed = 170.0f;
	static constexpr float forwardAcceleration = maxSpeed / 0.6f;
	static constexpr float strafeAcceleration = maxSpeed / 1.5f;

	static constexpr float bulletSpeed = 300.0f;
	static constexpr float bulletLifetime = 2.5f;
	static constexpr float shotCooldown = 0.3f;
	static constexpr float bulletSpawnOffsetY = 11.0f;

	static constexpr float maxAngularVelocity = 320.0f;
	static constexpr float rotateAcceleration = maxAngularVelocity / 0.4f;
	static constexpr float rotateDeceleration = rotateAcceleration * 2.0f;

	static constexpr float maxSpeedSq = maxSpeed * maxSpeed;
};
