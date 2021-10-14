#pragma once

class MoveList
{
public:
	struct Entry
	{
		Rigidbody Rb;
		Vector2 Pos;

		bool operator==(const MoveList::Entry& Other) const
		{
			return (Rb.entity == Other.Rb.entity);
		}
	};

	struct ColliderRanges
	{
		std::vector<MoveList::Entry>::iterator ShipBegin;
		std::vector<MoveList::Entry>::iterator ShipEnd;
		std::vector<MoveList::Entry>::iterator BulletBegin;
		std::vector<MoveList::Entry>::iterator BulletEnd;
		std::vector<MoveList::Entry>::iterator LargeBegin;
		std::vector<MoveList::Entry>::iterator LargeEnd;
		std::vector<MoveList::Entry>::iterator MediumBegin;
		std::vector<MoveList::Entry>::iterator MediumEnd;
		std::vector<MoveList::Entry>::iterator SmallBegin;
		std::vector<MoveList::Entry>::iterator SmallEnd;
	};

	void Enqueue(const MoveList::Entry& entry)
	{
		_Data.push_back(entry);
		++_ColliderCounts[static_cast<int>(entry.Rb.colliderType)];
	}

	void Clear()
	{
		_Data.clear();
		for (auto& Count : _ColliderCounts)
			Count = 0;
	}

	size_t Size() const
	{
		return _Data.size();
	}

	// ReSharper disable once CppInconsistentNaming
	std::vector<MoveList::Entry>::iterator begin()
	{
		return _Data.begin();
	}
	// ReSharper disable once CppInconsistentNaming
	std::vector<MoveList::Entry>::iterator end()
	{
		return _Data.end();
	}
	int GetColliderCount(const ColliderType& ColliderType)
	{
		return _ColliderCounts[static_cast<int>(ColliderType)];
	}

private:
	std::vector<MoveList::Entry> _Data;
	std::array<int, static_cast<int>(ColliderType::COUNT)> _ColliderCounts = {};
};

namespace std {
	template<> struct hash<MoveList::Entry>
	{
		inline size_t operator()(const MoveList::Entry& entry) const noexcept
		{
			return entry.Rb.entity.Hash();
		}
	};
}
