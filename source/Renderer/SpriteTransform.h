#pragma once

#include <SDL_rect.h>
#include "SpriteID.h"
#include "RenderQueue.h"

struct SpriteTransform
{
	SpriteID id;
	SDL_Rect position;
	float rotation;
	RenderQueue::Layer layer;
};