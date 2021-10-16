#include "MenuState.h"
#include "PlayState.h"

#include "../Math/Math.h"

#include "../ECS/Entity.h"

#include "../Platform/Game.h"

MenuState::MenuState(Game& game)
	: game(game),
	  title(Entity::Null()),
	  startButton(Entity::Null()),
	  quitButton(Entity::Null()),
	  euanityMeme(Entity::Null())
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
	for(Entity asteroid : floatingAsteroids)
	{
		game.entities.Destroy(asteroid);
	}
	game.entities.Destroy(title);
	game.entities.Destroy(startButton);
	game.entities.Destroy(quitButton);
	game.entities.Destroy(euanityMeme);
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
	const auto screenCenter = game.gameField.max * 0.5f;
	const Vector2 buttonSize(200.0f, 80.0f);

	const AABB titleAABB(screenCenter.y * 0.4f,
	                     (screenCenter.y * 0.4f) + 168,
	                     screenCenter.x - 311,
	                     screenCenter.x + 311);
	title = game.create.UIButton(titleAABB, SpriteID::MAIN_LOGO, []()
	{
	});

	const AABB playAABB((screenCenter.y * 1.1f),
	                    (screenCenter.y * 1.1f) + buttonSize.y,
	                    screenCenter.x - buttonSize.x * 0.5f,
	                    screenCenter.x + buttonSize.x * 0.5f);
	startButton = game.create.UIButton(playAABB, SpriteID::START_BUTTON,
	                                   [&]() -> void
	                                   {
		                                   game.ChangeState<PlayState>();
	                                   });


	const AABB quitAABB((screenCenter.y * 1.1f) + 90,
	                    (screenCenter.y * 1.1f) + buttonSize.y + 90,
	                    screenCenter.x - buttonSize.x * 0.5f,
	                    screenCenter.x + buttonSize.x * 0.5f);
	quitButton = game.create.UIButton(quitAABB, SpriteID::QUIT_BUTTON,
	                                  [&]() -> void
	                                  {
		                                  game.Quit();
	                                  });

	const AABB euanityAABB((screenCenter * 2) - Vector2(205.0f, 67.0f), (screenCenter * 2) - Vector2(5.0f, 5.0f));
	euanityMeme = game.create.UIButton(euanityAABB, SpriteID::SHITTY_LOGO, []()
	{
	});
}

void
MenuState::SpawnRandomAsteroids()
{
	std::vector<Vector2> asteroidPositions;

	const auto numAsteroids = 32;
	floatingAsteroids.reserve(numAsteroids);


	for(auto i = 0; i < numAsteroids; i++)
	{
		bool isValidPosition = false;
		int attempts         = 0;
		while(!isValidPosition)
		{
			if(attempts > 2048)
			{
				// Yea I know. It's temporary, ok?
				return;
			}

			const auto asteroidRadius = ColliderRadius::Large;

			auto startPos = Vector2 {
				Math::RandomRange(asteroidRadius, game.gameField.max.x - asteroidRadius),
				Math::RandomRange(asteroidRadius, game.gameField.max.y - asteroidRadius)
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
				const auto speed = 70.0f;
				auto startRot = static_cast<float>(rand() % 360);
				auto startVel = Vector2 { Math::RandomRange(-speed, speed), Math::RandomRange(-speed, speed) };
				auto rotVel   = Math::RandomRange(-45.0f, 45.0f);

				asteroidPositions.push_back(startPos);

				floatingAsteroids.push_back(game.create.Asteroid(startPos, startRot, startVel, rotVel, Create::AsteroidType::LARGE));
			}
			++attempts;
		}
	}
}
