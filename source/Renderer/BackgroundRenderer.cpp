#include "BackgroundRenderer.h"
#include "RenderQueue.h"
#include "..\ECS\SpriteManager.h"

BackgroundRenderer::BackgroundRenderer(const TransformManager& transformManager, const AABB& screen)
	: transformManager(transformManager),
	  playerShip(Entity::Null()),
	  screen(screen),
	  position(0.0f, 0.0f)
{
	SDL_Rect screenRect {};
	screenRect.x = 0;
	screenRect.y = 0;
	screenRect.w = static_cast<int>(screen.max.x);
	screenRect.h = static_cast<int>(screen.max.y);

	// Background
	background.ID       = SpriteID::STATIC_BACKGROUND;
	background.Layer    = RenderQueue::Layer::BACKGROUND;
	background.Rotation = 0;
	background.Position = screenRect;

	// Parallax 1
	parallax1.ID       = SpriteID::PARALLAX_BACKGROUND_1;
	parallax1.Layer    = RenderQueue::Layer::PARALLAX;
	parallax1.Rotation = 0;
	parallax1.Position = screenRect;

	// Parallax 2
	parallax2.ID       = SpriteID::PARALLAX_BACKGROUND_2;
	parallax2.Layer    = RenderQueue::Layer::PARALLAX;
	parallax2.Rotation = 0;
	parallax2.Position = screenRect;

	// Parallax 3
	parallax3.ID       = SpriteID::PARALLAX_BACKGROUND_3;
	parallax3.Layer    = RenderQueue::Layer::PARALLAX;
	parallax3.Rotation = 0;
	parallax3.Position = screenRect;
}

void
BackgroundRenderer::Render(RenderQueue& renderQueue, const float& deltaTime)
{
	// Static background
	renderQueue.Enqueue(SpriteID::STATIC_BACKGROUND, 0, RenderQueue::Layer::BACKGROUND);

	position.x += 20.0f * deltaTime;
	//position.y += 10.0f * deltaTime; // Corner rendering bugs. This is unfortunately the easiest fix..

	if(playerShip == Entity::Null())
	{
		parallax1.Position.x = static_cast<int>(floor(position.x));
		if(parallax1.Position.x < 0)
			parallax1.Position.x += static_cast<int>(screen.max.x);
		else if(parallax1.Position.x > screen.max.x)
			parallax1.Position.x -= static_cast<int>(screen.max.x);

		parallax1.Position.y = static_cast<int>(floor(position.y));
		if(parallax1.Position.y < 0)
			parallax1.Position.y += static_cast<int>(screen.max.y);
		else if(parallax1.Position.y > screen.max.y)
			parallax1.Position.y -= static_cast<int>(screen.max.y);


		parallax2.Position.x = static_cast<int>(floor(position.x / 2));
		if(parallax2.Position.x < 0)
			parallax2.Position.x += static_cast<int>(screen.max.x);
		else if(parallax2.Position.x > screen.max.x)
			parallax2.Position.x -= static_cast<int>(screen.max.x);

		parallax2.Position.y = static_cast<int>(floor(position.y / 2));
		if(parallax2.Position.y < 0)
			parallax2.Position.y += static_cast<int>(screen.max.y);
		else if(parallax2.Position.y > screen.max.y)
			parallax2.Position.y -= static_cast<int>(screen.max.y);


		parallax3.Position.x = static_cast<int>(floor(position.x / 4));
		if(parallax3.Position.x < 0)
			parallax3.Position.x += static_cast<int>(screen.max.x);
		else if(parallax3.Position.x > screen.max.x)
			parallax3.Position.x -= static_cast<int>(screen.max.x);

		parallax3.Position.y = static_cast<int>(floor(position.y / 4));
		if(parallax3.Position.y < 0)
			parallax3.Position.y += static_cast<int>(screen.max.y);
		else if(parallax3.Position.y > screen.max.y)
			parallax3.Position.y -= static_cast<int>(screen.max.y);


		background.Position.x = static_cast<int>(floor(position.x / 5));
		if(background.Position.x < 0)
			background.Position.x += static_cast<int>(screen.max.x);
		else if(background.Position.x > screen.max.x)
			background.Position.x -= static_cast<int>(screen.max.x);

		background.Position.y = static_cast<int>(floor(position.y / 5));
		if(background.Position.y < 0)
			background.Position.y += static_cast<int>(screen.max.y);
		else if(background.Position.y > screen.max.y)
			background.Position.y -= static_cast<int>(screen.max.y);
	}

	renderQueue.EnqueueLooped(background);
	renderQueue.EnqueueLooped(parallax1);
	renderQueue.EnqueueLooped(parallax2);
	renderQueue.EnqueueLooped(parallax3);
}
