#include "UIManager.h"

#include "../ECS/EntityManager.h"

#include "../Input/InputBuffer.h"

#include "../Renderer/RenderQueue.h"

UIManager::UIManager(EntityManager& entityManager, const InputBuffer& inputBuffer)
	: _EntityManager(entityManager),
	  _InputBuffer(inputBuffer),
	  _Active(Entity::Null()),
	  _Hot(Entity::Null())
{
}

void
UIManager::Render(RenderQueue& renderQueue)
{
	for(auto& button : _UIButtons)
	{
		if(DoButton(renderQueue, button))
		{
			button.Callback();
			return;
		}
	}
}

bool
UIManager::DoButton(RenderQueue& renderQueue, const UIButton& element)
{
	const auto isActive = element.Entity == _Active;
	auto wasClicked     = false;
	auto weAreHot       = false;

	if(element.Box.Contains(_InputBuffer.mousePos))
	{
		// Mouse is over us, so we mark ourselves as the hot entity.
		_Hot     = element.Entity;
		weAreHot = true;
	}

	if(isActive)
	{
		// We are the active UI element, meaning that we are holding ownership
		// over the context right now. We are responsible for freeing that
		// ownership should we need to.
		if(_InputBuffer.Contains(InputOneShot::MouseUp))
		{
			// We had ownership and the mouse was lifted. Is it still on top of us?
			if(weAreHot)
				wasClicked = true;

			// Either way, we have to release the context now.
			_Active = Entity::Null();
		}
	}
	else if(_Active == Entity::Null() && weAreHot && _InputBuffer.Contains(InputOneShot::MouseDown))
	{
		// Mouse is over us, it went down, and there is no active context. We can take ownership.
		_Active = element.Entity;
	}


	DrawButton(renderQueue, element, weAreHot, isActive);
	return wasClicked;
}

void
UIManager::DrawButton(RenderQueue& renderQueue, const UIButton& element, bool isHot, bool isActive)
{
	SDL_Rect targetRect;
	targetRect.x = static_cast<int>(element.Box.min.x);
	targetRect.y = static_cast<int>(element.Box.min.y);
	targetRect.w = static_cast<int>(element.Box.max.x - element.Box.min.x);
	targetRect.h = static_cast<int>(element.Box.max.y - element.Box.min.y);
	renderQueue.EnqueueScreenSpace(element.SpriteID, targetRect, 0, RenderQueue::Layer::UI);
}


void
UIManager::MakeButton(const Entity& entity, const AABB& box, const SpriteID spriteID, const std::function<void()> callback)
{
	_UIButtons.push_back(UIButton(entity, box, spriteID, callback));
}

void
UIManager::GarbageCollect()
{
	_UIButtons.erase(std::remove_if(_UIButtons.begin(), _UIButtons.end(),
	                                [&](UIButton& button) -> bool
	                                {
		                                return (!_EntityManager.Exists(button.Entity));
	                                }), _UIButtons.end()); // <-- Don't forget this bad boy here. C++ isn't user-friendly.
}

void
UIManager::Clear()
{
	_UIButtons.clear();
	_Active = Entity::Null();
	_Hot    = Entity::Null();
}
