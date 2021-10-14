#pragma once

#include <optional>

#include "..\Math\Vector2.h"

#include "Entity.h"
#include "Rigidbody.h"

#include "..\Physics\ColliderType.h"

class EntityManager;
class Physics;

//@TODO: Refactor to look more like TransformManager?

class RigidbodyManager
{
public:
	RigidbodyManager(const EntityManager& entityManager, const int capacity);

	void Allocate(const int capacity);

	void EnqueueAll(Physics& physics, const float& deltaTime);
	void Add(const Entity& entity, const ColliderType colliderType, const Vector2 velocity, const float rotVelocity);
	bool Lookup(const Entity entity, size_t* index) const;

	bool GetMutable(const Entity entity, Rigidbody*& rb);
	std::optional<Rigidbody> Get(Entity entity);

	uint32_t Count();
private:
	int size = 0;
	int capacity;

	void* buffer;
	Entity* entities;
	Rigidbody* rigidbodies;

	const EntityManager& entityManager;
};
