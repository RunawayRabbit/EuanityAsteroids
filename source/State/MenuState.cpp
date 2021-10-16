#include "MenuState.h"
#include "PlayState.h"

#include "../Math/Math.h"

#include "../ECS/Entity.h"

#include "../Platform/Game.h"

MenuState::MenuState(Game& game)
	: game(game),
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
	for(auto asteroid : _FloatingAsteroids)
	{
		game.Entities.Destroy(asteroid);
	}
	game.Entities.Destroy(_Title);
	game.Entities.Destroy(_StartButton);
	game.Entities.Destroy(_QuitButton);
	game.Entities.Destroy(_EuanityMeme);
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
MenuState::SpawnMenuButtons()
{
	const auto screenCenter = game.GameField.max * 0.5f;
	const Vector2 buttonSize(200.0f, 80.0f);

	const AABB titleAABB(screenCenter.y * 0.4f,
	                     (screenCenter.y * 0.4f) + 168,
	                     screenCenter.x - 311,
	                     screenCenter.x + 311);
	_Title = game.Create.UIButton(titleAABB, SpriteID::MAIN_LOGO, []()
	{
	});

	const AABB playAABB((screenCenter.y * 1.1f),
	                    (screenCenter.y * 1.1f) + buttonSize.y,
	                    screenCenter.x - buttonSize.x * 0.5f,
	                    screenCenter.x + buttonSize.x * 0.5f);
	_StartButton = game.Create.UIButton(playAABB, SpriteID::START_BUTTON,
	                                   [&]() -> void
	                                   {
		                                   game.ChangeState<PlayState>(false);
	                                   });


	const AABB quitAABB((screenCenter.y * 1.1f) + 90,
	                    (screenCenter.y * 1.1f) + buttonSize.y + 90,
	                    screenCenter.x - buttonSize.x * 0.5f,
	                    screenCenter.x + buttonSize.x * 0.5f);
	_QuitButton = game.Create.UIButton(quitAABB, SpriteID::QUIT_BUTTON,
	                                  [&]() -> void
	                                  {
		                                  game.Quit();
	                                  });

	const AABB euanityAABB((screenCenter * 2) - Vector2(205.0f, 67.0f), (screenCenter * 2) - Vector2(5.0f, 5.0f));
	_EuanityMeme = game.Create.UIButton(euanityAABB, SpriteID::SHITTY_LOGO, []()
	{
	});
}

void
MenuState::SpawnRandomAsteroids()
{
	std::vector<Vector2> asteroidPositions;

	const auto numAsteroids = 1024;
	_FloatingAsteroids.reserve(numAsteroids);


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

			auto asteroidSize = i < (numAsteroids/9) ? Create::AsteroidType::LARGE
            : i < (3*numAsteroids/4) ? Create::AsteroidType::RANDOM_MEDIUM
            : Create::AsteroidType::RANDOM_SMALL;
			const auto asteroidRadius = Create::GetCollisionRadiusFromColliderType(asteroidSize);

			auto startPos = Vector2 {
				Math::RandomRange(asteroidRadius, game.GameField.max.x - asteroidRadius),
				Math::RandomRange(asteroidRadius, game.GameField.max.y - asteroidRadius)
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
				auto startRot = static_cast<float>(rand() % 360);
				auto startVel = Vector2 { Math::RandomRange(-speed, speed), Math::RandomRange(-speed, speed) };
				auto rotVel   = Math::RandomRange(-45.0f, 45.0f);

				asteroidPositions.push_back(startPos);

				_FloatingAsteroids.push_back(game.Create.Asteroid(startPos, startRot, startVel, rotVel, asteroidSize));
			}
			++attempts;
		}
	}
}
