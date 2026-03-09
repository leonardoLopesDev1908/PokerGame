#include <iostream>
#include <olc_net_client.h>
#include <deque_card.h>

enum class PokerMessages
{
	Ping,
	Info,
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

	void raise()
	{
		net::tcp::message<PokerMessages> raise;
		raise.header.id = PokerMessages::Raise;
		send(raise);
	}

	void call()
	{
		net::tcp::message<PokerMessages> callMsg;
		callMsg.header.id = PokerMessages::Call;
		send(callMsg);
	}

	void fold()
	{
		net::tcp::message<PokerMessages> foldMsg;
		foldMsg.header.id = PokerMessages::Fold;
		send(foldMsg);
	}

	void end()
	{
		std::cout << "Requesting disconnection from server...\n";
		disconnect();
	}

private:
	long long int m_money;
};


int main()
{
	Client c;
	c.connect("127.0.0.1", 6000);

	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	INPUT_RECORD record;

	DWORD eventsRead;

	bool bQuit = false;
	while (!bQuit)
	{
		ReadConsoleInput(hInput, &record, 1, &eventsRead);

		if (record.EventType == KEY_EVENT)
		{
			KEY_EVENT_RECORD key = record.Event.KeyEvent;
			if (key.bKeyDown)
			{
				char k = key.uChar.AsciiChar;

				switch (k)
				{
					case '1':
					{
						c.ping();
						break;
					}
					case '2':
					{
						c.call();
						break;
					}
					case '3':
					{
						bQuit = true;
						break;
					}
					case '4':
					{
						//Raising double the bet by default (for now)
						c.raise();
						break;
					}
				}
			}
		}

		if (c.is_connected())
		{
			if (!c.incoming().empty())
			{
				auto msgIn = c.incoming().pop_front().message;
				std::string strMsg;

				switch (msgIn.header.id)
				{
				case PokerMessages::Ping:
				{
					msgIn >> strMsg;
					std::cout << strMsg;
					break;
				}
				case PokerMessages::Call:
				{
					msgIn >> strMsg;
					std::cout << strMsg;
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					break;
				}
				case PokerMessages::Info:
					msgIn >> strMsg;
					std::cout << strMsg;
					break;
				}
			}
		}
	}

	return 0;
}
