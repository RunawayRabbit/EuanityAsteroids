#pragma once

#include <array>

#include "..\ECS\Entity.h"

#include "..\Renderer\SpriteAtlas.h"
#include "..\Renderer\SpriteTransform.h"

#include "..\Math\Vector2.h"
#include "..\Math\AABB.h"

class TransformManager;
class RenderQueue;

class BackgroundRenderer
{
public:
	BackgroundRenderer(const TransformManager& transformManager, const AABB& screen);
	void Render(RenderQueue& renderQueue, const float& deltaTime);

private:
	const TransformManager& transformManager;
	Entity playerShip;

	Vector2 offset;
	const AABB screen;

	SpriteTransform background;
	SpriteTransform parallax1;
	SpriteTransform parallax2;
	SpriteTransform parallax3;

	Vector2 position;

	static constexpr int backgroundSizeX = 2000;
	static constexpr int backgroundSizeY = 2000;
};