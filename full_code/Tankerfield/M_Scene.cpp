#include <cstdlib>
#include <ctime>
#include <string>

#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "M_UI.h"
#include "M_Fonts.h"
#include "M_Input.h"
#include "M_Textures.h"
#include "M_Audio.h"
#include "M_Render.h"
#include "M_Window.h"
#include "M_Scene.h"
#include "M_Map.h"
#include "M_Pathfinding.h"
#include "M_ObjManager.h"
#include "M_Collision.h"
#include "Point.h"
#include "Brofiler/Brofiler.h"
#include "Rect.h"
#include "Object.h"
#include "PerfTimer.h"
#include "Obj_Tank.h"
#include "Object.h"
#include "Minimap.h"


M_Scene::M_Scene() : Module()
{
	name.assign("scene");
}

M_Scene::~M_Scene()
{}

bool M_Scene::Awake(pugi::xml_node& config)
{
	return true;
}

bool M_Scene::Start()
{
	// Load Map ===============================================================

	app->map->Load("maps/Map.tmx");

	// Create Entity / Object =================================================

	player = (Obj_Tank*)app->objectmanager->CreateObject(ObjectType::TANK, { 200.f,200.f });

	// Create Minimap =========================================================

	int screen_margin = 20;
	int minimap_width = 300;
	int minimap_height = 300;

	fRect rect = app->win->GetWindowRect();
	SDL_Rect  minimap_rect = {  (int)(rect.GetRight() - minimap_width - screen_margin) , (int)(rect.GetBottom() - minimap_height - screen_margin),  minimap_width , minimap_height };
	minimap = new Minimap(minimap_rect, 1600, PROJECTION_TYPE::ORTHOGONAL, SHAPE_TYPE::CIRCLE, INTERACTION_TYPE::MOUSE_DRAG, player);

	// Add indicators ========================================================

	minimap->AddIndicator(fPoint(5.f, 5.f), { 0,0,32,32 }, { 0,0,0,0 }, player);

	return true;
}

bool M_Scene::PreUpdate()
{
	if (app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
	{
		return false;
	}

	minimap->PreUpdate();

	return true;
}

bool M_Scene::Update(float dt)
{
	BROFILER_CATEGORY("M_SceneUpdate", Profiler::Color::Blue);

	minimap->Update(dt);
	return true;
}

bool M_Scene::PostUpdate(float dt)
{
	minimap->PostUpdate(dt);

	return true;
}

bool M_Scene::Reset()
{

	return true;
}

bool M_Scene::CleanUp()
{
	LOG("Freeing scene");
	
	return true;
}

