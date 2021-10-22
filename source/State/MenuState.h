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
	void SpawnShipSelectButtons();

private:
	void SpawnMenuButtons();
	void SpawnRandomAsteroids() const;

	Game& _Game;


	// Main Menu
	Entity _Title;
	Entity _StartButton;
	Entity _QuitButton;
	Entity _EuanityMeme;


	struct ShipSelect
	{
		Entity Background;
		Entity Text;
		Entity Ship;
	};

	// Ship Select
	ShipSelect _ShipSelect[3];
};
