

#include "..\GameObject\Create.h"

#include "..\Math\Math.h"

#include "..\Physics\Physics.h"

#include "Game.h"

Game::Game(std::string windowName, int width, int height) :
	Entities(Time),
	Renderer(windowName, width, height),
	RenderQueue(Renderer, width, height),
	Input(InputHandler(_IsRunning)),
	UI(Entities, Input.GetBuffer()),
	GameField(0.0f, (float)height, 0.0f, (float)width),
	Xforms(2), //intial capacity. Can resize dynamically.
	BackgroundRenderer(Xforms, AABB(Vector2::zero(), Vector2((float)width, (float)height))),
	Rigidbodies(Entities, 2),
	Sprites(Xforms, Entities, RenderQueue.GetSpriteAtlas(), 128),
	Create(*this, Entities, Xforms, Sprites, Rigidbodies, UI, Time),
	Physics(Xforms, Rigidbodies, AABB(Vector2::zero(), Vector2((float)width, (float)height))),
	_IsRunning(true)
{
	CurrentState = std::make_unique<MenuState>(*this);
	CurrentState->OnEnter();
}

Game::~Game()
{

}

bool Game::IsRunning() const
{
	return _IsRunning;
}

void Game::ProcessInput()
{
	Input.Clear();
	Input.ProcessInput();
}

void Game::Update(const float deltaTime)
{
	Time.Update(deltaTime);

	const auto& inputBuffer = Input.GetBuffer();

	CurrentState->Update(inputBuffer, deltaTime);

	Rigidbodies.EnqueueAll(Physics, deltaTime);
	Physics.Simulate(deltaTime);
	Sprites.Update(deltaTime);

	GarbageCollection();

	//std::cout <<
	//	rigidbodies.Count() << " " <<
	//	entities.Count() << " " <<
	//	xforms.Count() << "\n";
}

void Game::Render()
{
	RenderQueue.Clear();

	UI.Render(RenderQueue);

	BackgroundRenderer.Render(RenderQueue, Time.DeltaTime());

	Sprites.Render(RenderQueue);

	Renderer.Render(RenderQueue.GetRenderQueue());
}

void Game::Quit()
{
	_IsRunning = false;
}

void Game::GarbageCollection()
{
	// @TODO: This can be better.

	Xforms.GarbageCollect(Entities);
	UI.GarbageCollect();
	Entities.GarbageCollect();
}

void
Game::ResetAllSystems()
{
	Entities.Clear();
	Xforms.Clear();
	Rigidbodies.Clear();
	UI.Clear();
	Sprites.Clear();
	RenderQueue.SetCameraLocation(0,0);
}
