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

void Player::setMoney(long long newValue)
{
	m_money = newValue;
}

long long int Player::getMoney()
{
	return m_money;
}

long long int Player::getBet()
{
	return m_bet;
}
