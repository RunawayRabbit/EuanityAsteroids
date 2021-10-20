#include "BackgroundRenderer.h"

#include "RenderQueue.h"
#include "../Math/EuanityMath.h"


BackgroundRenderer::BackgroundRenderer(const Vector2Int screenDim)
	: _ScreenDim(screenDim)
{
	_Layers[0] = {
		SpriteID::STATIC_BACKGROUND,
        Vector2::Zero(),
        0.01f
    };

	_Layers[1] = {
		SpriteID::PARALLAX_BACKGROUND_3,
        Vector2::Zero(),
        0.06f
    };

	_Layers[2] = {
		SpriteID::PARALLAX_BACKGROUND_1,
        Vector2::Zero(),
        0.12f
    };

	_Layers[3] = {
		SpriteID::PARALLAX_BACKGROUND_2,
        Vector2::Zero(),
        0.18f
    };
}


void
BackgroundRenderer::DrawLayer(RenderQueue& renderQueue,
                              BackgroundLayer& layer,
                              const Vector2& delta)
{
	layer.Offset += -delta * layer.Speed;

	layer.Offset.x = Math::RepeatNeg(layer.Offset.x, BACKGROUND_SIZE_X);
	layer.Offset.y = Math::RepeatNeg(layer.Offset.y, BACKGROUND_SIZE_Y);

	SDL_Rect bgRect;
	bgRect.w = BACKGROUND_SIZE_X;
	bgRect.h = BACKGROUND_SIZE_Y;

	bgRect.x = static_cast<int>(layer.Offset.x);
	bgRect.y = static_cast<int>(layer.Offset.y);
	renderQueue.EnqueueScreenSpace(layer.SpriteID, bgRect, 0, RenderQueue::Layer::BACKGROUND);

	bgRect.x = static_cast<int>(layer.Offset.x);
	bgRect.y = static_cast<int>(layer.Offset.y) - BACKGROUND_SIZE_Y;
	renderQueue.EnqueueScreenSpace(layer.SpriteID, bgRect, 0, RenderQueue::Layer::BACKGROUND);

	bgRect.x = static_cast<int>(layer.Offset.x) - BACKGROUND_SIZE_X;
	bgRect.y = static_cast<int>(layer.Offset.y) - BACKGROUND_SIZE_Y;
	renderQueue.EnqueueScreenSpace(layer.SpriteID, bgRect, 0, RenderQueue::Layer::BACKGROUND);

	bgRect.x = static_cast<int>(layer.Offset.x) - BACKGROUND_SIZE_X;
	bgRect.y = static_cast<int>(layer.Offset.y);
	renderQueue.EnqueueScreenSpace(layer.SpriteID, bgRect, 0, RenderQueue::Layer::BACKGROUND);
}

void
BackgroundRenderer::Render(Camera& camera, RenderQueue& renderQueue, const float& deltaTime)
{
	const auto delta = camera.GetCameraVelocity() * deltaTime;

	for(auto& layer : _Layers)
	{
		DrawLayer(renderQueue, layer, delta);
	}
}
