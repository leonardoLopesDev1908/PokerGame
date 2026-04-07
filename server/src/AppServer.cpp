#include "Server.h"

int main()
{
	GameState gameState{};
	Game game(gameState);

	Server s(6000, gameState, game);
	s.start();

	while (true)
		s.update();

	return 0;
}
