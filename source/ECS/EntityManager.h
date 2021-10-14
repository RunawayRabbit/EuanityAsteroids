#pragma once

#include <unordered_set>
#include <queue>
#include <iostream>

#include "Entity.h"

class Timer;

class EntityManager
{
public:
	EntityManager(const Timer& Time);

	Entity Create();
	bool Exists(Entity Entity) const;
	void Destroy(Entity Entity);
	void DestroyDelayed(Entity Entity, const float& Seconds);

	void GarbageCollect();

	uint32_t Count();

private:
	void Destroy(Entity::EID EntityEID);
	void DestroyZombies(const Entity::EID Entity);

	const Timer& _Time;

	struct IDRange
	{
		Entity::EID Begin;
		Entity::EID End;
	};

	std::unordered_set<Entity::EID> _ZombieList;

	std::vector<IDRange> _OpenRanges;

	class death_row : public std::priority_queue<std::pair<float, Entity::EID>,
		std::vector<std::pair<float, Entity::EID>>,
		std::greater<std::pair<float, Entity::EID>>>
	{
	public:
		bool remove(const Entity::EID EID)
		{
			auto it = std::find_if(c.begin(), c.end(),
				[=](std::pair<float, Entity::EID> pair)
			{
				return pair.second == EID;
			});

			if (it != c.end())
			{
				c.erase(it);
				std::make_heap(c.begin(), c.end(), this->comp);
				return true;
			}
			return false;
		}
	};

	death_row _DeathRow;
};
