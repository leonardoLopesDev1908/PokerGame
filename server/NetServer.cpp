#include <olc_net_server.h>

#include <unordered_map>
#include <optional>
#include <array>

#include <deque_card.h>
#include <poker_messages.h>
#include <GameState.h>

#include <Game.h>
#include <IServer.h>

//Server deal only with the connection logic and create
//a separate entity to deal the game logic
class Server : public net::tcp::server<PokerMessages>, public IServer
{
public:

	explicit Server(uint16_t port, GameState& gameState, Game& game)
		: net::tcp::server<PokerMessages>(port),
		m_gameState(gameState), m_game(game)
	{
	}

	//Implementation to the game-server interface
	virtual void sendMessage(net::tcp::message<PokerMessages>& msg, uint32_t playerId)
	{
		m_gameState.withLock([&](GameState& state) {
			Player& player = state.getPlayer(playerId);
			player.message(msg);  
		});
	}

	virtual void messageAll(net::tcp::message<PokerMessages>& msg)
	{
		message_all(msg);
	}

	//Implementation to the net_server interface
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
				std::string message = "Player " + std::to_string(remote->getId() + 1) + " fold\n";
				std::cout << message;

				net::tcp::message<PokerMessages> returnMsg;
				returnMsg << message;
				returnMsg.header.id = PokerMessages::Info;
					
				m_gameState.removeActivePlayer(remote->getId());

				m_players[remote->getId()].folded = true;

				message_all(returnMsg, remote);
				{
					std::lock_guard<std::mutex> lck(m_mutex);
					m_playerAction = true;
				}
				m_cond.notify_one();
				break;
			}
			case PokerMessages::Call:
			{
			    if(m_players[remote->getId()].money < currentBet)
                {
                    //call with all-in   
                }
                std::string msgCall = "Player " + std::to_string(remote->getId() + 1) + " called\n";
				net::tcp::message<PokerMessages> returnMsg;
				returnMsg << msgCall;
				returnMsg.header.id = PokerMessages::Info;

				m_pot += currentBet;
				m_players[remote->getId()].money -= currentBet - m_players[remote->getId()].bet;
				m_players[remote->getId()].bet = currentBet;

				message_all(returnMsg, remote);
				{
					std::lock_guard<std::mutex> lck(m_mutex);
					m_playerAction = true;
				}
				m_cond.notify_one();
				
				break;
			}
			case PokerMessages::Raise:
			{
                if(m_players[remote->getId()].money < currentBet * 2)
                {
                    std::string invalidFold = "You do not have enough money to raise. Try call it\n";
                    net::tcp::message<PokerMessages> msgInvalidFold;
                    msgInvalidFold.header.id = PokerMessages::Info;
                    msgInvalidFold << invalidFold;
                    message_client(msgInvalidFold, remote);
                    break;
                }
				std::string msgRaise = "Player " + std::to_string(remote->getId() + 1) + " raised\n";
				net::tcp::message<PokerMessages> returnMsg;
				returnMsg.header.id = PokerMessages::Info;
				message_all(returnMsg, remote);

				currentBet *= 2;

				m_players[remote->getId()].money -= currentBet;
				m_players[remote->getId()].bet = currentBet;
				m_pot += currentBet;

				{
					std::lock_guard<std::mutex> lck(m_mutex);
					m_playerAction = true;
				}
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
			m_threadGame = std::thread([this](){
				m_game.game_loop();
			});
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

private:
	Game& m_game;
	GameState& m_gameState;

	std::thread m_threadGame;
	std::mutex m_mutex;
	std::condition_variable m_cond;
};

int main()
{
	GameState gameState{};
	Game game(gameState);

	Server s(6000, gameState, game);
	s.start();

	while (true)
		s.update();

	return 0;
}
