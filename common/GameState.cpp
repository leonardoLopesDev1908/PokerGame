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

template <typename Func>
void GameState::withLock(Func f)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    f(*this);
}

void GameState::updateCurrentBet(long long int newBet)
{
    m_currentBet = newBet;
}

void GameState::updateBlinds()
{
    m_smallBlind = (m_smallBlind + 1) % m_playersCount;
    m_bigBlind = (m_bigBlind + 1) % m_playersCount;
}

void GameState::updatePot(long long int amount)
{
    std::lock_guard<std::mutex> lck(mutex);
    m_pot += amount;
}

void GameState::updateActivePlayersId()
{
    m_activePlayersId = (m_activePlayersId + 1) % m_playersCount;
}

void GameState::changePlayerAction()
{
    std::lock_guard<std::mutex> lck(mutex);
    m_playerAction = m_playerAction ? false : true;
}

void GameState::updateCurrentId()
{
        
}

void GameState::updatePlayerBet(uint32_t id)
{
    m_players[id].bet = m_currentBet;
}

void GameState::removeActivePlayer(const uint32_t id)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    m_activePlayersId.erase(
        std::remove(m_activePlayersId.begin(), m_activePlayersId.end(),
            remote->getId()), m_activePlayersId.end()
    );
}

void GameState::raise()
{  
    std::lock_guard<std::mutex> lck(m_mutex);
    m_currentBet *= 2;
    m_pot += m_currentBet;
}

void GameState::call()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    m_pot += m_currentBet;
}

Player& GameState::getPlayer(uint32_t id)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    m_players.at(id);
}


    

