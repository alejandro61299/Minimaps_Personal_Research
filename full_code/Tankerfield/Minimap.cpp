#include "Minimap.h"
#include "App.h"
#include "Log.h"
#include "M_Map.h"
#include "M_Window.h"
#include "M_Render.h"
#include <list>

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
	return true;
}

bool Minimap::LoadTextureFromMap()
{
	texture_width = 0;
	texture_height = 0;

	int minimap_tile_width = 10;
	int minimap_tile_height = 10;

	// Search the widest & highest layer in order to obtain new texture size

	for (std::list<MapLayer*>::iterator iter = app->map->data.map_layers.begin(); iter != app->map->data.map_layers.end(); ++iter)
	{
		if ((*iter)->columns > texture_width)
		{
			texture_width = (*iter)->columns;
		}
		if ((*iter)->rows > texture_height)
		{
			texture_height = (*iter)->rows;
		}
	}

	// Multiply by tile 

	texture_width *= minimap_tile_width;
	texture_height *= minimap_tile_height;

	// Finaly we create the streaming texture

	minimap_texture = app->tex->CreateStreamingTexture(texture_width, texture_height);
	SDL_SetRenderDrawBlendMode(app->render->renderer, SDL_BLENDMODE_BLEND);
	int print_x_offset = (texture_height - 1) * .5f;
	SDL_Rect sprite_rect = { 0,0,0,0 };
	SDL_Rect section_to_print = { 0,0,0,0 };
	iPoint minimap_tile_pos = { 0,0 };
	uint id = 0;

	for (std::list<MapLayer*>::iterator iter = app->map->data.map_layers.begin(); iter != app->map->data.map_layers.end(); ++iter)
	{
		for (int y = 0; y < (*iter)->rows; ++y)
		{
			for (int x = 0; x < (*iter)->columns; ++x)
			{
				id = (*iter)->Get(x, y);
	
				if (id >= 119 && id <= 126)
				{
					sprite_rect = { 0,0 ,10,10 };
				}
				else if (id >= 135 && id <= 139)
				{
					sprite_rect = { 0,10 ,10,10 };
				}
	
				minimap_tile_pos = MapToMinimap(x, y);
	
				section_to_print.x = print_x_offset + minimap_tile_pos.x;
				section_to_print.y = minimap_tile_pos.y;
				section_to_print.w = minimap_tile_width;
				section_to_print.h = minimap_tile_height;
	
				app->tex->CopyTextureOn(minimap_texture, minimap_atlas, { section_to_print.x, section_to_print.y }, { sprite_rect.x, sprite_rect.y } , section_to_print.w, section_to_print.h);
			}
		}
	}
	
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

iPoint Minimap::MapToMinimap( const int x , const int y)
{
	return iPoint((x - y) * 10 * 0.5f, (x + y) * 10 * 0.5f);
}

//texture_width = 0;
//texture_height = 0;
//
//int minimap_tile_width = 10;
//int minimap_tile_height = 10;
//
//// Search the widest & highest layer in order to obtain new texture size
//
//for (std::list<MapLayer*>::iterator iter = app->map->data.map_layers.begin(); iter != app->map->data.map_layers.end(); ++iter)
//{
//	if ((*iter)->columns > texture_width)
//	{
//		texture_width = (*iter)->columns;
//	}
//	if ((*iter)->rows > texture_height)
//	{
//		texture_height = (*iter)->rows;
//	}
//}
//
//// Multiply by tile 
//
//texture_width *= minimap_tile_width;
//texture_height *= minimap_tile_height;
//
//int print_x_offset = (texture_height - 1) * .5f;
//SDL_Rect sprite_rect = { 0,0,0,0 };
//SDL_Rect section_to_print = { 0,0,0,0 };
//iPoint minimap_tile_pos = { 0,0 };
//uint id = 0;
//
//SDL_SetRenderDrawBlendMode(app->render->renderer, SDL_BLENDMODE_BLEND);
//for (std::list<MapLayer*>::iterator iter = app->map->data.map_layers.begin(); iter != app->map->data.map_layers.end(); ++iter)
//{
//	for (int y = 0; y < (*iter)->rows; ++y)
//	{
//		for (int x = 0; x < (*iter)->columns; ++x)
//		{
//			id = (*iter)->Get(x, y);
//
//			if (id >= 119 && id <= 126)
//			{
//				sprite_rect = { 0,0 ,10,10 };
//			}
//			else if (id >= 135 && id <= 139)
//			{
//				sprite_rect = { 0,10 ,10,10 };
//			}
//
//			minimap_tile_pos = MapToMinimap(x, y);
//
//			section_to_print.x = print_x_offset + minimap_tile_pos.x;
//			section_to_print.y = minimap_tile_pos.y;
//			section_to_print.w = minimap_tile_width;
//			section_to_print.h = minimap_tile_height;
//
//			SDL_RenderCopy(app->render->renderer, minimap_atlas, &sprite_rect, &section_to_print);
//
//		}
//	}
//}
//
//SDL_RenderPresent(app->render->renderer);
//
//minimap_texture = SDL_CreateTexture(app->render->renderer, SDL_PIXELFORMAT_RGBA8888, NULL, texture_width, texture_height);
//SDL_RenderClear(app->render->renderer);
