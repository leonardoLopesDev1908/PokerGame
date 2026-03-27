#pragma once

#include <olc_net_server.h>

#include <deque_card.h>
#include <poker_messages.h>
#include <GameState.h>

#include "omp/HandEvaluator.h"
#include "omp/Hand.h"

#include <optional>
#include <memory>
#include <array>
#include <vector>
#include <unordered_map>
#include <optional>

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

private:
	bool equalBets();
	void ending_round();
	void bettingRound(short& currentIndex);

private:
	IServer m_server;
	GameState& m_gameState;
	omp::HandEvaluator m_evaluator;

	std::condition_variable m_cond;
	std::mutex m_mutex;

	uint32_t currentId;
	short m_smallBlind = 1;
	short m_bigBlind = 2;
	long m_playersCount = 0;

	Deque m_deque;
};