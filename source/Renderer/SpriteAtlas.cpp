
#include <iostream> // @TODO: We need a logging singleton to stop this shit from happening..

#include <SDL_image.h>

#include "SpriteAtlas.h"
#include "Sprite.h"
#include "SpriteID.h"

#include "Renderer.h"

SpriteAtlas::SpriteAtlas(Renderer& renderer) : 
	spriteData((int)SpriteID::COUNT)
{
	LoadPNGs(renderer.GetRenderer());

	//spriteData[(int)SpriteID::NONE]; // null data, default, nothing, nadda, zip.

	CreateAnimatedSprites();
	CreateRegularSprites();
	CreateBackgroundSprites();
	CreateMenuSprites();
}

void SpriteAtlas::CreateAnimatedSprites()
{
	// @TODO: This data is implicitly coupled with the sprite sheets and their
	// loading indices. In an ideal world, this data would be read from a data
	// file and not hard-coded in this way!

	CreateSprite(SpriteID::SHIP_TRAIL, 0, 8, 15, 76, 48);
	CreateSprite(SpriteID::SHIP_TRAIL_1, 0, 8, 15, 108, 48);
	CreateSprite(SpriteID::SHIP_TRAIL_2, 0, 8, 15, 140, 48);
	CreateSprite(SpriteID::SHIP_TRAIL_3, 0, 8, 15, 172, 48);

	CreateSprite(SpriteID::SMALL_EXPLOSION, 0, 6, 6, 93, 157);
	CreateSprite(SpriteID::SMALL_EXPLOSION_1, 0, 16, 18, 151, 151);

	CreateSprite(SpriteID::EXPLOSION, 0, 6, 6, 93, 157);
	CreateSprite(SpriteID::EXPLOSION_1, 0, 16, 18, 151, 151);
	CreateSprite(SpriteID::EXPLOSION_2, 0, 36, 34, 207, 144);
	CreateSprite(SpriteID::EXPLOSION_3, 0, 48, 52, 8, 197);

	CreateSprite(SpriteID::BULLET, 0, 4, 8, 6, 131);
	CreateSprite(SpriteID::BULLET_1, 0, 4, 8, 198, 72);
}

void SpriteAtlas::CreateRegularSprites()
{
	// @TODO: This data is implicitly coupled with the sprite sheets and their
	// loading indices. In an ideal world, this data would be read from a data
	// file and not hard-coded in this way!

	CreateSprite(SpriteID::SHIP, 0, 25, 23, 3, 34);
	CreateSprite(SpriteID::LARGE_ASTEROID, 0, 58, 61, 66, 194);
	
	CreateSprite(SpriteID::MEDIUM_ASTEROID_1, 0, 25, 27, 134, 196); // radius 31.14 / 2
	CreateSprite(SpriteID::MEDIUM_ASTEROID_2, 0, 26, 25, 161, 197); // radius 30.61 / 2
	CreateSprite(SpriteID::MEDIUM_ASTEROID_3, 0, 26, 26, 133, 225); // radius 29.73 / 2
	CreateSprite(SpriteID::MEDIUM_ASTEROID_4, 0, 27, 30, 161, 225); // 31.3 / 2

	CreateSprite(SpriteID::SMOL_ASTEROID_1,	0, 8, 10, 199, 197); // 12.81
	CreateSprite(SpriteID::SMOL_ASTEROID_2, 0, 14, 11, 209, 196); // 14.14
	CreateSprite(SpriteID::SMOL_ASTEROID_3, 0, 12, 10, 226, 197); // 12.81
	CreateSprite(SpriteID::SMOL_ASTEROID_4, 0, 10, 7, 241, 200);
	CreateSprite(SpriteID::SMOL_ASTEROID_5, 0, 8, 12, 198, 209);
	CreateSprite(SpriteID::SMOL_ASTEROID_6, 0, 14, 14, 209, 209);
	CreateSprite(SpriteID::SMOL_ASTEROID_7, 0, 14, 13, 225, 209);
	CreateSprite(SpriteID::SMOL_ASTEROID_8, 0, 10, 12, 241, 209);
	CreateSprite(SpriteID::SMOL_ASTEROID_9, 0, 10, 14, 197, 225);
	CreateSprite(SpriteID::SMOL_ASTEROID_10, 0, 12, 13, 209, 226);
	CreateSprite(SpriteID::SMOL_ASTEROID_11, 0, 14, 14, 255, 255);
	CreateSprite(SpriteID::SMOL_ASTEROID_12, 0, 10, 12, 241, 227);
	CreateSprite(SpriteID::SMOL_ASTEROID_13, 0, 6, 7, 201, 241); // 9.22, veri smol
	CreateSprite(SpriteID::SMOL_ASTEROID_14, 0, 14, 10, 209, 241);
	CreateSprite(SpriteID::SMOL_ASTEROID_15, 0, 13, 13, 226, 241);
	CreateSprite(SpriteID::SMOL_ASTEROID_16, 0, 10, 10, 241, 241);

	CreateSprite(SpriteID::MUZZLE_FLASH, 0, 6, 5, 133, 75);

}

