

#include "EntityManager.h"
#include "RigidbodyManager.h"
#include "TransformManager.h"

#include "..\Physics\Physics.h"

RigidbodyManager::RigidbodyManager(const EntityManager& entityManager, const int capacity) :
	entityManager(entityManager)
{
	Allocate(capacity);
}

void RigidbodyManager::Add(const Entity& entity, const ColliderType colliderType, const Vector2 velocity, const float rotVelocity)
{
	if (size == capacity)
	{
		// We're about to overrun our buffer, we gotta scale.
		Allocate((size * 2));
	}

	// Insert our data at the back of the data store
	Rigidbody rb;
	rb.entity = entity; // @TODO: duplicate storage, clean up.
	rb.velocity = velocity;
	rb.colliderType = colliderType;
	rb.angularVelocity = rotVelocity;

	*(entities + size) = entity;
	*(rigidbodies + size) = rb;

	++size;
}

void RigidbodyManager::Allocate(const int newCapacity)
{
	capacity = newCapacity;

	// Allocate new memory
	const size_t elementSizeInBytes = sizeof(Entity) + sizeof(Rigidbody);
	void* newBuffer = new size_t[(elementSizeInBytes * newCapacity)];

	// Set up new pointers for where our data will go
	Entity* newEntities = (Entity*)newBuffer;
	Rigidbody* newRigidbodys = (Rigidbody*)(newEntities + newCapacity);

	if (size > 0)
	{
		// Copy the data to the new buffer
		memcpy(newEntities, entities, sizeof(Entity) * size);
		memcpy(newRigidbodys, rigidbodies, sizeof(Rigidbody) * size);
	}

	// Switch the pointers around
	entities = newEntities;
	rigidbodies = newRigidbodys;

	// Switch the buffers and free the old memory
	delete buffer;
	buffer = newBuffer;
}

bool RigidbodyManager::Lookup(const Entity entity, size_t* index) const
{
	Entity* entityIndex = entities;
	size_t i = 0;
	for (; i < capacity; ++i, ++entityIndex)
	{
		if (*entityIndex == Entity::Null())
		{
			// We hit the end of the index! This entity wasn't in the store!
			return false;
		}
		if (*entityIndex == entity)
		{
			*index = i;
			return true;
		}
	}
	return false;
}

bool RigidbodyManager::GetMutable(const Entity entity, Rigidbody*& rb)
{
	size_t index;
	if (Lookup(entity, &index))
	{
		rb = (rigidbodies + index);
		return true;
	}
	return false;
}

std::optional<Rigidbody>
RigidbodyManager::Get(const Entity entity)
{
	std::optional<Rigidbody> result;
	size_t index;
	if (Lookup(entity, &index))
	{
		result = *(rigidbodies + index);
	}
	return result;
}

uint32_t RigidbodyManager::Count()
{
	return size;
}


void RigidbodyManager::EnqueueAll(Physics& physics, const float& deltaTime)
{
	int i = 0;
	while (i < size)
	{
		Entity* entity = (entities + i);
		Rigidbody* rb = (rigidbodies + i);

		if (entityManager.Exists(*entity))
		{
			physics.Enqueue(*rb, deltaTime);
			++i;
		}
		else
		{
			// Transform appears to have been deleted.
			// Do the swap to remove it from the list.
			Entity* lastActiveEntity = entities + size - 1;
			Rigidbody* lastActiveRigidbody = rigidbodies + size - 1;

			*(entities + i) = *(lastActiveEntity);
			*(rigidbodies + i) = *(lastActiveRigidbody);

			--size;
		}
	}
}
