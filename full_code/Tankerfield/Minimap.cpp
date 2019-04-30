#include "Minimap.h"
#include "App.h"
#include "Log.h"
#include "M_Map.h"
#include "M_Window.h"
#include "M_Render.h"
#include <list>
#include "Object.h"

Minimap::Minimap( const SDL_Rect minimap_rect, const float texture_width, const PROJECTION_TYPE projection_type, const SHAPE_TYPE shape_type, const INTERACTION_TYPE interaction_type, Object* target) 
	: minimap_rect(minimap_rect), texture_width(texture_width), projection_type(projection_type), shape_type(shape_type), interaction_type(interaction_type), target_to_follow(target)
{
	if (interaction_type == INTERACTION_TYPE::FOLLOW_TARGET && target_to_follow == nullptr)
	{
		LOG("Minimap Error: Target nullptr , interaction type was changed to FREE_MOVEMENT");
		this->interaction_type = INTERACTION_TYPE::FREE_MOVEMENT;
	}

	// Load minimap  ================================================================

	LoadMinimap();

	// Load textures ===============================================================
	alpha_mask_texture = app->tex->Load("maps/circle_mask.png");
	SDL_BlendMode blend_mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_REV_SUBTRACT);
	SDL_SetTextureBlendMode(alpha_mask_texture, blend_mode);
}

Minimap::~Minimap()
{
	app->tex->UnLoad(alpha_mask_texture, TEXTURE_TYPE::NORMAL);

	if (blitted_texture != nullptr)
	{
		app->tex->UnLoad(blitted_texture, TEXTURE_TYPE::STREAMING_OR_TARGET);
	}
	if (minimap_texture != nullptr)
	{
		app->tex->UnLoad(minimap_texture, TEXTURE_TYPE::STREAMING_OR_TARGET);
	}
}

bool Minimap::PreUpdate()
{
	int x = 0, y = 0;
	app->input->GetMousePosition(x, y);

	//if (app->input->GetMouseButton(1) == KEY_REPEAT)
	//{
	//	fPoint mouse_point = { (float)x,  (float)y };

	//	if (PointInEllipse(mouse_point, fPoint(minimap_rect.x + minimap_rect.w *0.5f, minimap_rect.y + minimap_rect.h *0.5f), minimap_rect.w *0.5f, minimap_rect.h *0.5f))
	//	{
	//		Camera* camera = (*app->render->cameras.begin());
	//		camera_target_pos = MinimapToMap(x - minimap_pos.x, y - minimap_pos.y);
	//		camera_target_pos = app->map->MapToScreenF(camera_target_pos);
	//		camera_target_pos -= { camera->rect.w * 0.5f, camera->rect.h * 0.5f};
	//	}
	//}


	if (app->input->GetMouseButton(1) == KEY_DOWN)
	{
		SDL_Point mouse_point = { x, y };

		if (SDL_PointInRect(&mouse_point, &minimap_rect))
		{
			allow_interaction = true;
		}
	
	}
	
	if (app->input->GetMouseButton(1) == KEY_REPEAT)
	{
		if (allow_interaction)
		{
			Camera* camera = (*app->render->cameras.begin());
			camera_target_pos = (fPoint) MinimapToWorld(x /*- texture_pos.x*/, y/* - texture_pos.y*/);
			//camera_target_pos += fPoint(texture_pos.x, texture_pos.y);

			//Camera* camera = (*app->render->cameras.begin());
			//camera_target_pos = MinimapToMap(x - minimap_pos.x, y - minimap_pos.y);
			//camera_target_pos = app->map->MapToScreenF(camera_target_pos);
			//camera_target_pos -= { camera->rect.w * 0.5f, camera->rect.h * 0.5f};
			//
	/*		camera_target_pos = app->map->MapToScreenF(camera_target_pos);*/
			//camera_target_pos -= { camera->rect.w * 0.5f, camera->rect.h * 0.5f};
	
			//if (camera_target_pos.x < -app->map->data.tile_width* .5f * app->map->data.rows)
			//{
			//	camera_target_pos.x = -app->map->data.tile_width* .5f * app->map->data.rows;
			//}
			//
			//if  (camera_target_pos.x + camera->rect.w > app->map->data.tile_width* .5f * app->map->data.rows)
			//{
			//	camera_target_pos.x = app->map->data.tile_width* .5f * app->map->data.rows - camera->rect.w;
			//}
	
			//if (camera_target_pos.y < 0.f)
			//{
			//	camera_target_pos.y = 0.f;
			//}
	
			//if (camera_target_pos.y + camera->rect.h > app->map->data.tile_height * app->map->data.columns)
			//{
			//	camera_target_pos.y = app->map->data.tile_height * app->map->data.columns - camera->rect.h;
			//}
		}
	}

	if (app->input->GetMouseButton(1) == KEY_UP)
	{
		allow_interaction = false;
	}

	return true;
}

