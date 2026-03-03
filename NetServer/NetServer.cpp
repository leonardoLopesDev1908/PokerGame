#include <iostream>
#include <olc_net_server.h>

class Server : public net::tcp::server<net::MessageType>
{
public: 
	explicit Server(uint16_t port) : net::tcp::server<net::MessageType>(port)
	{
	}
};

int main()
{
	Server s(6000);
	s.start();

	std::cout << "A\n";

	while (true)
		s.update();

	return 0;
}