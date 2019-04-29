#ifndef _MINI_MAP_H__
#define _MINI_MAP_H__

#include <list>
#include "Point.h"
#include "SDL/include/SDL_rect.h"

class Object;
struct SDL_Texture;

class Minimap
{
public:

	Minimap(const fPoint position, float minimap_width, float minimap_height, float map_texture_width, float map_texture_height);

	bool PreUpdate();

	bool Update( float dt);

	bool PostUpdate();

public:

	void AddPonintedObject(Object* object_to_point);

	// Mesures transformations methods ==========================

	fPoint MapToMinimap(const float x, const float y);

	fPoint MinimapToMap(const float x, const float y);

	iPoint WorldToMinimap(const int x, const int y);

	iPoint MinimapToWorld(const int x, const int y);

	bool PointInEllipse(fPoint test, fPoint center, float width, float height);

private:

	bool LoadTextureFromMap();

	void UpdateMinimapTexture();

public:

	fPoint              position = { 0.f,0.f };         // UI Element Position
	Object*             target_to_follow = nullptr;

private:

	fPoint				minimap_pos = { 0.f, 0.f };

	// Camera ====================================

	fPoint				camera_target_pos = { 0,0 };

	// Info ======================================

	SDL_Rect            view_rect = { 0, 0, 0,0 };
	SDL_Rect            minimap_rect = { 0, 0, 0,0 };
	float				x_offset = 0;

	// Textures info ==============================

	SDL_Texture*		minimap_texture = nullptr;      // Scaled map texture
	SDL_Texture*        blitted_texture = nullptr;      // Blitted texture with masks
	SDL_Texture*		alpha_mask_texture = nullptr;   // Mask texture used to aplly alpha mask
	
	float				map_texture_width = 0.f;
	float				map_texture_height = 0.f;

	float				minimap_tile_width = 0.f;
	float				minimap_tile_height = 0.f;

	float				aspect_ratio_x = 0.f;
	float				aspect_ratio_y = 0.f;

	// Indicators ===================================

	std::list<Object*>	indicators_list;
};

#endif // !_MINI_MAP_H__
