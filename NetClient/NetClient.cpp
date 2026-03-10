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
	Sync
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
		msg.header.id = PokerMessages::Ping;
		msg << "ECHO\n";

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
						c.fold();
						break;
					}
					case '3':
					{
						c.call();
						break;
					}
					case '4':
					{
						//Raising double the bet by default (for now)
						c.raise();
						break;
					}
					case '5':
					{
						bQuit = false;
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
					{
						msgIn >> strMsg;
						std::cout << strMsg;
						break;
					}
					case PokerMessages::Sync:
					{
						std::cout << "It`s your turn: \n";
						std::cout << "2 - Fold;\n";
						std::cout << "3 - Call;\n";
						std::cout << "4 - Raise;\n";

						break;
					}
				}
			}
		}
	}

	return 0;
}
