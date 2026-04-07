#pragma once
#include <iostream>
#include <olc_net_client.h>
#include <deque_card.h>
#include <poker_messages.h>
#include <windows.h>

class Client : public net::tcp::client<PokerMessages>
{
public:

	void ping();
	void raise();
	void call();
	void fold();
	void end();

private:
	long long int m_money;
};


