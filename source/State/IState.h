
#pragma once

struct InputBuffer;

class IState
{
public:
	IState() {};
	virtual ~IState() {};

	virtual void OnEnter() = 0;
	virtual void OnExit() = 0;
	virtual void Update(const InputBuffer& inputBuffer, const float& deltaTime) = 0;
	virtual void Render() = 0;
};