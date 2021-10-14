#pragma once

#include <SDL.h>
#include <iostream> // for error reporting

#include "Renderer.h"

Renderer::Renderer(std::string windowName, int width, int height) :
	width(width),
	height(height)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		ExitWithSDLError("Error initializing SDL");

	window = SDL_CreateWindow(windowName.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_SHOWN);
	if (!window)
		ExitWithSDLError("Error creating window");

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
		ExitWithSDLError("Error getting renderer from window");

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	// @TODO: How exactly do we use this as a render target?
	/*
	postprocess = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window),
		SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_SetTextureBlendMode(postprocess, SDL_BLENDMODE_ADD); */

}
Renderer::~Renderer()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

void Renderer::ExitWithSDLError(const std::string errorMessage) const
{
	std::cout << errorMessage << ": " << SDL_GetError() << std::endl;
	system("pause");
}

void Renderer::Render(const std::vector<RenderQueue::Element>& renderQueue)
{
	SDL_RenderClear(renderer);

	for (auto& el : renderQueue)
	{
		SDL_RendererFlip flip = SDL_FLIP_NONE; // not yet supported
		SDL_Point* pivot = nullptr; // not yet supported
		SDL_RenderCopyEx(renderer, el.tex, &el.srcRect, &el.dstRect, el.angle, pivot, flip);
	}

	SDL_RenderPresent(renderer);
}





#if 0
void Renderer::ApplyBloom()
{
	constexpr uint8_t lumiThreshold = 180;

	Color* pixels;
	int pitch;
	SDL_LockTexture(postprocess, NULL, (void**)&pixels, &pitch);
	SDL_RenderReadPixels(renderer, NULL, 0, pixels, pitch);


	// Converts image to greyscale based on lumi
	/* 
	for (auto i = 0; i < width*height; i++)
	{
		Color* pixel = pixels + i;
		if (pixel->value == 0) continue;

		uint8_t lumi = (uint8_t)((0.299f * pixel->red) + (0.587f * pixel->green) + (0.114f * pixel->blue));
		lumi = lumi < lumiThreshold ? 0 : lumi;

		pixel->value = lumi;
	}*/
		
	GaussianBlur(pixels, width, height, 10.0f);

	SDL_UnlockTexture(postprocess);
	SDL_RenderCopy(renderer, postprocess, NULL, NULL);
}
#endif

