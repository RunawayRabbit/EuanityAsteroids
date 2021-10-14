#pragma once

#include <vector>

#include "..\GameObject\Player.h"

#include "IState.h"

class Game;

class PlayState : public IState
{
public:
	PlayState(Game& game);
	PlayState() = delete;

	virtual void OnEnter() override;
	virtual void OnExit() override;

	virtual void Update(const InputBuffer& inputBuffer, const float& deltaTime) override;
	virtual void Render() override;

private:
	Game& game;

	Player player;
	Entity gameOver;

	std::vector<Entity> currentAsteroids;

	int level;
	int lives;
	int score;

	bool waitingForNextLevel;
	bool waitingToSpawn;

	void SpawnFreshAsteroids(const int& count, const float& minVelocity, const float& maxVelocity);
	void SpawnPlayer();
	void QueueNextLevel();
	void SpawnNextLevel();

};