#include "Game.h"

#include "../GameObject/Create.h"
#include "../Math/EuanityMath.h"
#include "../Physics/Physics.h"

Game::Game(const std::string windowName, const int windowWidth, const int windowHeight, const Vector2& gameWorldDim)
	: GameCam(windowWidth, windowHeight),
	  DebugCam(windowWidth, windowHeight),
	  IsDebugCamera(false),
	  Renderer(windowName, windowWidth, windowHeight),
	  RenderQueue(Renderer, GameCam, gameWorldDim),
	  BackgroundRenderer(Xforms, AABB(static_cast<float>(windowWidth), static_cast<float>(windowHeight))),
	  Input(InputHandler(_IsRunning)),
	  Create(*this, Entities, Xforms, Sprites, Rigidbodies, UI, Time),
	  Entities(Time),
	  Xforms(512),
	  Sprites(Xforms, Entities, RenderQueue.GetSpriteAtlas(), 512),
	  UI(Entities, Input.GetBuffer()),
	  Physics(Xforms, Rigidbodies, gameWorldDim),
	  Rigidbodies(Entities, 1024),
	  GameFieldDim(gameWorldDim),
	  _IsRunning(true)
{
	GameCam.SetFocalPoint(gameWorldDim * 0.5f);

	DebugCam.SetFocalPoint(gameWorldDim * 0.5f);
	DebugCam.SetScale(0.4f);

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

	if(inputBuffer.Contains(InputOneShot::DEBUG_Camera))
	{
		IsDebugCamera = !IsDebugCamera;
	}

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
	auto& cam = IsDebugCamera ? DebugCam : GameCam;
	RenderQueue.CacheCameraInfo(&cam);

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
	GameCam.SetFocalPoint(GameFieldDim * 0.5f);
	GameCam.SetScale(1.0f);
}
