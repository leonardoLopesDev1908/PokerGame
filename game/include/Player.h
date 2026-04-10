#pragma once
#include <array>
#include <memory>
#include <olc_net_server.h>
#include <deque_card.h>
#include <poker_messages.h>

using PointerConnection = std::shared_ptr<net::tcp::connection<PokerMessages>>;

class Player
{
public:
	Player(uint32_t id, PointerConnection& connection,
		std::array<std::optional<Card>, 2> hand);

	Player();

	void reset_round();

	void message(net::tcp::message<PokerMessages>& msg);

    void setMoney(long long newMoney);

	long long int getMoney();

	long long int getBet();

	void setBet(long long int newBet);

	void setId(uint32_t id);
	void setConnection(PointerConnection connection);
	void setHand(std::array <std::optional<Card>, 2>);

public:
	uint32_t id = 0;
	PointerConnection connection = nullptr;
	std::array<std::optional<Card>, 2> hand = { std::nullopt, std::nullopt };
	bool m_folded = false;

private:
	long long int m_money = 2500;
	long long int m_bet = 0;
};
