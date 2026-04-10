#pragma once 
#include "GameState.h"

//This code agregates all shared data between server and game,
//as both need to access and modify data, this struct is a
//thread safe congregation of all the needed data.

void GameState::addPlayer(uint32_t id, PointerConnection& client)
{
    m_players.emplace(
        id, Player{ id, client, {std::nullopt, std::nullopt} }
    );
}

void GameState::changePlayerAction()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    m_playerAction = m_playerAction ? false : true;
    m_cond.notify_one();
}

void GameState::updateCurrentId()
{
}

void GameState::collectBlinds(short smallBlind, short bigBlind)
{
    Player& small = getPlayer(m_activePlayersId[smallBlind]);
    small.setMoney(small.getMoney() - m_currentBet / 2);
    small.setBet(m_currentBet / 2);

    std::string smallMsgStr = "Small blind collected from you\n";
    net::tcp::message<PokerMessages> smallMsg;
    smallMsg.header.id = PokerMessages::Info;
    smallMsg << smallMsgStr;
    small.message(smallMsg);

    Player& big = getPlayer(m_activePlayersId[bigBlind]);
    big.setMoney(big.getMoney() - m_currentBet);
    big.setBet(m_currentBet);

    std::string bigMsgStr = "Big blind collected from you\n";
    net::tcp::message<PokerMessages> bigMsg;
    bigMsg.header.id = PokerMessages::Info;
    bigMsg << bigMsgStr;
    big.message(bigMsg);
}

void GameState::updatePlayerBet(uint32_t id)
{
    m_players[id].setBet(m_currentBet);
}

void GameState::removeActivePlayer(const uint32_t id)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    m_activePlayersId.erase(
        std::remove(m_activePlayersId.begin(), m_activePlayersId.end(),
            id), m_activePlayersId.end()
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
    return m_players.at(id);
}

long long int GameState::getCurrentBet()
{
    return m_currentBet;
}

void GameState::setPlayerFold(const uint32_t id)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    m_players[id].m_folded = true;
}