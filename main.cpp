#include "engine.h"

int main()
{
	try {
		Game::GWorld world(40, 40);
		Gfx::GEngine gr("C:\\Users\\joey\\source\\repos\\game\\Debug\\assets\\atlas.png", "C:\\Users\\joey\\source\\repos\\game\\Debug\\fonts\\Hack-Regular.ttf", 800, 600, world);
		gr.init();
		for (int y = 0; y < 40; y++) {
			for (int x = 0; x < 40; x++) {
				world.set_tile(x, y, { Game::GRASS, 0 });
			}
		}

		gr.gfx_loop();
	} catch (std::exception& e) {
		std::cerr << "AN EXCEPTION OCCURRED: " << e.what() << " (SDL Error: " << SDL_GetError() << ")" << std::endl;
	}

	return 0;
}