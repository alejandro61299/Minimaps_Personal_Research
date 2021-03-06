
#include "Brofiler/Brofiler.h"

#include "Defs.h"
#include "Log.h"
#include "Point.h"

#include "App.h"
#include "M_Input.h"
#include "M_Window.h"
#include "M_Render.h"
#include "M_Map.h"
#include "Obj_Tank.h"
#include "M_ObjManager.h"
#include "M_Scene.h"
#include "MathUtils.h"
#include "Camera.h"

M_Render::M_Render() : Module()
{
	name.assign("renderer");
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
M_Render::~M_Render()
{
	for (std::vector<Camera*>::iterator item_cam = cameras.begin(); item_cam != cameras.end(); ++item_cam)
	{
		if ((*item_cam) != nullptr)
		{
			delete (*item_cam);
			(*item_cam) = nullptr;
		}
	}
	cameras.clear();
}

// Called before render is available
bool M_Render::Awake(pugi::xml_node& config)
{
	LOG("Create SDL rendering context");
	bool ret = true;
	// load flags
	Uint32 flags = SDL_RENDERER_ACCELERATED;
	flags |= SDL_RENDERER_TARGETTEXTURE;

	if (config.child("vsync").attribute("value").as_bool(true) == true)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;

		LOG("Using vsync");
	}

	SDL_RendererInfo info;
	int index = -1;

	for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i)
	{
		SDL_GetRenderDriverInfo(i, &info);

		if (info.name == std::string("opengles2"))
		{
			index = i;
		}
	}
	
	renderer = SDL_CreateRenderer(app->win->window, index, flags);

	if (renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{

		Camera* camera_aux = nullptr;
		camera_aux = new Camera();
		camera_aux->rect.w = app->win->screen_surface->w;
		camera_aux->rect.h = app->win->screen_surface->h;
		fPoint tank_1_pos_screen = app->map->MapToWorldF(app->scene->player->map_pos);
		camera_aux->rect.x = tank_1_pos_screen.x - camera_aux->rect.w*0.5f; //Magic numbres to pos the camera with the player in center
		camera_aux->rect.y = tank_1_pos_screen.y + camera_aux->rect.h + 40; //Magic numbres to pos the camera with the player in center
		camera_aux->screen_section = {
			0,
			0,
			(int)(app->win->screen_surface->w),
			(int)(app->win->screen_surface->h)
		};

		cameras.push_back(camera_aux);
	}

	return ret;
}

// Called before the first frame
bool M_Render::Start()
{
	LOG("render start");
	// back background
	SDL_RenderGetViewport(renderer, &viewport);
	return true;
}

// Called each loop iteration
bool M_Render::PreUpdate()
{
	SDL_RenderClear(renderer);
	return true;
}

bool M_Render::PostUpdate(float dt)
{

	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderPresent(renderer);

	return true;
}

// Called before quitting
bool M_Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

// Load Game State
bool M_Render::Load(pugi::xml_node& data)
{

	return true;
}

// Save Game State
bool M_Render::Save(pugi::xml_node& data) const
{
	pugi::xml_node cam = data.append_child("camera");

	return true;
}

void M_Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void M_Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_RenderSetViewport(renderer, &rect);
}

void M_Render::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, &viewport);
}

iPoint M_Render::ScreenToWorld(int x, int y, const Camera* camera) const
{
	iPoint ret;
	int scale = app->win->GetScale();
	
	ret.x = (x + camera->rect.x / scale);
	ret.y = (y + camera->rect.y / scale);

	return ret;
}


