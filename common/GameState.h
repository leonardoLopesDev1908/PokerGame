#pragma once
#include "Player.h"

class GameState
{
    friend class Game;
public:

    template <typename Func>
    void withLock(Func f);

    void updateCurrentBet(long long int newBet);

    void updateBlinds();

    void updatePot(long long int amount);

    void updateActivePlayersId();

    void changePlayerAction();

    void updateCurrentId();   

    void removeActivePlayer(const uint32_t id);

    Player& getPlayer(uint32_t id);

public:
    long long INITIAL_AMOUNT = 2500;
    std::vector<uint32_t> m_activePlayersId;
    long m_playersCount = 0;
    std::mutex m_mutex;

private:
    long long int currentBet = 50;
    short m_smallBlind = 1;
    short m_bigBlind = 2;

    long long int m_pot = 0;
    uint32_t currentId;

    std::unordered_map<uint32_t, Player> m_players;

    bool m_playerAction = false;
    bool m_run = true;
};



