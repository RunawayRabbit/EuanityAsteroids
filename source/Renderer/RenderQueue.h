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
	RenderQueue(Renderer& renderer, Camera& camera, const Vector2& gameWorldDim);

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

	void EnqueueBackground(SpriteID spriteID, float rotation, Layer layer);
	void EnqueueScreenSpace(SpriteID spriteID, const SDL_Rect& targetRect, float rotation, Layer layer);
	void EnqueueLooped(const SpriteTransform& transform);


	const SpriteAtlas& GetSpriteAtlas() const { return _SpriteAtlas; }
	const std::vector<Element>& GetRenderQueue();
	void CacheCameraInfo(Camera* cam);
	void Clear();

private:

	Camera* _Camera;

	Vector2 _GameWorldDim;

	SpriteAtlas _SpriteAtlas;
	std::vector<Element> _RenderQueue;

	AABB _CameraAABB;
	float _CameraScale;
};
