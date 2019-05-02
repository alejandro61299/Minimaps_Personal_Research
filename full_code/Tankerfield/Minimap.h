#ifndef _MINI_MAP_H__
#define _MINI_MAP_H__

#include <list>
#include "Point.h"
#include "SDL/include/SDL_rect.h"
#include "PugiXml/src/pugiconfig.hpp"
#include "PugiXml/src/pugixml.hpp"
#include "Module.h" // Delete this if you transform the Minimap class into a UI _Element

#define CAMERA_SPEED 50.F

class Object;
class Minimap;
class Camera;
struct SDL_Texture;
struct SDL_Color;

enum class INTERACTION_TYPE
{
	NO_TYPE,
	MOUSE_DRAG
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

// - Minimap Indicator Class used to contain icons, points and arrows info

class Minimap_Indicator
{
public:

	Minimap_Indicator(const fPoint map_pos, const SDL_Rect icon_rect = { 0,0,0,0 }, const SDL_Color color = { 0,0,0,0 }, Object*  target = nullptr);

	void Destroy();

private:

	bool UpdateFromTargetPosition();

private:

	fPoint     map_pos = { 0, 0 };        // - Map position in map units 
	SDL_Rect   icon_rect = { 0,0,0,0 };   // - Icon sprite rect , {0,0,0,0} = No icon
	SDL_Color  color = { 0,0,0,0 };       // - Point color, {0,0,0,0} = No point 
	Object*    target = nullptr;          // - Target used to update map_pos, nullptr = static map_pos
	bool       to_destroy = false;        // - Indicator used to known when is ready to be destroied

private:

	friend Minimap;
};


// Minimap Class, genere a texture and handle minimap indicators
class Minimap : public Module
{
public:

	Minimap(
		const SDL_Rect minimap_rect,
		const float texture_width,
		const PROJECTION_TYPE projection_type = PROJECTION_TYPE::ISOMETRIC,
		const SHAPE_TYPE shape_type = SHAPE_TYPE::RECTANGLE,
		const INTERACTION_TYPE interaction_type = INTERACTION_TYPE::MOUSE_DRAG,
		Object* target = nullptr);

	~Minimap();

	bool PreUpdate() override;

	bool Update(float dt) override;

	bool PostUpdate(float dt) override;


public:

	void	AddIndicator(const fPoint map_pos, const SDL_Rect icon_rect = { 0,0,0,0 }, const SDL_Color color = { 0,0,0,0 }, Object*  target = nullptr);

	void	SetInteractionType(const INTERACTION_TYPE new_type);

	void	SetShapeType(const SHAPE_TYPE new_type);

	fPoint  GetTextureScreenPos();

	// Mesures transformations methods ==========================

	fPoint	MapToMinimap(const float x, const float y);

	fPoint	MinimapToMap(const float x, const float y);

	fPoint	WorldToMinimap(const float x, const float y);

	fPoint	MinimapToWorld(const float x, const float y);

	bool	PointInEllipse(fPoint test, fPoint center, float width, float height);

private:

	void	MouseDragInput( float dt);

	bool	LoadMinimap();

	bool    LoadMinimapData();

	bool	LoadMinimapTexture();

	void	UpdateMinimapTexture();

public:

	Object*             target_to_follow = nullptr;

private:
	Camera*             camera = nullptr;
	fPoint				camera_target_pos = { 0,0 };

	// General info ======================================

	SDL_Rect            minimap_rect = { 0, 0, 0,0 };   // - Determinate the minimap position on screen & its scope
	bool                minimap_loaded = false;         

	PROJECTION_TYPE		projection_type = PROJECTION_TYPE::ISOMETRIC;
	SHAPE_TYPE			shape_type = SHAPE_TYPE::RECTANGLE;
	INTERACTION_TYPE	interaction_type = INTERACTION_TYPE::MOUSE_DRAG;

	// Textures info ====================================

	SDL_Texture*		minimap_texture = nullptr;      // - Scaled map texture
	SDL_Texture*		alpha_mask_texture = nullptr;   // - Mask texture used to aplly alpha mask
	SDL_Texture*        final_texture = nullptr;        // - Final texture with masks
	SDL_Texture*		icons_texture = nullptr;

	float				x_offset = 0;                   // - Offset between texture and his map 0,0 represented in pixels
	fPoint				texture_pos = { 0.f, 0.f };     // - Texture position respect minimap_rect position
	float				texture_width = 0.f;            
	float				texture_height = 0.f;

	float				minimap_tile_width = 0.f;
	float				minimap_tile_height = 0.f;

	float				aspect_ratio_x = 0.f;           
	float				aspect_ratio_y = 0.f;


	// Values ===========================================

	bool                allow_interaction = false;

	// Indicators =======================================

	std::list<Minimap_Indicator*>	indicators_list;

};

#endif // !_MINI_MAP_H__


