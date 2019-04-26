#ifndef _MINI_MAP_H__
#define _MINI_MAP_H__

#include "Point.h"

struct SDL_Texture;

class Minimap
{
public:

	Minimap();

	bool Update();

	bool PostUpdate();

public:

	bool LoadTextureFromMap();

	iPoint MapToMinimap(const int x, const int y);

	//fPoint MapToMinimap(const fPoint map_point);

private:

	fPoint          position = { 0.f,0.f };
	SDL_Texture*    minimap_texture = nullptr;
	SDL_Texture*    minimap_atlas = nullptr;
	int             texture_width = 0;
	int             texture_height = 0;
};

#endif // !_MINI_MAP_H__
