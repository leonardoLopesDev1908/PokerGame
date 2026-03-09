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
	}

	void ending_round()
	{

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
				echoMessage << msg;
				remote->send(echoMessage);
				break;
			}
			case PokerMessages::Fold:
			{
				std::string message = "Fold of Player " + std::to_string(remote->getId());
				std::cout << message;

				net::tcp::message<PokerMessages> returnMsg;
				returnMsg << message;

				auto end = std::remove(m_remainingPlayers.begin(), m_remainingPlayers.end(),
								remote->getId());

				message_all(returnMsg, remote);
				break;
			}
			case PokerMessages::Call:
			{
				std::string msgCall = "Player " + std::to_string(remote->getId()) + " called\n";
				net::tcp::message<PokerMessages> returnMsg;
				returnMsg << msgCall;
				returnMsg.header.id = PokerMessages::Call;

				m_pot += currentBet;
				m_players[remote->getId()] -= currentBet;

				message_all(returnMsg, remote);
				break;
			}
			case PokerMessages::Raise:
			{
				std::string bet;
				net::tcp::message<PokerMessages> copy;
				msg >> bet;
				std::cout << "Raise: " << bet << '\n';

				m_players[remote->getId()] -= std::stoi(bet);

				std::cout << "Player " << remote->getId() << " net ammount: "
					<< m_players[remote->getId()];
				break;
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
		auto& client = m_connections.back();

		std::cout << "On client connect\n";
		std::cout << ++m_playersCount << "º player connected\n";
		m_players.insert({client->getId(), INITIAL_AMOUNT});
		m_hands[client->getId()] = { std::nullopt, std::nullopt };
		if (m_playersCount == 5)
		{
			start_game();	
		}
		else
		{
			net::tcp::message<PokerMessages> msg;
			msg << "Waiting " << std::to_string(5 - m_playersCount) << " players\n";
			message_all(msg);
			wait_client_connect();
		}
	}

	virtual void on_client_disconnect()
	{
	}
	
public:
	long long int currentBet = 50;

private:
	long long INITIAL_AMOUNT = 2500;
	long long int m_pot = 0;
	long m_playersCount = 0;

	std::array<uint64_t, 5> m_remainingPlayers;
	std::unordered_map<uint64_t, long long int> m_players;
	std::unordered_map<uint64_t, std::array<std::optional<Card>, 2>> m_hands;

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