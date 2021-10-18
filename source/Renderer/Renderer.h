#pragma once

#include <string>
#include <SDL_render.h>

#include "../Math/Vector2Int.h"
#include "RenderQueue.h"

class Renderer
{
public:
	Renderer(std::string windowName, int width, int height);
	Renderer() = delete;
	Renderer(Renderer&) = delete;
	~Renderer();

	void Render(const std::vector<RenderQueue::Element>& renderQueue) const;

	SDL_Renderer* GetRenderer() const { return _Renderer; } // @TODO: This should never be necessary, get rid of it!
	Vector2Int GetWindowDim() const;
	Vector2 GetWindowDimFloat() const;

private:
	static void ExitWithSDLError(std::string errorMessage);
	SDL_Window* _Window;
	SDL_Renderer* _Renderer;

	int _Width;
	int _Height;
};
