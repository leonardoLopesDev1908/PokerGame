#include <olc_net_server.h>

class IServer
{
public:
	virtual void sendMessage(net::tcp::message<PokerMessages>& msg, uint32_t playerId);
	virtual void messageAll(net::tcp::message<PokerMessages>& msg);
};