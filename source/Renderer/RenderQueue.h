#pragma once

#include <vector>

#include <SDL_render.h>

#include "SpriteAtlas.h"
#include "SpriteID.h"

class OBB;
class AABB;
struct SpriteTransform;

class RenderQueue
{
public:
	RenderQueue(Renderer& renderer, int screenWidth, int screenHeight);

	enum class Layer
	{
		BACKGROUND,
		PARALLAX,
		DEFAULT,
		PARTICLE,
		UI,

		COUNT
	};

	struct Element
	{
		SDL_Texture* tex;
		SDL_Rect srcRect;
		SDL_Rect dstRect;
		float angle;
		RenderQueue::Layer layer;
	};

	const int screenWidth;
	const int screenHeight;

	void Enqueue(SpriteID spriteID, const SDL_Rect& targetRect, const float rotation, const RenderQueue::Layer layer);
	void Enqueue(SpriteID spriteID, const float rotation, const RenderQueue::Layer layer);
	void EnqueueLooped(const SpriteTransform& transform);

	const SpriteAtlas& GetSpriteAtlas() { return spriteAtlas; }
	const std::vector<RenderQueue::Element>& GetRenderQueue();
	void Clear();

private:
	// helper functions
	void DrawAtTop(const SpriteTransform& transform, const AABB& screenAABB);
	void DrawAtBottom(const SpriteTransform& transform, const AABB& screenAABB);
	void DrawAtLeft(const SpriteTransform& transform, const AABB& screenAABB);
	void DrawAtRight(const SpriteTransform& transform, const AABB& screenAABB);

	SpriteAtlas spriteAtlas;
	std::vector<RenderQueue::Element> renderQueue;
};