void M_Render::Blit( SDL_Texture* texture,  const int screen_x, const int screen_y, Camera* current_camera, const SDL_Rect* section) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_Rect rect_in_screen;
	SDL_Rect spritesheet_rect{ 0,0,0,0 };

	//Transform the rect in the word to the rect in screen =======================
	rect_in_screen.x = -current_camera->rect.x + screen_x * scale;
	rect_in_screen.y = -current_camera->rect.y + screen_y * scale;

	if (section != NULL)
	{
		spritesheet_rect = *section;
		rect_in_screen.w = section->w * scale;
		rect_in_screen.h = section->h * scale;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect_in_screen.w, &rect_in_screen.h);
		spritesheet_rect.w = rect_in_screen.w;
		spritesheet_rect.h = rect_in_screen.h;
	}
	//Move the rect_in_screen to their correct screen =========================== 	
	rect_in_screen.x += current_camera->screen_section.x;
	rect_in_screen.y += current_camera->screen_section.y;

	//Print the rect_in_screen ============================================
	if (SDL_RenderCopy(renderer, texture, &spritesheet_rect, &rect_in_screen) )
	{
		LOG("Cannot blit to main_object. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}
}


void M_Render::BlitScaled(SDL_Texture* texture, const int screen_x, const int screen_y, Camera* current_camera, const SDL_Rect* section, float custom_scale) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_Rect rect_in_screen;
	SDL_Rect spritesheet_rect{ 0,0,0,0 };

	//Transform the rect in the word to the rect in screen =======================
	rect_in_screen.x = -current_camera->rect.x + screen_x * scale;
	rect_in_screen.y = -current_camera->rect.y + screen_y * scale;

	if (section != NULL)
	{
		spritesheet_rect = *section;
		rect_in_screen.w = section->w * scale * custom_scale;
		rect_in_screen.h = section->h * scale * custom_scale;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect_in_screen.w, &rect_in_screen.h);
		spritesheet_rect.w = rect_in_screen.w;
		spritesheet_rect.h = rect_in_screen.h;
	}
	//Move the rect_in_screen to their correct screen =========================== 	
	rect_in_screen.x += current_camera->screen_section.x;
	rect_in_screen.y += current_camera->screen_section.y;

	//Print the rect_in_screen ============================================
	if (SDL_RenderCopy(renderer, texture, &spritesheet_rect, &rect_in_screen))
	{
		LOG("Cannot blit to main_object. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}
}

void M_Render::BlitUI(SDL_Texture* texture, int screen_x, int screen_y, const SDL_Rect* section,  Camera* camera, const int alpha) const
{
	if (alpha == 0.f)
	{
		return;
	}

	SDL_Rect rect;
	rect.x = screen_x ;
	rect.y = screen_y ;

	if (camera != nullptr)
	{
		SDL_RenderSetClipRect(renderer, &camera->screen_section);
	}

	if (section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}

	if (alpha != 255)
	{
		SDL_SetTextureAlphaMod(texture, alpha);
	}

	if (SDL_RenderCopy(renderer, texture, section, &rect) != 0)
	{
		LOG("Cannot blit to main_object. SDL_RenderCopy error: %s", SDL_GetError());
	}

	SDL_RenderSetClipRect(renderer, nullptr);

	if (alpha != 255)
	{
		SDL_SetTextureAlphaMod(texture, 255);
	}
}

bool M_Render::DrawQuad(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);

    std::vector<Camera*>::iterator item_cam;
	for (item_cam = app->render->cameras.begin(); item_cam != app->render->cameras.end(); ++item_cam)
	{
		/*SDL_RenderSetClipRect(app->render->renderer, &(*item_cam)->screen_section);*/
		if (use_camera)
		{
			rec.x = (int)(-(*item_cam)->rect.x + rect.x * scale);
			rec.y = (int)(-(*item_cam)->rect.y + rect.y * scale);
			rec.w *= scale;
			rec.h *= scale;
		}

		int result = (filled) ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

		if (result != 0)
		{
			LOG("Cannot draw quad to main_object. SDL_RenderFillRect error: %s", SDL_GetError());
			ret = false;
		}
	}

	//SDL_RenderSetClipRect(app->render->renderer, nullptr);

	return ret;
}

