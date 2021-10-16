#pragma once

#include <optional>
#include <unordered_map>

#include "Transform.h"
#include "Entity.h"

class TransformManager
{
public:
	explicit TransformManager(int capacity);

	std::optional<Transform> Get(Entity entity) const;
	std::optional<Transform*> GetMutable(Entity entity);

	void Add(Entity entity, Transform transform);
	void GarbageCollect(const EntityManager& entityManager);

	size_t Count() const;
	void Clear();

private:
	std::unordered_map<Entity, Transform> _Transforms;
};
