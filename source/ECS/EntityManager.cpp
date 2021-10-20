#include "EntityManager.h"

#include <assert.h>

#include "../State/Timer.h"


//@TODO: Does this class GUARANTEE that it will never return or create a null entity implicitly?

EntityManager::EntityManager(const Timer& time)
	//@NOTE @IMPORTANT: It is unlikely that we'll destroy more than 1024 entities in two frames, but if we do, bump this number!
	: ZombieList(1024),
	  _Time(time)
{
	// @NOTE: Entity.id == 0 is considered to be the null entity and is never available.

	_OpenRanges.push_back({ 1, Entity::EID_MAX });
}

Entity
EntityManager::Create()
{
	if(_OpenRanges[0].Begin <= _OpenRanges[0].End)
	{
		Entity RetVal;
		RetVal._ID = _OpenRanges[0].Begin++;
		if(_OpenRanges[0].Begin > _OpenRanges[0].End && _OpenRanges.size() > 1)
		{
			// We've outrun this range, progress to the next.
			_OpenRanges.erase(_OpenRanges.begin());
		}

		return RetVal;
	}

	static_assert("We have overrun our entity limit!");
	return Entity::Null();
}

bool
EntityManager::Exists(const Entity entity) const
{
	if(entity._ID == 0)
		return false; // Hard-coded zero entity

	// Binary search ranges
	size_t min = 0;
	auto max   = _OpenRanges.size() - 1;

	while(true)
	{
		const auto i = (min + max) / 2;

		if(entity._ID < _OpenRanges[i].Begin)
		{
			if(i == min)
				return !ZombieList.Contains(entity);
			max = i - 1;
		}
		else if(entity._ID > _OpenRanges[i].End)
		{
			if(i == max)
				return !ZombieList.Contains(entity);
			min = i + 1;
		}
		else
		{
			return false;
		}
	}
}

void
EntityManager::Destroy(const Entity entity)
{
	if(entity == Entity::Null())
		return; // The null entity cannot be destroyed! It is too POWERFULLLL!!!

	if(!ZombieList.Contains(entity))
	{
		++_ZombieCount;
		ZombieList.Enqueue(entity);
		assert(!ZombieList.IsFull());
	}
	else
	{
		//std::cout << "Attempting to destroy zombie entity " << entity.ToString() << "!\n";
	}
}


void
EntityManager::DestroyZombies(const Entity entity)
{
	_DeathRow.remove(entity);

	const Entity::EID entityEID      = entity._ID;
	const Entity::EID entityPlusOne  = entityEID + 1;
	const Entity::EID entityMinusOne = entityEID - 1;

	for(auto Range = _OpenRanges.begin();
	    Range != _OpenRanges.end(); ++Range)
	{
		if(entityEID < Range->Begin)
		{
			if(entityPlusOne == Range->Begin)
			{
				--Range->Begin;
				return;
			}
			else
			{
				// We need to add a new range.
				_OpenRanges.insert(Range, { entityEID, entityEID });
				return;
			}
		}

		if(entityMinusOne == Range->End)
		{
			// Extend the open range to include the destroyed entity
			++Range->End;
			const auto nextRange = Range + 1;
			if(Range->End + 1 == nextRange->Begin)
			{
				// Combine this range with the one above it
				Range->End = nextRange->End;
				_OpenRanges.erase(nextRange);
			}
			return;
		}

		if(entityPlusOne == Range->Begin)
		{
			// Extend the open range to include the destroyed entity
			--Range->Begin;
			return;
		}
	}
}

void
EntityManager::DestroyDelayed(Entity entity, const float& seconds)
{
	_DeathRow.push({ _Time.Now() + seconds, entity });
}

void
EntityManager::GarbageCollect()
{
	// There are two concepts at play for entity deletion.
	// DeathRow is a list of entities and timers. When the timers expire, the entities will be Destroy()ed.

	while(_DeathRow.size() > 0)
	{
		const auto& [deathTime, entity] = _DeathRow.top();
		if(deathTime < _Time.Now())
		{
			// std::cout << "Killing Entity " << entity.ToString() << " on Death Row.\n";

			Destroy(entity);
			_DeathRow.pop();
		}
		else
		{
			break;
		}
	}

	// Destroy() adds those entities to the ZombieList, where they will be stored for one frame. Entities on the
	// ZombieList return false if queried via EntityManager::Exists() which gives the ECS managers one frame to evict
	// the data related to that entity before the entityID is returned to the pool, ready to be reused.

	for(uint16_t i = 0; i < _PrevZombieCount; ++i)
	{
		DestroyZombies(ZombieList.Dequeue().value());
	}
	_PrevZombieCount = _ZombieCount;
	_ZombieCount     = 0;
}

uint32_t
EntityManager::Count()
{
	uint32_t count = _OpenRanges[0].Begin - 1;

	for(auto i = 1; i < _OpenRanges.size(); ++i)
	{
		const auto previous = _OpenRanges[i - 1];
		const auto current  = _OpenRanges[i];

		count += current.Begin - previous.End - 1;
	}

	return count;
}

void
EntityManager::Clear()
{
	ZombieList.Clear();

	//@TODO: Pretty clear that this range-based storage system needs to be it's own class.
	_OpenRanges.clear();
	_OpenRanges.push_back({ 1, Entity::EID_MAX });

	_DeathRow = DeathRow();
}
