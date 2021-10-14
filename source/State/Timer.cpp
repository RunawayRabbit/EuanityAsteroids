
#include "Timer.h"

void Timer::Update(float deltaTime)
{
	this->deltaTime = deltaTime;
	this->time += deltaTime;

	while (queuedCalls.size() > 0)
	{
		auto& element = queuedCalls.top();
		if (element.first < Now())
		{
			element.second();
			queuedCalls.pop();
		}
		else
			return;
	}
}

void Timer::ExecuteDelayed(const float& seconds, std::function<void()> function)
{
	queuedCalls.push({ Now() + seconds, function });
}