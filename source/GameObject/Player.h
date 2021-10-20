#pragma once

#include "ShipType.h"
#include "WeaponType.h"

#include "../ECS/Entity.h"
#include "../ECS/EntityManager.h"
#include "../Physics/Physics.h"

#include "../Math/Vector2Int.h"

#include "../Renderer/SpriteID.h"

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
	void Kill(const Entity& playerEntity, const Vector2& playerVelocity);

	void Update(const InputBuffer& inputBuffer, const float& deltaTime);
	bool IsAlive() const
	{
		return _EntityManager.Exists(_Entity);
	}

	Vector2 GetPlayerPosition() const;
	Vector2Int GetPlayerPositionInt() const;
	Vector2 GetPlayerForward() const;

	Vector2 GetPlayerVelocity() const;
	void TakeDamage(int damage);

private:

	void RenderThruster(Entity& thruster, const Vector2& thrusterOffset, const float& thrusterRotation, const Transform& parentTrans, SpriteID spriteID) const;
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

	int _Health;

	float _ShotTimer;

	ShipType _Ship;
	WeaponType _Weapon;
};
