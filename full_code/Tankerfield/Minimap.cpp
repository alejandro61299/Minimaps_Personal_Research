#include "Minimap.h"
#include "App.h"
#include "Log.h"
#include "M_Map.h"
#include "M_Window.h"
#include "M_Render.h"
#include <list>
#include "Object.h"

Minimap::Minimap(const fPoint position): position(position)
{
	minimap_rect.x = position.x;
	minimap_rect.y = position.y;

	minimap_atlas = app->tex->LoadStreamingTex("maps/minimap_atlas.png");
	circle_mask = app->tex->Load("maps/circle_mask.png");
}

bool Minimap::Update()
{
	return false;
}

bool Minimap::PostUpdate()
{
	// Draw minimap texture  ==========================================

	Camera* camera = (*app->render->cameras.begin());
	app->render->BlitUI(minimap_texture, position.x, position.y , NULL, camera);

	// Draw pointed objects  ==========================================

	//SDL_Rect sprite_rect = { 0,0,0,0 };

	//for (std::list<Object*>::iterator iter = pointed_objects.begin(); iter != pointed_objects.end(); ++iter)
	//{
	//	iPoint object_pos = MapToMinimap((*iter)->pos_map.x, (*iter)->pos_map.y);
	//	sprite_rect = { 1, 0, 9, 9 };
	//	app->render->BlitUI(minimap_atlas, object_pos.x - sprite_rect.w * .5f, object_pos.y - sprite_rect.h * .5f, &sprite_rect);
	//}

	// Draw minimap camera  ==========================================

	iPoint pos = WorldToMinimap(camera->camera_pos.x, camera->camera_pos.y) + (iPoint)position;

	SDL_Rect camera_rect = { pos.x , pos.y, camera->screen_section.w * aspect_ratio_x ,  camera->screen_section.h * aspect_ratio_y};

 	app->render->DrawQuad(camera_rect,255,255,255,255, false, false);

	app->render->DrawQuad(minimap_rect, 255, 255, 255, 255, false, false);

	return true;
}

void Minimap::AddPonintedObject(Object * object_to_point)
{
	pointed_objects.push_back(object_to_point);
}

bool Minimap::LoadTextureFromMap(const int width, const int height)
{
	// Set Minimap Values =============================

	minimap_rect.w = texture_width = width;
	minimap_rect.h = texture_height = height;



	minimap_tile_width = texture_width / app->map->data.columns;
	minimap_tile_height = texture_height / app->map->data.rows;

	aspect_ratio_x = (float)texture_width / (float)(app->map->data.tile_width * app->map->data.rows);
	aspect_ratio_y = (float)texture_height / (float)(app->map->data.tile_height * app->map->data.columns);

	print_x_offset = texture_width * 0.5f;

	// Finaly we create the streaming texture

	SDL_Rect sprite_rect = { 0,0,0,0 };
	SDL_Rect section_to_print = { 0,0,0,0 };
	fPoint minimap_tile_pos = { 0,0 };

	// changes render to target texture

	minimap_texture = app->tex->CreateTargetTexture( texture_width, texture_height);
	SDL_SetTextureBlendMode(minimap_texture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(app->render->renderer, minimap_texture);

	 //search throght layers and decide what minimap tex correspons to a specific map id

	for (std::list<MapLayer*>::iterator iter = app->map->data.map_layers.begin(); iter != app->map->data.map_layers.end(); ++iter)
	{
		for (int y = 0; y < (*iter)->rows; ++y)
		{
			for (int x = 0; x < (*iter)->columns; ++x)
			{
				sprite_rect = (*app->map->data.tilesets.begin())->GetTileRect((*iter)->Get(x, y));

				minimap_tile_pos = MapToMinimap(x, y);

				if (minimap_tile_width > 1.f && minimap_tile_height > 1.f)
				{
					section_to_print = { (int)minimap_tile_pos.x, (int)minimap_tile_pos.y, (int)minimap_tile_width, (int)minimap_tile_height };
				}
				else
				{
					section_to_print = { (int)minimap_tile_pos.x, (int)minimap_tile_pos.y, 1, 1};
				}

				SDL_RenderCopy(app->render->renderer, (*app->map->data.tilesets.begin())->texture , &sprite_rect, &section_to_print);
			}
		}
	}

	SDL_SetRenderTarget(app->render->renderer, NULL);

	return true;
}

fPoint Minimap::MapToMinimap(const float x, const float y)
{
	return fPoint((x - y) * minimap_tile_width * 0.5f + print_x_offset , (x + y) * minimap_tile_height * 0.5f);
}

fPoint Minimap::MinimapToMap(const float x, const float y)
{
	fPoint ret = { 0.f, 0.f };

	float half_width = minimap_tile_width * 0.5f;
	float half_height = minimap_tile_height * 0.5f;

	float x_mod = x - print_x_offset - position.x;
	float y_mod = y - position.y;

	ret.x = (x_mod / half_width + y_mod / half_height) * 0.5f;
	ret.y = (y_mod / half_height - x_mod / half_width) * 0.5f;

	return ret;
}

iPoint Minimap::WorldToMinimap(const int x, const int y)
{
	return iPoint(x * aspect_ratio_x + print_x_offset, y * aspect_ratio_y);
}

iPoint Minimap::MinimapToWorld(const int x, const int y)
{
	return iPoint(   (float)(x - print_x_offset ) / aspect_ratio_x , (float)y / aspect_ratio_y);
}

