#include <iostream>

#include "SpriteManager.h"


#include "EntityManager.h"
#include "../Renderer/Sprite.h"
#include "../Renderer/RenderQueue.h"
#include "../Renderer/SpriteTransform.h"


SpriteManager::SpriteManager(const TransformManager& transManager,
                             const EntityManager& entityManager,
                             const SpriteAtlas& spriteAtlas,
                             const int capacity)
	: _TransManager(transManager),
	  _SpriteAtlas(spriteAtlas),
	  _Repeating(transManager, entityManager, capacity),
	  _NonRepeating(transManager, entityManager, capacity),
	  _ScreenSpace(transManager, entityManager, capacity)
{
}

void
SpriteManager::Render(RenderQueue& renderQueue) const
{
	_Repeating.RenderLooped(renderQueue);
	_NonRepeating.RenderScreenSpace(renderQueue);
	_ScreenSpace.RenderScreenSpace(renderQueue);
}

void
SpriteManager::Create(const Entity entity,
                      const SpriteID spriteID,
                      const RenderQueue::Layer layer,
                      const float scale,
                      const RenderFlags renderFlags)
{
	//@TODO: These APIs aren't following the same conventions...

	auto transOpt = _TransManager.Get(entity);
	if(!transOpt.has_value())
	{
		assert(!"SpriteManager needs the Transform to already exist " +
			"before calling Create on an entity, please make sure you always make the transform first!");
	}

	const auto [TransPos, TransRot] = transOpt.value();

	const auto [id, tex, rect] = _SpriteAtlas.Get(spriteID);

	SpriteTransform spriteTransform;

	spriteTransform.ID = spriteID;

	const auto newWidth  = static_cast<int>(rect.w * scale);
	const auto newHeight = static_cast<int>(rect.h * scale);

	SDL_Rect transPosition;
	transPosition.x          = (static_cast<int>(TransPos.x) - newWidth / 2);
	transPosition.y          = (static_cast<int>(TransPos.y) - newHeight / 2);
	transPosition.w          = newWidth;
	transPosition.h          = newHeight;
	spriteTransform.Position = transPosition;
	spriteTransform.Rotation = TransRot;
	spriteTransform.Layer    = layer;

	Create(entity, spriteTransform, renderFlags);
}

void
SpriteManager::Create(const Entity entity, const SpriteTransform& spriteTransform, const RenderFlags renderFlags)
{
	switch(renderFlags)
	{
		case RenderFlags::FIXED:
		{
			_NonRepeating.Create(entity, spriteTransform.ID, spriteTransform);
			break;
		}

		case RenderFlags::REPEATING:
		{
			_Repeating.Create(entity, spriteTransform.ID, spriteTransform);
			break;
		}
		case RenderFlags::SCREEN_SPACE:
		{
			_ScreenSpace.Create(entity, spriteTransform.ID, spriteTransform);
			break;
		}
	}
}

void
SpriteManager::Update(const float deltaTime)
{
	_Repeating.Update(_SpriteAtlas, deltaTime);
	_NonRepeating.Update(_SpriteAtlas, deltaTime);
	_ScreenSpace.Update(_SpriteAtlas, deltaTime);
}

void
SpriteManager::Clear()
{
	_Repeating.Clear();
	_NonRepeating.Clear();
	_ScreenSpace.Clear();
}


// SPRITE CATEGORY

SpriteManager::SpriteCategory::SpriteCategory(const TransformManager& transManager, const EntityManager& entityManager, const int capacity)
	: _TransManager(transManager),
	  _EntityManager(entityManager)
{
	Allocate(capacity);
}

void
SpriteManager::SpriteCategory::Allocate(const int newCapacity)
{
	_Capacity = newCapacity;

	// Allocate new memory
	const auto elementSizeInBytes = sizeof(Entity) + sizeof(SpriteTransform);
	void* newBuffer               = new size_t[(elementSizeInBytes * newCapacity)];

	// Set up new pointers for where our data will go
	Entity* newEntities            = static_cast<Entity*>(newBuffer);
	SpriteTransform* newTransforms = reinterpret_cast<SpriteTransform*>(newEntities + newCapacity);

	if(_Size > 0)
	{
		// Copy the data to the new buffer
		memcpy(newEntities, _Entities, sizeof(Entity) * _Size);
		memcpy(newTransforms, _Transforms, sizeof(SpriteTransform) * _Size);
	}

	// Switch the pointers around
	_Entities   = newEntities;
	_Transforms = newTransforms;

	// Switch the buffers and free the old memory
	// ReSharper disable once CppDeletingVoidPointer
	delete _Buffer;
	_Buffer = newBuffer;
}


