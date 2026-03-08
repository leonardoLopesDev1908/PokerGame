#include <deque>
#include <algorithm>
#include <random>

class Card
{
	enum class Rank
	{
		TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
		TEN, JACK, QUEEN, KING, ACE
	};

	enum class Suit
	{
		HEARTS, SPADES, DIAMOND, CLUBS
	};

public:
	Suit suit;
	Rank rank;

	Card(int aSuit, int aRank)
	{
		suit = static_cast<Suit>(aSuit);
		rank = static_cast<Rank>(aRank);
	}
};

class Deque
{
	public:
		Deque()
		{
			fill();
		}

		Card& getCard()
		{
			auto& card = m_cards.back();
			m_cards.pop_back();
			return card;
		}

		void putCard(Card&& c)
		{
			m_cards.push_back(c);
		}

		void shuffle()
		{
			std::shuffle(m_cards.begin(), m_cards.end(), std::mt19937{ std::random_device{}()});
		}

private:
	void fill()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 13; j++)
			{
				m_cards.emplace_back(i, j);
			}
		}
		shuffle();
	}

private:
	std::deque<Card> m_cards;
};