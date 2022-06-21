#include "world.h"

OPEN_NAMESPACE(Game)

GWorld::GWorld(int w, int h) :
	dim({w, h}),
	cash(20000)
{
	map.resize((size_t)dim.x * dim.y);
	std::fill(map.begin(), map.end(), no_tile);
}

GTile GWorld::get_tile(int x, int y)
{
	if (x < 0 || y < 0 ||
		x >= dim.x || y >= dim.y)
		return no_tile;

	return map.at((size_t)y * dim.y + x);
}

void GWorld::set_tile(int x, int y, GTile tile)
{
	map.at((size_t)y * dim.y + x) = tile;
}

void GWorld::place_tile(int x, int y, GTileType type)
{
	if (!build || type == NOTHING || cash <= 0) return;

	GTile tile = get_tile(x, y);

	if (tile.type != type) {
		cash -= 5 * type;
	}

	set_tile(x, y, { type, tile.height });
}

void GWorld::lift_drop_tile(int x, int y, int off)
{
	if (cash <= 0) return;

	GTile tile = get_tile(x, y);

	if (off) cash -= 20;

	set_tile(x, y, { tile.type, (tile.height + off) % height_limit });
}

void GWorld::announce(bool& show, const char* str, ...)
{
	va_list va;
	va_start(va, str);

	if (!show) {
		ImGui::SetNextWindowPos(ImVec2(
			ImGui::GetIO().DisplaySize.x * 0.5f,
			ImGui::GetIO().DisplaySize.y * 0.5f
		), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		ImGui::SetNextWindowFocus();
		ImGui::Begin("announcement", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
		ImGui::Text(str, va);
		show = ImGui::Button("Close");
		ImGui::End();
	}

	va_end(va);
}

void GWorld::show_gui()
{
	announce(intro_shown, 
		"Welcome to the unnamed city building game. You've been given\n"
		"some money to play around with. Feel free to make whatever  \n"
		"you want. The world is your canvas.");

	ImGui::SetNextWindowSize(ImVec2(300, 300));
	ImGui::Begin("City", NULL, ImGuiWindowFlags_NoResize);

	ImGui::Text("Cash: "); ImGui::SameLine();
	ImGui::TextColored(cash >= 0 ? ImVec4(0, 255, 0, 255) : ImVec4(255, 0, 0, 255), "$%d", cash);

	if (build) {
		ImGui::Combo("Tile", &current, items, IM_ARRAYSIZE(items));
		if (ImGui::Button("Stop Building"))
			build = false;

		ImGui::Text("Radius");
		ImGui::SameLine();
		radius += !!ImGui::Button("+");
		ImGui::SameLine();
		radius -= !!ImGui::Button("-");
		if (radius < 0) radius++;
	}
	else {
		if (ImGui::Button("Build"))
			build = true;
	}

	ImGui::End();
}

void GWorld::tick()
{
	show_gui();
}

GScheduler::GScheduler(GWorld& w) :
	world(w) {}



CLOSE_NAMESPACE