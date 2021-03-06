
#include <string>

#include "Brofiler/Brofiler.h"
#include "PugiXml/src/pugiconfig.hpp"
#include "PugiXml/src/pugixml.hpp"

#include "Log.h"

#include "App.h"
#include "M_Render.h"
#include "M_ObjManager.h"
#include "M_Textures.h"
#include "Object.h"
#include "M_Audio.h"
#include "M_Window.h"
#include "M_Collision.h"
#include "M_Scene.h"
#include "PugiXml/src/pugiconfig.hpp"
#include "PugiXml/src/pugixml.hpp"
#include <string>
#include <algorithm>
#include "Obj_Tank.h"
#include "M_Map.h"
#include "Brofiler/Brofiler.h"
#include "Camera.h"

M_ObjManager::M_ObjManager()
{
	name.assign("object_manager");
}

// Destructor
M_ObjManager::~M_ObjManager()
{

}

bool M_ObjManager::Awake(pugi::xml_node& config)
{
	bool ret = true;
	std::list<Object*>::iterator iterator;

	for (iterator = objects.begin(); iterator!=objects.end(); iterator++)
	{
		if ((*iterator) != nullptr) 
			(*iterator)->Awake(config);
	}

	return ret;
}

bool M_ObjManager::Start()
{
	bool ret = true;
	return ret;
}

bool M_ObjManager::PreUpdate()
{
	BROFILER_CATEGORY("Object Manager: PreUpdate", Profiler::Color::Lavender);
	std::list<Object*>::iterator iterator;

	for (iterator = objects.begin(); iterator != objects.end(); iterator++)
	{
		if ((*iterator) != nullptr)
		{
			(*iterator)->PreUpdate();
		}
	}
	return true;
}

bool M_ObjManager::Update(float dt)
{
	BROFILER_CATEGORY("Object Manager: Update", Profiler::Color::ForestGreen);

	for (std::list<Object*>::iterator iterator = objects.begin(); iterator != objects.end();)
	{
		if ((*iterator) != nullptr)
		{
			(*iterator)->Update(dt);

			if ((*iterator)->to_remove)
			{
				//When we remove an element from the list, the other elements shift 1 space to our position
				//So we don't need increment the iterator to go to the next one
				//if ((*iterator)->type == ObjectType::TANK)
				//{
				//	Obj_Tank* aux = (Obj_Tank*)(*iterator);
				//	obj_tanks.remove((Obj_Tank*)(*iterator));
				//}
				if ((*iterator)->type == ObjectType::TESLA_TROOPER || (*iterator)->type == ObjectType::BRUTE)
				{
					enemies.remove((*iterator));
				}

				if ((*iterator)->coll != nullptr)
				{
					(*iterator)->coll->object = nullptr;
					(*iterator)->coll->Destroy();
					(*iterator)->coll = nullptr;
				}

				delete((*iterator));
				(*iterator) = nullptr;
				iterator = objects.erase(iterator);
			}
			else
			{
				// Update Components ======================================
				if ((*iterator)->coll != nullptr)
				{
					(*iterator)->coll->SetPosToObj();
				}

				if ((*iterator)->curr_anim != nullptr)
				{
					(*iterator)->curr_anim->NextFrame(dt);
				}

				++iterator;
			}
		}
		else
		{
			++iterator;
		}
	}
	
	return true;
}

