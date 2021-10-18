#include "Game.h"

#include "../GameObject/Create.h"
#include "../Math/EuanityMath.h"
#include "../Physics/Physics.h"

Game::Game(const std::string windowName, const int width, const int height)
	: Camera(AABB(static_cast<float>(width), static_cast<float>(height)), AABB(static_cast<float>(width), static_cast<float>(height))),
	  Renderer(windowName, width, height),
	  RenderQueue(Renderer, Camera, width, height),
	  BackgroundRenderer(Xforms, AABB(static_cast<float>(width), static_cast<float>(height))),
	  Input(InputHandler(_IsRunning)),
	  Create(*this, Entities, Xforms, Sprites, Rigidbodies, UI, Time),
	  Entities(Time),
	  Xforms(1024),
	  Sprites(Xforms, Entities, RenderQueue.GetSpriteAtlas(), 128),
	  UI(Entities, Input.GetBuffer()),
	  Physics(Xforms, Rigidbodies, AABB(static_cast<float>(width), static_cast<float>(height))),
	  Rigidbodies(Entities, 1024),
	  GameField(static_cast<float>(width), static_cast<float>(height)),
	  _IsRunning(true)
{
	CurrentState = std::make_unique<MenuState>(*this);
	CurrentState->OnEnter();
}

Game::~Game()
{
}

bool
Game::IsRunning() const
{
	return _IsRunning;
}

void
Game::ProcessInput()
{
	Input.Clear();
	Input.ProcessInput();
}

void
Game::Update(const float deltaTime)
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

void
Game::Render()
{
	RenderQueue.Clear();

	UI.Render(RenderQueue);

	BackgroundRenderer.Render(RenderQueue, Time.DeltaTime());

	Sprites.Render(RenderQueue);

	Renderer.Render(RenderQueue.GetRenderQueue());
}

void
Game::Quit()
{
	_IsRunning = false;
}

void
Game::GarbageCollection()
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
	Camera.SetCameraView(GameField);
}
