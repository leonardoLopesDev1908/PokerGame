#include <olc_net_server.h>
#include <unordered_map>
#include <deque_card.h>
#include <optional>
#include <array>

enum class PokerMessages
{
	Ping,
	Info,
	Fold,
	Call,
	Raise,
	Sync
};

class Server : public net::tcp::server<PokerMessages>
{
public:

	explicit Server(uint16_t port)
		: net::tcp::server<PokerMessages>(port)
	{
	}

	void start_game()
	{
		net::tcp::message<PokerMessages> msg;
		msg.header.id = PokerMessages::Info;
		msg << "Starting game...\n";

		message_all(msg);

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		m_deque.shuffle();

		std::ostringstream oss;
		std::string msgStr;

		for (auto& c : m_connections)
		{
			msg.clear();
			oss.str("");

			oss << "Your cards: \n";
			for (int i = 0; i < 2; i++)
			{
				m_hands[c->getId()][i] = m_deque.getCard();
				oss << m_hands[c->getId()][i].value()
					<< (i == 0 ? ", " : "");
			}

			msgStr = oss.str();
			msg << msgStr;
			message_client(msg, c);
		}
		game();
	}

	void game()
	{
		m_players[m_smallBlind] -= currentBet / 2;
		m_players[m_bigBlind] -= currentBet;

		bool bEnding = false;
		int currentPlayerIdx = 3;
		uint32_t currentId;
		std::string msgStr;

		net::tcp::message<PokerMessages> msg;
		msg.header.id = PokerMessages::Info;
		
		while (!bEnding)
		{
			currentId = m_playersOrder[currentPlayerIdx];

			msgStr = "Player " + std::to_string(currentId) + " time\n";
			msgStr += "Current bet is $" + std::to_string(currentBet) + '\n';
			msgStr += "Waiting player " + std::to_string(currentId) + " decision\n";
			msg << msgStr;
			
			//Change message_all to a send just for the remaining players
			message_all(msg);
			
			//Send a Sync msg to tell the player its his time to play
			net::tcp::message<PokerMessages> sync;
			sync.header.id = PokerMessages::Sync;
			message_client(sync, m_playersConnections[currentId]);

			std::unique_lock<std::mutex> lck(m_mutex);
			m_cond.wait(lck, [this]{ return m_playerAction; });

			m_playerAction = false;
			msgStr.clear();
			msg.clear();

			//The 5th player keeps the time and never passes it
			currentPlayerIdx = (currentPlayerIdx <= 5) ? currentPlayerIdx++ : 0;
		}

		ending_round();
	}

	void ending_round()
	{
		m_smallBlind++;
		m_bigBlind++;

	}

	virtual void on_message(net::tcp::message<PokerMessages>& msg,
		std::shared_ptr<net::tcp::connection<PokerMessages>> remote)
	{
		switch (msg.header.id)
		{
			case PokerMessages::Ping:
			{
				std::cout << "Ping from client " << remote->getId() << '\n';
				net::tcp::message<PokerMessages> echoMessage;
				echoMessage.header.id = PokerMessages::Ping;
				echoMessage << msg;
				remote->send(echoMessage);
				break;
			}
			case PokerMessages::Fold:
			{
				std::string message = "Player " + std::to_string(remote->getId()) + " fold\n";
				std::cout << message;

				net::tcp::message<PokerMessages> returnMsg;
				returnMsg << message;
				returnMsg.header.id = PokerMessages::Info;
					
				auto end = std::remove(m_playersOrder.begin(), m_playersOrder.end(),
								remote->getId());

				message_all(returnMsg, remote);
				m_playerAction = true;
				m_cond.notify_one();
				break;
			}
			case PokerMessages::Call:
			{
				std::string msgCall = "Player " + std::to_string(remote->getId()) + " called\n";
				net::tcp::message<PokerMessages> returnMsg;
				returnMsg << msgCall;
				returnMsg.header.id = PokerMessages::Info;

				m_pot += currentBet;
				m_players[remote->getId()] -= currentBet;

				message_all(returnMsg, remote);
				m_playerAction = true;
				m_cond.notify_one();
				break;
			}
			case PokerMessages::Raise:
			{
				std::string msgRaise = "Player " + std::to_string(remote->getId()) + " raised\n";
				net::tcp::message<PokerMessages> returnMsg;
				returnMsg.header.id = PokerMessages::Info;
				
				currentBet *= 2;

				m_players[remote->getId()] -= currentBet;
				m_pot += currentBet;

				m_playerAction = true;
				m_cond.notify_one();
				break;
			}
		}
	}

	virtual void on_client_connect()
	{
		auto& client = m_connections.back();

		m_playersOrder.push_back(client->getId());
		std::cout << ++m_playersCount << "º player connected\n";

		m_players.insert({client->getId(), INITIAL_AMOUNT});
		m_hands[client->getId()] = { std::nullopt, std::nullopt };
		m_playersConnections[client->getId()] = client;

		if (m_playersCount == 5)
		{
			std::cout << "Lets start the game\n";
			m_threadGame = std::thread([this](){start_game();});
			m_threadGame.detach();
		}
		else
		{
			net::tcp::message<PokerMessages> msg;
			msg << "Waiting " << std::to_string(5 - m_playersCount) << " players\n";
			msg.header.id = PokerMessages::Info;
			message_all(msg);
			wait_client_connect();
		}
	}

	virtual void on_client_disconnect()
	{
	}

public:
	long long int currentBet = 50;
	short m_smallBlind = 1;
	short m_bigBlind = 2;

private:
	long long INITIAL_AMOUNT = 2500;
	long long int m_pot = 0;
	long m_playersCount = 0;

	std::vector<uint32_t> m_playersOrder;
	std::unordered_map<uint32_t, long long int> m_players;
	std::unordered_map<uint32_t, std::array<std::optional<Card>, 2>> m_hands;
	std::unordered_map <uint32_t, std::shared_ptr<net::tcp::connection<PokerMessages>>> m_playersConnections;

	std::thread m_threadGame;
	std::mutex m_mutex;
	std::condition_variable m_cond;

	bool m_playerAction = false;

	Deque m_deque;
};

int main()
{
	Server s(6000);
	s.start();

	while (true)
		s.update();

	return 0;
}