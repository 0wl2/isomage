#pragma once

#include "util.h"

#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

OPEN_NAMESPACE(Game)

enum GTileType {
	NOTHING = -1,
	HILIGHT = 0,
	GRASS = 1,
	WATER = 2,
	CONCRETE = 3,
	ROAD = 4,
	RESIDENTIAL = 5
};

struct GTile {
	GTileType type;
	int height;
} static no_tile = { NOTHING, 0 };

class GWorld {
public:
	GWorld(int, int);
	
	void tick();
	void show_gui();

	GTile get_tile(int, int);
	void set_tile(int, int, GTile);

	void place_tile(int, int, GTileType);
	void lift_drop_tile(int, int, int);
	
	int cash;
	bool build = false;
	bool intro_shown = false;

	GVec2<int> dim;
	int height_limit = 60;
	int radius = 0;

	bool draw_line = false;
	GVec2<int> line[2] = { invalid, invalid };

	std::vector<GTile> map;

	int current = 0;
	const char* items[5] = { "Grass", "Water", "Concrete", "Road", "Residential" };

private:
	void announce(bool&, const char*, ...);
};

class GScheduler {
public:
	GScheduler(GWorld&);
private:
	GWorld& world;
};

CLOSE_NAMESPACE