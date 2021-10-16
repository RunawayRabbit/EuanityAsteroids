#include <math.h> // for floor
#include <numeric> // for accumulate
#include <iostream> // For printing debug info
#include <string> // for std::to_string

#include <SDL.h>
#include <vector>

#include "FrameTimer.h"

FrameTimer::FrameTimer(const float fps)
	: nextRenderTimeIndex(0),
	  nextUpdateTimeIndex(0),
	  prefFrequencySeconds(static_cast<float>(SDL_GetPerformanceFrequency())),
	  fixedDeltaTime(1000.0f / fps),
	  fixedDeltaTimeTicks(static_cast<uint64_t>(static_cast<float>(Now().e) / fps))
{
	renderTimes = std::vector<float>(timerCount, 0.0f);
	updateTimes = std::vector<float>(timerCount, 0.0f);
}

Timestamp
FrameTimer::Now()
{
	return { SDL_GetPerformanceCounter() };
}

float
FrameTimer::GetSecondsElapsed(Timestamp start, Timestamp end = Now()) const
{
	return static_cast<float>(end.e - start.e) / prefFrequencySeconds;
}

// Estimated render time in milliseconds, based on the render times of the last 10 frames.
float
FrameTimer::EstimatedRenderTime() const
{
	return 1000.0f * (std::accumulate(renderTimes.begin(), renderTimes.end(), 0.0f) / renderTimes.size());
}

float
FrameTimer::EstimatedUpdateTime() const
{
	return 1000.0f * (std::accumulate(updateTimes.begin(), updateTimes.end(), 0.0f) / updateTimes.size());
}

void
FrameTimer::PrintDebugStats() const
{
	static auto lastPrint = Now();

	static auto framesElapsed = 0;

	if(GetSecondsElapsed(lastPrint) > 1.0f)
	{
		const auto now = Now();

		const auto averageUpdateTimePerFrame =
			std::to_string(EstimatedUpdateTime());

		const auto averageRenderTimePerFrame =
			std::to_string(EstimatedRenderTime());

		std::cout << "Average Framerate: " << framesElapsed
			<< ".   Average Update Time: "
			<< averageUpdateTimePerFrame << " ms."
			<< "   Average Render Time: "
			<< averageRenderTimePerFrame << " ms.\n";

		lastPrint     = now;
		framesElapsed = 0;
	}
	else
	{
		framesElapsed++;
	}
}

void
FrameTimer::Sleep(const Timestamp frameBegin) const
{
	const auto frameTime  = GetSecondsElapsed(frameBegin) * 1000.0f;
	const auto renderTime = EstimatedRenderTime();

	const auto sleepTime = static_cast<int32_t>(floor(fixedDeltaTime - renderTime - frameTime) - 1);


	if(sleepTime > 1)
	{
		SDL_Delay(sleepTime);
	}

	//spin for the rest
	const auto targetFrameTime = (fixedDeltaTime - renderTime) / 1000.0f;
	const auto epsilon         = 0.0001f; // subtle adjustment to account for this loop's overhead.
	while(GetSecondsElapsed(frameBegin) < (targetFrameTime - epsilon));
}

void
FrameTimer::UpdateEstimatedRenderTime(Timestamp renderBegin)
{
	nextRenderTimeIndex += 1;
	nextRenderTimeIndex %= renderTimes.size();

	renderTimes.at(nextRenderTimeIndex) = GetSecondsElapsed(renderBegin);
}

void
FrameTimer::UpdateEstimatedUpdateTime(Timestamp updateBegin)
{
	nextUpdateTimeIndex += 1;
	nextUpdateTimeIndex %= updateTimes.size();

	updateTimes.at(nextUpdateTimeIndex) = GetSecondsElapsed(updateBegin);
}
