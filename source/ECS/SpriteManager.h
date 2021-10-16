#pragma once

#include "Entity.h"

#include "../Renderer/RenderQueue.h"
#include "../Renderer/SpriteAtlas.h"


#include "TransformManager.h"

class AABB;

class SpriteManager
{
public:
	SpriteManager(const TransformManager& transManager, const EntityManager& entityManager, const SpriteAtlas& spriteAtlas, const int capacity);
	SpriteManager() = delete;

	void Create(Entity entity, SpriteID spriteID, RenderQueue::Layer layer, bool shouldRepeatAtEdges = true);

	void Render(RenderQueue& renderQueue) const;

	void Update(float deltaTime);

	void Clear();

private:
	const TransformManager& _TransManager;

	class SpriteCategory
	{
	public:
		SpriteCategory(const TransformManager& transManager, const EntityManager& entityManager, int capacity);
		SpriteCategory() = delete;

		void Allocate(int newCapacity);

		void Create(Entity entity, SpriteID spriteID, SpriteTransform trans);

		void Update(const SpriteAtlas& spriteAtlas, float deltaTime);

		void Render(RenderQueue& renderQueue) const;
		void RenderLooped(RenderQueue& renderQueue) const;

		void Clear();

	private:
		const TransformManager& _TransManager;
		const EntityManager& _EntityManager;

		int _Size = 0;
		int _AnimatedSize = 0;
		int _Capacity;

		void* _Buffer;
		Entity* _Entities;
		SpriteTransform* _Transforms;

		std::vector<float> _CurrentFrameTimes;
	};

	const SpriteAtlas& _SpriteAtlas;

	SpriteCategory _Repeating;
	SpriteCategory _NonRepeating;
};
