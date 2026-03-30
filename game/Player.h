#include <olc_net_server.h>

#include <deque_card.h>
#include <poker_messages.h>

#include "omp/Hand.h"

#include <optional>
#include <memory>
#include <array>

class Player
{
public:
	void reset_round();

	void message(net::tcp::message<PokerMessages>& msg);

    void setMoney(long long newMoney);
public:
	uint32_t id;
	std::shared_ptr<net::tcp::connection<PokerMessages>> connection;
	std::array<std::optional<Card>, 2> hand = { std::nullopt, std::nullopt };
	bool folded = false;

private:
	std::mutex m_mutex;
	long long int m_money = 2500;
	long long int m_bet = 0;
};
