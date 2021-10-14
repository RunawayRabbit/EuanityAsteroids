#pragma once

#include <queue>
#include <functional>
#include <vector>
#include <algorithm>

class Timer
{
public:
	Timer() : deltaTime(0), time(0) {};

	void Update(float deltaTime);

	const float& Now() const { return time; };
	const float& DeltaTime() { return deltaTime; }
	void ExecuteDelayed(const float& seconds, std::function<void()> function);

private:
	float deltaTime;
	float time;

	class QueueCompare
	{
	public:
		bool operator()(std::pair<float, std::function<void()>>& a, std::pair<float, std::function<void()>>& b)
		{
			return a.first > b.first;
		}
	};

	std::priority_queue<std::pair<float, std::function<void()>>,
		std::vector<std::pair<float, std::function<void()>>>,
		QueueCompare> queuedCalls;


};