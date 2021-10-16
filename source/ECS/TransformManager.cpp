
#include "TransformManager.h"
#include "EntityManager.h"


TransformManager::TransformManager(int capacity)
{
	transforms.reserve(capacity);
}


std::optional<Transform>
TransformManager::Get(const Entity Entity) const
{
	std::optional<Transform> Result;

	const auto Search = transforms.find(Entity);

	if (Search != transforms.end()) {
		Result = Search->second;
	}
	return Result;
}

std::optional<Transform*>
TransformManager::GetMutable(const Entity entity)
{
	std::optional<Transform*> Result;
	auto Search = transforms.find(entity);

	if (Search != transforms.end()) {
		Result = &Search->second;
	}
	return Result;
}


void TransformManager::Add(const Entity entity, const Transform transform)
{
	transforms.insert_or_assign(entity, transform);
}

void TransformManager::GarbageCollect(const EntityManager& entityManager)
{
	for(const auto& entity : entityManager.ZombieList)
	{
		transforms.erase(entity);
	}
}

size_t TransformManager::Count() const
{
	return transforms.size();
}
