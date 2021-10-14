#pragma once

#include <functional>


#include "Entity.h"
#include "..\Math\AABB.h"

#include "..\Renderer\SpriteID.h"

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

private:
	struct UIButton
	{
		UIButton(const Entity& entity, const AABB& box, SpriteID spriteID, std::function<void()> callback) :
			entity(entity),
			box(box),
			spriteID(spriteID),
			callback(callback)
		{};

		Entity entity;
		AABB box;
		SpriteID spriteID;
		std::function<void()> callback;
	};

	bool DoButton(RenderQueue& renderQueue, const UIButton& element);
	void DrawButton(RenderQueue& renderQueue, const UIButton& element, bool isHot, bool isActive);
	Entity hot;
	Entity active;

	std::vector<UIButton> UIButtons;

	const InputBuffer& inputBuffer;
	EntityManager& entityManager;
};