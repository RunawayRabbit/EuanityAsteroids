#pragma once

#include <functional>

#include "Entity.h"
#include "../Math/AABB.h"
#include "../Renderer/SpriteID.h"

struct InputBuffer;
class RenderQueue;
class EntityManager;

class UIManager
{
public:
	UIManager(EntityManager& entityManager, const InputBuffer& inputBuffer);

	void MakeButton(const Entity& entity, const AABB& box, SpriteID spriteID, std::function<void()> callback);

	void Render(RenderQueue& renderQueue);

	void GarbageCollect();
	void Clear();

private:
	struct UIButton
	{
		// ReSharper disable once CppParameterMayBeConst
		UIButton(const Entity& entity, const AABB& box, SpriteID spriteID, const std::function<void()> callback)
			: Entity(entity),
			  Box(box),
			  SpriteID(spriteID),
			  Callback(callback)
		{
		};

		Entity Entity;
		AABB Box;
		SpriteID SpriteID;
		std::function<void()> Callback;
	};

	bool DoButton(RenderQueue& renderQueue, const UIButton& element);
	static void DrawButton(RenderQueue& renderQueue, const UIButton& element, bool isHot, bool isActive);
	Entity _Hot;
	Entity _Active;

	std::vector<UIButton> _UIButtons;

	const InputBuffer& _InputBuffer;
	EntityManager& _EntityManager;
};
