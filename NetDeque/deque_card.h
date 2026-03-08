#include <deque>
#include <algorithm>

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

	Card()
	{}
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
			return m_cards.pop_back();
		}

		void putCard(Card&& c)
		{
			m_cards.push_back(c);
		}

		void shuffle()
		{
			std::shuffle(m_cards.begin(), m_cards.end());
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