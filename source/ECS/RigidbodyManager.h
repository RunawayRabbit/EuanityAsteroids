#pragma once

#include <optional>

#include "../Math/Vector2.h"

#include "Entity.h"
#include "Rigidbody.h"

#include "../Physics/ColliderType.h"

class EntityManager;
class Physics;

class RigidbodyManager
{
public:
	RigidbodyManager(const EntityManager& entityManager, int capacity);

	void Allocate(int capacity);

	void EnqueueAll(Physics& physics, const float& deltaTime);
	void Add(const Entity& entity, ColliderType colliderType, Vector2 velocity, float rotVelocity);
	bool Lookup(Entity entity, size_t* index) const;

	bool GetMutable(Entity entity, Rigidbody*& rb);
	std::optional<Rigidbody> Get(Entity entity) const;

	uint32_t Count() const;
	void Clear();
private:
	int _Size = 0;
	int _Capacity;

	void* _Buffer;
	Entity* _Entities;
	Rigidbody* _Rigidbodies;

	const EntityManager& _EntityManager;
};
