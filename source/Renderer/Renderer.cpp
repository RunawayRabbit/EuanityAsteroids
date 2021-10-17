#pragma once

#include <SDL.h>
#include <iostream> // for error reporting

#include "Renderer.h"

Renderer::Renderer(const std::string windowName, const int width, const int height)
	: _Width(width),
	  _Height(height),
	  _FocalPointX(0),
	  _FocalPointY(0)
{
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		ExitWithSDLError("Error initializing SDL");

	_Window = SDL_CreateWindow(windowName.c_str(),
	                           SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                           width, height, SDL_WINDOW_SHOWN);
	if(!_Window)
		ExitWithSDLError("Error creating window");

	_Renderer = SDL_CreateRenderer(_Window, -1, SDL_RENDERER_ACCELERATED);
	if(!_Renderer)
		ExitWithSDLError("Error getting renderer from window");

	SDL_SetRenderDrawColor(_Renderer, 0, 0, 0, 255);

	// @TODO: How exactly do we use this as a render target?
	/*
	postprocess = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window),
		SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_SetTextureBlendMode(postprocess, SDL_BLENDMODE_ADD); */
}

Renderer::~Renderer()
{
	SDL_DestroyRenderer(_Renderer);
	SDL_DestroyWindow(_Window);
}

void
Renderer::ExitWithSDLError(const std::string errorMessage)
{
	std::cout << errorMessage << ": " << SDL_GetError() << std::endl;
	system("pause");
}

void
Renderer::Render(const std::vector<RenderQueue::Element>& renderQueue) const
{
	SDL_RenderClear(_Renderer);

	for(const auto& [tex, srcRect, dstRect, angle, layer] : renderQueue)
	{
		auto finalDstRect = dstRect;
		finalDstRect.x += _FocalPointX;
		finalDstRect.y += _FocalPointY;
		const auto flip  = SDL_FLIP_NONE; // not yet supported
		SDL_Point* pivot = nullptr;       // not yet supported
		SDL_RenderCopyEx(_Renderer, tex, &srcRect, &finalDstRect, angle, pivot, flip);
	}

	SDL_RenderPresent(_Renderer);
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
