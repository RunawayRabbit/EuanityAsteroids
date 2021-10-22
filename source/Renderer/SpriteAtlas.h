#pragma once

#include <SDL_render.h>
#include <vector>
#include <string>

#include "../Renderer/Sprite.h"
#include "../Renderer/SpriteID.h"

class Renderer;


class SpriteAtlas
{
public:
	SpriteAtlas(Renderer& renderer);
	SpriteAtlas() = delete;

	Sprite Get(SpriteID id) const
	{
		return _SpriteData[static_cast<int>(id)];
	}

	static bool IsAnimated(const SpriteID& id)
	{
		return static_cast<int>(id) < static_cast<int>(SpriteID::_END_ANIMATED);
	}

private:
	void CreateAnimatedSprites();
	void CreateRegularSprites();
	void CreateBackgroundSprites();
	void CreateMenuSprites();
	void CreateSprite(SpriteID id, int texIndex, int width, int height, int x, int y);

	void LoadPNGs(SDL_Renderer* renderer);
	static SDL_Texture* PNGToTexture(SDL_Renderer* renderer, std::string path);

	std::vector<SDL_Texture*> _LoadedImages;
	std::vector<Sprite> _SpriteData;
};
