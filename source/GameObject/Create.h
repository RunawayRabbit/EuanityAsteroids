#pragma once

#include <array>
#include <functional>

#include "..\Physics\ColliderType.h"
#include "..\Renderer\SpriteAtlas.h"

#include "..\Math\Vector2.h"

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

	Create(Game& game, EntityManager& entities, TransformManager& transforms, SpriteManager& sprites,
		RigidbodyManager& rigidbodies, UIManager& uiManager, Timer& timer);

	Entity Asteroid(const Vector2& position, const float& rotation,
		const Vector2& velocity, const float& rotVelocity, const AsteroidType& asteroidType) const;
	std::array<Entity, 4> SplitAsteroid(const Entity& asteroid, const float& splitImpulse) const;

	Entity Bullet(const Vector2& position, const float& rotation, const float& speed, const float& secondsToLive) const;

	Entity Ship(const Vector2& position, const float& rotation, const Vector2& initialVelocity = Vector2::zero(), const float& initialAngularVelocity = 0) const;
	Entity ShipThruster(const Entity& ship, const Vector2& thrusterOffset, const float& thrusterRotation, SpriteID spriteID) const;

	[[maybe_unused]] Entity SmallExplosion(const Vector2& position) const;
	[[maybe_unused]] Entity LargeExplosion(const Vector2& position) const;

	Entity UIButton(const AABB& position, SpriteID spriteID, std::function<void()> callback) const;

	Entity GameOver(int score, const Vector2& gameOverPos);

private:

	static ColliderType GetColliderFor(const AsteroidType& asteroidType);
	SpriteID GetSpriteFor(const AsteroidType& asteroidType) const;

	Game& game;
	EntityManager& entityManager;
	TransformManager& transManager;
	RigidbodyManager& rigidbodyManager;
	SpriteManager& spriteManager;
	UIManager& uiManager;
	Timer& timer;
};
