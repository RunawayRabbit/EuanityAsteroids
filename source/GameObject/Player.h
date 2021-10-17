#pragma once

#include "../ECS/Entity.h"
#include "../ECS/EntityManager.h"
#include "../Physics/Physics.h"

#include "../Math/Vector2Int.h"

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
	bool IsAlive() const
	{
		return _EntityManager.Exists(_Entity);
	}

	Vector2 GetPlayerPosition() const;
	Vector2Int GetPlayerPositionInt() const;

	Vector2 GetPlayerVelocity() const;

private:

	void RenderThruster(Entity& thruster, const Vector2& thrusterOffset, const float& thrusterRotation, const Transform& parentTrans, const SpriteID spriteID) const;
	void DestroyThruster(Entity& thruster) const;

	RigidbodyManager& _RigidbodyManager;
	Physics& _Physics;

	EntityManager& _EntityManager;
	TransformManager& _TransformManager;
	const Create& _Create;

	Entity _Entity;

	Entity _MainThruster;
	Entity _StrafeThrusterLeft;
	Entity _StrafeThrusterRight;

	float _ShotTimer;

	// All of the tweakables live here!

	// @TODO: Pull all of these out into a struct that we can use, so that we
	//  can implement multiple ship types!
	static constexpr float MAIN_THRUSTER_X = 0.0f;
	static constexpr float MAIN_THRUSTER_Y = 18.0f;

	static constexpr float STRAFE_THRUSTER_X = 4.0f;
	static constexpr float STRAFE_THRUSTER_Y = 13.0f;

	static constexpr float MAX_SPEED = 210.0f;
	static constexpr float FORWARD_ACCELERATION = MAX_SPEED / 0.6f;
	static constexpr float STRAFE_ACCELERATION = MAX_SPEED / 1.5f;

	static constexpr float BULLET_SPEED = 400.0f;
	static constexpr float BULLET_LIFETIME = 2.8f;
	static constexpr float SHOT_COOLDOWN = 0.3f;
	static constexpr float BULLET_SPAWN_OFFSET_Y = 11.0f;

	static constexpr float MAX_ANGULAR_VELOCITY = 320.0f;
	static constexpr float ROTATE_ACCELERATION = MAX_ANGULAR_VELOCITY / 0.4f;
	static constexpr float ROTATE_DECELERATION = ROTATE_ACCELERATION * 2.0f;

	static constexpr float MAX_SPEED_SQ = MAX_SPEED * MAX_SPEED;
};
