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
	void ProcessCollisions();
	void RespawnAsteroids();
	void UpdateCamera() const;

private:
	Game& _Game;

	Player _Player;
	Entity _GameOver;

	std::vector<Entity> _CurrentAsteroids;

	int _Lives;
	uint32_t _Score;

	bool _WaitingForNextLevel;
	bool _WaitingToSpawn;
	inline static const float CAMERA_VELOCITY_FACTOR = 0.3f;;

	void SpawnPlayer();
	void SpawnFirstAsteroid();

};
