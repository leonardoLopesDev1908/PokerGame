#include <iostream>
#include <olc_net_client.h>

class Client : public net::tcp::client<net::MessageType>
{
	void ping()
	{
	
	}

	void raise()
	{

	}

	void call()
	{

	}

	void fold()
	{
	}
};

int main()
{
	Client c{};
	c.connect("127.0.0.1", 6000);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	char text[512];
	while (true)
	{
		std::cout << "Message: ";
		std::cin >> text;

		net::tcp::message<net::MessageType> msg;

		msg << text;
		c.send(msg);
		std::cout << '\n';
	}

	return 0;
}
