#include "Minimap.h"
#include "App.h"
#include "Log.h"
#include "M_Map.h"
#include "M_Window.h"
#include "M_Render.h"
#include <list>
#include "Object.h"

Minimap::Minimap()
{
	minimap_atlas = app->tex->LoadStreamingTex("maps/minimap_atlas.png");
}

bool Minimap::Update()
{
	return false;
}

bool Minimap::PostUpdate()
{
	SDL_Rect draw_rect = { 0, 0, texture_width , texture_height };
	app->render->BlitUI(minimap_texture, 0, 0, &draw_rect, (*app->render->cameras.begin()));

	SDL_Rect sprite_rect = { 0,0,0,0 };

	for (std::list<Object*>::iterator iter = pointed_objects.begin(); iter != pointed_objects.end(); ++iter)
	{
		iPoint object_pos = MapToMinimap((*iter)->pos_map.x, (*iter)->pos_map.y);
		sprite_rect = { 1, 0, 9, 9 };
		app->render->BlitUI(minimap_atlas, object_pos.x + print_x_offset - sprite_rect.w * .5f, object_pos.y - sprite_rect.h * .5f, &sprite_rect);
	}
	return true;
}

void Minimap::AddPonintedObject(Object * object_to_point)
{
	pointed_objects.push_back(object_to_point);
}

bool Minimap::LoadTextureFromMap()
{

	minimap_tile_width = 1;
	minimap_tile_height = 1;

	// Search the widest & highest layer in order to obtain new texture size

	texture_width = app->map->data.columns;
	texture_height = app->map->data.rows;

	texture_width *= minimap_tile_width;
	texture_height *= minimap_tile_height;

	print_x_offset = texture_width * 0.5f;

	// Finaly we create the streaming texture

	SDL_SetRenderDrawBlendMode(app->render->renderer, SDL_BLENDMODE_BLEND);
	SDL_Rect sprite_rect = { 0,0,0,0 };
	SDL_Rect section_to_print = { 0,0,0,0 };
	iPoint minimap_tile_pos = { 0,0 };
	uint id = 0;

	// changes render to target texture

	minimap_texture = app->tex->CreateTargetTexture( texture_width, texture_height);

	SDL_SetRenderTarget(app->render->renderer, minimap_texture);

	// search throght layers and decide what minimap tex correspons to a specific map id

	for (std::list<MapLayer*>::iterator iter = app->map->data.map_layers.begin(); iter != app->map->data.map_layers.end(); ++iter)
	{
		for (int y = 0; y < (*iter)->rows; ++y)
		{
			for (int x = 0; x < (*iter)->columns; ++x)
			{
				id = (*iter)->Get(x, y) - 1;

				if (id == 119)
				{
					sprite_rect = { 0,0 ,1,1 };
				}
				else if (id == 135)
				{
					sprite_rect = { 0,1 ,1,1 };
				}
				else
				{
					continue;
				}

				minimap_tile_pos = MapToMinimap(x, y);

				section_to_print.x = print_x_offset + minimap_tile_pos.x;
				section_to_print.y = minimap_tile_pos.y;
				section_to_print.w = minimap_tile_width;
				section_to_print.h = minimap_tile_height;

				// render the desired texture to texture

				SDL_RenderCopy(app->render->renderer, minimap_atlas, &sprite_rect, &section_to_print);
			}
		}
	}

	//Reset render target 
	SDL_SetRenderTarget(app->render->renderer, NULL);

	if (minimap_texture == nullptr)
	{
		LOG("Cannot Create Minimap Texture, Error: %s", SDL_GetError());
		return false;
	}
	else
	{
		LOG("Created minimap texture");
		return true;
	}
}


iPoint Minimap::MapToMinimap(const int x, const int y)
{
	return iPoint((x - y) * minimap_tile_width * 0.5f, (x + y) * minimap_tile_height * 0.5f);
}

void Minimap::Draw()
{


}