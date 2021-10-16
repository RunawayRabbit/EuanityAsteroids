#include "EntityManager.h"
#include "RigidbodyManager.h"
#include "../Physics/Physics.h"

RigidbodyManager::RigidbodyManager(const EntityManager& entityManager, const int capacity)
	: _EntityManager(entityManager)
{
	Allocate(capacity);
}

void
RigidbodyManager::Add(const Entity& entity, const ColliderType colliderType, const Vector2 velocity, const float rotVelocity)
{
	if(_Size == _Capacity)
	{
		// We're about to overrun our buffer, we gotta scale.
		Allocate((_Size * 2));
	}

	// Insert our data at the back of the data store
	Rigidbody rb;
	rb.entity          = entity; // @TODO: duplicate storage, clean up?
	rb.velocity        = velocity;
	rb.colliderType    = colliderType;
	rb.angularVelocity = rotVelocity;

	*(_Entities + _Size)    = entity;
	*(_Rigidbodies + _Size) = rb;

	++_Size;
}

void
RigidbodyManager::Allocate(const int capacity)
{
	_Capacity = capacity;

	// Allocate new memory
	const auto elementSizeInBytes = sizeof(Entity) + sizeof(Rigidbody);
	void* newBuffer               = new size_t[(elementSizeInBytes * capacity)];

	// Set up new pointers for where our data will go
	const auto newEntities    = static_cast<Entity*>(newBuffer);
	const auto newRigidbodies = reinterpret_cast<Rigidbody*>(newEntities + capacity);

	if(_Size > 0)
	{
		// Copy the data to the new buffer
		memcpy(newEntities, _Entities, sizeof(Entity) * _Size);
		memcpy(newRigidbodies, _Rigidbodies, sizeof(Rigidbody) * _Size);
	}

	// Switch the pointers around
	_Entities    = newEntities;
	_Rigidbodies = newRigidbodies;

	// Switch the buffers and free the old memory
	// ReSharper disable once CppDeletingVoidPointer
	delete _Buffer;
	_Buffer = newBuffer;
}

bool
RigidbodyManager::Lookup(const Entity entity, size_t* index) const
{
	auto entityIndex = _Entities;
	size_t i         = 0;
	for(; i < _Size; ++i, ++entityIndex)
	{
		if(*entityIndex == Entity::Null())
		{
			// We hit the end of the index! This entity wasn't in the store!
			return false;
		}
		if(*entityIndex == entity)
		{
			*index = i;
			return true;
		}
	}
	return false;
}

bool
RigidbodyManager::GetMutable(const Entity entity, Rigidbody*& rb)
{
	size_t index;
	if(Lookup(entity, &index))
	{
		rb = (_Rigidbodies + index);
		return true;
	}
	return false;
}

std::optional<Rigidbody>
RigidbodyManager::Get(const Entity entity) const
{
	std::optional<Rigidbody> result;
	size_t index;
	if(Lookup(entity, &index))
	{
		result = *(_Rigidbodies + index);
	}
	return result;
}

uint32_t
RigidbodyManager::Count() const
{
	return _Size;
}

void
RigidbodyManager::Clear()
{
	_Size = 0;
}


void
RigidbodyManager::EnqueueAll(Physics& physics, const float& deltaTime)
{
	auto i = 0;
	while(i < _Size)
	{
		const auto entity = (_Entities + i);
		const auto rb     = (_Rigidbodies + i);

		if(_EntityManager.Exists(*entity))
		{
			physics.Enqueue(*rb, deltaTime);
			++i;
		}
		else
		{
			// Transform appears to have been deleted.
			// Do the swap to remove it from the list.
			const auto lastActiveEntity    = _Entities + _Size - 1;
			const auto lastActiveRigidbody = _Rigidbodies + _Size - 1;

			*(_Entities + i)    = *(lastActiveEntity);
			*(_Rigidbodies + i) = *(lastActiveRigidbody);

			--_Size;
		}
	}
}