bool Minimap::Update(float dt)
{

	// Camera movement =========================================

	//Camera* camera = (*app->render->cameras.begin());
	//camera->MoveToScreenPoint(dt, camera_target_pos);

	// Update texture rect =====================================

	if (interaction_type == INTERACTION_TYPE::FOLLOW_TARGET)
	{
		fPoint offset = MapToMinimap(target_to_follow->pos_map.x, target_to_follow->pos_map.y);
		texture_pos = { minimap_rect.x + minimap_rect.w* .5f - offset.x, minimap_rect.y + minimap_rect.h* .5f - offset.y };
	}
	else
	{
		texture_pos = { minimap_rect.x + minimap_rect.w* .5f - texture_width* .5f , minimap_rect.y + minimap_rect.h* .5f - texture_height* .5f};
	}

	return false;
}

bool Minimap::PostUpdate()
{
	// Draw minimap texture  ==========================================

	SDL_Rect sprite_rect = { 0,0,minimap_rect.w, minimap_rect.h };
	Camera* camera = (*app->render->cameras.begin());

	UpdateMinimapTexture();
	app->render->BlitUI(blitted_texture, minimap_rect.x, minimap_rect.y, &sprite_rect, camera);

	// Draw pointed objects  ==========================================

	SDL_RenderSetClipRect(app->render->renderer, &minimap_rect);

	for (std::list<Object*>::iterator iter = indicators_list.begin(); iter != indicators_list.end(); ++iter)
	{
		fPoint object_pos = MapToMinimap((*iter)->pos_map.x, (*iter)->pos_map.y) + (fPoint)texture_pos;
		sprite_rect = { (int)object_pos.x - 3,(int)object_pos.y - 3,6, 6 };
		app->render->DrawQuad(sprite_rect, 255, 0, 0, 255, true, false);
	}

	SDL_RenderSetClipRect(app->render->renderer, NULL);

	// Draw minimap camera  ==========================================

	iPoint pos = WorldToMinimap(camera->camera_pos.x, camera->camera_pos.y);
	SDL_Rect camera_rect = { pos.x , pos.y, camera->screen_section.w * aspect_ratio_x ,  camera->screen_section.h * aspect_ratio_y };
	app->render->DrawQuad(camera_rect, 255, 255, 255, 255, false, false);
	app->render->DrawQuad(minimap_rect, 255, 255, 255, 255, false, false);


	return true;
}

bool Minimap::LoadMinimap()
{
	bool ret = true;

	if (LoadMinimapData() == false)
	{
		ret = false;
	}

	if (ret == false || LoadMinimapTexture() == false)
	{
		ret = false;
	}

	minimap_loaded = ret;

	return ret;
}

