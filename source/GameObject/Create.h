#pragma once

#include <array>
#include <functional>

#include "WeaponType.h"
#include "../Physics/ColliderType.h"
#include "../Renderer/SpriteAtlas.h"

#include "../Math/Vector2.h"

class ShipInfo;
class Game;

class Entity;
class EntityManager;
class TransformManager;
class RigidbodyManager;
class SpriteManager;
class UIManager;
class Timer;

class Create
{
public:
	enum class AsteroidType
	{
		LARGE,

		RANDOM_MEDIUM,
		MEDIUM_1,
		MEDIUM_2,
		MEDIUM_3,
		MEDIUM_4,

		RANDOM_SMALL,
		SMALL_1,
		SMALL_2,
		SMALL_3,
		SMALL_4,
		SMALL_5,
		SMALL_6,
		SMALL_7,
		SMALL_8,
		SMALL_9,
		SMALL_10,
		SMALL_11,
		SMALL_12,
		SMALL_13,
		SMALL_14,
		SMALL_15,
		SMALL_16,
	};

	static float GetCollisionRadiusFromColliderType(const AsteroidType& type);

	Create(Game& game, EntityManager& entities, TransformManager& transforms, SpriteManager& sprites,
		RigidbodyManager& rigidbodies, UIManager& uiManager, Timer& timer);

	Entity Asteroid(const Vector2& position, const float& rotation,
		const Vector2& velocity, const float& rotVelocity, const AsteroidType& asteroidType) const;
	std::array<Entity, 4> SplitAsteroid(const Entity& asteroid, const float& splitImpulse) const;

	Entity MuzzleFlash(const Vector2& position, const Vector2& velocity) const;
	Entity Bullet(BulletType bulletType, const Vector2& position, const Vector2& velocity, const float& secondsToLive) const;

	Entity Ship(const ShipInfo& shipInfo, const Vector2& position, const float& rotation, const Vector2& initialVelocity = Vector2::Zero(), const float& initialAngularVelocity = 0) const;
	Entity ShipThruster(const Entity& ship, const Vector2& thrusterOffset, const float& thrusterRotation, SpriteID spriteID) const;

	[[maybe_unused]] Entity TinyExplosion(const Vector2& position, const Vector2& velocity = Vector2::Zero(), const float& rotVelocity = 0) const;
	[[maybe_unused]] Entity SmallExplosion(const Vector2& position, const Vector2& velocity = Vector2::Zero(), const float& rotVelocity = 0) const;
	[[maybe_unused]] Entity LargeExplosion(const Vector2& position, const Vector2& velocity = Vector2::Zero(), const float& rotVelocity = 0) const;

	Entity UIButton(const AABB& position, SpriteID spriteID, std::function<void()> callback) const;

	Entity GameOver(int score);

private:

	static ColliderType GetColliderFor(const AsteroidType& asteroidType);
	SpriteID GetSpriteFor(const AsteroidType& asteroidType) const;

	Game& _Game;
	EntityManager& _EntityManager;
	TransformManager& _TransManager;
	RigidbodyManager& _RigidbodyManager;
	SpriteManager& _SpriteManager;
	UIManager& _UIManager;
	Timer& _Timer;
};
