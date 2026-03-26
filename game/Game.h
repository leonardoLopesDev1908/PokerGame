#pragma once

#include <deque_card.h>
#include <poker_messages.h>
#include <olc_net_server.h>
#include "omp/HandEvaluator.h"
#include "omp/Hand.h"
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

struct Player
{
	uint32_t id;
	long long int money = 2500;
	long long int bet = 0;
	std::array<std::optional<Card>, 2> hand = { std::nullopt, std::nullopt };
	std::shared_ptr<net::tcp::connection<PokerMessages>> connection;
	bool folded = false;

	void reset_round()
	{
		bet = 0;
		hand = { std::nullopt, std::nullopt };
		folded = false;
	}
};

class Game
{
public:
		
	void game_loop();
	void start_game();
	void game();

private:
	bool equalBets();
	void ending_round();
	void bettingRound(short& currentIndex);
};