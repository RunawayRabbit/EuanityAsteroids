#pragma once

#include <set>
#include <map>

#include "..\Math\Vector2.h"

#include "InputDefinitions.h"

struct InputBuffer
{
	std::set<InputOneShot> oneShots;
	std::set<InputToggle> toggles;

	Vector2 mousePos;

	inline bool Contains(const InputToggle& toggle) const
	{
		return toggles.find(toggle) != toggles.end();
	}

	inline bool Contains(const InputOneShot& oneShot) const
	{
		return oneShots.find(oneShot) != oneShots.end();
	}
};