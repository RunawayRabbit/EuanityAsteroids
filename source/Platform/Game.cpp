#include "Game.h"

#include "../GameObject/Create.h"
#include "../Math/EuanityMath.h"
#include "../Physics/Physics.h"

Game::Game(const std::string windowName, const int windowWidth, const int windowHeight, const Vector2& gameWorldDim)
	: GameCam(this, windowWidth, windowHeight),
	  DebugCam(this, windowWidth, windowHeight),
	  IsDebugCamera(false),
	  Renderer(windowName, windowWidth, windowHeight),
	  RenderQueue(Renderer, GameCam, gameWorldDim),
	  BackgroundRenderer({windowWidth, windowHeight}),
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

	Rigidbodies.EnqueueAll(Physics, deltaTime);

	CurrentState->Update(inputBuffer, deltaTime);

	Physics.Simulate(deltaTime);
	Sprites.Update(deltaTime);

	GarbageCollection();

	auto& cam = IsDebugCamera ? DebugCam : GameCam;
	cam.Update(deltaTime);

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

	BackgroundRenderer.Render(cam, RenderQueue, Time.DeltaTime());

	Sprites.Render(RenderQueue);

	UI.Render(RenderQueue);

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

Vector2
Game::WrapToGameField(const Vector2& point) const
{
	return Vector2(Math::RepeatNeg(point.x, GameFieldDim.x),
	               Math::RepeatNeg(point.y, GameFieldDim.y));
}

bool
Game::GameFieldContains(const Vector2& point) const
{
	const auto gameField = AABB(0,GameFieldDim.y, 0, GameFieldDim.x);
	return gameField.Contains(point);
}
