
#include "..\Platform\Game.h"

#include "PlayState.h"

#include "..\Math\Math.h"

PlayState::PlayState(Game& game) :
	game(game),
	player(game.entities, game.rigidbodies, game.xforms, game.create, game.physics),
	level(0),
	lives(3),
	score(0),
	waitingForNextLevel(false),
	waitingToSpawn(false)
{}


void PlayState::OnEnter()
{
	SpawnFreshAsteroids(1, 20.0f, 25.0f);
	SpawnPlayer();
}

void PlayState::OnExit()
{
	for (Entity asteroid : currentAsteroids)
	{
		game.entities.Destroy(asteroid);
	}

	game.entities.Destroy(player.entity);
	game.entities.Destroy(gameOver);
}


void PlayState::Render()
{
}


void PlayState::Update(const InputBuffer& inputBuffer, const float& deltaTime)
{
	// Find out what collided, do stuff to fix it.
	for (const auto& collision : game.physics.GetCollisionReport())
	{
		if (collision.EntityAType == ColliderType::SHIP)
		{
			player.Kill(collision.A);
		}
		if (collision.EntityAType == ColliderType::BULLET)
		{
			Transform bullet = game.xforms.Get(collision.A).value();

			currentAsteroids.erase(
				std::remove(currentAsteroids.begin(), currentAsteroids.end(), collision.B),
				currentAsteroids.end());

			auto newAsteroids = game.create.SplitAsteroid(collision.B, 5.0f);
			if(newAsteroids.at(0) != Entity::Null())
				currentAsteroids.insert(currentAsteroids.end(), newAsteroids.begin(), newAsteroids.end());
			game.create.SmallExplosion(bullet.pos);
			game.entities.Destroy(collision.A);

			score += 10;
		}
	}

	player.Update(inputBuffer, deltaTime);

	if (!waitingToSpawn && !player.IsAlive())
	{
		if (--lives <= 0)
		{
			waitingToSpawn = true;
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

	if (currentAsteroids.size() == 0)
		QueueNextLevel();
}

void PlayState::QueueNextLevel()
{
	if (waitingForNextLevel) return;
	waitingForNextLevel = true;

	game.time.ExecuteDelayed(3.0f, [&]()
		{
			SpawnNextLevel();
		});
}

void PlayState::SpawnNextLevel()
{
	++level;
	if (level == 1)
		SpawnFreshAsteroids(3, 25.0f, 30.0f);
	else if (level == 2)
		SpawnFreshAsteroids(6, 30.0f, 35.0f);
	else if (level == 3)
		SpawnFreshAsteroids(9, 35.0f, 40.0f);
	else if(level == 4)
		SpawnFreshAsteroids(9, 35.0f, 45.0f);
	else
		SpawnFreshAsteroids(9, 35.0f, 50.0f);

	waitingForNextLevel = false;
}

void PlayState::SpawnPlayer()
{
	player.Spawn(game.gameField.max * 0.5f, 0);
}

void PlayState::SpawnFreshAsteroids(const int& count, const float& minVelocity, const float& maxVelocity)
{
	currentAsteroids.reserve(count);

	int leftRightCount = count / 3;
	float yBucketWidth = (game.gameField.max.y - ColliderRadius::Large) / leftRightCount;
	for (int i = 0; i < leftRightCount; ++i)
	{
		// Static in X, variable in Y
		Vector2 startPos{};
		startPos.x = 0;
		startPos.y = Math::RandomRange(ColliderRadius::Large + (yBucketWidth)*i, ColliderRadius::Large + (yBucketWidth)*i + 1);

		float startRot = (float)(rand() % 360);

		Vector2 velDir = Vector2::Forward().RotateDeg(Math::RandomRange(0.0f, 360.0f));
		Vector2 vel = velDir * Math::RandomRange(minVelocity, maxVelocity);
		float rotVel = Math::RandomRange(-45.0f, 45.0f);

		currentAsteroids.push_back(game.create.Asteroid(startPos, startRot, vel, rotVel, Create::AsteroidType::LARGE));
	}

	int topBottomCount = count - leftRightCount;
	float xBucketWidth = (game.gameField.max.x - ColliderRadius::Large) / topBottomCount;
	for (int i = 0; i < topBottomCount; ++i)
	{
		// Static in Y, variable in X
		Vector2 startPos{};
		startPos.x = Math::RandomRange(ColliderRadius::Large + (xBucketWidth)*i, ColliderRadius::Large + (xBucketWidth)*i + 1);
		startPos.y = 0;

		float startRot = (float)(rand() % 360);

		Vector2 velDir = Vector2::Forward().RotateDeg(Math::RandomRange(0.0f, 360.0f));
		Vector2 vel = velDir * Math::RandomRange(minVelocity, maxVelocity);
		float rotVel = Math::RandomRange(-45.0f, 45.0f);

		currentAsteroids.push_back(game.create.Asteroid(startPos, startRot, vel, rotVel, Create::AsteroidType::LARGE));
	}
}