bool Minimap::LoadMinimapData()
{
	if (app->map->MapLoaded() == false) // If there isn't a map loaded break load
	{
		return false;
	}

	float tile_width = app->map->data.tile_width;
	float tile_height = app->map->data.tile_height;
	float half_tile_width = (float)app->map->data.tile_width * 0.5f;
	float half_tile_height = (float)app->map->data.tile_height * 0.5f;

	// Number of tiles that fit on one side of the map =============================================

	float tiles_amount = (float)(app->map->data.columns + app->map->data.rows)* 0.5f;

	// We found texture height from the width with a rule of 3  ===================================

	if (projection_type == PROJECTION_TYPE::ORTHOGONAL)
	{
		texture_height = texture_width;
	}
	else
	{
		texture_height = (texture_width * (tiles_amount * tile_height)) / (tiles_amount* tile_width);
	}

	// We also find a constant to transform from pixels in the world to pixels in the minimap  ====

	aspect_ratio_x = texture_width / (tile_width * tiles_amount);
	aspect_ratio_y = texture_height / (tile_height *tiles_amount);

	// Now we have enough information to know the size of minimap tiles ===========================

	minimap_tile_width = texture_width / tiles_amount;
	minimap_tile_height = texture_height / tiles_amount;

	// Finally, the blit x offset ===============================================================

	x_offset = (float)app->map->data.rows *minimap_tile_width * 0.5f;

	return true;
}

bool Minimap::LoadMinimapTexture()
{
	// Create Target Textures ====================================

	blitted_texture = app->tex->CreateTargetTexture(minimap_rect.w, minimap_rect.h);
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

void Minimap::UpdateMinimapTexture()
{
	// Set render target =========================================================

	SDL_SetRenderTarget(app->render->renderer, blitted_texture);
	SDL_SetTextureBlendMode(blitted_texture, SDL_BLENDMODE_BLEND);

	// Clear texture =============================================================

	SDL_SetRenderDrawColor(app->render->renderer, 0, 0, 0, 255);
	SDL_RenderClear(app->render->renderer);

	// Blit minimap texture  =====================================================

	Camera* camera = (*app->render->cameras.begin());

	if (interaction_type == INTERACTION_TYPE::FOLLOW_TARGET)
	{
		fPoint offset = MapToMinimap(target_to_follow->pos_map.x, target_to_follow->pos_map.y);
		app->render->BlitUI(minimap_texture, minimap_rect.w * .5f - offset.x, minimap_rect.h * .5f - offset.y, NULL, camera);
	}
	else
	{
		app->render->BlitUI(minimap_texture, minimap_rect.w * .5f - texture_width* 0.5f, minimap_rect.h * .5f - texture_height * 0.5f, NULL, camera);
	}

	// Blit minimap texture  =====================================================
	if (shape_type == SHAPE_TYPE::CIRCLE)
	{
		SDL_RenderCopy(app->render->renderer, alpha_mask_texture, NULL, NULL);
	}

	// Reset render target ======================================================

	SDL_SetRenderTarget(app->render->renderer, NULL);
}

// - Map coordinates to Pixels Coordinate Minimap
fPoint Minimap::MapToMinimap(const float x, const float y)
{
	return fPoint((x - y) * minimap_tile_width * 0.5f + x_offset, (x + y) * minimap_tile_height * 0.5f);
}

// - Pixel Minimap Coordinate to Map Coordinates
fPoint Minimap::MinimapToMap(const float x, const float y) 
{
	fPoint ret = { 0.f, 0.f };

	float half_width = minimap_tile_width * .5f;
	float half_height = minimap_tile_height * .5f;

	float x_mod = x - x_offset;
	float y_mod = y;

	ret.x = (x_mod / half_width + y_mod / half_height) * .5f;
	ret.y = (y_mod / half_height - x_mod / half_width) * .5f;

	return ret;
}

// - World/Screen Pixel Coordinates to Pixels Minimap Coordinate 
iPoint Minimap::WorldToMinimap(const int x, const int y)
{
	return iPoint(x * aspect_ratio_x + x_offset + texture_pos.x, y * aspect_ratio_y + texture_pos.y);
}

// - Pixels Minimap Coordinate to World/Screen Pixel Coordinates
iPoint Minimap::MinimapToWorld(const int x, const int y)
{
	return iPoint((float)(x - x_offset - texture_pos.x) / aspect_ratio_x, (float) (y  - texture_pos.y )/ aspect_ratio_y);
}

bool Minimap::PointInEllipse(fPoint test, fPoint center, float width, float height)
{
	float dx = test.x - center.x;
	float dy = test.y - center.y;
	return (dx * dx) / (width * width) + (dy * dy) / (height * height) <= 1;
}

void Minimap::AddPonintedObject(Object * object_to_point)
{
	indicators_list.push_back(object_to_point);
}

