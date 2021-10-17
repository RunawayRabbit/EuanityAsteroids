#include "../Platform/Game.h"

#include "PlayState.h"

#include "../Math/Math.h"

PlayState::PlayState(Game& game)
	: _Game(game),
	  _Player(game.Entities, game.Rigidbodies, game.Xforms, game.Create, game.Physics),
	  _GameOver(),
	  _Level(0),
	  _Lives(3),
	  _Score(0),
	  _WaitingForNextLevel(false),
	  _WaitingToSpawn(false)
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
}


void
PlayState::Render()
{
}


void
PlayState::Update(const InputBuffer& inputBuffer, const float& deltaTime)
{
	// Find out what collided, do stuff to fix it.
	for(const auto& [A, B, EntityAType, EntityBType, MassA, MassB, TimeOfCollision] : _Game.Physics.GetCollisionReport())
	{
		if(EntityAType == ColliderType::SHIP &&
			(EntityBType == ColliderType::LARGE_ASTEROID ||
				EntityBType == ColliderType::MEDIUM_ASTEROID))
		{
			// Players don't die to small asteroids. I have decided this.
			_Player.Kill(A);
		}

		if(EntityAType == ColliderType::BULLET)
		{
			auto [BulletPos, BulletRot] = _Game.Xforms.Get(A).value();

			_CurrentAsteroids.erase(
				std::remove(_CurrentAsteroids.begin(), _CurrentAsteroids.end(), B),
				_CurrentAsteroids.end());

			auto newAsteroids = _Game.Create.SplitAsteroid(B, 15.0f);

			if(EntityBType == ColliderType::LARGE_ASTEROID &&
				newAsteroids.at(0) != Entity::Null())
			{
				_CurrentAsteroids.insert(_CurrentAsteroids.end(), newAsteroids.begin(), newAsteroids.end());
			}

			// ReSharper disable once CppExpressionWithoutSideEffects
			_Game.Create.SmallExplosion(BulletPos);
			_Game.Entities.Destroy(A);

			_Score += 10;
		}
	}

	_Player.Update(inputBuffer, deltaTime);

	if(_Player.IsAlive())
	{
		auto [x,y]           = _Player.GetPlayerPosition();
		const auto playerVel = _Player.GetPlayerVelocity();

		_Game.RenderQueue.SetCameraLocation(
			static_cast<int>(x + (playerVel.x * CAMERA_VELOCITY_FACTOR)),
			static_cast<int>(y + (playerVel.y * CAMERA_VELOCITY_FACTOR)));
	}
	else
	{
		if(!_WaitingToSpawn)
		{
			if(--_Lives <= 0)
			{
				_WaitingToSpawn = true;
				_GameOver       = _Game.Create.GameOver(_Score);
			}
			else
			{
				_WaitingToSpawn = true;
				_Game.Time.ExecuteDelayed(1.5f, [&]()
				{
					SpawnPlayer();
					_WaitingToSpawn = false;
				});
			}
		}
	}


	if(_CurrentAsteroids.size() == 0)
		QueueNextLevel();
}

void
PlayState::QueueNextLevel()
{
	if(_WaitingForNextLevel)
		return;
	_WaitingForNextLevel = true;

	_Game.Time.ExecuteDelayed(2.0f, [&]()
	{
		SpawnNextLevel();
	});
}

void
PlayState::SpawnNextLevel()
{
	++_Level;
	if(_Level == 1)
		SpawnFreshAsteroids(5, 30.0f, 40.0f);
	else if(_Level == 2)
		SpawnFreshAsteroids(8, 35.0f, 45.0f);
	else if(_Level == 3)
		SpawnFreshAsteroids(12, 45.0f, 50.0f);
	else if(_Level == 4)
		SpawnFreshAsteroids(15, 60.0f, 65.0f);
	else if(_Level == 5)
		SpawnFreshAsteroids(20, 70.0f, 80.0f);
	else
		SpawnFreshAsteroids(30, 100.0f, 120.0f);

	_WaitingForNextLevel = false;
}

void
PlayState::SpawnPlayer()
{
	_Player.Spawn(_Game.GameField.max * 0.5f, 0);
}

void
PlayState::SpawnFreshAsteroids(const int& count, const float& minVelocity, const float& maxVelocity)
{
	_CurrentAsteroids.reserve(count);

	const auto leftRightCount = count / 3;
	const auto yBucketWidth   = (_Game.GameField.max.y - ColliderRadius::Large) / leftRightCount;
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

		_CurrentAsteroids.push_back(_Game.Create.Asteroid(startPos, startRot, vel, rotVel, Create::AsteroidType::LARGE));
	}

	const auto topBottomCount = count - leftRightCount;
	const auto xBucketWidth   = (_Game.GameField.max.x - ColliderRadius::Large) / topBottomCount;
	for(auto i = 0; i < topBottomCount; ++i)
	{
		// Static in Y, variable in X
		Vector2 startPos {};
		startPos.x = Math::RandomRange(ColliderRadius::Large + (xBucketWidth) * i, ColliderRadius::Large + (xBucketWidth) * i + 1);
		startPos.y = 0;

		auto startRot = static_cast<float>(rand() % 360);

		auto velDir = Vector2::Forward().RotateDeg(Math::RandomRange(0.0f, 360.0f));
		auto vel    = velDir * Math::RandomRange(minVelocity, maxVelocity);
		auto rotVel = Math::RandomRange(-45.0f, 45.0f);

		_CurrentAsteroids.push_back(_Game.Create.Asteroid(startPos, startRot, vel, rotVel, Create::AsteroidType::LARGE));
	}
}
