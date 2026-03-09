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

std::ostream& operator<< (std::ostream& os, const Card& card)
{

	static const char* ranks[] = {
		"2", "3", "4", "5", "6", "7", "8",
		"9", "10", "J", "Q", "K", "A"
	};

	static const char* suits[] = {
		"Hearts", "Spades", "Diamond", "Clubs"
	};

	os << ranks[static_cast<int>(card.rank)] << " of " 
		<< suits[static_cast<int>(card.suit)] << '\n';

	return os;
}

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
			std::shuffle(m_cards.begin(), m_cards.end(), 
				std::mt19937{ std::random_device{}()});
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