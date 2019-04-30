#include "Minimap.h"
#include "App.h"
#include "Log.h"
#include "M_Map.h"
#include "M_Window.h"
#include "M_Render.h"
#include <list>
#include "Object.h"

Minimap::Minimap(const fPoint position, float minimap_width, float minimap_height, float texture_width) : position(position), texture_width(texture_width)
{
	// Set view rect =============================================================

	view_rect = { (int)position.x, (int)position.y, (int)minimap_width, (int)minimap_height };

	// Set texture rect =============================================================

	minimap_rect = { 0, 0, (int)texture_width, (int)texture_height };

	// Load Textures ================================================================

	LoadTextureFromMap();

	alpha_mask_texture = app->tex->Load("maps/circle_mask.png");

}

bool Minimap::PreUpdate()
{
	int x = 0, y = 0;
	app->input->GetMousePosition(x, y);

	if (app->input->GetMouseButton(1) == KEY_REPEAT)
	{
		fPoint mouse_point = { (float)x,  (float)y };
		
		if (PointInEllipse(mouse_point, fPoint(view_rect.x + view_rect.w *0.5f, view_rect.y + view_rect.h *0.5f), view_rect.w *0.5f, view_rect.h *0.5f))
		{
			Camera* camera = (*app->render->cameras.begin());
			camera_target_pos = MinimapToMap(x - minimap_pos.x, y - minimap_pos.y);
			camera_target_pos = app->map->MapToScreenF(camera_target_pos);
			camera_target_pos -= { camera->rect.w * 0.5f, camera->rect.h * 0.5f};
		}
	}

	return true;
}

bool Minimap::Update(float dt)
{
	Camera* camera = (*app->render->cameras.begin());
	camera->MoveToScreenPoint(dt, camera_target_pos);

	// Update texture rect =====================================

	fPoint offset = MapToMinimap( target_to_follow->pos_map.x, target_to_follow->pos_map.y);
	minimap_pos = { view_rect.x + view_rect.w* 0.5f - offset.x, view_rect.y + view_rect.h* 0.5f - offset.y};

	minimap_rect.x = (int)minimap_pos.x;
	minimap_rect.y = (int)minimap_pos.y;


	UpdateMinimapTexture();

	return false;
}

bool Minimap::PostUpdate()
{
	// Draw minimap texture  ==========================================
	SDL_Rect sprite_rect = { 0,0,view_rect.w, view_rect.h };
	Camera* camera = (*app->render->cameras.begin());
	app->render->BlitUI(blitted_texture, view_rect.x , view_rect.y , &sprite_rect, camera);

	// Draw pointed objects  ==========================================



	for (std::list<Object*>::iterator iter = indicators_list.begin(); iter != indicators_list.end(); ++iter)
	{
		fPoint object_pos = MapToMinimap((*iter)->pos_map.x, (*iter)->pos_map.y) + (fPoint)minimap_pos;
		sprite_rect = { (int)object_pos.x -3,(int)object_pos.y -3,6, 6 };
		app->render->DrawQuad(sprite_rect, 255, 0, 0, 255, true, false);
	}

	// Draw minimap camera  ==========================================

	iPoint pos = WorldToMinimap(camera->camera_pos.x, camera->camera_pos.y) + (iPoint)minimap_pos;
	SDL_Rect camera_rect = { pos.x , pos.y, camera->screen_section.w * aspect_ratio_x ,  camera->screen_section.h * aspect_ratio_y };
	app->render->DrawQuad(camera_rect, 255, 255, 255, 255, false, false);
	app->render->DrawQuad(view_rect, 255, 255, 255, 255, false, false);


	return true;
}

bool Minimap::LoadTextureFromMap()
{
	// Set Minimap Info ========================================

	float tiles_average = (float)app->map->data.columns * 0.5f + (float)app->map->data.rows * 0.5f;

	texture_height = (texture_width * (tiles_average* (float)app->map->data.tile_height)) / (tiles_average* (float)app->map->data.tile_width);

	minimap_tile_width = texture_width / tiles_average;
	minimap_tile_height = texture_height / tiles_average;

	aspect_ratio_x = (float)texture_width /  ((float)app->map->data.tile_width * tiles_average);
	aspect_ratio_y = (float)texture_height / ((float)app->map->data.tile_height *tiles_average);

	x_offset = (float)app->map->data.rows *minimap_tile_width * 0.5f;

	// Create Target Textures ====================================

	blitted_texture = app->tex->CreateTargetTexture(view_rect.w, view_rect.h);
	minimap_texture = app->tex->CreateTargetTexture( texture_width, texture_height);

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

				SDL_RenderCopy(app->render->renderer, tileset->texture , &sprite_rect, &section_to_print);
			}
		}
	}

	// Reset target texture ==================================================

	SDL_SetRenderTarget(app->render->renderer, NULL);

	return true;
}

