

#include "..\GameObject\Create.h"

#include "..\Math\Math.h"

#include "..\Physics\Physics.h"

#include "Game.h"

Game::Game(std::string windowName, int width, int height) :
	entities(time),
	renderer(windowName, width, height),
	renderQueue(renderer, width, height),
	input(InputHandler(isRunning)),
	UI(entities, input.GetBuffer()),
	gameField(0.0f, (float)height, 0.0f, (float)width),
	xforms(2), //intial capacity. Can resize dynamically.
	backgroundRenderer(xforms, AABB(Vector2::zero(), Vector2((float)width, (float)height))),
	rigidbodies(entities, 2),
	sprites(xforms, entities, renderQueue.GetSpriteAtlas(), 128),
	create(*this, entities, xforms, sprites, rigidbodies, UI, time),
	physics(xforms, rigidbodies, AABB(Vector2::zero(), Vector2((float)width, (float)height))),
	isRunning(true)
{
	currentState = std::make_unique<MenuState>(*this);
	currentState->OnEnter();
}

Game::~Game()
{

}

bool Game::IsRunning() const
{
	return isRunning;
}

void Game::ProcessInput()
{
	input.Clear();
	input.ProcessInput();
}

void Game::Update(float deltaTime)
{
	time.Update(deltaTime);

	const InputBuffer& inputBuffer = input.GetBuffer();

	currentState->Update(inputBuffer, deltaTime);

	rigidbodies.EnqueueAll(physics, deltaTime);
	physics.Simulate(deltaTime);
	sprites.Update(deltaTime);

	GarbageCollection();

	//std::cout <<
	//	rigidbodies.Count() << " " <<
	//	entities.Count() << " " <<
	//	xforms.Count() << "\n";
}

void Game::Render()
{
	renderQueue.Clear();

	UI.Render(renderQueue);

	backgroundRenderer.Render(renderQueue, time.DeltaTime());

	sprites.Render(renderQueue);

	renderer.Render(renderQueue.GetRenderQueue());
}

void Game::Quit()
{
	isRunning = false;
}

void Game::GarbageCollection()
{
	// @TODO: This can be better.

	entities.GarbageCollect();
	xforms.GarbageCollect(entities);
	UI.GarbageCollect();
}
