#include "Server.h"

using PointerConnection = std::shared_ptr<net::tcp::connection<PokerMessages>>;

Server::Server(uint16_t port, GameState& gameState, Game& game)
	: net::tcp::server<PokerMessages>(port),
	m_gameState(gameState), m_game(game)
{
	m_game.setServer(this);
}

void Server::sendMessage(net::tcp::message<PokerMessages>& msg, uint32_t playerId)
{
	m_gameState.withLock([&](GameState& state) {
		Player& player = state.getPlayer(playerId);
		player.message(msg);  
	});
}

void Server::messageAll(net::tcp::message<PokerMessages>& msg)
{
	message_all(msg);
}

//Implementation to the net_server interface
void Server::on_message(net::tcp::message<PokerMessages>& msg,
						PointerConnection remote)
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
					
			//m_gameState.removeActivePlayer(remote->getId());

			m_gameState.setPlayerFold(remote->getId());

			message_all(returnMsg, remote);
			
			m_gameState.changePlayerAction();
			break;
		}
		case PokerMessages::Call:
		{
			/*if (m_players[remote->getId()].money < currentBet)
			{
				call with all-in   
			}*/
			std::string msgCall = "Player " + std::to_string(remote->getId() + 1) + " called\n";
			std::cout << msgCall;

			net::tcp::message<PokerMessages> returnMsg;
			returnMsg << msgCall;
			returnMsg.header.id = PokerMessages::Info;

			m_gameState.call();

			m_gameState.withLock([&](GameState& state){
				Player& player = state.getPlayer(remote->getId());
				player.setMoney(player.getMoney() - (state.getCurrentBet() - player.getBet()));
			});
				
			m_gameState.withLock([&](GameState& state){
				state.updatePlayerBet(remote->getId());
			});

			message_all(returnMsg, remote);
				
			m_gameState.changePlayerAction();
				
			break;
		}
		case PokerMessages::Raise:
		{
			/*if(m_players[remote->getId()].money < currentBet * 2)
			{
				std::string invalidFold = "You do not have enough money to raise. Try call it\n";
				net::tcp::message<PokerMessages> msgInvalidFold;
				msgInvalidFold.header.id = PokerMessages::Info;
				msgInvalidFold << invalidFold;
				message_client(msgInvalidFold, remote);
				break;
			}*/
			std::string msgRaise = "Player " + std::to_string(remote->getId() + 1) + " raised\n";
			net::tcp::message<PokerMessages> returnMsg;
			returnMsg.header.id = PokerMessages::Info;
			returnMsg << msgRaise;
			message_all(returnMsg, remote);

			m_gameState.raise();

			m_gameState.withLock([&](GameState& state) {
				Player& player = state.getPlayer(remote->getId());
				player.setMoney(player.getMoney() - state.getCurrentBet());
			});

			m_gameState.withLock([&](GameState& state) {
				state.updatePlayerBet(remote->getId());
			});

			m_gameState.changePlayerAction();
			break;
		}
	}
}

void Server::on_client_connect()
{
	auto& client = m_connections.back();

	m_gameState.m_activePlayersId.push_back(client->getId());
	std::cout << ++m_gameState.m_playersCount << "º player connected\n";

	m_gameState.addPlayer(client->getId(), client);

	if (m_gameState.m_playersCount == 5)
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
		msg << "Waiting " << std::to_string(5 - m_gameState.m_playersCount) << " players\n";
		msg.header.id = PokerMessages::Info;
		message_all(msg);
		wait_client_connect();
	}
}

void Server::on_client_disconnect()
{
}