void Minimap::UpdateMinimapTexture()
{
	SDL_SetRenderTarget(app->render->renderer, blitted_texture);
	SDL_SetTextureBlendMode(blitted_texture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor( app->render->renderer ,0, 0, 0, 255);
	SDL_RenderClear(app->render->renderer);
	Camera* camera = (*app->render->cameras.begin());
	fPoint offset = MapToMinimap(target_to_follow->pos_map.x, target_to_follow->pos_map.y);

	app->render->BlitUI(minimap_texture,  view_rect.w * .5f - offset.x , view_rect.h * .5f - offset.y, NULL, camera);

	SDL_BlendMode mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_REV_SUBTRACT);

	SDL_SetTextureBlendMode(alpha_mask_texture, mode);
	SDL_RenderCopy(app->render->renderer, alpha_mask_texture, NULL, NULL);

	SDL_SetRenderTarget(app->render->renderer, NULL);
}

fPoint Minimap::MapToMinimap(const float x, const float y)
{
	return fPoint((x - y) * minimap_tile_width * 0.5f + x_offset , (x + y) * minimap_tile_height * 0.5f);
}

fPoint Minimap::MinimapToMap(const float x, const float y)
{
	fPoint ret = { 0.f, 0.f };

	float half_width = minimap_tile_width   * .5f;
	float half_height = minimap_tile_height * .5f;

	float x_mod = x - x_offset;
	float y_mod = y;

	ret.x = (x_mod / half_width + y_mod / half_height) * .5f;
	ret.y = (y_mod / half_height - x_mod / half_width) * .5f;

	return ret;
}

iPoint Minimap::WorldToMinimap(const int x, const int y)
{
	return iPoint(x * aspect_ratio_x + x_offset, y * aspect_ratio_y);
}

iPoint Minimap::MinimapToWorld(const int x, const int y)
{
	return iPoint(   (float)(x - x_offset ) / aspect_ratio_x , (float)y / aspect_ratio_y);
}

bool Minimap::PointInEllipse(fPoint test, fPoint center, float width, float height) {
	float dx = test.x - center.x;
	float dy = test.y - center.y;
	return (dx * dx) / (width * width) + (dy * dy) / (height * height) <= 1;
}

void Minimap::AddPonintedObject(Object * object_to_point)
{
	indicators_list.push_back(object_to_point);
}


//int x = 0, y = 0;
//app->input->GetMousePosition(x, y);

//if (app->input->GetMouseButton(1) == KEY_DOWN)
//{
//
//	SDL_Point mouse_point = { x, y };

//	if (SDL_PointInRect(&mouse_point, &minimap->minimap_rect))
//	{
//		allow_interaction = true;
//	}

//}

//if (app->input->GetMouseButton(1) == KEY_REPEAT)
//{
//	if (allow_interaction)
//	{
//		Camera* camera = (*app->render->cameras.begin());
//		camera_target_pos = minimap->MinimapToMap(x, y);
//		camera_target_pos = app->map->MapToScreenF(camera_target_pos);
//		camera_target_pos -= { camera->rect.w * 0.5f, camera->rect.h * 0.5f};

//		if (camera_target_pos.x < -app->map->data.tile_width* .5f * app->map->data.rows)
//		{
//			camera_target_pos.x = -app->map->data.tile_width* .5f * app->map->data.rows;
//		}
//		
//		if  (camera_target_pos.x + camera->rect.w > app->map->data.tile_width* .5f * app->map->data.rows)
//		{
//			camera_target_pos.x = app->map->data.tile_width* .5f * app->map->data.rows - camera->rect.w;
//		}

//		if (camera_target_pos.y < 0.f)
//		{
//			camera_target_pos.y = 0.f;
//		}

//		if (camera_target_pos.y + camera->rect.h > app->map->data.tile_height * app->map->data.columns)
//		{
//			camera_target_pos.y = app->map->data.tile_height * app->map->data.columns - camera->rect.h;
//		}
//	}
//}

//if (app->input->GetMouseButton(1) == KEY_UP)
//{
//	allow_interaction = false;
//}


	//app->render->DrawLineSplitScreen(minimap_rect.x + minimap_rect.w* .5f, minimap_rect.y, minimap_rect.x + minimap_rect.w, minimap_rect.y + minimap_rect.h *0.5f,						0, 0, 0, 255);
	//app->render->DrawLineSplitScreen(minimap_rect.x + minimap_rect.w , minimap_rect.y + minimap_rect.h *0.5f , minimap_rect.x + minimap_rect.w* .5f, minimap_rect.y + minimap_rect.h ,	0, 0, 0, 255);
	//app->render->DrawLineSplitScreen(minimap_rect.x + minimap_rect.w* .5f, minimap_rect.y + minimap_rect.h, minimap_rect.x, minimap_rect.y + minimap_rect.h *0.5f,						0, 0, 0, 255);
	//app->render->DrawLineSplitScreen(minimap_rect.x, minimap_rect.y + minimap_rect.h *0.5f, minimap_rect.x + minimap_rect.w* .5f, minimap_rect.y ,										0, 0, 0, 255);
