#pragma once

#include <string>

#include "../Renderer/Renderer.h"
#include "../Renderer/RenderQueue.h"
#include "../Renderer/BackgroundRenderer.h"

#include "../ECS/EntityManager.h"
#include "../ECS/TransformManager.h"
#include "../ECS/SpriteManager.h"
#include "../ECS/RigidbodyManager.h"
#include "../ECS/UIManager.h"

#include "../GameObject/Create.h"

#include "../Physics/Physics.h"

#include "../Input/InputHandler.h"

#include "../State/Timer.h"
#include "../State/MenuState.h"
#include "../State/GameplayState.h"


class Game
{
public:
	Game(std::string windowName, int windowWidth, int windowHeight, const Vector2& gameWorldDim);
	Game() = delete;
	Game(Game&) = delete;

	~Game();

	bool IsRunning() const;

	void ProcessInput();
	void HandleDebugInput(const InputBuffer& inputBuffer);
	void Update(float realDeltaTime);
	void Render();
	void Quit();

	//@NOTE: This has to be declared out here so that the state machine compilation units
	// can see the code and generate their templated versions. I don't like this one bit.
	template<typename T>
	void ChangeState(const bool shouldResetSystems)
	{
		CurrentState->OnExit();

		if(shouldResetSystems)
		{
			ResetAllSystems();
		}

		CurrentState = std::make_unique<T>(*this);
		CurrentState->OnEnter();
	}

	void GarbageCollection();
	void ResetAllSystems();

	Vector2 WrapToGameField(const Vector2& point) const;
	bool GameFieldContains(const Vector2& point) const;

	GameplayState GameState;

	// Camera Stuff
	bool IsDebugCamera;
	Camera GameCam;
	Camera DebugCam;

	// Renderer Stuff
	Renderer Renderer;
	RenderQueue RenderQueue;
	BackgroundRenderer BackgroundRenderer;

	//Input
	InputHandler Input;

	// GameObject Creator
	Create Create;

	// IDK what this is honestly
	Timer Time;

	// ECS Systems
	EntityManager Entities;
	TransformManager Xforms;
	SpriteManager Sprites;
	UIManager UI;

	// Physics
	Physics Physics;
	RigidbodyManager Rigidbodies;
	const Vector2 GameFieldDim;

	// Gameplay
	std::unique_ptr<IState> CurrentState;

private:
	bool _IsRunning;
	float _TimeFactor;
};
