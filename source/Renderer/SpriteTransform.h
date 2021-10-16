#pragma once

#include <SDL_rect.h>
#include "SpriteID.h"
#include "RenderQueue.h"

struct SpriteTransform
{
	SpriteID ID;
	SDL_Rect Position;
	float Rotation;
	RenderQueue::Layer Layer;
};
