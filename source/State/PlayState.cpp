#include "PlayState.h"

#include "../Math/EuanityMath.h"
#include "../Math/Circle.h"
#include "../Platform/Game.h"

PlayState::PlayState(Game& game)
	: _Game(game),
	  _Player(game),
	  _GameOver(),
	  _Lives(3),
	  _Score(0),
	  _WaitingForNextLevel(false),
	  _WaitingToSpawn(false),
	  _CurrentCamZoom(1),
	  _CamZoomVelocity(0)
{
}


void
PlayState::OnEnter()
{
	_Player.Spawn(_Game.GameFieldDim * 0.5f, 0);
	SpawnFirstAsteroid();
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
PlayState::ProcessCollisions()
{
	for(const auto& [A, B, EntityAType, EntityBType, MassA, MassB, TimeOfCollision] : _Game.Physics.GetCollisionReport())
	{
		if(!_Game.Entities.Exists(A) || !_Game.Entities.Exists(B))
		{
			// Don't double-process things that were destroyed earlier in the collision list!
			continue;
		}
		if(EntityAType == ColliderType::SHIP)
		{
			if(EntityBType == ColliderType::LARGE_ASTEROID ||
				EntityBType == ColliderType::MEDIUM_ASTEROID)
			{
				// Player dies when they crash into Large or Medium asteroids.
				auto playerVel = _Game.Rigidbodies.Get(A).value().velocity;
				_Player.Kill(A, playerVel);
			}
			else if(EntityBType == ColliderType::SMOL_ASTEROID)
			{
				// Small asteroids bounce if slow, explode if fast.
				// Fast collisions reduce player HP.
				auto playerVel   = _Game.Rigidbodies.Get(A).value().velocity;
				auto asteroidVel = _Game.Rigidbodies.Get(B).value().velocity;

				const auto relativeSpeedSq = (playerVel - asteroidVel).LengthSq();

				if(relativeSpeedSq > 50.0f * 50.0f)
				{
					// Too fast, deal damage
					_Player.TakeDamage(1);
					auto asteroidPos = _Game.Xforms.Get(B).value().pos;
					// ReSharper disable once CppExpressionWithoutSideEffects
					_Game.Create.SmallExplosion(asteroidPos, asteroidVel);
					_Game.Entities.Destroy(B);
				}
				else
				{
					// just bounce!
				}
			}
		}
		if(EntityAType == ColliderType::BULLET || EntityAType == ColliderType::BOUNCY_BULLET)
		{
			auto [BulletPos, BulletRot] = _Game.Xforms.Get(A).value();

			_CurrentAsteroids.erase(
				std::remove(_CurrentAsteroids.begin(), _CurrentAsteroids.end(), B),
				_CurrentAsteroids.end());


			// ReSharper disable CppExpressionWithoutSideEffects
			_Game.Create.SplitAsteroid(B, 15.0f);

			_Game.Create.SmallExplosion(BulletPos);
			// ReSharper restore CppExpressionWithoutSideEffects

			if(EntityAType == ColliderType::BULLET)
			{
				// Regular bullets die on collision, super ones don't!
				_Game.Entities.Destroy(A);
			}

			_Score += 100;
		}
	}
}

void
PlayState::RespawnAsteroids()
{
	// Max difficulty at 8.
	const int difficultyLevel = static_cast<uint32_t>(floorf(logf(_Score) - 5.0f)) - 1;

#pragma region DifficultySettings
	int numDesiredAsteroids;
	float asteroidSpeedMin;
	float asteroidSpeedMax;
	if(difficultyLevel < 2)
	{
		numDesiredAsteroids = 1;
		asteroidSpeedMin    = 30.0f;
		asteroidSpeedMax    = 45.0f;
	}
	else if(difficultyLevel == 2)
	{
		numDesiredAsteroids = 5;
		asteroidSpeedMin    = 35.0f;
		asteroidSpeedMax    = 45.0f;
	}
	else if(difficultyLevel == 3)
	{
		numDesiredAsteroids = 8;
		asteroidSpeedMin    = 40.0f;
		asteroidSpeedMax    = 50.0f;
	}
	else if(difficultyLevel == 4)
	{
		numDesiredAsteroids = 12;
		asteroidSpeedMin    = 50.0f;
		asteroidSpeedMax    = 60.0f;
	}
	else if(difficultyLevel == 5)
	{
		numDesiredAsteroids = 17;
		asteroidSpeedMin    = 60.0f;
		asteroidSpeedMax    = 80.0f;
	}
	else if(difficultyLevel == 6)
	{
		numDesiredAsteroids = 24;
		asteroidSpeedMin    = 80.0f;
		asteroidSpeedMax    = 100.0f;
	}
	else if(difficultyLevel == 7)
	{
		numDesiredAsteroids = 30;
		asteroidSpeedMin    = 100.0f;
		asteroidSpeedMax    = 140.0f;
	}
	else // >= 8. DEATH SCREEN!!!!!
	{
		numDesiredAsteroids = 500;
		asteroidSpeedMin    = 240.0f;
		asteroidSpeedMax    = 380.0f;
	}
#pragma endregion DifficultySettings

	_CurrentAsteroids.reserve(numDesiredAsteroids);
	for(auto i = _CurrentAsteroids.size(); i < numDesiredAsteroids; ++i)
	{
		auto playerPos = _Player.GetPlayerPosition();
		Vector2 spawnPosition;
		do
		{
			spawnPosition   = playerPos + (Math::RandomOnUnitCircle() * 750.0f);
			spawnPosition.x = Math::Repeat(spawnPosition.x, _Game.GameFieldDim.x);
			spawnPosition.y = Math::Repeat(spawnPosition.y, _Game.GameFieldDim.y);
		} while(_Game.GameCam.GetCameraView().Contains(spawnPosition));

		const auto piOverFour = Math::PI * 0.25f;
		auto spawnVelocity    = (playerPos - spawnPosition).Normalized().RotateRad(Math::RandomRange(-piOverFour, piOverFour)) *
			Math::RandomRange(
				asteroidSpeedMin, asteroidSpeedMax);

		_CurrentAsteroids.push_back(_Game.Create.Asteroid(spawnPosition,
		                                                  Math::RandomRange(0.0f, 360.0f),
		                                                  spawnVelocity,
		                                                  Math::RandomRange(15.0f, 40.0f),
		                                                  Create::AsteroidType::LARGE));
	}
}

void
PlayState::UpdateCamera(const float& deltaTime)
{
	const auto playerVel = _Player.GetPlayerVelocity();

	const auto newCamZoom =
		Math::Remap(playerVel.Length(),
		            0, 300.0f,
		            1.5f, 0.8f);

	_CurrentCamZoom = Math::SmoothDamp(_CurrentCamZoom, newCamZoom, _CamZoomVelocity, 2.0f, 1.0f, deltaTime);

	_Game.GameCam.SetScale(_CurrentCamZoom);

	const auto focalPoint = _Player.GetPlayerPosition() + (playerVel * 1.2f) +
		_Player.GetPlayerForward() * -15.0f;
	_Game.GameCam.SetFocalPoint(focalPoint);
}

void
PlayState::Update(const InputBuffer& inputBuffer, const float& deltaTime)
{
	ProcessCollisions();

	_Player.Update(inputBuffer, deltaTime);

	if(_Player.IsAlive())
	{
		UpdateCamera(deltaTime);
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
					RespawnPlayer();
					_WaitingToSpawn = false;
				});
			}
		}
	}

	RespawnAsteroids();
}

void
PlayState::RespawnPlayer()
{
	static const auto RESPAWN_CHECK_RADIUS = 100.0f;

	const auto spawnPoint = _Game.WrapToGameField(_Game.GameCam.GetFocalPoint() + (_Game.GameFieldDim * 0.5f));

	auto testCircle   = Circle(spawnPoint, RESPAWN_CHECK_RADIUS);
	while(_Game.Physics.IsOverlappingAnything(testCircle, ColliderType::SHIP))
	{
		// Our selected spawnpoint is not safe to spawn in. Random walk to a new point!
		testCircle.Center = _Game.WrapToGameField(testCircle.Center + (Math::RandomOnUnitCircle() * RESPAWN_CHECK_RADIUS));
	}

	_Player.Spawn(testCircle.Center, 0);
}

void
PlayState::SpawnFirstAsteroid()
{
	const auto playerPos = _Player.GetPlayerPosition();
	const auto startPos  = playerPos + Vector2(-400.0f, 0.0f);
	const auto velocity  = (playerPos - startPos).Normalized() * 30.0f;

	_CurrentAsteroids.push_back(_Game.Create.Asteroid(startPos, 240, velocity, 60, Create::AsteroidType::LARGE));
}
