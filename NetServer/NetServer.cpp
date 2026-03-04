#include <iostream>
#include <olc_net_server.h>

enum class PokerMessages
{
	Ping,
	Connect,
	Fold,
	Call,
	Raise,
	Transaction,
	End
};

class Server : public net::tcp::server<PokerMessages>
{
public: 

	explicit Server(uint16_t port)
		: net::tcp::server<PokerMessages>(port)
	{
	}

	virtual void on_message(const net::tcp::message<PokerMessages>& msg,
		std::shared_ptr<net::tcp::connection<PokerMessages>> remote = nullptr)
	{
		switch (msg.header.id)
		{
			case PokerMessages::Ping:
			{
				if (remote)
				{
					std::cout << "Ping from client " << remote->getId() << '\n';
					net::tcp::message<PokerMessages> echoMessage;
					echoMessage << msg;
				}
				break;
			}
			case PokerMessages::Connect:
			{

			}
			case PokerMessages::Fold:
			{
				std::string message = "Fold of Player " + std::to_string(remote->getId());
				std::cout << message;

				net::tcp::message<PokerMessages> msg;
				msg << message;
				

				message_all(msg, remote);
			}
			case PokerMessages::Call:
			{

			}
			case PokerMessages::Raise:
			{

			}
			case PokerMessages::Transaction:
			{

			}
			case PokerMessages::End:
			{

			}
		}
	}

	virtual void on_client_connect()
	{
	}

	virtual void on_client_disconnect()
	{
	}
};

int main()
{
	Server s(6000);
	s.start();

	while (true)
		s.update();

	return 0;
}