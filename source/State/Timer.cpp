
#include "Timer.h"

void Timer::Update(const float deltaTime)
{
	this->_DeltaTime = deltaTime;
	this->_Time += deltaTime;

	while (_QueuedCalls.size() > 0)
	{
		const auto& [time, fn] = _QueuedCalls.top();
		if (time < Now())
		{
			fn();
			_QueuedCalls.pop();
		}
		else
			return;
	}
}

void Timer::ExecuteDelayed(const float& seconds, std::function<void()> function)
{
	_QueuedCalls.push({ Now() + seconds, function });
}
