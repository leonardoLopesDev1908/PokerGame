#include "Client.h"

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
		DWORD eventsAvailable = 0;
		GetNumberOfConsoleInputEvents(hInput, &eventsAvailable);

		if (eventsAvailable > 0)
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
						bQuit = true;
						break;
					}
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
