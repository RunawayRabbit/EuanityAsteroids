#include <iostream>

//#include <vld.h>

#include "Game.h"
#include "FrameTimer.h"

int main(int argc, char* args[])
{
	// Game Setup
	std::string windowName = "Just Asteroids";
	const auto screenWidth = 800;
	const auto screenHeight = 600;
	Game game(windowName, screenWidth, screenHeight);

	// Frame Timer Setup
	const auto updatesPerSecond = 60;
	const auto frameTime = 1.0f / updatesPerSecond;
	FrameTimer timer(updatesPerSecond);

	while (game.IsRunning())
	{
		const auto UpdateBegin = timer.Now();

		game.ProcessInput();
		game.Update(frameTime);
		timer.UpdateEstimatedUpdateTime(UpdateBegin);

		// lock framerate
		timer.Sleep(UpdateBegin);

		const auto renderBegin = timer.Now();
		game.Render();
		timer.UpdateEstimatedRenderTime(renderBegin);

		//timer.PrintDebugStats(); // Frame timing
	}

	return EXIT_SUCCESS;
}
