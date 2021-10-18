#pragma once

#include <vector>

#include <SDL_render.h>


#include "Camera.h"
#include "SpriteAtlas.h"
#include "SpriteID.h"

class OBB;
class AABB;
struct SpriteTransform;

class RenderQueue
{
public:
	RenderQueue(Renderer& renderer, Camera& camera, int screenWidth, int screenHeight);

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
		SDL_Texture* Tex;
		SDL_Rect SrcRect;
		SDL_Rect DstRect;
		float Angle;
		Layer Layer;
	};

	void Enqueue(SpriteID spriteID, float rotation, Layer layer);
	void EnqueueScreenSpace(SpriteID spriteID, const SDL_Rect& targetRect, float rotation, Layer layer);
	void EnqueueLooped(const SpriteTransform& transform);


	const SpriteAtlas& GetSpriteAtlas() const { return _SpriteAtlas; }
	const std::vector<Element>& GetRenderQueue();
	void Clear();

private:
	// helper functions
	void DrawAtTop(const SpriteTransform& transform, const AABB& screenAABB);
	void DrawAtBottom(const SpriteTransform& transform, const AABB& screenAABB);
	void DrawAtLeft(const SpriteTransform& transform, const AABB& screenAABB);
	void DrawAtRight(const SpriteTransform& transform, const AABB& screenAABB);

	Camera& _Camera;

	const int _GameFieldWidth;
	const int _GameFieldHeight;

	SpriteAtlas _SpriteAtlas;
	std::vector<Element> _RenderQueue;
};
