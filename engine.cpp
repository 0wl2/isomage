#include "engine.h"

OPEN_NAMESPACE(Gfx)

GTexture::GTexture(SDL_Renderer** rend, const char* filename, Game::GTileType ty) :
	type(ty)
{
	texture = IMG_LoadTexture(*rend, filename);
	if (!texture) {
		throw std::runtime_error("Could not get texture");
	}

	SDL_QueryTexture(texture, NULL, NULL, &dim.x, &dim.y);
}

GTexture::GTexture(SDL_Texture* tex) :
	texture(tex),
	type(Game::NOTHING)
{
	if (!texture) {
		throw std::runtime_error("Invalid texture used in constructor");
	}

	SDL_QueryTexture(texture, NULL, NULL, &dim.x, &dim.y);
}

GTexture::~GTexture()
{
	SDL_DestroyTexture(texture);
}

GAtlas::GAtlas(
	const char* filename,
	int w, 
	int h, 
	int aw, 
	int ah) :
	rend(NULL),
	texsize({ w, h }),
	atlassize({ aw, ah })
{
	atlas = IMG_Load(filename);
	if (!atlas) {
		throw std::runtime_error("Could not open texture atlas file");
	}
}

GAtlas::~GAtlas()
{
	SDL_FreeSurface(atlas);
}

SDL_Texture* GAtlas::get_texture(int idx)
{
	SDL_Rect src = {(idx % atlassize.x) * texsize.x, (idx / atlassize.y) * texsize.y, texsize.x, texsize.y},
		dst = { 0, 0, atlassize.x * texsize.x, atlassize.y * texsize.y };
	SDL_Surface* surface = SDL_CreateRGBSurface(
		0,
		64,
		64,
		32,
		0xff,
		0xff00,
		0xff0000,
		0xff000000
	);

	SDL_BlitSurface(atlas, &src, surface, &dst);
	if (!surface) {
		throw std::runtime_error("Could not blit atlas");
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(rend, surface);
	if (!texture) {
		throw std::runtime_error("Could not create texture of surface");
	}

	SDL_FreeSurface(surface);
	return texture;
}

GEngine::GEngine(const char* atl, const char* fontname, int w, int h, Game::GWorld& wr) :
	scrsize({ w, h }),
	window(NULL),
	rend(NULL),
	font(NULL),
	quit(false),
	event({}),
	view({ 9, 5 }),
	world(wr),
	atlas(atl, 64, 64, 5, 5),
	cur_tile(invalid),
	mouse(0),
	state(0),
	mousepos({ 0, 0 })
{
	if (SDL_Init(SDL_INIT_EVERYTHING)) 
		throw std::runtime_error("Could not initialize SDL");

	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) 
		throw std::runtime_error("Could not initialize SDL_Image");

	if (TTF_Init() == -1) 
		throw std::runtime_error("Could not initialize SDL_ttf");

	font = TTF_OpenFont(fontname, 14);
}

void GEngine::init()
{
	window = SDL_CreateWindow(
		"untitled game",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		scrsize.x,
		scrsize.y,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (!window) {
		throw std::runtime_error("Could not create SDL window");
	}

	rend = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED
	);

	if (!rend) {
		throw std::runtime_error("Could not create SDL renderer");
	}

	atlas.rend = rend;

	textures.push_back(GTexture(atlas.get_texture(Game::HILIGHT)));
	textures.push_back(GTexture(atlas.get_texture(Game::GRASS)));
	textures.push_back(GTexture(atlas.get_texture(Game::WATER)));
	textures.push_back(GTexture(atlas.get_texture(Game::CONCRETE)));
	textures.push_back(GTexture(atlas.get_texture(Game::ROAD)));
	textures.push_back(GTexture(atlas.get_texture(Game::RESIDENTIAL)));

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;
	io.WantCaptureMouse = true;
	ImGui::StyleColorsClassic();
	
	ImGui_ImplSDL2_InitForSDLRenderer(window, rend);
	ImGui_ImplSDLRenderer_Init(rend);
}

