#pragma once

#pragma warning(push, 0)
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#pragma warning(pop)

#undef main
#include "util.h"
#include "world.h"

#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

OPEN_NAMESPACE(Gfx)

// magic numbers
#define TEX_TOP_OFF -16
#define TEX_MIDDLE -1
#define TEX_RIGHT_OFF -14
#define TEX_RIGHT_TOP -32

// Texture wrapper
class GTexture {
public:
	GTexture(SDL_Renderer**, const char*, Game::GTileType);
	GTexture(SDL_Texture*);
	
	GTexture(const GTexture&) = delete;
	GTexture(GTexture&& other) noexcept :
		type(other.type),
		dim(other.dim),
		texture(NULL)
	{
		std::swap(texture, other.texture);
	}

	~GTexture();

	SDL_Texture* texture;
	Game::GTileType type;
	GVec2<int> dim;
};

// uniformly sized texture atlas
class GAtlas {
public:
	GAtlas(const char*, int, int, int, int);
	GAtlas() :
		rend(NULL),
		atlas(NULL),
		texsize({ 0,0 }),
		atlassize({ 0,0 }) {}

	~GAtlas();

	SDL_Texture* get_texture(int);

	GVec2<int> texsize;

	GVec2<int> atlassize;
	SDL_Renderer* rend;
	SDL_Surface* atlas;
};

// Engine object
class GEngine {
public:
	GEngine(const char*, const char*, int, int, Game::GWorld&);
	~GEngine();

	void init();
	void handle_inputs();
	void gfx_loop();
	void process_mouse();
	void draw_line();

	void render_tile(int, int, Game::GTile);
	void render_map();
	void radius(GVec2<int>, std::function<void(GVec2<int>)>, int);
	void line(GVec2<int>, GVec2<int>, std::function<void(GVec2<int>)>);
	
	GVec2<int> xy_to_iso(GVec2<int>);
	GVec2<int> iso_to_xy(GVec2<int>, GVec2<int>);
	GVec2<int> try_tiles(GVec2<int>);

	void write_text(std::string, SDL_Color, int, int);

	bool quit;

private:
	int state;
	int mouse;
	
	SDL_Window* window;
	SDL_Renderer* rend;
	SDL_Event event;
	TTF_Font* font;
	GAtlas atlas;

	GVec2<int> scrsize;
	GVec2<int> view;
	GVec2<int> mousepos;
	GVec2<int> cur_tile;

	std::vector<GTexture> textures;
	Game::GWorld& world;

	void move_view(GVec2<int>);
};

CLOSE_NAMESPACE