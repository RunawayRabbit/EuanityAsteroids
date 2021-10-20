#pragma once

#include "../Math/Vector2.h"
#include "../Math/Vector2Int.h"
#include "Camera.h"

enum class SpriteID;
class RenderQueue;

class BackgroundRenderer
{
public:
	explicit BackgroundRenderer(Vector2Int screenDim);

	void Render(Camera& camera, RenderQueue& renderQueue, const float& deltaTime);

private:
	const Vector2Int _ScreenDim;

	struct BackgroundLayer
	{
		SpriteID SpriteID;
		Vector2 Offset;
		float Speed;
	};

	static void DrawLayer(RenderQueue& renderQueue, BackgroundLayer& layer, const Vector2& delta);

	BackgroundLayer _Layers[4];

	static constexpr int BACKGROUND_SIZE_X = 2000;
	static constexpr int BACKGROUND_SIZE_Y = 2000;
};
