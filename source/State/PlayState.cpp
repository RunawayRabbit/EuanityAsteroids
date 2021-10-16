#include "../Platform/Game.h"

#include "PlayState.h"

#include "../Math/Math.h"

PlayState::PlayState(Game& game)
	: game(game),
	  player(game.entities, game.rigidbodies, game.xforms, game.create, game.physics),
	  gameOver(),
	  level(0),
	  lives(3),
	  score(0),
	  waitingForNextLevel(false),
	  waitingToSpawn(false)
{
}


void
PlayState::OnEnter()
{
	SpawnFreshAsteroids(1, 20.0f, 25.0f);
	SpawnPlayer();
}

void
PlayState::OnExit()
{
	for(Entity asteroid : currentAsteroids)
	{
		game.entities.Destroy(asteroid);
	}

	game.entities.Destroy(player.entity);
	game.entities.Destroy(gameOver);
}


void
PlayState::Render()
{
}


void
PlayState::Update(const InputBuffer& inputBuffer, const float& deltaTime)
{
	// Find out what collided, do stuff to fix it.
	for(const auto& [A, B, EntityAType, EntityBType, MassA, MassB, TimeOfCollision] : game.physics.GetCollisionReport())
	{
		if(EntityAType == ColliderType::SHIP)
		{
			player.Kill(A);
		}
		if(EntityAType == ColliderType::BULLET)
		{
			auto [BulletPos, BulletRot] = game.xforms.Get(A).value();

			currentAsteroids.erase(
				std::remove(currentAsteroids.begin(), currentAsteroids.end(), B),
				currentAsteroids.end());

			auto newAsteroids = game.create.SplitAsteroid(B, 15.0f);
			if(newAsteroids.at(0) != Entity::Null())
				currentAsteroids.insert(currentAsteroids.end(), newAsteroids.begin(), newAsteroids.end());

			// ReSharper disable once CppExpressionWithoutSideEffects
			game.create.SmallExplosion(BulletPos);
			game.entities.Destroy(A);

			score += 10;
		}
	}

	player.Update(inputBuffer, deltaTime);

	if(!waitingToSpawn && !player.IsAlive())
	{
		if(--lives <= 0)
		{
			waitingToSpawn      = true;
			Vector2 gameOverPos = game.gameField.max * 0.5f;
			gameOverPos.y -= 100.0f;
			gameOver = game.create.GameOver(score, gameOverPos);
		}
		else
		{
			waitingToSpawn = true;
			game.time.ExecuteDelayed(1.5f, [&]()
			{
				SpawnPlayer();
				waitingToSpawn = false;
			});
		}
	}

	if(currentAsteroids.size() == 0)
		QueueNextLevel();
}

void
PlayState::QueueNextLevel()
{
	if(waitingForNextLevel)
		return;
	waitingForNextLevel = true;

	game.time.ExecuteDelayed(2.0f, [&]()
	{
		SpawnNextLevel();
	});
}

void
PlayState::SpawnNextLevel()
{
	++level;
	if(level == 1)
		SpawnFreshAsteroids(3, 30.0f, 40.0f);
	else if(level == 2)
		SpawnFreshAsteroids(6, 35.0f, 45.0f);
	else if(level == 3)
		SpawnFreshAsteroids(9, 45.0f, 50.0f);
	else if(level == 4)
		SpawnFreshAsteroids(10, 60.0f, 65.0f);
	else
		SpawnFreshAsteroids(12, 70.0f, 80.0f);

	waitingForNextLevel = false;
}

void
PlayState::SpawnPlayer()
{
	player.Spawn(game.gameField.max * 0.5f, 0);
}

void
PlayState::SpawnFreshAsteroids(const int& count, const float& minVelocity, const float& maxVelocity)
{
	currentAsteroids.reserve(count);

	const auto leftRightCount = count / 3;
	const auto yBucketWidth   = (game.gameField.max.y - ColliderRadius::Large) / leftRightCount;
	for(auto i = 0; i < leftRightCount; ++i)
	{
		// Static in X, variable in Y
		Vector2 startPos {};
		startPos.x = 0;
		startPos.y = Math::RandomRange(ColliderRadius::Large + (yBucketWidth) * i, ColliderRadius::Large + (yBucketWidth) * i + 1);

		auto startRot = static_cast<float>(rand() % 360);

		auto velDir = Vector2::Forward().RotateDeg(Math::RandomRange(0.0f, 360.0f));
		auto vel    = velDir * Math::RandomRange(minVelocity, maxVelocity);
		auto rotVel = Math::RandomRange(-45.0f, 45.0f);

		currentAsteroids.push_back(game.create.Asteroid(startPos, startRot, vel, rotVel, Create::AsteroidType::LARGE));
	}

	const auto topBottomCount = count - leftRightCount;
	const auto xBucketWidth  = (game.gameField.max.x - ColliderRadius::Large) / topBottomCount;
	for(auto i = 0; i < topBottomCount; ++i)
	{
		// Static in Y, variable in X
		Vector2 startPos {};
		startPos.x = Math::RandomRange(ColliderRadius::Large + (xBucketWidth) * i, ColliderRadius::Large + (xBucketWidth) * i + 1);
		startPos.y = 0;

		float startRot = static_cast<float>(rand() % 360);

		Vector2 velDir = Vector2::Forward().RotateDeg(Math::RandomRange(0.0f, 360.0f));
		Vector2 vel    = velDir * Math::RandomRange(minVelocity, maxVelocity);
		float rotVel   = Math::RandomRange(-45.0f, 45.0f);

		currentAsteroids.push_back(game.create.Asteroid(startPos, startRot, vel, rotVel, Create::AsteroidType::LARGE));
	}
}
