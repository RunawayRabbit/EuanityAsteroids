#include "MenuState.h"
#include "PlayState.h"

#include "../Math/EuanityMath.h"

#include "../ECS/Entity.h"

#include "../Platform/Game.h"

MenuState::MenuState(Game& game)
	: _Game(game),
	  _Title(Entity::Null()),
	  _StartButton(Entity::Null()),
	  _QuitButton(Entity::Null()),
	  _EuanityMeme(Entity::Null())
{
}

void
MenuState::OnEnter()
{
	SpawnRandomAsteroids();
	SpawnMenuButtons();
}


void
MenuState::OnExit()
{
	_Game.ResetAllSystems();
}

void
MenuState::Update(const InputBuffer& inputBuffer, const float& deltaTime)
{
}

void
MenuState::Render()
{
}

void
MenuState::SpawnShipSelectButtons()
{
	_Game.Entities.Destroy(_Title);
	_Game.Entities.Destroy(_StartButton);
	_Game.Entities.Destroy(_QuitButton);
	_Game.Entities.Destroy(_EuanityMeme);

	const auto screenCenter = _Game.Renderer.GetWindowDim() * 0.5f;
	const Vector2 buttonSize(158.0f, 207.0f);

	const float buttonStride = 320.0f;

	AABB button(screenCenter.y - buttonSize.y, screenCenter.y + buttonSize.y,
	                  screenCenter.x - buttonSize.x - buttonStride,
	                  screenCenter.x + buttonSize.x - buttonStride);

	Transform textTrans;
	textTrans.pos = Vector2(screenCenter.x - buttonStride, screenCenter.y + 90.0f);

	Transform shipTrans;
	shipTrans.pos = Vector2(screenCenter.x - buttonStride, screenCenter.y - 65.0f);

	_ShipSelect[0].Background =
        _Game.Create.UIButton(button, SpriteID::SHIP_SELECT_BACKGROUND,
                              [&]() -> void
                              {
                                  _Game.GameState.PlayerShipType = ShipInfo::ShipType::FastWeak;
                                  _Game.ChangeState<PlayState>(false);
                              });


	_ShipSelect[0].Text = _Game.Create.StaticSprite(
        textTrans, SpriteID::SHIP_DESCRIPTION_1, RenderQueue::Layer::UI_TOP,
        SpriteManager::RenderFlags::SCREEN_SPACE, 2.0f);

	_ShipSelect[0].Ship = _Game.Create.StaticSprite(
        shipTrans, SpriteID::SHIP_1, RenderQueue::Layer::UI_TOP,
        SpriteManager::RenderFlags::SCREEN_SPACE, 3.0f);


	button.left += buttonStride;
	button.right += buttonStride;

	textTrans.pos.x += buttonStride;
	shipTrans.pos.x += buttonStride;

	_ShipSelect[1].Background =
    _Game.Create.UIButton(button, SpriteID::SHIP_SELECT_BACKGROUND,
                          [&]() -> void
                          {
                              _Game.GameState.PlayerShipType = ShipInfo::ShipType::Normal;
                              _Game.ChangeState<PlayState>(false);
                          });


	_ShipSelect[1].Text = _Game.Create.StaticSprite(
        textTrans, SpriteID::SHIP_DESCRIPTION_2, RenderQueue::Layer::UI_TOP,
        SpriteManager::RenderFlags::SCREEN_SPACE, 2.0f);

	_ShipSelect[1].Ship = _Game.Create.StaticSprite(
        shipTrans, SpriteID::SHIP_2, RenderQueue::Layer::UI_TOP,
        SpriteManager::RenderFlags::SCREEN_SPACE, 3.0f);

	button.left += buttonStride;
	button.right += buttonStride;

	textTrans.pos.x += buttonStride;
	shipTrans.pos.x += buttonStride;

	_ShipSelect[2].Background =
    _Game.Create.UIButton(button, SpriteID::SHIP_SELECT_BACKGROUND,
                          [&]() -> void
                          {
                              _Game.GameState.PlayerShipType = ShipInfo::ShipType::SlowPowerful;
                              _Game.ChangeState<PlayState>(false);
                          });


	_ShipSelect[2].Text = _Game.Create.StaticSprite(
        textTrans, SpriteID::SHIP_DESCRIPTION_3, RenderQueue::Layer::UI_TOP,
        SpriteManager::RenderFlags::SCREEN_SPACE, 2.0f);

	_ShipSelect[2].Ship = _Game.Create.StaticSprite(
        shipTrans, SpriteID::SHIP_3, RenderQueue::Layer::UI_TOP,
        SpriteManager::RenderFlags::SCREEN_SPACE, 3.0f);
}

