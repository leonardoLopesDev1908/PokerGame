#include "Game.h"
#include "IServer.h"


Game::Game(GameState& gameState) 
	: m_gameState(gameState)
{}

void Game::game_loop()
{
	while (m_gameState.m_run)
	{
		start_game();
		run_game();
		ending_round();
	}
}

void Game::start_game()
{
	net::tcp::message<PokerMessages> msg;
	msg.header.id = PokerMessages::Info;
	msg << "Starting game...\n";

	m_server.messageAll(msg);

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	m_deque.shuffle();

	std::ostringstream oss;
	std::string msgStr;

	for (auto& p : m_gameState.m_players)
	{
		msg.clear();
		oss.str("");

		auto& c = p.second.connection;

		oss << "Your cards: \n";
		for (int i = 0; i < 2; i++)
		{
			m_gameState.m_players[c->getId()].hand[i] = m_deque.getCard();
			oss << m_gameState.m_players[c->getId()].hand[i].value() << (i == 0 ? ", " : "");
		}

		msgStr = oss.str();
		msg << msgStr;
		c->send(msg);
	}
}

void Game::run_game()
{

	{
		std::cout << "Enter the game\n";

		//Small and big blind bets
		m_gameState.m_players[
			m_gameState.m_smallBlind].money -= m_gameState.currentBet / 2;

		m_gameState.m_players[
			m_gameState.m_smallBlind].bet = m_gameState.currentBet / 2;

		m_gameState.m_players[m_bigBlind].money -= m_gameState.currentBet;
		m_gameState.m_players[m_bigBlind].bet = m_gameState.currentBet;

		short currentIndex = 2;
		currentId = m_gameState.m_activePlayersId[currentIndex];

		std::vector<Card> communityCards;

		net::tcp::message<PokerMessages> msgCards;
		msgCards.header.id = PokerMessages::Info;

		std::ostringstream oss;

		Stage stage = Stage::PreFlop;

		//Informing the players about their money
		//at the start of each round
		net::tcp::message<PokerMessages> moneyInfo;
		moneyInfo.header.id = PokerMessages::Info;
		for (auto& p : m_gameState.m_players)
		{
			moneyInfo << "Your money: $" << std::to_string(p.second.money) << "\n";
			p.second.connection->send(moneyInfo);
			moneyInfo.clear();
		}

		while (stage != Stage::Ending && !m_gameState.m_activePlayersId.empty())
		{
			switch (stage)
			{
			case Stage::PreFlop:
			{
				std::cout << "Preflop\n";

				bettingRound(currentIndex);
				stage = m_gameState.m_activePlayersId.size() == 1 ? Stage::CardChecking : Stage::Flop;
				break;
			}
			case Stage::Flop:
			{
				currentIndex = 0;
				currentId = m_gameState.m_activePlayersId[currentIndex];
				std::cout << "Flop\n";

				oss << "FLOP\n";
				for (int i = 0; i < 3; i++)
				{
					communityCards.push_back(m_deque.getCard());
					oss << communityCards.back() << " ";
					oss << '\n';
				}
				msgCards << oss.str();
				m_server.messageAll(msgCards);
				oss.str("");

				bettingRound(currentIndex);
				stage = m_gameState.m_activePlayersId.size() == 1 ? Stage::CardChecking : Stage::Turn;
				break;
			}
			case Stage::Turn:
			{
				currentIndex = 0;
				currentId = m_gameState.m_activePlayersId[currentIndex];
				std::cout << "Turn\n";
				oss << "TURN\n";
				communityCards.push_back(m_deque.getCard());
				oss << communityCards.back() << " ";
				oss << '\n';

				msgCards << oss.str();
				m_server.messageAll(msgCards);
				oss.str("");

				bettingRound(currentIndex);
				stage = m_gameState.m_activePlayersId.size() == 1 ? Stage::CardChecking : Stage::River;
				break;
			}
			case Stage::River:
			{
				currentIndex = 0;
				currentId = m_gameState.m_activePlayersId[currentIndex];
				std::cout << "River\n";
				oss << "RIVER\n";
				communityCards.push_back(m_deque.getCard());
				oss << communityCards.back() << " ";
				oss << '\n';

				msgCards << oss.str();
				m_server.messageAll(msgCards);
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

				if (m_gameState.m_activePlayersId.size() == 1)
				{
					std::cout << "Player " << m_gameState.m_players[m_gameState.m_activePlayersId[0]].id << " won!\n";
					winMsgStr += "You've won the round! ";
					winMsgStr += "The pot for you is: " + std::to_string(m_gameState.m_pot) + '\n';

					winMsg << winMsgStr;
					m_gameState.m_players[m_gameState.m_activePlayersId[0]].connection->send(winMsg);

					m_gameState.m_players[m_gameState.m_activePlayersId[0]].money += m_gameState.m_pot;
				}
				else
				{
					//This loop uses OMPEval lib to evaluate every remaning hand 
					// https://github.com/zekyll/OMPEval
					//Uses static_cast to cast enums Rank and Suit to ints

					uint32_t idWinner;
					long evaluatorIdx = 0;
					for (auto& id : m_gameState.m_activePlayersId)
					{
						omp::Hand hand;
						auto pHand = m_gameState.m_players[id].hand;

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
					m_server.messageAll(winMsg);

					winMsg.clear();
					winMsg << "You won! + $" << std::to_string(m_gameState.m_pot) << "\n";
					m_server.sendMessage(winMsg, idWinner);
				}
				stage = Stage::Ending;
				break;
			}
			}
		}
		msgCards.clear();
	}
}

void Game::stop()
{
	m_gameState.m_run = false;
}

bool Game::equalBets()
{
	if (m_gameState.m_activePlayersId.empty())
		return true;

	for (auto id : m_gameState.m_activePlayersId)
	{
		if (m_gameState.m_players[id].bet != m_gameState.currentBet)
			return false;
	}
	return true;
}

void Game::ending_round()
{
	m_gameState.m_pot = 0;
	m_smallBlind++;
	m_bigBlind++;

	m_gameState.m_activePlayersId.clear();
	for (auto& p : m_gameState.m_players)
	{
		p.second.reset_round();
		m_gameState.m_activePlayersId.push_back(p.first);
	}

	m_deque.reset();
}


void Game::bettingRound(short& currentIndex)
{
	uint8_t actionsThisRound = 0;
	uint8_t playersNeeded = static_cast<int>(m_gameState.m_activePlayersId.size());

	while (m_gameState.m_activePlayersId.size() > 1 &&
		(actionsThisRound < playersNeeded || !equalBets()))
	{
		if (currentIndex > m_gameState.m_activePlayersId.size())
			currentIndex = 0;

		if (!m_gameState.m_players[currentId].folded)
		{
			std::string msgStr;
			net::tcp::message<PokerMessages> msg;
			msg.header.id = PokerMessages::Info;

			msgStr = "Player " + std::to_string(currentId + 1) + " time\n";
			msgStr += "Current bet is $" + std::to_string(m_gameState.currentBet) + '\n';
			msgStr += "Waiting player " + std::to_string(currentId + 1) + " decision\n";
			msg << msgStr;

			for (auto p : m_gameState.m_activePlayersId)
				m_gameState.m_players[p].connection->send(msg);

			//Send a Sync msg to tell the player its his time to play
			net::tcp::message<PokerMessages> sync;
			sync.header.id = PokerMessages::Sync;
			m_server.sendMessage(sync, currentId);

			msgStr.clear();
			msg.clear();

			std::unique_lock<std::mutex> lck(m_mutex);
			m_cond.wait(lck, [this] { return m_gameState.m_playerAction; });
			m_gameState.m_playerAction = false;

			actionsThisRound++;

		}
		currentIndex = (currentIndex + 1) % static_cast<int>(m_gameState.m_activePlayersId.size());
		currentId = m_gameState.m_activePlayersId[currentIndex];
	}
}


