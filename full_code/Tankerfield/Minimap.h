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

	enum class INTERACTION_TYPE
	{
		FOLLOW_TARGET,
		FREE_MOVEMENT
	};

	enum class PROJECTION_TYPE
	{
		ORTHOGONAL,
		ISOMETRIC,
	};

	enum class SHAPE_TYPE
	{
		CIRCLE,
		RECTANGLE
	};


	Minimap(
		const SDL_Rect minimap_rect,
		const float texture_width, 
		const PROJECTION_TYPE projection_type = PROJECTION_TYPE::ISOMETRIC, 
		const SHAPE_TYPE shape_type = SHAPE_TYPE::RECTANGLE, 
		const INTERACTION_TYPE interaction_type = INTERACTION_TYPE::FREE_MOVEMENT, 
		Object* target = nullptr);

	~Minimap();

	bool PreUpdate();

	bool Update(float dt);

	bool PostUpdate();

public:

	void AddPonintedObject(Object* object_to_point);

	// Mesures transformations methods ==========================

	fPoint	MapToMinimap(const float x, const float y);

	fPoint	MinimapToMap(const float x, const float y);

	iPoint	WorldToMinimap(const int x, const int y);

	iPoint	MinimapToWorld(const int x, const int y);

	bool	PointInEllipse(fPoint test, fPoint center, float width, float height);

private:

	bool LoadMinimap();

	bool    LoadMinimapData();

	bool	LoadMinimapTexture();

	void	UpdateMinimapTexture();

public:

	fPoint              position = { 0.f,0.f };         // UI Element Position
	Object*             target_to_follow = nullptr;

private:

	fPoint				camera_target_pos = { 0,0 };

	// General info ======================================

	fPoint				minimap_pos = { 0.f, 0.f };
	SDL_Rect            minimap_rect = { 0, 0, 0,0 };
	bool                minimap_loaded = false;
	PROJECTION_TYPE		projection_type = PROJECTION_TYPE::ISOMETRIC;
	SHAPE_TYPE			shape_type = SHAPE_TYPE::RECTANGLE;
	INTERACTION_TYPE	interaction_type = INTERACTION_TYPE::FREE_MOVEMENT;

	// Textures info ====================================

	SDL_Texture*		minimap_texture = nullptr;      // Scaled map texture
	SDL_Texture*        blitted_texture = nullptr;      // Blitted texture with masks
	SDL_Texture*		alpha_mask_texture = nullptr;   // Mask texture used to aplly alpha mask

	float				texture_width = 0.f;
	float				texture_height = 0.f;

	float				minimap_tile_width = 0.f;
	float				minimap_tile_height = 0.f;

	float				aspect_ratio_x = 0.f;
	float				aspect_ratio_y = 0.f;

	float				x_offset = 0;

	// Values ===========================================

	bool                allow_interaction = false;

	// Indicators =======================================

	std::list<Object*>	indicators_list;

};

#endif // !_MINI_MAP_H__


