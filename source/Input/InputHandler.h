#pragma once

#include <string>
#include <map>
#include <functional>

#include <SDL_keycode.h>

#include "InputBuffer.h"
#include "InputContext.h"


class InputHandler;
class InputContext;
class Game;

typedef std::function<void(const InputBuffer& inputs)> InputCallback;

class InputHandler
{
public:
	InputHandler(bool& isRunning);
	InputHandler() = delete;
	// ~InputHandler();


	// @TODO: We ended up not using these. Do we still need them?
	void Dispatch() const;
	void AddCallback(InputCallback callback, int priority);

	const InputBuffer& GetBuffer() const;
	void Clear();

	void AddContext(const std::string name, InputContext context, bool activated = true);
	void ActivateContext(const std::string name);
	void DeactivateContext(const std::string name);

	void ProcessInput();

private:
	std::map<const std::string, InputContext> _inactive;
	std::map<const std::string, InputContext> _active;

	std::multimap<int, InputCallback> _callbacks;
	InputBuffer _currentBuffer;

	void ProcessKey(const SDL_Keycode key, const bool isDown, const bool wasDown);

	bool& isRunning;
};