GEngine::~GEngine()
{
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	TTF_CloseFont(font);
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(window);

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

GVec2<int> GEngine::iso_to_xy(GVec2<int> vec, GVec2<int> vect)
{
	GVec2<int> r = {
		(view.x * (atlas.texsize.x / 2)) + (vec.x - vec.y) * (atlas.texsize.x / 2),
		(view.y * (atlas.texsize.y / 4)) + (vec.x + vec.y) * (atlas.texsize.y / 4)
	};

	return r;
}

struct distance {
	GVec2<int> c;
	int d;
};

GVec2<int> GEngine::xy_to_iso(GVec2<int> vec)
{
	GVec2<int> c = { ((vec.x) / (atlas.texsize.x / 2)) - view.x, ((vec.y) / (atlas.texsize.y / 4)) - view.y }, r;
	
	r.x = (c.y + c.x) / 2;
	r.y = (c.y - c.x) / 2;

	if (r.x < 0 || r.x >= world.dim.x + 1 ||
		r.y < 0 || r.y >= world.dim.y + 1) {
		r.x = r.y = -1;
		return r;
	}

	std::vector<distance> dists;

	radius(r, [&](GVec2<int> v) {
		GVec2<int> a = iso_to_xy(v, { atlas.texsize.x, atlas.texsize.y }),
			ce = { a.x + (atlas.texsize.x / 2), a.y + (atlas.texsize.y / 2) },
			d = { vec.x - ce.x, vec.y - ce.y };

		dists.push_back({v, (int)std::sqrt(
			std::pow(d.x, 2) +
			std::pow(d.y, 2)
		)});
	}, 1);

	std::sort(dists.begin(), dists.end(), [](distance i, distance j) { return i.d < j.d; });
	
	r.x = dists.front().c.x;
	r.y = dists.front().c.y;

	r.x = SDL_clamp(r.x, 0, world.dim.x - 1);
	r.y = SDL_clamp(r.y, 0, world.dim.y - 1);

	return r;
}

GVec2<int> GEngine::try_tiles(GVec2<int> initial)
{
	GVec2<int> q;

	for (int el = 60; el >= 0; el -= 20) {
		q = xy_to_iso({ initial.x, initial.y + el });
		if (world.get_tile(q.x, q.y).height == el)
			return q;
	}

	return invalid;
}

void GEngine::render_tile(int x, int y, Game::GTile tile)
{
	GVec2<int> td = {
		textures.at(tile.type).dim.x,
		textures.at(tile.type).dim.y
	}, rd = iso_to_xy({x, y}, td);

	SDL_Rect r = { rd.x, rd.y - tile.height, td.x, td.y};
	SDL_RenderCopy(rend, textures.at(tile.type).texture, NULL, &r);
}

void GEngine::render_map()
{
	for (int y = 0; y < world.dim.y; y++) {
		for (int x = 0; x < world.dim.x; x++) {
			render_tile(x, y, world.get_tile(x, y));
		}
	}
}

void GEngine::write_text(std::string str, SDL_Color col, int x, int y)
{
	SDL_Surface* sr = TTF_RenderText_Solid(font, str.c_str(), col);
	if (!sr) throw std::runtime_error("Failed to create text surface");
	
	try {
		GTexture t = GTexture(SDL_CreateTextureFromSurface(rend, sr));
		SDL_FreeSurface(sr);
		SDL_Rect r = { x, y, t.dim.x, t.dim.y };
		SDL_RenderCopy(rend, t.texture, NULL, &r);
	
	}
	catch (std::exception& e) {
		SDL_FreeSurface(sr);
		throw std::runtime_error(e.what());
	}
}

void GEngine::radius(GVec2<int> vec, std::function<void(GVec2<int>)> f, int rad)
{
	Game::GTile tile = { Game::HILIGHT, 0 };

	if (!rad) {
		f(vec);
		return;
	}

	for (int y = vec.y - rad; y < vec.y + rad + 1; y++) {
		for (int x = vec.x - rad; x < vec.x + rad + 1; x++) {
			if (x < 0 ||
				y < 0 ||
				x >= world.dim.x || 
				y >= world.dim.y) 
				continue;

			f({ x, y });
		}
	}
}

void GEngine::line(GVec2<int> p0, GVec2<int> p1, std::function<void(GVec2<int>)> f)
{
	GVec2<int> s = { p0.x < p1.x ? 1 : -1, p0.y < p1.y ? 1 : -1 },
		d = {std::abs(p1.x - p0.x), -std::abs(p1.y - p0.y)};
	int err = d.x + d.y, e;

	while (true) {
		f(p0);
		
		if (p0 == p1) break;

		e = 2 * err;
		if (e >= d.y) {
			if (p0.x == p1.x) break;
			err += d.y;
			p0.x += s.x;
		}

		if (e <= d.x) {
			if (p0.y == p1.y) break;
			err += d.x;
			p0.y += s.y;
		}
	}
}

void GEngine::move_view(GVec2<int> off)
{
	view.x = SDL_clamp(view.x + off.x, 0, world.dim.x + 1);
	view.y = SDL_clamp(view.y + off.y, -world.dim.y - 50, 0);
}

void GEngine::handle_inputs()
{
	SDL_PumpEvents();

	state = SDL_GetMouseState(&mousepos.x, &mousepos.y);
	mouse = 0;

	while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type) {
		case SDL_QUIT: quit = true;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_w: move_view({0,1}); break;
			case SDLK_a: move_view({1,0}); break;
			case SDLK_s: move_view({0,-1}); break;
			case SDLK_d: move_view({-1,0}); break;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!ImGui::GetIO().WantCaptureMouse) {
				mouse |= (state & SDL_BUTTON_RMASK);
				mouse |= (state & SDL_BUTTON_MMASK);
				mouse |= (state & SDL_BUTTON_LMASK);
			}
			else mouse = 0;

			break;
		}
	}
}

