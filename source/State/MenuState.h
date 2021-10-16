#pragma once

#include <vector>

#include "IState.h"

#include "../GameObject/Create.h"
#include "../ECS/Entity.h"

class Game;

class MenuState final : public IState
{
public:
	explicit MenuState(Game& game);
	MenuState() = delete;

	void OnEnter() override;
	void OnExit() override;

	void Update(const InputBuffer& inputBuffer, const float& deltaTime) override;
	void Render() override;

private:
	void SpawnMenuButtons();
	void SpawnRandomAsteroids();

	Game& game;

	std::vector<Entity> _FloatingAsteroids;

	Entity _Title;

	Entity _StartButton;
	Entity _QuitButton;
	Entity _EuanityMeme;
};
