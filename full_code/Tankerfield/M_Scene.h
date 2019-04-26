#ifndef __M_SCENE_H__
#define __M_SCENE_H__

#include "SDL/include/SDL_rect.h"
#include "Module.h"

class Obj_Tank;
class Object;
class Minimap;

class M_Scene : public Module
{
public:

	Obj_Tank * player = nullptr;
	Minimap * minimap = nullptr;

public:

	M_Scene();

	virtual ~M_Scene();

	bool Awake(pugi::xml_node&) override;

	bool Start() override;

	bool PreUpdate() override;

	bool Update(float dt) override;

	bool PostUpdate(float dt) override;

	bool Reset();

	bool CleanUp() override;

	bool draw_debug = false;

};

#endif // __j1SCENE_H__