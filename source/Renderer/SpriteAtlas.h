#pragma once

#include <SDL_render.h>
#include <vector>
#include <string>

#include "..\Renderer\Sprite.h"
#include "..\Renderer\SpriteID.h"

class Renderer;


class SpriteAtlas
{
public:
	SpriteAtlas(Renderer& renderer);
	SpriteAtlas() = delete;

	inline const Sprite Get(SpriteID id) const
	{
		return spriteData[(int)id];
	}

	static inline bool isAnimated(const SpriteID& id)
	{
		return (int)id < (int)SpriteID::_END_ANIMATED;
	}

private:
	void CreateAnimatedSprites();
	void CreateRegularSprites();
	void CreateBackgroundSprites();
	void CreateMenuSprites();
	void CreateSprite(SpriteID id, int texIndex, int width, int height, int x, int y);

	void LoadPNGs(SDL_Renderer* renderer);
	SDL_Texture* PNGToTexture(SDL_Renderer* renderer, const std::string path) const;

	std::vector<SDL_Texture*> loadedImages;
	std::vector<Sprite> spriteData;
};