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

#include "../Math/AABB.h"

#include "../State/Timer.h"
#include "../State/MenuState.h"


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
	void ChangeState(bool shouldResetSystems)
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
	const AABB GameField;

	// Gameplay
	std::unique_ptr<IState> CurrentState;

private:
	bool _IsRunning;
};
