#pragma once 
#include <deque_card.h>
#include <Game.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <omp/HandEvaluator.h>
#include <functional>

//This code agregates all shared data between server and game,
//as both need to access and modify data, this struct is a
//thread safe congregation of all the needed data.

/*gameState.withLock([&](GameState& state) {
    state.m_pot += state.currentBet;
    state.m_players[id].money += 250;
};
*/


class GameState
{
public:

    void withPlayer(uint32_t id, std::function<void> lambda)
    {
        std::lock_guard<std::mutex> lck(mutex);
        lambda();
    }

    void updateCurrentBet(long long int newBet)
    {
        m_currentBet = newBet;
    }

    void updateBlinds()
    {
        m_smallBlind = (m_smallBlind + 1) % m_playersCount;
        m_bigBlind = (m_bigBlind + 1) % m_playersCount;
    }

    void updatePot(long long int amount)
    {
        std::lock_guard<std::mutex> lck(mutex);
        m_pot += amount;
    }

    void updateActivePlayersId()
    {
        m_activePlayersId = (m_activePlayersId + 1) % m_playersCount;
    }

    void changePlayerAction()
    {
        std::lock_guard<std::mutex> lck(mutex);
        m_playerAction = m_playerAction ? false : true;
    }

    void updateCurrentId()
    {
    
    }
    
private:
    long long int currentBet = 50;
	short m_smallBlind = 1;
	short m_bigBlind = 2;
	long long INITIAL_AMOUNT = 2500;

	long long int m_pot = 0;
	long m_playersCount = 0;
    uint32_t currentId;
	
    std::unordered_map<uint32_t, Player> m_players;
	std::vector<uint32_t> m_activePlayersId;

	omp::HandEvaluator evaluator;
	std::thread m_threadGame;
	
    bool m_playerAction = false;
	bool run = true;

};



