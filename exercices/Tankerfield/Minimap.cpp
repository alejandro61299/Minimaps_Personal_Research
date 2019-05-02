#include "Minimap.h"
#include "App.h"
#include "Log.h"
#include "M_Map.h"
#include "M_Window.h"
#include "M_Render.h"
#include <list>
#include "Object.h"
#include <string>


// Minimap Class Methods ====================================================================================

Minimap::Minimap(const SDL_Rect minimap_rect, const float texture_width, const SHAPE_TYPE shape_type, const INTERACTION_TYPE interaction_type, Object* target)
	: minimap_rect(minimap_rect), texture_width(texture_width), shape_type(shape_type), interaction_type(interaction_type), target_to_follow(target)
{
	SetInteractionType(interaction_type);
	camera = (*app->render->cameras.begin());;

	// Load minimap  ================================================================

	LoadMinimap();

	// Load textures ===============================================================
	alpha_mask_texture = app->tex->Load("textures/minimap/circle_mask.png");
	icons_texture = app->tex->Load("textures/minimap/icons.png");
	SDL_BlendMode blend_mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_REV_SUBTRACT);
	SDL_SetTextureBlendMode(alpha_mask_texture, blend_mode); // This belnd mode become transaparent all pixels under the painted area of texture

}

Minimap::~Minimap()
{
	for (std::list<Minimap_Indicator*>::iterator iter = indicators_list.begin(); iter != indicators_list.end(); ++iter)
	{
		delete(*iter);
	}

	indicators_list.clear();

	if (alpha_mask_texture != nullptr)
	{
		app->tex->UnLoad(alpha_mask_texture, TEXTURE_TYPE::NORMAL);
	}
	if (icons_texture != nullptr)
	{
		app->tex->UnLoad(icons_texture, TEXTURE_TYPE::NORMAL);
	}
	if (final_texture != nullptr)
	{
		app->tex->UnLoad(final_texture, TEXTURE_TYPE::STREAMING_OR_TARGET);
	}
	if (minimap_texture != nullptr)
	{
		app->tex->UnLoad(minimap_texture, TEXTURE_TYPE::STREAMING_OR_TARGET);
	}
}

