#include <olc_net_server.h>
#include <unordered_map>
#include <deque_card.h>
#include <optional>
#include <array>
#include "omp/HandEvaluator.h"
#include "omp/Hand.h"

enum class Stage : uint8_t
{
	PreFlop,
	Flop,
	Turn,
	River,
	CardChecking,
	Ending
};

enum class PokerMessages
{
	Ping,
	Info,
	Fold,
	Call,
	Raise,
	Sync
};

struct Player
{
	uint32_t id;
	long long int money = 2500;
	long long int currentBet = 0;
	std::array<std::optional<Card>, 2> hand = { std::nullopt, std::nullopt };
	std::shared_ptr<net::tcp::connection<PokerMessages>> connection;
	bool folded = false;

	void reset_round()
	{
		currentBet = 0;
		hand = { std::nullopt, std::nullopt };
		folded = false;
	}
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
				m_players[c->getId()].hand[i] = m_deque.getCard();
				oss << m_players[c->getId()].hand[i].value() << (i == 0 ? ", " : "");
			}

			msgStr = oss.str();
			msg << msgStr;
			message_client(msg, c);
		}
		game();
	}

	void game()
	{
		std::cout << "Enter the game\n";
		//Rebuild small and big logics
		/*m_playersMoney[m_smallBlind] -= currentBet / 2;
		m_playersMoney[m_bigBlind] -= currentBet;*/
		int currentPlayerIdx = 2;

		std::vector<Card> communityCards;

		net::tcp::message<PokerMessages> msgCards;
		msgCards.header.id = PokerMessages::Info;

		Stage stage = Stage::PreFlop;

		std::cout << "enter the sleep\n";
		std::this_thread::sleep_for(std::chrono::seconds(2));

		while (stage != Stage::Ending && !m_activePlayersId.empty())
		{
			currentId = m_activePlayersId[currentPlayerIdx];
			switch (stage)
			{
			case Stage::PreFlop:
			{
				bettingRound(currentPlayerIdx);
				stage = Stage::Flop;
				break;
			}
			case Stage::Flop:
			{
				for (int i = 0; i < 3; i++)
				{
					communityCards.push_back(m_deque.getCard());
					msgCards << communityCards.back() << " ";
					msgCards << '\n';
				}

				message_all(msgCards);

				bettingRound(currentPlayerIdx);
				stage = Stage::Turn;
				break;
			}
			case Stage::Turn:
			{
				communityCards.push_back(m_deque.getCard());
				msgCards << communityCards.back() << " ";
				msgCards << '\n';
				message_all(msgCards);

				bettingRound(currentPlayerIdx);
				stage = Stage::River;
				break;
			}
			case Stage::River:
			{
				communityCards.push_back(m_deque.getCard());
				msgCards << communityCards.back() << " ";
				msgCards << '\n';
				message_all(msgCards);

				bettingRound(currentPlayerIdx);
				stage = Stage::CardChecking;
				break;
			}
			case Stage::CardChecking:
			{
				std::string winMsgStr;
				net::tcp::message<PokerMessages> winMsg;
				winMsg.header.id = PokerMessages::Info;

				if (m_activePlayersId.size() == 1)
				{
					std::cout << "Player " << m_players[m_activePlayersId[0]].id << " won!\n";
					winMsgStr += "You've won the round! ";
					winMsgStr += "The pot for you is: " + std::to_string(m_pot) + '\n';

					winMsg << winMsgStr;
					m_players[m_activePlayersId[0]].connection->send(winMsg);

					m_players[m_activePlayersId[0]].money += m_pot;
				}
				else
				{
					//This loop uses OMPEval lib to evaluate every remaning hand 
					// https://github.com/zekyll/OMPEval
					//Uses static_cast to cast enums Rank and Suit to ints

					uint32_t idWinner;
					long evaluatorIdx = 0;
					for (auto& id : m_activePlayersId)
					{
						omp::Hand hand;
						auto pHand = m_players[id].hand;

						hand += 4 * static_cast<int>(pHand[0]->rank) + static_cast<int>(pHand[0]->suit);
						hand += 4 * static_cast<int>(pHand[1]->rank) + static_cast<int>(pHand[1]->suit);

						for (auto& card : communityCards)
						{
							hand += 4 * static_cast<int>(card.rank) + static_cast<int>(card.suit);
						}

						int value = evaluator.evaluate(hand);
						if (value > evaluatorIdx)
						{
							evaluatorIdx = value;
							idWinner = id;
						}
					}

					std::cout << "Player " << idWinner << " won the round\n";
					net::tcp::message<PokerMessages> winMsg;
					winMsg << "You won! + $" << std::to_string(m_pot) << "\n";
					message_client(winMsg, m_players[idWinner].connection);

				}
				stage = Stage::Ending;
				break;
			}
			}
		}
		msgCards.clear();
		ending_round();
	}

	private:
		bool equalBets()
		{
			if (m_activePlayersId.empty())
				return true;

			auto firstBet = m_players[m_activePlayersId[0]].currentBet;
			for (auto id : m_activePlayersId)
			{
				if (m_players[id].currentBet != firstBet)
					return false;
			}
			return true;
		}


	void ending_round()
	{
		m_pot = 0;
		m_smallBlind++;
		m_bigBlind++;

		m_activePlayersId.clear();
		for (auto& p : m_players)
		{
			p.second.reset_round();
			m_activePlayersId.push_back(p.first);
		}

		m_deque.reset();
		start_game();
	}

	void bettingRound(int& currentPlayerIdx)
	{

		while (m_activePlayersId.size() > 1 && !equalBets())
		{
			std::cout << "enter the bettingRound\n";
			std::this_thread::sleep_for(std::chrono::seconds(2));
			std::string msgStr;
			net::tcp::message<PokerMessages> msg;
			msg.header.id = PokerMessages::Info;

			msgStr = "Player " + std::to_string(currentPlayerIdx) + " time\n";
			msgStr += "Current bet is $" + std::to_string(currentBet) + '\n';
			msgStr += "Waiting player " + std::to_string(currentPlayerIdx) + " decision\n";
			msg << msgStr;

			for (auto p : m_activePlayersId)
				m_players[p].connection->send(msg);

			//Send a Sync msg to tell the player its his time to play
			net::tcp::message<PokerMessages> sync;
			sync.header.id = PokerMessages::Sync;
			message_client(sync, m_players[currentId].connection);

			std::unique_lock<std::mutex> lck(m_mutex);
			m_cond.wait(lck, [this] { return m_playerAction; });
			m_playerAction = false;

			msgStr.clear();
			msg.clear();
			currentPlayerIdx = (currentPlayerIdx + 1) % static_cast<int>(m_activePlayersId.size());
			currentId = m_activePlayersId[currentPlayerIdx];
		}
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
					
				m_activePlayersId.erase(
					std::remove(m_activePlayersId.begin(), m_activePlayersId.end(),
						remote->getId()), m_activePlayersId.end()
				);

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
				m_players[remote->getId()].money -= currentBet;

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

				m_players[remote->getId()].money -= currentBet;
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

		m_activePlayersId.push_back(client->getId());
		std::cout << ++m_playersCount << "º player connected\n";

		m_players.insert({ client->getId(),
				{client->getId(), INITIAL_AMOUNT, 0, {std::nullopt, std::nullopt},
				client, false}
		});

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
	int currentId;

	std::unordered_map<uint32_t, Player> m_players;
	std::vector<uint32_t> m_activePlayersId;

	omp::HandEvaluator evaluator;
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