#include "Player.h"


void Player::reset_round()
{
	m_bet = 0;
	hand = { std::nullopt, std::nullopt };
	folded = false;
}

void Player::message(net::tcp::message<PokerMessages>& msg)
{
	connection->send(msg);
}

void Player::updateMoney(long long newValue)
{
	m_money = newValue;
}
