#include <iostream>

//#include <vld.h>

#include "Game.h"
#include "FrameTimer.h"

int
main(int /*argc*/, char* /*args[]*/)
{
	// Game Setup
	const std::string windowName = "Just Asteroids";

	const auto screenWidth  = 1600;
	const auto screenHeight = 900;

	const auto gameWorldDim = Vector2::One() * 2000.0f;
	Game game(windowName, screenWidth, screenHeight, gameWorldDim);

	// Frame Timer Setup
	const auto updatesPerSecond = 60;

	const auto frameTime = 1.0f / updatesPerSecond;
	FrameTimer timer(updatesPerSecond);

	while(game.IsRunning())
	{
		const auto updateBegin = timer.Now();

		game.ProcessInput();
		game.Update(frameTime);
		timer.UpdateEstimatedUpdateTime(updateBegin);

		// lock framerate
		timer.Sleep(updateBegin);

		const auto renderBegin = timer.Now();
		game.Render();
		timer.UpdateEstimatedRenderTime(renderBegin);

		timer.PrintDebugStats(); // Frame timing
	}

	return EXIT_SUCCESS;
}