void SpriteAtlas::CreateBackgroundSprites()
{
	CreateSprite(SpriteID::STATIC_BACKGROUND, 2, 2000, 2000, 0, 0);
	CreateSprite(SpriteID::PARALLAX_BACKGROUND_1, 3, 2000, 2000, 0, 0);
	CreateSprite(SpriteID::PARALLAX_BACKGROUND_2, 4, 2000, 2000, 0, 0);
	CreateSprite(SpriteID::PARALLAX_BACKGROUND_3, 5, 2000, 2000, 0, 0);
}

void SpriteAtlas::CreateMenuSprites()
{
	CreateSprite(SpriteID::MAIN_LOGO, 7, 623, 168, 0, 0);
	CreateSprite(SpriteID::SHITTY_LOGO, 1, 600, 186, 0, 0);
	CreateSprite(SpriteID::START_BUTTON, 6, 200, 80, 0, 0);
	CreateSprite(SpriteID::QUIT_BUTTON, 6, 200, 80, 0, 80);
	CreateSprite(SpriteID::GAME_OVER, 8, 200, 160, 0, 0);
}

void SpriteAtlas::CreateSprite(SpriteID id, int texIndex, int width, int height, int x, int y)
{
	Sprite sprite;
	sprite.id = id; //@TODO: Redundant?
	sprite.texture = loadedImages[texIndex];
	sprite.source.w = width;
	sprite.source.h = height;
	sprite.source.x = x;
	sprite.source.y = y;

	spriteData[(int)sprite.id] = sprite;
}

void SpriteAtlas::LoadPNGs(SDL_Renderer* renderer)
{
	// Perform ALL Image loading Here!
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
	{
		std::cout << ("Error initializing PNG extensions: ") << SDL_GetError();
	}

	loadedImages.push_back(PNGToTexture(renderer, "resources/asteroids-arcade.png")); //0 
	loadedImages.push_back(PNGToTexture(renderer, "resources/crappy_logo.png")); //1
	loadedImages.push_back(PNGToTexture(renderer, "resources/bkgd_0.png")); //2
	loadedImages.push_back(PNGToTexture(renderer, "resources/bkgd_6.png")); //3 
	loadedImages.push_back(PNGToTexture(renderer, "resources/bkgd_7.png")); //4 
	loadedImages.push_back(PNGToTexture(renderer, "resources/bkgd_2.png")); //5 
	loadedImages.push_back(PNGToTexture(renderer, "resources/MenuButtons.png")); //6
	loadedImages.push_back(PNGToTexture(renderer, "resources/MainLogo.png")); //7
	loadedImages.push_back(PNGToTexture(renderer, "resources/GameOver.png")); //8

	IMG_Quit(); // Shut down the image loading stuff, we don't need it anymore.
}

SDL_Texture* SpriteAtlas::PNGToTexture(SDL_Renderer* renderer, const std::string path) const
{
	SDL_Surface* surf = IMG_Load(path.c_str());

	if (!surf)
	{
		std::cout << ("Failed to load " + path + ".\n") << SDL_GetError();
	}

	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
	if (!tex)
	{
		SDL_FreeSurface(surf);
		SDL_DestroyTexture(tex);

		std::cout << ("Failed to convert " + path + " to a texture.\n") << SDL_GetError();
	}

	SDL_FreeSurface(surf);

	return tex;
}