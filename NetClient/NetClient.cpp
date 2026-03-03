#include <iostream>
#include <olc_net_client.h>


enum class ClientMessages
{
	Ping, 
	Connect,
	Fold,
	Call,
	Raise,
	Transaction,
	End
}

class Client : public net::tcp::client<PokerMessages>
{
public:

	enum class TransactionType
	{
		Lost,
		Win
	}

	
	void ping()
	{
		net::tcp::message<PokerMessages> msg;
		msg << "ECHO\n";

		msg.header.id = ClientMessages::Ping;

		c.send(msg);
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

	void end()
	{
		std::cout << "Requesting disconnection from server...\n";
		c.disconnect();
	}

	bool update_money(long int value, TransactionType type)
	{
		if(type == TransactionType::Lost)
		{
			m_money -= value;
		}
		else if(type == TransactionType::Win)
		{
			m_money += value;
		}
		else{
			std::cerr << "Invalid type of transaction: " << type << '\n';
			return false;
		}
		
		return true;
	}

private:
	long long int m_money;
};


int main()
{
	Client c{};
	c.connect("127.0.0.1", 6000);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	char text[512];

	bool bQuit = false;
	while (!bQuit)
	{

		if (GetAsyncKeyState('1') && 0x8000) c.ping();
		else if(GetAsyncKeyState('2') && 0x8000) 
		{
			c.end();
			bQuit = true;
			break;
		}
		else if(GetAsyncKeyState('3') && 0x8000) c.call();

		if(!c.incoming().empty())
		{
			auto msg = c.incoming().pop_front();
			switch(msg.header.id)
			{
				

			}
		}
	}

	return 0;
}