void
MenuState::SpawnMenuButtons()
{
	const auto screenCenter = _Game.Renderer.GetWindowDim() * 0.5f;
	const Vector2 buttonSize(200.0f, 80.0f);

	const AABB titleAABB(screenCenter.y * 0.4f,
	                     (screenCenter.y * 0.4f) + 168,
	                     screenCenter.x - 311,
	                     screenCenter.x + 311);
	_Title = _Game.Create.UIButton(titleAABB, SpriteID::MAIN_LOGO, []()
	{
	});

	const AABB playAABB((screenCenter.y * 1.1f),
	                    (screenCenter.y * 1.1f) + buttonSize.y,
	                    screenCenter.x - buttonSize.x * 0.5f,
	                    screenCenter.x + buttonSize.x * 0.5f);
	_StartButton = _Game.Create.UIButton(playAABB, SpriteID::START_BUTTON,
	                                     [&]() -> void
	                                     {
		                                     SpawnShipSelectButtons();
		                                     //_Game.ChangeState<PlayState>(false);
	                                     });


	const AABB quitAABB((screenCenter.y * 1.1f) + 90,
	                    (screenCenter.y * 1.1f) + buttonSize.y + 90,
	                    screenCenter.x - buttonSize.x * 0.5f,
	                    screenCenter.x + buttonSize.x * 0.5f);
	_QuitButton = _Game.Create.UIButton(quitAABB, SpriteID::QUIT_BUTTON,
	                                    [&]() -> void
	                                    {
		                                    _Game.Quit();
	                                    });

	const AABB euanityAABB((screenCenter * 2) - Vector2(205.0f, 67.0f), (screenCenter * 2) - Vector2(5.0f, 5.0f));
	_EuanityMeme = _Game.Create.UIButton(euanityAABB, SpriteID::SHITTY_LOGO, []()
	{
	});
}

void
MenuState::SpawnRandomAsteroids() const
{
	std::vector<Vector2> asteroidPositions;

	const auto numAsteroids = 1024;


	for(auto i = 0; i < numAsteroids; i++)
	{
		auto isValidPosition = false;
		auto attempts        = 0;
		while(!isValidPosition)
		{
			if(attempts > 8192)
			{
				// Yea I know.
				return;
			}

			auto asteroidSize = i < (numAsteroids / 9)
			                    ? Create::AsteroidType::LARGE
			                    : i < (3 * numAsteroids / 4)
			                    ? Create::AsteroidType::RANDOM_MEDIUM
			                    : Create::AsteroidType::RANDOM_SMALL;
			const auto asteroidRadius = Create::GetCollisionRadiusFromColliderType(asteroidSize);

			auto startPos = Vector2 {
				Math::RandomRange(asteroidRadius, _Game.GameFieldDim.x - asteroidRadius),
				Math::RandomRange(asteroidRadius, _Game.GameFieldDim.y - asteroidRadius)
			};

			isValidPosition = true;

			Circle newAsteroid(startPos, asteroidRadius);
			for(auto& existingAsteroid : asteroidPositions)
			{
				Circle existing(existingAsteroid, asteroidRadius);
				if(existing.Overlaps(newAsteroid))
				{
					isValidPosition = false;
					break;
				}
			}

			if(isValidPosition)
			{
				const auto speed = 90.0f;
				auto startRot    = static_cast<float>(rand() % 360);
				auto startVel    = Vector2 { Math::RandomRange(-speed, speed), Math::RandomRange(-speed, speed) };
				auto rotVel      = Math::RandomRange(-45.0f, 45.0f);

				asteroidPositions.push_back(startPos);

				// ReSharper disable once CppExpressionWithoutSideEffects
				_Game.Create.Asteroid(startPos, startRot, startVel, rotVel, asteroidSize);
			}
			++attempts;
		}
	}
}
