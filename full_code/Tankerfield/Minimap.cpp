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
	app->render->BlitUI(minimap_texture, 0, 0, NULL, camera);

	// Draw pointed objects  ==========================================

	//SDL_Rect sprite_rect = { 0,0,0,0 };

	//for (std::list<Object*>::iterator iter = pointed_objects.begin(); iter != pointed_objects.end(); ++iter)
	//{
	//	iPoint object_pos = MapToMinimap((*iter)->pos_map.x, (*iter)->pos_map.y);
	//	sprite_rect = { 1, 0, 9, 9 };
	//	app->render->BlitUI(minimap_atlas, object_pos.x - sprite_rect.w * .5f, object_pos.y - sprite_rect.h * .5f, &sprite_rect);
	//}

	// Draw minimap camera  ==========================================

	iPoint pos = WorldToMinimap(camera->camera_pos.x, camera->camera_pos.y);

	SDL_Rect camera_rect = { pos.x , pos.y, camera->screen_section.w * aspect_ratio_x ,  camera->screen_section.h * aspect_ratio_y};

 	app->render->DrawQuad(camera_rect,255,255,255,255, false, false);

	return true;
}

void Minimap::AddPonintedObject(Object * object_to_point)
{
	pointed_objects.push_back(object_to_point);
}

bool Minimap::LoadTextureFromMap()
{

	minimap_tile_width = 2;
	minimap_tile_height = 1;

	// Search the widest & highest layer in order to obtain new texture size

	texture_width = app->map->data.columns * minimap_tile_width;
	texture_height = app->map->data.rows * minimap_tile_height;

	aspect_ratio_x = (float)texture_width / (float)(app->map->data.tile_width * app->map->data.rows);
	aspect_ratio_y = (float)texture_height / (float)(app->map->data.tile_height * app->map->data.columns);

	print_x_offset = texture_width * 0.5f;

	// Finaly we create the streaming texture

	SDL_Rect sprite_rect = { 0,0,0,0 };
	SDL_Rect section_to_print = { 0,0,0,0 };
	iPoint minimap_tile_pos = { 0,0 };
	uint id = 0;

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

				section_to_print = { (int)minimap_tile_pos.x, (int)minimap_tile_pos.y, (int)minimap_tile_width, (int)minimap_tile_height };

				SDL_RenderCopy(app->render->renderer, (*app->map->data.tilesets.begin())->texture , &sprite_rect, &section_to_print);
			}
		}
	}

	// Apply alpha mask 
	section_to_print = { 0, 0, 400, 400 };
	sprite_rect = { 0,0,400,400 };

	////SDL_BlendMode mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_SRC_COLOR, SDL_BLENDFACTOR_DST_COLOR, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDFACTOR_DST_ALPHA, SDL_BLENDOPERATION_SUBTRACT);
	//SDL_BlendMode mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_REV_SUBTRACT);
	//SDL_SetTextureBlendMode(circle_mask, SDL_BLENDMODE_NONE);
	//SDL_SetTextureBlendMode(minimap_texture, mode);
	//LOG("%s", SDL_GetError() );
	////SDL_RenderCopy(app->render->renderer, circle_mask, &sprite_rect, &section_to_print);
	//SDL_SetTextureBlendMode(minimap_texture, SDL_BLENDMODE_BLEND);
	//SD
	// Reset render target 

	SDL_SetRenderTarget(app->render->renderer, NULL);

	return true;
}

iPoint Minimap::MapToMinimap(const int x, const int y)
{
	return iPoint((x - y) * minimap_tile_width * 0.5f + print_x_offset, (x + y) * minimap_tile_height * 0.5f);
}

iPoint Minimap::WorldToMinimap(const int x, const int y)
{
	return iPoint(x * aspect_ratio_x + print_x_offset, y * aspect_ratio_y);
}

void Minimap::Draw()
{


}