#include <iostream>
#include <olc_net_client.h>


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

class Client : public net::tcp::client<PokerMessages>
{
public:

	enum class TransactionType
	{
		Lost,
		Win
	};

	void ping()
	{
		std::cout << "Pinging server...\n";
		net::tcp::message<PokerMessages> msg;
		msg << "ECHO\n";

		msg.header.id = PokerMessages::Ping;

		send(msg);
	}

	void raise(long long int value)
	{

	}

	void call()
	{
		std::cout << "Calling...\n";
		net::tcp::message<PokerMessages> callMsg;
		callMsg.header.id = PokerMessages::Call;

		send(callMsg);
	}

	void fold()
	{
	}

	void end()
	{
		std::cout << "Requesting disconnection from server...\n";
		disconnect();
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
			std::cerr << "Invalid type of transaction \n";
			return false;
		}
		
		return true;
	}

private:
	long long int m_money;
};


int main()
{
	Client c;
	c.connect("127.0.0.1", 6000);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	bool key[5] = { false };
	bool old_key[5] = { false };

	bool bQuit = false;
	while (!bQuit)
	{
		key[1] = GetAsyncKeyState('1') & 0x8000;
		key[2] = GetAsyncKeyState('2') & 0x8000;
		key[3] = GetAsyncKeyState('3') & 0x8000;
		key[4] = GetAsyncKeyState('4') & 0x8000;

		if (key[1] && !old_key[1]) c.ping();
		if (key[2] && !old_key[2]) bQuit = true;
		if (key[3] && !old_key[3]) c.call();

		for (int i = 1; i <= 4; i++) old_key[i] = key[i];

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (c.is_connected())
		{
			if (!c.incoming().empty())
			{
				auto msgIn = c.incoming().pop_front().message;

				switch (msgIn.header.id)
				{
				case PokerMessages::Ping:
				{
					std::string a;
					msgIn >> a;
					std::cout << a;
					break;
				}
				case PokerMessages::Call:
				{
					std::string a;
					msgIn >> a;
					std::cout << a;
					break;
				}
				}
			}
		}
	}

	return 0;
}
