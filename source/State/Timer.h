#pragma once

#include <queue>
#include <functional>
#include <vector>

class Timer
{
public:
	Timer() : _DeltaTime(0), _Time(0) {};

	void Update(float deltaTime);

	const float& Now() const { return _Time; };
	const float& DeltaTime() const { return _DeltaTime; }
	void ExecuteDelayed(const float& seconds, std::function<void()> function);

private:
	float _DeltaTime;
	float _Time;

	class QueueCompare
	{
	public:
		bool operator()(std::pair<float, std::function<void()>>& a, std::pair<float, std::function<void()>>& b) const
		{
			return a.first > b.first;
		}
	};

	std::priority_queue<std::pair<float, std::function<void()>>,
		std::vector<std::pair<float, std::function<void()>>>,
		QueueCompare> _QueuedCalls;


};
