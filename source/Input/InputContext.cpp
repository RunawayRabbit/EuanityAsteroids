
// Used to catch out-of-range from std::map.
#include <stdexcept> 

#include "InputContext.h"

InputContext::InputContext(bool fillWithDefaults)
{
	//@TODO: Input remapping.

	//DEFAULT INPUTS
	if(fillWithDefaults)
	{
		AddToggle(SDLK_w, InputToggle::MoveForward);
		AddToggle(SDLK_a, InputToggle::RotateLeft);
		AddToggle(SDLK_d, InputToggle::RotateRight);
	
		AddToggle(SDLK_q, InputToggle::StrafeLeft);
		AddToggle(SDLK_e, InputToggle::StrafeRight);

		AddToggle(SDLK_SPACE, InputToggle::Shoot);

		AddOneShot(SDL_EXT_MOUSE1_DOWN, InputOneShot::MouseDown);
		AddOneShot(SDL_EXT_MOUSE1_UP, InputOneShot::MouseUp);
	}
}

InputContext::~InputContext()
{
	//
}

InputOneShot InputContext::ContainsOneShot(const SDL_Keycode key) const
{
	auto it = _oneShots.find(key);
	if (it != _oneShots.end())
		return it->second;
	else
		return InputOneShot::NONE;
	
}

InputToggle InputContext::ContainsToggle(const SDL_Keycode key) const
{
	auto it = _toggles.find(key);
	if (it != _toggles.end())
		return it->second;
	else
		return InputToggle::NONE;
}


void InputContext::AddOneShot(const SDL_Keycode key, InputOneShot oneShot)
{
	_oneShots.insert(std::pair<SDL_Keycode, InputOneShot>(key, oneShot));
}
void InputContext::AddToggle(const SDL_Keycode key, InputToggle toggle)
{
	_toggles.insert(std::pair<SDL_Keycode, InputToggle>(key, toggle));
}