void
SpriteManager::SpriteCategory::RenderScreenSpace(RenderQueue& renderQueue) const
{
	for(auto i = 0; i < _Size; i++)
	{
		const SpriteTransform* transform = _Transforms + i;
		renderQueue.EnqueueScreenSpace(transform->ID, transform->Position, transform->Rotation, transform->Layer);
	}
}

void
SpriteManager::SpriteCategory::RenderLooped(RenderQueue& renderQueue) const
{
	for(auto i = 0; i < _Size; i++)
	{
		const SpriteTransform* transform = _Transforms + i;
		renderQueue.EnqueueLooped(*transform);
	}
}

void
SpriteManager::SpriteCategory::Clear()
{
	_Size         = 0;
	_AnimatedSize = 0;
}

void
SpriteManager::SpriteCategory::Create(const Entity entity, const SpriteID spriteID, const SpriteTransform trans)
{
	if(_Size == _Capacity)
	{
		// We're about to overrun our buffer, we gotta scale.
		Allocate(static_cast<size_t>(_Size * 2));
	}

	// Insert our data at the back of the data store
	*(_Entities + _Size)   = entity;
	*(_Transforms + _Size) = trans;

	if(SpriteAtlas::IsAnimated(spriteID))
	{
		// @TODO: This isn't the most efficient algorithm but I'm assuming the compiler will fix it..?
		std::swap(*(_Entities + _Size), *(_Entities + _CurrentFrameTimes.size()));
		std::swap(*(_Transforms + _Size), *(_Transforms + _CurrentFrameTimes.size()));

		_CurrentFrameTimes.push_back(SpriteAnimationData::FRAME_TIME[static_cast<int>(spriteID)]);
	}

	++_Size;
}

void
SpriteManager::SpriteCategory::Update(const SpriteAtlas& spriteAtlas, const float deltaTime)
{
	auto i = 0;
	while(i < _Size)
	{
		const auto entity = (_Entities + i);
		auto spriteTrans  = (_Transforms + i);

		auto transform = _TransManager.Get(*entity);
		if(_EntityManager.Exists(*entity) && transform.has_value())
		{
			if(SpriteAtlas::IsAnimated(spriteTrans->ID))
			{
				_CurrentFrameTimes[i] -= deltaTime;
				if(_CurrentFrameTimes[i] < 0.0f)
				{
					spriteTrans->ID = SpriteAnimationData::NEXT_FRAME_INDEX[static_cast<int>(spriteTrans->ID)];
					_CurrentFrameTimes[i] += SpriteAnimationData::FRAME_TIME[static_cast<int>(spriteTrans->ID)];

					const auto [spriteID, tex, src] = spriteAtlas.Get(spriteTrans->ID);
					spriteTrans->Position.w         = src.w;
					spriteTrans->Position.h         = src.h;
				}
			}

			spriteTrans->Rotation   = transform.value().rot;
			spriteTrans->Position.x = static_cast<int>(floor(transform.value().pos.x - static_cast<float>(spriteTrans->Position.w) / 2.0f));
			spriteTrans->Position.y = static_cast<int>(floor(transform.value().pos.y - static_cast<float>(spriteTrans->Position.h) / 2.0f));

			++i;
		}
		else
		{
			// Entity or Transform appears to have been deleted.
			// Do the swap to remove it from the list.

			const auto lastEntity    = _Entities + _Size - 1;
			const auto lastTransform = _Transforms + _Size - 1;

			size_t swapTarget = i;

			if(SpriteAtlas::IsAnimated(spriteTrans->ID))
			{
				// Maintain sorted order for animated sprites
				swapTarget = _CurrentFrameTimes.size() - 1;

				Entity* lastAnimatedEntity             = _Entities + swapTarget;
				SpriteTransform* lastAnimatedTransform = _Transforms + swapTarget;

				*(_Entities + i)   = *(lastAnimatedEntity);
				*(_Transforms + i) = *(lastAnimatedTransform);

				_CurrentFrameTimes[i] = _CurrentFrameTimes.back();
				_CurrentFrameTimes.pop_back();
			}

			*(_Entities + swapTarget)   = *(lastEntity);
			*(_Transforms + swapTarget) = *(lastTransform);

			--_Size;
		}
	}
}
