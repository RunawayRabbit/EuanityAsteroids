#pragma once

#include <queue>
#include <iterator>

#include "../Platform/RingBuffer.h"
#include "Entity.h"

class Timer;

class EntityManager
{
public:

	explicit EntityManager(const Timer& time);

	Entity Create();
	bool Exists(Entity entity) const;
	void Destroy(Entity entity);
	void DestroyDelayed(Entity entity, const float& seconds);

	void GarbageCollect();

	uint32_t Count();
	void Clear();

	RingBuffer<Entity> ZombieList;

private:
	void DestroyZombies(Entity entity);

	const Timer& _Time;

	struct IDRange
	{
		Entity::EID Begin;
		Entity::EID End;
	};

	uint_fast16_t _PrevZombieCount = 0;
	uint_fast16_t _ZombieCount     = 0;

	std::vector<IDRange> _OpenRanges;

	class DeathRow
		: public std::priority_queue<std::pair<float, Entity>,
		                             std::vector<std::pair<float, Entity>>,
		                             std::greater<std::pair<float, Entity>>>
	{
	public:
		// ReSharper disable once CppInconsistentNaming
		bool remove(const Entity entity)
		{
			const auto it = std::find_if(c.begin(), c.end(),
			                             [=](std::pair<float, Entity> pair)
			                             {
				                             return pair.second == entity;
			                             });

			if(it != c.end())
			{
				c.erase(it);
				std::make_heap(c.begin(), c.end(), this->comp);
				return true;
			}
			return false;
		}
	};

	DeathRow _DeathRow;
};
