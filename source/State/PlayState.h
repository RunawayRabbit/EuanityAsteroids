#pragma once

#include <vector>

#include "../GameObject/Player.h"

#include "IState.h"

class Game;

class PlayState final : public IState
{
public:
	explicit PlayState(Game& game);
	PlayState() = delete;

	void OnEnter() override;
	void OnExit() override;

	void Update(const InputBuffer& inputBuffer, const float& deltaTime) override;
	void Render() override;

private:
	Game& _Game;

	Player _Player;
	Entity _GameOver;

	std::vector<Entity> _CurrentAsteroids;

	int _Level;
	int _Lives;
	int _Score;

	bool _WaitingForNextLevel;
	bool _WaitingToSpawn;
	inline static const float CAMERA_VELOCITY_FACTOR = 0.3f;;

	void SpawnFreshAsteroids(const int& count, const float& minVelocity, const float& maxVelocity);
	void SpawnPlayer();
	void QueueNextLevel();
	void SpawnNextLevel();

};
