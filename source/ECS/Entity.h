#pragma once

#include <stdint.h>
#include <string>

class Entity
{
public:
	//@NOTE: Change these to 32/64 if necessary
	typedef uint16_t EID;
	static const EID EID_MAX = UINT16_MAX;

	bool operator==(const Entity& other) const
	{
		return _ID == other._ID;
	}

	bool operator!=(const Entity& other) const
	{
		return _ID != other._ID;
	}

	size_t Hash() const {
		return std::hash<uint32_t>{}(_ID);
	}

	static Entity Null()
	{
		Entity retVal;
		retVal._ID = 0;
		return retVal;
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
		const auto copy = *this;
		++_ID;
		return copy;
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
