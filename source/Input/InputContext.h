#pragma once


#include <map>
#include <SDL_keycode.h>

#include "InputDefinitions.h"

// Hack to add mouse stuff to SDL's keycodes. Saves me a lot of work.
// SDL maps it's keycodes to chars, but skips all uppercase letters.
// We take advantage of that here.
enum SDL_KeyCode_Extensions
{
	SDL_EXT_MOUSE1_DOWN = 'A',
	SDL_EXT_MOUSE1_UP = 'B',
	SDL_EXT_MOUSE2_DOWN = 'C',
	SDL_EXT_MOUSE2_UP = 'D',
};

class InputContext
{
public:
	InputContext() = delete;
	InputContext(bool fillWithDefaults);
	~InputContext();

	InputOneShot ContainsOneShot(const SDL_Keycode key) const;
	InputToggle ContainsToggle(const SDL_Keycode key) const;

	void AddOneShot(const SDL_Keycode key, InputOneShot oneShot);
	void AddToggle(const SDL_Keycode key, InputToggle toggle);

private:
	std::map<SDL_Keycode, InputOneShot> _oneShots;
	std::map<SDL_Keycode, InputToggle> _toggles;
};