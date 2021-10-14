#pragma once

#include <string>
#include <SDL_render.h>

#include "RenderQueue.h"

class Renderer
{
public:
	Renderer(std::string windowName, int width, int height);
	Renderer() = delete;
	Renderer(Renderer&) = delete;
	~Renderer();

	void Render(const std::vector<RenderQueue::Element>& renderQueue);

	SDL_Renderer* GetRenderer() const { return renderer; } // @TODO: This should never be necessary, get rid of it!

private:
	void ExitWithSDLError(const std::string errorMessage) const;
	SDL_Window* window;
	SDL_Renderer* renderer;

	int width;
	int height;

};
