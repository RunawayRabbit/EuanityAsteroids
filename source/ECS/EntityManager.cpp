#include "EntityManager.h"
#include "../State/Timer.h"

//@TODO: Does this class GUARANTEE that it will never return or create a null entity implicitly?

EntityManager::EntityManager(const Timer& Time)
	: _Time(Time)
{
	// @NOTE: Entity.id == 0 is considered to be the null entity and is never available.

	_OpenRanges.push_back({ 1, Entity::ENTITYID_MAX });
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
EntityManager::Exists(const Entity Entity) const
{
	if(Entity._ID == 0)
		return false; // Hard-coded zero entity

	// Binary search ranges
	size_t Min = 0;
	auto Max   = _OpenRanges.size() - 1;

	while(true)
	{
		const auto i = (Min + Max) / 2;

		if(Entity._ID < _OpenRanges[i].Begin)
		{
			if(i == Min)
				return !_ZombieList.count(Entity._ID);
			Max = i - 1;
		}
		else if(Entity._ID > _OpenRanges[i].End)
		{
			if(i == Max)
				return !_ZombieList.count(Entity._ID);
			Min = i + 1;
		}
		else
		{
			return false;
		}
	}

	static_assert("Unreachable Code");
}

void
EntityManager::Destroy(const Entity Entity)
{
	Destroy(Entity._ID);
}

void
EntityManager::Destroy(const Entity::EID EntityEID)
{
	if(EntityEID == 0)
		return; // The nullentity cannot be destroyed! It is too POWERFULLLL!!!

	_ZombieList.insert(EntityEID);
}

void
EntityManager::DestroyZombies(const Entity::EID EntityEID)
{
	_DeathRow.remove(EntityEID);

	const Entity::EID EntityPlusOne  = EntityEID + 1;
	const Entity::EID EntityMinusOne = EntityEID - 1;

	for(auto Range = _OpenRanges.begin();
	    Range != _OpenRanges.end(); ++Range)
	{
		if(EntityEID < Range->Begin)
		{
			if(EntityPlusOne == Range->Begin)
			{
				--Range->Begin;
				return;
			}
			else
			{
				// We need to add a new range.
				_OpenRanges.insert(Range, { EntityEID, EntityEID });
				return;
			}
		}

		if(EntityMinusOne == Range->End)
		{
			// Extend the open range to include the destroyed entity
			++Range->End;
			const auto NextRange = Range + 1;
			if(Range->End + 1 == NextRange->Begin)
			{
				// Combine this range with the one above it
				Range->End = NextRange->End;
				_OpenRanges.erase(NextRange);
			}
			return;
		}

		if(EntityPlusOne == Range->Begin)
		{
			// Extend the open range to include the destroyed entity
			--Range->Begin;
			return;
		}
	}
}

void
EntityManager::DestroyDelayed(Entity Entity, const float& Seconds)
{
	_DeathRow.push({ _Time.Now() + Seconds, Entity._ID });
}

void
EntityManager::GarbageCollect()
{
	while(_DeathRow.size() > 0)
	{
		auto& element = _DeathRow.top();
		if(element.first < _Time.Now())
		{
			Destroy(element.second);
			_DeathRow.pop();
		}
		else
		{
			break;
		}
	}

	for(auto Zombie : _ZombieList)
	{
		DestroyZombies(Zombie);
	}

	_ZombieList.clear();
}

uint32_t
EntityManager::Count()
{
	uint32_t Count = _OpenRanges[0].Begin - 1;

	for(auto i = 1; i < _OpenRanges.size(); ++i)
	{
		auto Previous = _OpenRanges[i - 1];
		auto Current  = _OpenRanges[i];

		Count += Current.Begin - Previous.End - 1;
	}

	return Count;
}
