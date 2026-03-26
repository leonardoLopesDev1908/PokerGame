#include "Game.h"
#include "IServer.h"

class Game
{
public:

	Game(
		net::tcp::server<PokerMessages>& server,
		std::unordered_map<uint32_t, Player>& players,
		std::atomic<long long int>& currentBet,
		std::atomic<bool>& run,
		bool& playerAction,
		std::atomic<long long int>& pot
	) 
		: m_server(server), m_players(players), m_playerAction(playerAction),
		m_currentBet(currentBet), m_run(run), m_pot(pot)
	{
		game_loop();
	}

	void game_loop()
	{
		while (m_run)
		{
			start_game();
			game();
			ending_round();
		}
	}

	void start_game()
	{
		net::tcp::message<PokerMessages> msg;
		msg.header.id = PokerMessages::Info;
		msg << "Starting game...\n";

		m_server.message_all(msg);

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		m_deque.shuffle();

		std::ostringstream oss;
		std::string msgStr;

		for (auto& p : m_players)
		{
			msg.clear();
			oss.str("");

			auto& c = p.second.connection;

			oss << "Your cards: \n";
			for (int i = 0; i < 2; i++)
			{
				m_players[c->getId()].hand[i] = m_deque.getCard();
				oss << m_players[c->getId()].hand[i].value() << (i == 0 ? ", " : "");
			}

			msgStr = oss.str();
			msg << msgStr;
			c->send(msg);
		}
	}

	void game()
	{

		{
			std::cout << "Enter the game\n";

			//Small and big blind bets
			m_players[m_smallBlind].money -= currentBet / 2;
			m_players[m_smallBlind].bet = currentBet / 2;

			m_players[m_bigBlind].money -= currentBet;
			m_players[m_bigBlind].bet = currentBet;

			short currentIndex = 2;
			currentId = m_activePlayersId[currentIndex];

			std::vector<Card> communityCards;

			net::tcp::message<PokerMessages> msgCards;
			msgCards.header.id = PokerMessages::Info;

			std::ostringstream oss;

			Stage stage = Stage::PreFlop;

			//Informing the players about their money
			//at the start of each round
			net::tcp::message<PokerMessages> moneyInfo;
			moneyInfo.header.id = PokerMessages::Info;
			for (auto& p : m_players)
			{
				moneyInfo << "Your money: $" << std::to_string(p.second.money) << "\n";
				p.second.connection->send(moneyInfo);
				moneyInfo.clear();
			}

			while (stage != Stage::Ending && !m_activePlayersId.empty())
			{
				switch (stage)
				{
				case Stage::PreFlop:
				{
					std::cout << "Preflop\n";

					bettingRound(currentIndex);
					stage = m_activePlayersId.size() == 1 ? Stage::CardChecking : Stage::Flop;
					break;
				}
				case Stage::Flop:
				{
					currentIndex = 0;
					currentId = m_activePlayersId[currentIndex];
					std::cout << "Flop\n";

					oss << "FLOP\n";
					for (int i = 0; i < 3; i++)
					{
						communityCards.push_back(m_deque.getCard());
						oss << communityCards.back() << " ";
						oss << '\n';
					}
					msgCards << oss.str();
					m_server.message_all(msgCards);
					oss.str("");

					bettingRound(currentIndex);
					stage = m_activePlayersId.size() == 1 ? Stage::CardChecking : Stage::Turn;
					break;
				}
				case Stage::Turn:
				{
					currentIndex = 0;
					currentId = m_activePlayersId[currentIndex];
					std::cout << "Turn\n";
					oss << "TURN\n";
					communityCards.push_back(m_deque.getCard());
					oss << communityCards.back() << " ";
					oss << '\n';

					msgCards << oss.str();
					m_server.message_all(msgCards);
					oss.str("");

					bettingRound(currentIndex);
					stage = m_activePlayersId.size() == 1 ? Stage::CardChecking : Stage::River;
					break;
				}
				case Stage::River:
				{
					currentIndex = 0;
					currentId = m_activePlayersId[currentIndex];
					std::cout << "River\n";
					oss << "RIVER\n";
					communityCards.push_back(m_deque.getCard());
					oss << communityCards.back() << " ";
					oss << '\n';

					msgCards << oss.str();
					m_server.message_all(msgCards);
					oss.str("");

					bettingRound(currentIndex);
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

							int value = m_evaluator.evaluate(hand);
							if (value > evaluatorIdx)
							{
								evaluatorIdx = value;
								idWinner = id;
							}
						}

						std::cout << "Player " << idWinner << " won the round\n";
						net::tcp::message<PokerMessages> winMsg;
						winMsg.header.id = PokerMessages::Info;

						winMsg << "Player " << idWinner << " won the round\n";
						m_server.message_all(winMsg);

						winMsg.clear();
						winMsg << "You won! + $" << std::to_string(m_pot) << "\n";
						m_server.message_client(winMsg, m_players[idWinner].connection);
					}
					stage = Stage::Ending;
					break;
				}
				}
			}
			msgCards.clear();
		}
	}

	void stop()
	{
		m_run = false;
	}

private:
	private:
		bool equalBets()
		{
			if (m_activePlayersId.empty())
				return true;

			for (auto id : m_activePlayersId)
			{
				if (m_players[id].bet != m_currentBet)
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
		}


		void bettingRound(short& currentIndex)
		{
			uint8_t actionsThisRound = 0;
			uint8_t playersNeeded = static_cast<int>(m_activePlayersId.size());

			while (m_activePlayersId.size() > 1 &&
				(actionsThisRound < playersNeeded || !equalBets()))
			{
				if (currentIndex > m_activePlayersId.size())
					currentIndex = 0;

				if (!m_players[currentId].folded)
				{
					std::string msgStr;
					net::tcp::message<PokerMessages> msg;
					msg.header.id = PokerMessages::Info;

					msgStr = "Player " + std::to_string(currentId + 1) + " time\n";
					msgStr += "Current bet is $" + std::to_string(m_currentBet) + '\n';
					msgStr += "Waiting player " + std::to_string(currentId + 1) + " decision\n";
					msg << msgStr;

					for (auto p : m_activePlayersId)
						m_players[p].connection->send(msg);

					//Send a Sync msg to tell the player its his time to play
					net::tcp::message<PokerMessages> sync;
					sync.header.id = PokerMessages::Sync;
					m_server.sendMessage(sync, m_players[currentId]);

					msgStr.clear();
					msg.clear();

					std::unique_lock<std::mutex> lck(m_mutex);
					m_cond.wait(lck, [this] { return m_playerAction; });
					m_playerAction = false;

					actionsThisRound++;

				}
				currentIndex = (currentIndex + 1) % static_cast<int>(m_activePlayersId.size());
				currentId = m_activePlayersId[currentIndex];
			}
		}

private: 
	IServer m_server;
	omp::HandEvaluator m_evaluator;
	std::unordered_map<uint32_t, Player>& m_players;
	std::vector<uint32_t> m_activePlayersId;

	long long INITIAL_AMOUNT = 2500;

	std::condition_variable m_cond;
	std::mutex m_mutex;
	
	std::atomic<long long int>& m_currentBet;
	std::atomic<bool>& m_run;
	bool& m_playerAction;
	std::atomic<long long int>& m_pot;

	uint32_t currentId;
	short m_smallBlind = 1;
	short m_bigBlind = 2;
	long m_playersCount = 0;
	
	Deque m_deque;
};