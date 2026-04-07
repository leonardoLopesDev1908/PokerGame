#pragma once
#include "Player.h"

#include <poker_messages.h>
#include <memory>
#include <olc_net_server.h>
#include <unordered_map>
#include <vector>
#include <mutex>

using PointerConnection = std::shared_ptr<net::tcp::connection<PokerMessages>>;

class GameState
{
    friend class Game;
public:

    template <typename Func>
    void withLock(Func f)
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        f(*this);
    }

    void updateBlinds();

    void addPlayer(uint32_t id, PointerConnection& connection);

    void changePlayerAction();

    void updateCurrentId();   

    void collectBlinds(short small, short big);

    void updatePlayerBet(uint32_t id);

    void removeActivePlayer(const uint32_t id);

    void raise();

    void call();

    Player& getPlayer(uint32_t id);

    long long int getCurrentBet();

public:
    long long INITIAL_AMOUNT = 2500;
    std::vector<uint32_t> m_activePlayersId;
    long m_playersCount = 0;
    std::mutex m_mutex;

private:
    long long int m_currentBet = 50;
    long long int m_pot = 0;

    std::unordered_map<uint32_t, Player> m_players;

    bool m_playerAction = false;
    bool m_run = true;
};



