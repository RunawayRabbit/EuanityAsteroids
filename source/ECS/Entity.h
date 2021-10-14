#pragma once

#include <stdint.h>
#include <string>

class Entity
{
public:
	//@NOTE: Change these to 32/64 if necessary
	typedef uint16_t EID;
	static const EID ENTITYID_MAX = UINT16_MAX;

	bool operator==(const Entity& Other) const
	{
		return _ID == Other._ID;
	}

	bool operator!=(const Entity& Other) const
	{
		return _ID != Other._ID;
	}

	size_t Hash() const {
		return std::hash<uint32_t>{}(_ID);
	}

	static Entity Null()
	{
		Entity RetVal;
		RetVal._ID = 0;
		return RetVal;
	}

	std::string ToString() const
	{
		return std::to_string(_ID);
	}

	bool operator<(const Entity& Other) const
	{
		return this->_ID < Other._ID;
	}

private:
	
	EID _ID;
	Entity& operator++()
	{
		++_ID;
		return *this;
	}

	Entity operator++(int)
	{
		const auto Copy = *this;
		++_ID;
		return Copy;
	}
	friend class EntityManager;
};

namespace std {
	template<>
	struct hash<Entity> {
		size_t operator()(const Entity& Entity) const noexcept
		{
			return Entity.Hash();
		}
	};
}