bool M_ObjManager::PostUpdate(float dt)
{
	BROFILER_CATEGORY("Object Manger: PostUpdate", Profiler::Color::ForestGreen);
	std::vector<Object*> draw_objects;

	for (std::list<Object*>::iterator item = objects.begin(); item != objects.end(); ++item)
	{
		if (*item != nullptr)
		{
			(*item)->CalculateDrawVariables();
		}
	}

	for (std::vector<Camera*>::iterator item_cam = app->render->cameras.begin(); item_cam != app->render->cameras.end(); ++item_cam)
	{
		SDL_RenderSetClipRect(app->render->renderer, &(*item_cam)->screen_section);

		for (std::list<Object*>::iterator item = objects.begin(); item != objects.end(); ++item)
		{
			if (app->render->IsOnCamera((*item)->pos_screen.x - (*item)->draw_offset.x, (*item)->pos_screen.y - (*item)->draw_offset.y, (*item)->frame.w, (*item)->frame.h, (*item_cam)))
			{
				draw_objects.push_back(*item);
			}
		}

		std::sort(draw_objects.begin(), draw_objects.end(), M_ObjManager::SortByYPos);

		//Draw all the shadows first
		for (std::vector<Object*>::iterator item = draw_objects.begin(); item != draw_objects.end(); ++item)
		{
		  (*item)->DrawShadow((*item_cam), dt);
		}

		//Draw the objects above the shadows
		for (std::vector<Object*>::iterator item = draw_objects.begin(); item != draw_objects.end(); ++item)
		{
		  (*item)->Draw(dt, (*item_cam));

		  if (app->scene->draw_debug) {
			  (*item)->DrawDebug((*item_cam));
			  DrawDebug((*item), (*item_cam));
		  }
		}
		draw_objects.clear();
    }
	SDL_RenderSetClipRect(app->render->renderer, nullptr);
   
	return true;
}

// Called before quitting
bool M_ObjManager::CleanUp()
{
	DeleteObjects();

	return true;
}

bool M_ObjManager::Reset()
{
	for (std::list<Object*>::iterator iterator = objects.begin(); iterator != objects.end();)
	{
		if ((*iterator)->coll != nullptr)
		{
			(*iterator)->CleanUp();
			(*iterator)->coll->Destroy();
			(*iterator)->coll = nullptr;
		}

		delete((*iterator));
		(*iterator) = nullptr;
		iterator = objects.erase(iterator);
	}

	obj_tanks.clear();
	objects.clear();

	return true;
}

Object* M_ObjManager::CreateObject(ObjectType type, fPoint pos)
{
	Object* ret = nullptr;
	switch (type)
	{
	case ObjectType::TANK:
		ret = new Obj_Tank(pos);
		ret->type = ObjectType::TANK;
		obj_tanks.push_back((Obj_Tank*)ret);
		break;
	}

	if (ret != nullptr)
	{
		ret->Start();
		objects.push_back(ret);
	}
  
	return ret;
}


void M_ObjManager::DeleteObjects()
{
	for (std::list<Object*>::iterator iterator = objects.begin(); iterator != objects.end(); ++iterator)
	{
		if ((*iterator) != nullptr)
		{
			(*iterator)->CleanUp();
			delete (*iterator);
			(*iterator) = nullptr;
		}
	}

	objects.clear();
	obj_tanks.clear();
	enemies.clear();
}

std::list<Object*> M_ObjManager::GetObjects() const
{
	return this->objects;
}

void M_ObjManager::DrawDebug(const Object* obj, Camera* camera)
{
	SDL_Rect section = { obj->pos_screen.x - obj->draw_offset.x, obj->pos_screen.y - obj->draw_offset.y, obj->frame.w, obj->frame.h };

	Uint8 alpha = 0;
	switch (obj->type)
	{
	case ObjectType::TANK:
		app->render->DrawQuad(section, 255, 0, 0, alpha);
		break;
	case ObjectType::STATIC:
		app->render->DrawQuad(section, 0, 255, 0, alpha);
		break;
	case ObjectType::TESLA_TROOPER:
		app->render->DrawQuad(section, 0, 0, 255, alpha);
		break;
	case ObjectType::EXPLOSION:
		app->render->DrawQuad(section, 255, 0, 255, alpha);
	default:
		break;
	}

	app->render->DrawCircle(obj->pos_screen.x + obj->pivot.x, obj->pos_screen.y + obj->pivot.y, 3, camera, 0, 255, 0);
}





bool M_ObjManager::Load(pugi::xml_node& load)
{
	bool ret = true;

	return ret;
}

bool M_ObjManager::Save(pugi::xml_node& save) const
{
	bool ret = true;

	return ret;
}


bool M_ObjManager::SortByYPos(Object * obj1, Object * obj2)
{
	return obj1->pivot.y + obj1->pos_screen.y < obj2->pivot.y + obj2->pos_screen.y;
}