#pragma once

#include <SDL.h>
#include <iostream> // for error reporting

#include "Renderer.h"

Renderer::Renderer(const std::string windowName, const int width, const int height)
	: _Width(width),
	  _Height(height)
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
}

Renderer::~Renderer()
{
	SDL_DestroyRenderer(_Renderer);
	SDL_DestroyWindow(_Window);
}

Vector2Int
Renderer::GetWindowDim() const
{
	return Vector2Int{_Width, _Height};
}

Vector2
Renderer::GetWindowDimFloat() const
{
	return Vector2(static_cast<float>(_Width), static_cast<float>(_Height));
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
		const auto flip  = SDL_FLIP_NONE; // not yet supported
		SDL_Point* pivot = nullptr;       // not yet supported
		SDL_RenderCopyEx(_Renderer, tex, &srcRect, &dstRect, angle, pivot, flip);
	}

	SDL_RenderPresent(_Renderer);
}
