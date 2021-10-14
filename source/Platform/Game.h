#pragma once

#include <string>
#include <vector>

#include "../Renderer/Renderer.h"
#include "../Renderer/RenderQueue.h"
#include "../Renderer/BackgroundRenderer.h"

#include "../ECS/EntityManager.h"
#include "../ECS/TransformManager.h"
#include "../ECS/SpriteManager.h"
#include "../ECS/RigidbodyManager.h"
#include "../ECS/UIManager.h"

#include "../GameObject/Create.h"
#include "../GameObject/Player.h"

#include "../Physics/Physics.h"

#include "../Input/InputHandler.h"

#include "../Math/AABB.h"

#include "..\State\GameMode.h"
#include "..\State\Timer.h"
#include "..\State\MenuState.h"


class Game
{
public:
	Game(std::string windowName, int width, int height);
	Game() = delete;
	Game(Game&) = delete;

	~Game();

	bool IsRunning() const;

	void ProcessInput();
	void Update(float deltaTime);
	void Render();
	void Quit();

	//@NOTE: This has to be declared out here so that the state machine compilation units
	// can see the code and generate their templated versions. I don't like this one bit.
	template<typename T>
	void ChangeState()
	{
		currentState->OnExit();
		currentState = std::make_unique<T>(*this);
		currentState->OnEnter();
	}

	void GarbageCollection();

	// Renderer Stuff
	Renderer renderer;
	RenderQueue renderQueue;
	BackgroundRenderer backgroundRenderer;

	//Input
	InputHandler input;

	// GameObject Creator
	Create create;

	// IDK what this is honestly
	Timer time;

	// ECS Systems
	EntityManager entities;
	TransformManager xforms;
	SpriteManager sprites;
	UIManager UI;

	// Physics
	Physics physics;
	RigidbodyManager rigidbodies;
	const AABB gameField;

	// Gameplay
	std::unique_ptr<IState> currentState;

	bool isRunning;
};