#pragma once

class MoveList
{
public:
	struct Entry
	{
		Rigidbody Rb;
		Vector2 Pos;

		bool operator==(const Entry& other) const
		{
			return (Rb.entity == other.Rb.entity);
		}
	};

	struct ColliderRanges
	{
		std::vector<Entry>::iterator ShipBegin;
		std::vector<Entry>::iterator ShipEnd;
		std::vector<Entry>::iterator BulletBegin;
		std::vector<Entry>::iterator BulletEnd;
		std::vector<Entry>::iterator LargeBegin;
		std::vector<Entry>::iterator LargeEnd;
		std::vector<Entry>::iterator MediumBegin;
		std::vector<Entry>::iterator MediumEnd;
		std::vector<Entry>::iterator SmallBegin;
		std::vector<Entry>::iterator SmallEnd;
	};

	void Enqueue(const Entry& entry)
	{
		_Data.push_back(entry);
		++_ColliderCounts[static_cast<int>(entry.Rb.colliderType)];
	}

	void Clear()
	{
		_Data.clear();
		for(auto& count : _ColliderCounts)
			count = 0;
	}

	size_t Size() const
	{
		return _Data.size();
	}

	// ReSharper disable once CppInconsistentNaming
	std::vector<Entry>::iterator begin()
	{
		return _Data.begin();
	}

	// ReSharper disable once CppInconsistentNaming
	std::vector<Entry>::iterator end()
	{
		return _Data.end();
	}

	int GetColliderCount(const ColliderType& colliderType) const
	{
		return _ColliderCounts[static_cast<int>(colliderType)];
	}

	int GetColliderCountsInRange(ColliderType first, ColliderType last) const
	{
		int accumulator = 0;
		for(auto i = static_cast<int>(first); i <= static_cast<int>(last); ++i)
		{
			accumulator += _ColliderCounts[i];
		}

		return accumulator;
	}

private:
	std::vector<Entry> _Data;
	std::array<int, static_cast<int>(ColliderType::COUNT)> _ColliderCounts = {};
};

namespace std
{
template <> struct hash<MoveList::Entry>
{
	size_t operator()(const MoveList::Entry& entry) const noexcept
	{
		return entry.Rb.entity.Hash();
	}
};
}