void GEngine::process_mouse()
{
	if (cur_tile != invalid) {
		radius(cur_tile, [&](GVec2<int> v) {
			render_tile(v.x, v.y, {Game::HILIGHT, world.get_tile(v.x, v.y).height});
		}, world.radius);

		if (!world.build) return;

		if (mouse & SDL_BUTTON_LMASK) {
			if (world.line[1] != invalid) {
				world.line[1] = invalid;
			}
			else
				radius(cur_tile, [&](GVec2<int> v) {
					world.place_tile(v.x, v.y, (Game::GTileType)(world.current + 1));
				}, world.radius);
		}

		if (mouse & SDL_BUTTON_MMASK) {
			if (world.line[1] != invalid) {
				world.line[1] = invalid;
			}
			else {
				if (!world.draw_line) {
					world.line[1] = invalid;
					world.line[0] = cur_tile;
				}
				else {
					world.line[1] = cur_tile;
					line(world.line[0], world.line[1], [&](GVec2<int> p) {
						world.place_tile(p.x, p.y, (Game::GTileType)(world.current + 1));
					});
				}
			}

			world.draw_line = !world.draw_line;
		}

		if (mouse & SDL_BUTTON_RMASK) {
			if (world.line[1] != invalid) {
				world.line[1] = invalid;
			}
			else
				radius(cur_tile, [&](GVec2<int> v) {
					world.lift_drop_tile(v.x, v.y, 20);
				}, world.radius);
		}
	}
}

void GEngine::draw_line()
{
	if (world.line[1] != invalid)
		line(world.line[0], world.line[1], [&](GVec2<int> p) {
			render_tile(p.x, p.y, { Game::HILIGHT, 0 });
		});
}

void GEngine::gfx_loop()
{
	while (!quit) {
		handle_inputs();

		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		SDL_SetRenderDrawColor(rend, 0x53, 0x36, 0x21, 255);
		SDL_RenderClear(rend);

		cur_tile = try_tiles(mousepos);
		
		render_map();
		process_mouse();

		world.tick();

		draw_line();
		
		ImGui::Render();
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(rend);
	}
}

CLOSE_NAMESPACE