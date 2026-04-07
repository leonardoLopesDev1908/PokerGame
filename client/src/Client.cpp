#include "Client.h"

void Client::ping()
{
	std::cout << "Pinging server...\n";
	net::tcp::message<PokerMessages> msg;
	msg.header.id = PokerMessages::Ping;
	msg << "ECHO\n";

	send(msg);
}

void Client::raise()
{
	net::tcp::message<PokerMessages> raise;
	raise.header.id = PokerMessages::Raise;
	send(raise);
}

void Client::call()
{
	net::tcp::message<PokerMessages> callMsg;
	callMsg.header.id = PokerMessages::Call;
	send(callMsg);
}

void Client::fold()
{
	net::tcp::message<PokerMessages> foldMsg;
	foldMsg.header.id = PokerMessages::Fold;
	send(foldMsg);
}

void Client::end()
{
	std::cout << "Requesting disconnection from server...\n";
	disconnect();
}
