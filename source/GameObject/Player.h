#pragma once

#include "ShipInfo.h"
#include "WeaponType.h"

#include "../ECS/Entity.h"
#include "../ECS/EntityManager.h"
#include "../Physics/Physics.h"

#include "../Math/Vector2Int.h"
#include "../Platform/Game.h"

#include "../Renderer/SpriteID.h"

class Game;
class RigidbodyManager;
class EntityManager;
class TransformManager;
class Create;

struct InputBuffer;

class Player
{
public:
	explicit Player(Game& game);

	void Spawn(const Vector2& startPos, const float& startRot);
	void Kill(const Entity& playerEntity, const Vector2& playerVelocity);

	void Update(const InputBuffer& inputBuffer, const float& deltaTime);
	bool IsAlive() const;

	Vector2 GetPlayerPosition() const;
	Vector2Int GetPlayerPositionInt() const;
	Vector2 GetPlayerForward() const;

	Vector2 GetPlayerVelocity() const;
	void TakeDamage(int damage);

	 ColliderType GetShipColliderType() const;

private:

	void RenderThruster(Entity& thruster, const Vector2& thrusterOffset, const float& thrusterRotation, const Transform& parentTrans, SpriteID spriteID) const;
	void DestroyThruster(Entity& thruster) const;

	Game& _Game;

	Entity _Entity;

	Entity _MainThruster;
	Entity _StrafeThrusterLeft;
	Entity _StrafeThrusterRight;

	int _Health;

	float _ShotTimer;

	ShipInfo _Ship;
	WeaponType _Weapon;

};
