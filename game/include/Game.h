#pragma once

#include "GameState.h"
#include "HandEvaluator.h"
#include "IServer.h"

#include <mutex>
#include <condition_variable>

enum class Stage : uint8_t
{
	PreFlop,
	Flop,
	Turn,
	River,
	CardChecking,
	Ending
};

class Game
{
public:
	Game(GameState& gameState);
	
	Game& operator=(GameState& gameState);
	
	void game_loop();
	
	void start_game();
	
	void run_game();
	
	void stop();
	
	void setServer(IServer* server);

private:
	bool equalBets();

	void ending_round();

	void bettingRound(short& currentIndex);

private:
	IServer* m_server = nullptr;
	GameState& m_gameState;
	omp::HandEvaluator m_evaluator;

	std::condition_variable m_cond;
	std::mutex m_mutex;

	uint32_t currentId = 0;
	short m_smallBlind = 0;
	short m_bigBlind = 1;
	long m_playersCount = 0;

	Deque m_deque;
};