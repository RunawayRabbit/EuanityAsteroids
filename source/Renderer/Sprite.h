#pragma once

#include <SDL_render.h>

#include "spriteID.h"

struct Sprite
{
	SpriteID id;
	SDL_Texture* texture;
	SDL_Rect source;
};