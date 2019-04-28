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

// Destructor
M_Scene::~M_Scene()
{}

// Called before render is available
bool M_Scene::Awake(pugi::xml_node& config)
{
	return true;
}

// Called before the first frame
bool M_Scene::Start()
{
	// Load Map
	app->map->Load("maps/Map.tmx");

	// Create Entity / Object

	player = (Obj_Tank*)app->objectmanager->CreateObject(ObjectType::TANK, { 50.F,50.F });
	player->camera_follow = false;
	// Create Minimap & Load 

	minimap = new Minimap( fPoint( 300.f,300.f));
	minimap->LoadTextureFromMap( 400, 200 );

	// Add pointing units

	minimap->AddPonintedObject(player);

	return true;
}

// Called each loop iteration
bool M_Scene::PreUpdate()
{
	if (app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
	{
		return false;
	}

	if (app->input->GetMouseButton(1) == KEY_REPEAT)
	{
		int x = 0, y = 0;
		app->input->GetMousePosition(x, y);

		Camera* camera = (*app->render->cameras.begin());
		camera_target_pos =  minimap->MinimapToMap( x , y);
		camera_target_pos = app->map->MapToScreenF(camera_target_pos);
	}



	return true;
}

// Called each loop iteration
bool M_Scene::Update(float dt)
{
	BROFILER_CATEGORY("M_SceneUpdate", Profiler::Color::Blue);
	Camera* camera = (*app->render->cameras.begin());
	camera->MoveToScreenPoint(dt, camera_target_pos);

	return true;
}

// Called each loop iteration
bool M_Scene::PostUpdate(float dt)
{
	minimap->PostUpdate();

	return true;
}

// Called before quitting
bool M_Scene::Reset()
{

	return true;
}

bool M_Scene::CleanUp()
{
	LOG("Freeing scene");
	
	return true;
}

