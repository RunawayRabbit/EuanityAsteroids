
#include "UIManager.h"

#include "..\ECS\EntityManager.h"

#include "..\Input\InputBuffer.h"

#include "..\Renderer\RenderQueue.h"
#include "..\ECS\EntityManager.h"

#include <iostream>

UIManager::UIManager(EntityManager& entityManager, const InputBuffer& inputBuffer) :
	entityManager(entityManager),
	inputBuffer(inputBuffer),
	active(Entity::Null()),
	hot(Entity::Null())
{
}

void UIManager::Render(RenderQueue& renderQueue)
{
	for (auto& button : UIButtons)
	{
		if (DoButton(renderQueue, button))
		{
			button.callback();
			return;
		}

	}
}

bool UIManager::DoButton(RenderQueue& renderQueue, const UIButton& element)
{
	bool wasClicked = false;
	bool isActive = element.entity == active;
	bool weAreHot = false;

	if (element.box.Contains(inputBuffer.mousePos))
	{
		// Mouse is over us, so we mark ourselves as the hot entity.
		hot = element.entity;
		weAreHot = true;
	}

	if (isActive)
	{
		// We are the active UI element, meaning that we are holding ownership
		// over the context right now. We are responsible for freeing that
		// ownership should we need to.
		if (inputBuffer.Contains(InputOneShot::MouseUp))
		{
			// We had ownership and the mouse was lifted. Is it still on top of us?
			if (weAreHot)
				wasClicked = true;
			
			// Either way, we have to release the context now.
			active = Entity::Null();
		}
	}
	else if (active == Entity::Null() && weAreHot && inputBuffer.Contains(InputOneShot::MouseDown))
	{
		// Mouse is over us, it went down, and there is no active context. We can take ownership.
		active = element.entity;
	}


	DrawButton(renderQueue, element, weAreHot, isActive);
	return wasClicked;
}

void UIManager::DrawButton(RenderQueue& renderQueue, const UIButton& element, bool isHot, bool isActive)
{
	SDL_Rect targetRect{};
	targetRect.x = (int)(element.box.min.x);
	targetRect.y = (int)(element.box.min.y);
	targetRect.w = (int)(element.box.max.x - element.box.min.x);
	targetRect.h = (int)(element.box.max.y - element.box.min.y);
	renderQueue.Enqueue(element.spriteID, targetRect, 0, RenderQueue::Layer::UI);
}


void UIManager::MakeButton(const Entity& entity, const AABB& box, SpriteID spriteID, std::function<void()> callback)
{
	UIButtons.push_back(UIButton(entity, box, spriteID, callback));
}

void UIManager::GarbageCollect()
{
	UIButtons.erase(std::remove_if(UIButtons.begin(), UIButtons.end(),
		[&](UIButton& button) -> bool
		{
			return (!entityManager.Exists(button.entity));
		}), UIButtons.end()); // <-- Don't forget this bad boy here. C++ isn't user-friendly.
}