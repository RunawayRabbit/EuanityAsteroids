
#include <math.h> // for floor
#include <numeric> // for accumulate
#include <iostream> // For printing debug info
#include <string> // for std::to_string

#include <SDL.h>
#include <vector>

#include "FrameTimer.h"

FrameTimer::FrameTimer(float fps) :
	prefFrequencySeconds((float)SDL_GetPerformanceFrequency()),
	fixedDeltaTime(1000.0f / fps),
	fixedDeltaTimeTicks(static_cast<uint64_t>(static_cast<float>(Now().e) / fps))
{
	renderTimes = std::vector<float>(timerCount, 0.0f);
	updateTimes = std::vector<float>(timerCount, 0.0f);
}

Timestamp FrameTimer::Now()
{
	return { SDL_GetPerformanceCounter() };
}

float FrameTimer::GetSecondsElapsed(Timestamp start, Timestamp end = Now()) const
{
	return (float)(end.e - start.e) / prefFrequencySeconds;
}

// Estimated render time in miliseconds, based on the render times of the last 10 frames.
float FrameTimer::EstimatedRenderTime() const
{
	return 1000.0f * (std::accumulate(renderTimes.begin(), renderTimes.end(), 0.0f) / renderTimes.size());
}

float FrameTimer::EstimatedUpdateTime() const
{
	return 1000.0f * (std::accumulate(updateTimes.begin(), updateTimes.end(), 0.0f) / updateTimes.size());
}

void FrameTimer::PrintDebugStats() const
{
	static Timestamp lastPrint = Now();
	static int framesElapsed = 0;

	if(GetSecondsElapsed(lastPrint) > 1.0f)
	{
		Timestamp now = Now();

		std::string averageUpdateTimePerFrame =
			std::to_string(EstimatedUpdateTime());

		std::string averageRenderTimePerFrame =
			std::to_string(EstimatedRenderTime());

		std::cout << "Average Framerate: " << framesElapsed
			<< ".   Average Update Time: "
			<< averageUpdateTimePerFrame << " ms."
		<< "   Average Render Time: "
			<< averageRenderTimePerFrame << " ms.\n";

		lastPrint = now;
		framesElapsed = 0;
	}
	else
	{
		framesElapsed++;
	}
}

void FrameTimer::Sleep(Timestamp frameBegin) const
{
	float frameTime = GetSecondsElapsed(frameBegin) * 1000.0f;
	float renderTime = EstimatedRenderTime();

	int32_t sleepTime = static_cast<int32_t>(floor(fixedDeltaTime - renderTime - frameTime) - 1);


	if (sleepTime > 1)
	{
		SDL_Delay(sleepTime);
	}

	//spin for the rest
	float targetFrameTime = (fixedDeltaTime - renderTime) / 1000.0f;
	float epsilon = 0.0001f; // subtle adjustment to account for this loop's overhead.
	while (GetSecondsElapsed(frameBegin) < (targetFrameTime - epsilon));
}

void FrameTimer::UpdateEstimatedRenderTime(Timestamp renderBegin)
{
	nextRenderTimeIndex += 1;
	nextRenderTimeIndex %= renderTimes.size();

	renderTimes.at(nextRenderTimeIndex) = GetSecondsElapsed(renderBegin);
}

void FrameTimer::UpdateEstimatedUpdateTime(Timestamp updateBegin)
{
	nextUpdateTimeIndex += 1;
	nextUpdateTimeIndex %= updateTimes.size();

	updateTimes.at(nextUpdateTimeIndex) = GetSecondsElapsed(updateBegin);
}