bool M_Render::DrawQuadUI( const SDL_Rect section, Camera* camera, const SDL_Color color)
{
	if (color.a == 0.f)
	{
		return true;
	}

	SDL_Rect rect(section);

	if (camera != nullptr)
	{
		SDL_RenderSetClipRect(renderer, &camera->screen_section);
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

	SDL_RenderFillRect(renderer, &rect);

	if (camera != nullptr)
	{
		SDL_RenderSetClipRect(renderer, nullptr);
	}

	return true;
}

bool M_Render::DrawIsometricQuad(float x, float y, float w, float h, SDL_Color color, const Camera* camera)
{
	fPoint point_1, point_2, point_3, point_4;

	// top_left 
	point_1 = app->map->MapToWorldF({x, y});
	// top_right
	point_2 = app->map->MapToWorldF({x + w, y});
	// bot_right
	point_3 = app->map->MapToWorldF({x + w, y + h});
	// bot_left
	point_4 = app->map->MapToWorldF({x, y + h});


    SDL_Rect rect_tile{
    	point_4.x,
    	point_1.y,
    	point_2.x - point_4.x,
    	point_3.y - point_1.y
    };
    if (SDL_HasIntersection(&rect_tile, &camera->rect))
    {
    	app->render->DrawLineSplitScreen(point_1.x, point_1.y, point_2.x, point_2.y, color.r, color.g, color.b, color.a, camera);
    	app->render->DrawLineSplitScreen(point_2.x, point_2.y, point_3.x, point_3.y, color.r, color.g, color.b, color.a, camera);
    	app->render->DrawLineSplitScreen(point_3.x, point_3.y, point_4.x, point_4.y, color.r, color.g, color.b, color.a, camera);
    	app->render->DrawLineSplitScreen(point_4.x, point_4.y, point_1.x, point_1.y, color.r, color.g, color.b, color.a, camera);
     }
   
	return true;
}

void M_Render::DrawIsometricLine(fPoint point_1, fPoint point_2, SDL_Color color, const Camera* camera)
{
	fPoint p_1, p_2;

	p_1 = app->map->MapToWorldF(point_1);
	p_2 = app->map->MapToWorldF(point_2);

	app->render->DrawLineSplitScreen(p_1.x, p_1.y, p_2.x, p_2.y, color.r, color.g, color.b, color.a, camera);
}


bool M_Render::DrawLineSplitScreen( int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, const Camera* camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;

	if (camera != nullptr)
	{
	
		x1 += camera->screen_section.x;
		x2 += camera->screen_section.x;
		y1 += camera->screen_section.y;
		y2 += camera->screen_section.y;

		result = SDL_RenderDrawLine(renderer, -camera->rect.x + x1 * scale, -camera->rect.y + y1 * scale, -camera->rect.x + x2 * scale, -camera->rect.y + y2 * scale);
	}
	else
	{
		result = SDL_RenderDrawLine(renderer, x1 * scale, y1 * scale, x2 * scale, y2 * scale);
	}

	if (result != 0)
	{
		LOG("Cannot draw quad to main_object. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool M_Render::DrawCircle(int x, int y, int radius, Camera* camera, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_Point points[360];

	float factor = (float)M_PI / 180.0f;

	int x_in_viewport = x;
	int y_in_viewport = y;

	x_in_viewport += camera->screen_section.x;
	y_in_viewport += camera->screen_section.y;


	if (use_camera)
	{
		for (uint i = 0; i < 360; ++i)
		{
			points[i].x = (int)(-camera->rect.x + x_in_viewport + radius * cos(i * factor));
			points[i].y = (int)(-camera->rect.y + y_in_viewport + radius * sin(i * factor));
		}
	}

	else
	{
		for (uint i = 0; i < 360; ++i)
		{
			points[i].x = (int)(x_in_viewport + radius * cos(i * factor));
			points[i].y = (int)(y_in_viewport + radius * sin(i * factor));
		}
	}

	result = SDL_RenderDrawPoints(renderer, points, 360);

	if (result != 0)
	{
		LOG("Cannot draw quad to main_object. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
	
}

bool M_Render::DrawIsoCircle(int x, int y, int radius, Camera* camera, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_Point points[360];

	float factor = (float)M_PI / 180.0f;

	int x_in_viewport = x + camera->screen_section.x;
	int y_in_viewport = y + camera->screen_section.y;

	if (use_camera)
	{
		for (uint i = 0; i < 360; ++i)
		{
			points[i].x = (int)(-camera->rect.x + x_in_viewport + radius * cos(i * factor));
			points[i].y = (int)(-camera->rect.y + y_in_viewport + radius * 0.5f * sin(i * factor));
		}
	}

	else
	{
		for (uint i = 0; i < 360; ++i)
		{
			points[i].x = (int)(x_in_viewport + radius * cos(i * factor));
			points[i].y = (int)(y_in_viewport + radius * 0.5f * sin(i * factor));
		}
	}

	result = SDL_RenderDrawPoints(renderer, points, 360);

	if (result != 0)
	{
		LOG("Cannot draw quad to main_object. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;

}


bool M_Render::IsOnCamera(const int & x, const int & y, const int & w, const int & h, Camera* camera) const
{
	int scale = app->win->GetScale();

	SDL_Rect r = { x*scale,y*scale,w*scale,h*scale };

	return SDL_HasIntersection(&r, &camera->rect);
}