bool Minimap::PreUpdate()
{
	// Debug ================================================================

	// Change Interaction Type ------------------------------
	
	if (app->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
	{
		if (interaction_type == INTERACTION_TYPE::NO_TYPE)
		{
			SetInteractionType(INTERACTION_TYPE::MOUSE_DRAG);
		}
		else
		{
			SetInteractionType(INTERACTION_TYPE::NO_TYPE);
		}
	}

	// Change Shape Type ------------------------------------

	if (app->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
	{
		if (shape_type == SHAPE_TYPE::CIRCLE)
		{
			SetShapeType(SHAPE_TYPE::RECTANGLE);
		}
		else
		{
			SetShapeType(SHAPE_TYPE::CIRCLE);
		}
	}

	// Add advice ------------------------------------------

	SDL_Point mouse_point;
	app->input->GetMousePosition(mouse_point.x, mouse_point.y);

	if (app->input->GetMouseButton(3) == KEY_DOWN)
	{
		if (SDL_PointInRect(&mouse_point, &minimap_rect))
		{
			AddIndicator(MinimapToMap(mouse_point.x - GetTextureScreenPos().x, mouse_point.y - GetTextureScreenPos().y), { 32, 32, 32, 32 });
		}
		else
		{
			AddIndicator(app->map->ScreenToMapF(mouse_point.x + camera->rect.x, mouse_point.y + camera->rect.y), { 32, 32, 32, 32 });
		}

	}

	// Reset indicators ------------------------------------

	if (app->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
	{
		for (std::list<Minimap_Indicator*>::iterator iter = indicators_list.begin(); iter != indicators_list.end(); ++iter)
		{
			(*iter)->Destroy();
		}
	}

	return true;
}

bool Minimap::Update(float dt)
{
	// Update input & camera ===================================

	fPoint offset;

	switch (interaction_type)
	{
	case INTERACTION_TYPE::NO_TYPE:

		offset = MapToMinimap(target_to_follow->map_pos.x, target_to_follow->map_pos.y);
		texture_pos = fPoint(minimap_rect.w * .5f, minimap_rect.h * .5f) - MapToMinimap(target_to_follow->map_pos.x, target_to_follow->map_pos.y);
		camera->MoveToObject(dt, target_to_follow);         // Caemra movement ----------
		camera_target_pos = camera->camera_pos;
		break;
	case INTERACTION_TYPE::MOUSE_DRAG:

		MouseDragInput(dt);
		camera->MoveToScreenPoint(dt, camera_target_pos);	// Caemra movement ----------

		break;
	}

	// Update indicators positions ============================

	for (std::list<Minimap_Indicator*>::iterator iter = indicators_list.begin(); iter != indicators_list.end();)
	{
		if ((*iter)->to_destroy == true && (*iter)->target != target_to_follow)
		{
			delete(*iter);
			iter = indicators_list.erase(iter);
		}
		else
		{
			(*iter)->UpdateFromTargetPosition();
			++iter;
		}
	}

	return true;
}

bool Minimap::PostUpdate(float dt)
{
	// Update Minimap texture ===================================================

	UpdateFinalTexture();

	// Draw final texture =======================================================

	app->render->BlitUI(final_texture, minimap_rect.x, minimap_rect.y, NULL, camera);

	// Draw debug ===============================================================

	// Minimap screen rect --------------------------
	app->render->DrawQuad(minimap_rect, 255, 255, 255, 255, false, false);

	// Minimap texture screem rect ------------------

	SDL_Rect texture_debug_rect = { minimap_rect.x + texture_pos.x , minimap_rect.y + texture_pos.y, texture_width, texture_height };
	app->render->DrawQuad(texture_debug_rect, 255, 0, 0, 255, false, false);

	return true;
}

void Minimap::UpdateFinalTexture()
{
	// Set render target =========================================================

	SDL_SetRenderTarget(app->render->renderer, final_texture);
	SDL_SetTextureBlendMode(final_texture, SDL_BLENDMODE_BLEND);

	// Clear texture =============================================================

	SDL_SetRenderDrawColor(app->render->renderer, 0, 0, 0, 255);
	SDL_RenderClear(app->render->renderer);

	// Draw minimap texture  =====================================================

	app->render->BlitUI(minimap_texture, texture_pos.x, texture_pos.y, NULL, camera);

	// Draw minimap indicators ==================================================

	SDL_Rect section_to_print = { 0,0,0,0 };
	fPoint pos = { 0,0 };

	for (std::list<Minimap_Indicator*>::iterator iter = indicators_list.begin(); iter != indicators_list.end(); ++iter)
	{
		pos = texture_pos +  MapToMinimap((*iter)->map_pos.x, (*iter)->map_pos.y);

		if ((*iter)->icon_rect.w != 0 && (*iter)->icon_rect.h != 0)
		{
			section_to_print = { (int) (pos.x - (*iter)->icon_rect.w* 0.5f)  ,  (int)(pos.y - (*iter)->icon_rect.h* 0.5f), (*iter)->icon_rect.w, (*iter)->icon_rect.h };
			SDL_RenderCopy(app->render->renderer, icons_texture, &(*iter)->icon_rect, &section_to_print);
		}
		else
		{
			section_to_print = { (int)(pos.x - 2), (int) (pos.y - 2), 4, 4 };
			app->render->DrawQuad(section_to_print, 255, 0, 0, 255, true, false);
		}
	}

	// Draw minimap camera rect =================================================

	pos = texture_pos + WorldToMinimap(camera->camera_pos.x, camera->camera_pos.y) ;

	SDL_Rect camera_rect = { pos.x , pos.y, camera->screen_section.w * aspect_ratio_x ,  camera->screen_section.h * aspect_ratio_y };
	app->render->DrawQuad(camera_rect, 255, 255, 255, 255, false, false);

	// Draw alpha mask texture  =================================================
	if (shape_type == SHAPE_TYPE::CIRCLE)
	{
		SDL_RenderCopy(app->render->renderer, alpha_mask_texture, NULL, NULL);
	}

	// Reset render target ======================================================

	SDL_SetRenderTarget(app->render->renderer, NULL);
}

bool Minimap::LoadMinimap()
{
	bool ret = true;

	if (LoadMinimapInfo() == false)
	{
		ret = false;
	}

	if (ret == false || GenerateMinimapTexture() == false)
	{
		ret = false;
	}

	minimap_loaded = ret;

	return ret;
}

bool Minimap::LoadMinimapInfo()
{
	if (app->map->MapLoaded() == false) { return false; }  // If there isn't a map loaded break load

	// Information from map data ==================================================================

	float tile_width = app->map->data.tile_width;
	float tile_height = app->map->data.tile_height;
	float half_tile_width = (float)app->map->data.tile_width * 0.5f;
	float half_tile_height = (float)app->map->data.tile_height * 0.5f;

	// Number of tiles that fit on one side of the map =============================================

	float tiles_amount = (float)(app->map->data.columns + app->map->data.rows)* 0.5f;

	// Map Pixel Mesures ==========================================================================

	float map_width = tile_width * tiles_amount;
	float map_height = tile_height * tiles_amount;

	// We found texture height from the width with a rule of 3  ===================================

	texture_height = (texture_width * map_height) / map_width;

	// Now we have enough information to know the size of minimap tiles ===========================

	minimap_tile_width = texture_width / tiles_amount;
	minimap_tile_height = texture_height / tiles_amount;

	// We also find a constant to transform from pixels in the world to pixels in the minimap  ====

	aspect_ratio_x = texture_width / map_width;
	aspect_ratio_y = texture_height / map_height;

	// Finally, the blit x offset ===============================================================

	x_offset = (float)app->map->data.rows *minimap_tile_width * 0.5f;

	return true;
}

bool Minimap::GenerateMinimapTexture()
{
	// Create Target Textures ====================================

	final_texture = app->tex->CreateTargetTexture(minimap_rect.w, minimap_rect.h);
	minimap_texture = app->tex->CreateTargetTexture(texture_width, texture_height);

	SDL_SetRenderTarget(app->render->renderer, minimap_texture);
	SDL_SetTextureBlendMode(minimap_texture, SDL_BLENDMODE_BLEND);

	// Blit scaled map to minimap texture =========================

	SDL_Rect sprite_rect = { 0,0,0,0 };
	SDL_Rect section_to_print = { 0,0,0,0 };
	fPoint minimap_tile_pos = { 0,0 };

	for (std::list<MapLayer*>::iterator iter = app->map->data.map_layers.begin(); iter != app->map->data.map_layers.end(); ++iter)
	{
		for (int y = 0; y < (*iter)->rows; ++y)
		{
			for (int x = 0; x < (*iter)->columns; ++x)
			{
				Tileset* tileset = app->map->GetTilesetFromTileId((*iter)->Get(x, y));
				sprite_rect = tileset->GetTileRect((*iter)->Get(x, y));

				minimap_tile_pos = MapToMinimap(x, y);

				if (minimap_tile_width > 1.f && minimap_tile_height > 1.f)
				{
					section_to_print = { (int)minimap_tile_pos.x, (int)minimap_tile_pos.y, (int)minimap_tile_width, (int)minimap_tile_height };
				}
				else
				{
					section_to_print = { (int)minimap_tile_pos.x, (int)minimap_tile_pos.y, 1, 1 };
				}

				SDL_RenderCopy(app->render->renderer, tileset->texture, &sprite_rect, &section_to_print);
			}
		}
	}

	// Reset target texture ==================================================

	SDL_SetRenderTarget(app->render->renderer, NULL);

	return true;
}

void Minimap::MouseDragInput(float dt)
{
	int x = 0, y = 0;
	app->input->GetMousePosition(x, y);

	if (app->input->GetMouseButton(1) == KEY_DOWN)
	{
		SDL_Point mouse_point = { x , y };

		if (SDL_PointInRect(&mouse_point, &minimap_rect))
		{
			allow_interaction = true;
		}
	}

	if (app->input->GetMouseButton(1) == KEY_REPEAT)
	{

		float half_camera_w = camera->rect.w *aspect_ratio_x * 0.5f;
		float half_camera_h = camera->rect.h *aspect_ratio_y * 0.5f;

		if (allow_interaction)
		{
			if (x - half_camera_w < minimap_rect.x)
			{
				x = minimap_rect.x + half_camera_w;
				texture_pos.x += CAMERA_SPEED * dt;
			}

			if (x + half_camera_w > minimap_rect.x + minimap_rect.w)
			{
				x = minimap_rect.x + minimap_rect.w - half_camera_w;
				texture_pos.x -= CAMERA_SPEED * dt;
			}

			if (y - half_camera_h < minimap_rect.y)
			{
				y = minimap_rect.y + half_camera_h;
				texture_pos.y += CAMERA_SPEED * dt;
			}

			if (y + half_camera_h > minimap_rect.y + minimap_rect.h)
			{
				y = minimap_rect.y + minimap_rect.h - half_camera_h;
				texture_pos.y -= CAMERA_SPEED * dt;
			}

			camera_target_pos = MinimapToWorld(x - GetTextureScreenPos().x - half_camera_w, y - GetTextureScreenPos().y - half_camera_h);
		}
	}

	if (app->input->GetMouseButton(1) == KEY_UP)
	{
		allow_interaction = false;
	}
}

// - Map Coordinates to Minimap Pixels Coordinate   
fPoint Minimap::MapToMinimap(const float x, const float y)
{
	/*return fPoint((x - y) * minimap_tile_width * 0.5f + x_offset, (x + y) * minimap_tile_height * 0.5f);*/
	return fPoint();
}

// - Minimap Pixel Coordinate to Map Coordinates
fPoint Minimap::MinimapToMap(const float x, const float y) 
{
	// TODO 1: Knowing how to transform coordinates of the map to coordinates of the mini map,
	// complete the function to go from coordinates of the minimap to coordinates of the map

	return fPoint();
}

// - World Pixels Coordinates to Minimap Pixels Coordinate 
fPoint Minimap::WorldToMinimap(const float x, const float y)
{
	return fPoint(x * aspect_ratio_x + x_offset, y * aspect_ratio_y);
}

// - Minimap Pixels Coordinate to World Pixels Coordinates
fPoint Minimap::MinimapToWorld(const float x, const float y)
{
	return fPoint((x - x_offset) / aspect_ratio_x,  y/ aspect_ratio_y);
}

bool Minimap::PointInEllipse(fPoint test, fPoint center, float width, float height)
{
	float dx = test.x - center.x;
	float dy = test.y - center.y;
	return (dx * dx) / (width * width) + (dy * dy) / (height * height) <= 1;
}

Minimap_Indicator* Minimap::AddIndicator(const fPoint map_pos, const SDL_Rect icon_rect, const SDL_Color color, Object * target)
{
	Minimap_Indicator* new_indicator = new Minimap_Indicator(map_pos, icon_rect, color, target);
	indicators_list.push_back(new_indicator);
	return new_indicator;
}

void Minimap::SetInteractionType( const INTERACTION_TYPE new_type)
{
	if (new_type == INTERACTION_TYPE::NO_TYPE && target_to_follow == nullptr)
	{
		LOG("Minimap Error: Target nullptr , interaction type was changed to FREE_MOVEMENT");
		interaction_type = INTERACTION_TYPE::MOUSE_DRAG;
	}
	else
	{
		interaction_type = new_type;
	}
}

void Minimap::SetShapeType(const SHAPE_TYPE new_type)
{
	shape_type = new_type;
}

fPoint Minimap::GetTextureScreenPos()
{
	return fPoint(minimap_rect.x + texture_pos.x , minimap_rect.y + texture_pos.y);
}

// Minimap Indicator Class Methods =========================================================================

Minimap_Indicator::Minimap_Indicator(const fPoint map_pos, const SDL_Rect icon_rect, const SDL_Color color, Object * target): map_pos(map_pos), icon_rect(icon_rect), color(color), target(target)
{}

void Minimap_Indicator::Destroy()
{
	to_destroy = true;
}

bool Minimap_Indicator::UpdateFromTargetPosition()
{
	if (target != nullptr)
	{
		map_pos = target->map_pos;
		return true;
	}
	else
	{
		return false;
	}
}
