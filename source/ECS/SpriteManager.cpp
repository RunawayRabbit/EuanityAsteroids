
#include <iostream>

#include "SpriteManager.h"

#include "..\Renderer\Sprite.h"
#include "..\Renderer\RenderQueue.h"
#include "..\Renderer\SpriteTransform.h"

#include "..\Math\OBB.h"
#include "..\Math\AABB.h"


SpriteManager::SpriteManager(const TransformManager& transManager, const EntityManager& entityManager, const SpriteAtlas& spriteAtlas, const int capacity) :
	spriteAtlas(spriteAtlas),
	repeating(transManager, entityManager, capacity),
	nonRepeating(transManager, entityManager, capacity),
	transManager(transManager)
{}

void SpriteManager::Render(RenderQueue& renderQueue) const
{
	repeating.RenderLooped(renderQueue, renderQueue.screenWidth, renderQueue.screenHeight);
	nonRepeating.Render(renderQueue);
}

void SpriteManager::Create(const Entity entity, const SpriteID spriteID, const RenderQueue::Layer layer, const bool shouldRepeatAtEdges)
{
	//@TODO: These APIs aren't following the same conventions...

	// Get the date we need to work with.
	const auto [TransPos, TransRot] = transManager.Get(entity).value();

	const Sprite sprite = spriteAtlas.Get(spriteID);

	SpriteTransform spriteTransform;

	spriteTransform.id = spriteID;

	SDL_Rect transPosition;
	transPosition.x = ((int)TransPos.x - sprite.source.w / 2);
	transPosition.y = ((int)TransPos.y - sprite.source.h / 2);
	transPosition.w = sprite.source.w;
	transPosition.h = sprite.source.h;
	spriteTransform.position = transPosition;
	spriteTransform.rotation = TransRot;
	spriteTransform.layer = layer;

	// Pipe the call to the correct function.
	if (shouldRepeatAtEdges)
	{
		repeating.Create(entity, spriteID, spriteTransform);
	}
	else
	{
		nonRepeating.Create(entity, spriteID, spriteTransform);
	}
}

void SpriteManager::Update(const float deltaTime)
{
	repeating.Update(spriteAtlas, deltaTime);
	nonRepeating.Update(spriteAtlas, deltaTime);
}



// SPRITE CATEGORY

#pragma warning(push)
SpriteManager::SpriteCategory::SpriteCategory(const TransformManager& transManager, const EntityManager& entityManager, const int capacity) :
	transManager(transManager),
	entityManager(entityManager)
{
	Allocate(capacity);
}
#pragma warning(pop)


void SpriteManager::SpriteCategory::Allocate(const int newCapacity)
{
	capacity = newCapacity;

	// Allocate new memory
	const size_t elementSizeInBytes = sizeof(Entity) + sizeof(SpriteTransform);
	void* newBuffer = new size_t[(elementSizeInBytes * newCapacity)];

	// Set up new pointers for where our data will go
	Entity* newEntities = (Entity*)newBuffer;
	SpriteTransform* newTransforms = (SpriteTransform*)(newEntities + newCapacity);

	if (size > 0)
	{
		// Copy the data to the new buffer
		memcpy(newEntities, entities, sizeof(Entity) * size);
		memcpy(newTransforms, transforms, sizeof(SpriteTransform) * size);
	}

	// Switch the pointers around
	entities = newEntities;
	transforms = newTransforms;

	// Switch the buffers and free the old memory
	delete buffer;
	buffer = newBuffer;
}

void SpriteManager::SpriteCategory::Render(RenderQueue& renderQueue) const
{
	for (auto i = 0; i < size; i++)
	{
		const SpriteTransform* transform = transforms + i;
		renderQueue.Enqueue(transform->id, transform->position, transform->rotation, transform->layer);
	}
}

void SpriteManager::SpriteCategory::RenderLooped(RenderQueue& renderQueue, const int screenWidth, const int screenHeight) const
{
	AABB screenAABB(Vector2::zero(), Vector2((float)screenWidth, (float)screenHeight));

	for (auto i = 0; i < size; i++)
	{
		const SpriteTransform* transform = transforms + i;
		renderQueue.EnqueueLooped(*transform);
	}
}


void SpriteManager::SpriteCategory::Create(const Entity entity, const SpriteID spriteID, const SpriteTransform trans)
{
	if (size == capacity)
	{
		// We're about to overrun our buffer, we gotta scale.
		Allocate((size_t)(size * 2));
	}

	// Insert our data at the back of the data store
	*(entities + size) = entity;
	*(transforms + size) = trans;

	if (SpriteAtlas::isAnimated(spriteID))
	{
		// @TODO: This isn't the most efficient algorithm but I'm assuming the compiler will fix it..?
		std::swap(*(entities + size), *(entities + currentFrameTimes.size()));
		std::swap(*(transforms + size), *(transforms + currentFrameTimes.size()));

		currentFrameTimes.push_back(SpriteAnimationData::frameTime[(int)spriteID]);
	}

	++size;
}

void SpriteManager::SpriteCategory::Update(const SpriteAtlas& spriteAtlas, const float deltaTime)
{
	int i = 0;
	while(i < size)
	{
		const auto entity = (entities + i);
		SpriteTransform* spriteTrans = (transforms + i);

		auto transform = transManager.Get(*entity);
		if (entityManager.Exists(*entity) && transform.has_value())
		{
			if (SpriteAtlas::isAnimated(spriteTrans->id))
			{
				currentFrameTimes[i] -= deltaTime;
				if (currentFrameTimes[i] < 0.0f)
				{
					spriteTrans->id = SpriteAnimationData::nextFrameIndex[static_cast<int>(spriteTrans->id)];
					currentFrameTimes[i] += SpriteAnimationData::frameTime[static_cast<int>(spriteTrans->id)];

					const Sprite newSprite = spriteAtlas.Get(spriteTrans->id);
					spriteTrans->position.w = newSprite.source.w;
					spriteTrans->position.h = newSprite.source.h;
				}
			}

			spriteTrans->rotation   = transform.value().rot;
			spriteTrans->position.x = static_cast<int>(floor(transform.value().pos.x - static_cast<float>(spriteTrans->position.w) / 2.0f));
			spriteTrans->position.y = static_cast<int>(floor(transform.value().pos.y - static_cast<float>(spriteTrans->position.h) / 2.0f));

			++i;
		}
		else
		{
			// Entity or Transform appears to have been deleted.
			// Do the swap to remove it from the list.

			Entity* lastEntity = entities + size - 1;
			SpriteTransform* lastTransform = transforms + size - 1;

			size_t swapTarget = i;

			if (SpriteAtlas::isAnimated(spriteTrans->id))
			{
				// Maintain sorted order for animated sprites
				swapTarget = currentFrameTimes.size() - 1;

				Entity* lastAnimatedEntity = entities + swapTarget;
				SpriteTransform* lastAnimatedTransform = transforms + swapTarget;

				*(entities + i) = *(lastAnimatedEntity);
				*(transforms + i) = *(lastAnimatedTransform);

				currentFrameTimes[i] = currentFrameTimes.back();
				currentFrameTimes.pop_back();
			}

			*(entities + swapTarget) = *(lastEntity);
			*(transforms + swapTarget) = *(lastTransform);

			--size;
		}
	}
}
