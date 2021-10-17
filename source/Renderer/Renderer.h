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

	void SetCameraLocation(const int& x, const int& y);

	void Render(const std::vector<RenderQueue::Element>& renderQueue) const;

	SDL_Renderer* GetRenderer() const { return _Renderer; } // @TODO: This should never be necessary, get rid of it!

private:
	static void ExitWithSDLError(std::string errorMessage);
	SDL_Window* _Window;
	SDL_Renderer* _Renderer;

	int _Width;
	int _Height;

	int _FocalPointX;
	int _FocalPointY;
};
