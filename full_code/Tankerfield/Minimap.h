#ifndef _MINI_MAP_H__
#define _MINI_MAP_H__

#include <list>
#include "Point.h"

class Object;
struct SDL_Texture;

class Minimap
{
public:

	Minimap();

	bool Update();

	bool PostUpdate();

public:

	void AddPonintedObject(Object* object_to_point);

	bool LoadTextureFromMap();

	iPoint MapToMinimap(const int x, const int y);

	void Draw();

private:

	// General ===================================

	fPoint               position = { 0.f,0.f };
	std::list<Object*>   pointed_objects;

	// Info ======================================

	int                  print_x_offset = 0;
	SDL_Texture*         minimap_texture = nullptr;
	SDL_Texture*         minimap_atlas = nullptr;
	int                  texture_width = 0;
	int                  texture_height = 0;
	int                  minimap_tile_width = 10;
	int                  minimap_tile_height = 10;
};

#endif // !_MINI_MAP_H__
