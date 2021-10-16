
#include "TransformManager.h"
#include "EntityManager.h"


TransformManager::TransformManager(const int capacity)
{
	_Transforms.reserve(capacity);
}


std::optional<Transform>
TransformManager::Get(const Entity entity) const
{
	std::optional<Transform> result;

	const auto Search = _Transforms.find(entity);

	if (Search != _Transforms.end()) {
		result = Search->second;
	}
	return result;
}

std::optional<Transform*>
TransformManager::GetMutable(const Entity entity)
{
	std::optional<Transform*> result;
	auto Search = _Transforms.find(entity);

	if (Search != _Transforms.end()) {
		result = &Search->second;
	}
	return result;
}


void TransformManager::Add(const Entity entity, const Transform transform)
{
	_Transforms.insert_or_assign(entity, transform);
}

void TransformManager::GarbageCollect(const EntityManager& entityManager)
{
	for(const auto& entity : entityManager.ZombieList)
	{
		_Transforms.erase(entity);
	}
}

size_t TransformManager::Count() const
{
	return _Transforms.size();
}

void
TransformManager::Clear()
{
	_Transforms.clear();
}
