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

	explicit Server(uint16_t port, GameState& gameState, Game& game);
	
    virtual void sendMessage(net::tcp::message<PokerMessages>& msg, uint32_t playerId);

	virtual void messageAll(net::tcp::message<PokerMessages>& msg);

    virtual void on_message(net::tcp::message<PokerMessages>& msg,
		std::shared_ptr<net::tcp::connection<PokerMessages>> remote);

	virtual void on_client_connect();

	virtual void on_client_disconnect();

private:
	Game& m_game;
	GameState& m_gameState;

	std::thread m_threadGame;
	std::mutex m_mutex;
	std::condition_variable m_cond;
};

