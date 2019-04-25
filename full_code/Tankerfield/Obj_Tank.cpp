#include <math.h>

#include "PugiXml/src/pugiconfig.hpp"
#include "PugiXml/src/pugixml.hpp"

#include "app.h"
#include "Obj_Tank.h"
#include "M_Textures.h"
#include "M_Render.h"
#include "M_Collision.h"
#include "M_Input.h"
#include "Log.h"
#include "M_Map.h"
#include "M_Scene.h"
#include "M_ObjManager.h"
#include "M_Window.h"
#include "M_Audio.h"
#include "PerfTimer.h"
#include "MathUtils.h"
#include "Obj_Bullet.h"
#include "Bullet_Missile.h"
#include "Bullet_Laser.h"
#include "Healing_Bullet.h"
#include "Obj_HealingAnimation.h"
#include "Obj_Fire.h"
#include "Obj_PickUp.h"
#include "M_AnimationBank.h"
#include "Player_GUI.h"
#include "UI_InGameElement.h"
#include "M_UI.h"
#include "M_ObjManager.h"
#include "Camera.h"

int Obj_Tank::number_of_tanks = 0;

Obj_Tank::Obj_Tank(fPoint pos) : Object(pos)
{
	number_of_tanks++;
}

Obj_Tank::~Obj_Tank()
{
	number_of_tanks--;

	if (camera_player != nullptr)
	{
		camera_player->assigned = false;
		camera_player = nullptr;
	}
}

bool Obj_Tank::Start()
{
	pugi::xml_node tank_node = app->config.child("object").child("tank");

	pugi::xml_node tank_node_recoil = app->config.child("object").child("tank").child("recoil");

	velocity_recoil_decay = tank_node_recoil.child("velocity_recoil_decay").attribute("value").as_float();
	velocity_recoil_speed_max = tank_node_recoil.child("velocity_recoil_speed_max").attribute("value").as_float();
	velocity_recoil_speed_max_charged = tank_node_recoil.child("velocity_recoil_speed_max_charged").attribute("value").as_float();
	lerp_factor_recoil = tank_node_recoil.child("lerp_factor_recoil").attribute("value").as_float();

	//Textures-------------------------------------------------

	//-- Base
	base_tex_orange = app->tex->Load(tank_node.child("spritesheets").child("base_orange").text().as_string());
	base_tex_green = app->tex->Load(tank_node.child("spritesheets").child("base_green").text().as_string());
	base_tex_pink = app->tex->Load(tank_node.child("spritesheets").child("base_pink").text().as_string());
	base_tex_blue = app->tex->Load(tank_node.child("spritesheets").child("base_blue").text().as_string());

	base_shadow_tex = app->tex->Load(tank_node.child("spritesheets").child("base_shadow").text().as_string());
	SDL_SetTextureBlendMode(base_shadow_tex, SDL_BLENDMODE_MOD);

	//-- Turr
	std::string aux = tank_node.child("spritesheets").child("turr_orange").text().as_string();
	turr_tex_orange = app->tex->Load(tank_node.child("spritesheets").child("turr_orange").text().as_string());
	turr_tex_green = app->tex->Load(tank_node.child("spritesheets").child("turr_green").text().as_string());
	turr_tex_pink = app->tex->Load(tank_node.child("spritesheets").child("turr_pink").text().as_string());
	turr_tex_blue = app->tex->Load(tank_node.child("spritesheets").child("turr_blue").text().as_string());
	turr_shadow_tex = app->tex->Load(tank_node.child("spritesheets").child("turr_shadow").text().as_string());
	SDL_SetTextureBlendMode(turr_shadow_tex, SDL_BLENDMODE_MOD);
	//-- Revive 
	cycle_bar_tex = app->tex->Load(tank_node.child("spritesheets").child("cycle_bar_tex").text().as_string());
	cycle_bar_anim.frames = app->anim_bank->LoadFrames(app->anim_bank->animations_xml_node.child("cycle-progress-bar"));

	//sfx -------------------------------------------------------------------------------------------------------
	shot_sound = app->audio->LoadFx(tank_node.child("sounds").child("basic_shot").attribute("sound").as_string());
	movement_sfx = app->audio->LoadFx("audio/Fx/vlanstab.wav");
	revive_sfx = app->audio->LoadFx("audio/Fx/tank/revivir.wav");
	die_sfx = app->audio->LoadFx("audio/Fx/tank/death-sfx.wav");


	kb_up		= SDL_SCANCODE_W;
	kb_left		= SDL_SCANCODE_A;
	kb_down		= SDL_SCANCODE_S;
	kb_right	= SDL_SCANCODE_D;
	kb_item		= SDL_SCANCODE_Q;
	kb_interact	= SDL_SCANCODE_E;
	kb_ready	= SDL_SCANCODE_Z;
	curr_tex = base_tex_green;
	turr_tex = turr_tex_green;


	kb_shoot = SDL_BUTTON_LEFT;

	rotate_base.frames = app->anim_bank->LoadFrames(tank_node.child("animations").child("rotate_base"));
	curr_anim = &rotate_base;

	rotate_turr.frames = app->anim_bank->LoadFrames(tank_node.child("animations").child("rotate_turr"));

	curr_speed = speed = 5.f;//TODO: Load from xml

	cos_45 = cosf(-45.f * DEGTORAD);
	sin_45 = sinf(-45.f * DEGTORAD);

	coll = app->collision->AddCollider(pos_map, 0.8f, 0.8f, Collider::TAG::PLAYER,0.f,this);
	coll->AddRigidBody(Collider::BODY_TYPE::DYNAMIC);
	coll->SetObjOffset({ -0.4f, -0.4f });

	cannon_height = 11.f;
	cannon_length = 0.f;

	gamepad_move		= Joystick::LEFT;
	gamepad_aim			= Joystick::RIGHT;
	gamepad_shoot		= SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
	gamepad_item		= SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
	gamepad_interact	= SDL_CONTROLLER_BUTTON_X;

	draw_offset.x = 46;
	draw_offset.y = 46;

	base_angle_lerp_factor = 11.25f;
	shot_angle_lerp_factor = 11.25f;

	revive_range = 2.5f;
	revive_range_squared = revive_range * revive_range;
	revive_life = 100;
	revive_time = 3000.f;

	//Life inicialistation
	//item = ObjectType::HEALTH_BAG;

	std::vector<Camera*>::iterator item_cam;

	for (item_cam = app->render->cameras.begin(); item_cam != app->render->cameras.end(); ++item_cam)
	{
		if (!(*item_cam)->assigned)
		{
			(*item_cam)->assigned = true;
			camera_player = (*item_cam);
			break;
		}
	}

	return true;
}

bool Obj_Tank::PreUpdate()
{

	if (controller == nullptr)
	{
		controller = app->input->GetAbleController();
	}

	SelectInputMethod();

	return true;
}

bool Obj_Tank::Update(float dt)
{
	Movement(dt);
	CameraMovement(dt);
	return true;
}

void Obj_Tank::CameraMovement(float dt)
{
	if (camera_player == nullptr)
		return;

	camera_player->ResetCamera();
	camera_player->FollowPlayer(dt, this);
	camera_player->ShakeCamera(dt);
}

fPoint Obj_Tank::GetTurrPos()
{
	return turr_pos;
}

void Obj_Tank::Movement(float dt)
{
	fPoint input_dir(0.f, 0.f);

	if (move_input == INPUT_METHOD::KEYBOARD_MOUSE)
	{
		InputMovementKeyboard(input_dir);
	}
	else if (move_input == INPUT_METHOD::CONTROLLER)
	{
		InputMovementController(input_dir);
	}

	//The tank has to go up in isometric space, so we need to rotate the input vector by 45 degrees
	fPoint iso_dir(0.f, 0.f);
	iso_dir.x = input_dir.x * cos_45 - input_dir.y * sin_45;
	iso_dir.y = input_dir.x * sin_45 + input_dir.y * cos_45;
	iso_dir.Normalize();

	if (!iso_dir.IsZero())
	{
		float target_angle = atan2(input_dir.y, -input_dir.x) * RADTODEG;
		//Calculate how many turns has the base angle and apply them to the target angle
		float turns = floor(angle / 360.f);
		target_angle += 360.f * turns;
		//Check which distance is shorter. Rotating clockwise or counter-clockwise
		if (abs((target_angle + 360.f) - angle) < abs(target_angle - angle))
		{
			target_angle += 360.f;
		}
		angle = lerp(angle, target_angle, base_angle_lerp_factor * dt);

	}

	velocity = iso_dir * curr_speed * dt;  

	pos_map += velocity;
}


void Obj_Tank::InputMovementKeyboard(fPoint & input)
{
	if (app->input->GetKey(kb_up) == KEY_DOWN || app->input->GetKey(kb_up) == KEY_REPEAT)
	{
		//app->render->camera.y -= floor(100.0f * dt);
		input.y -= 1.f;
	}
	if (app->input->GetKey(kb_left) == KEY_DOWN || app->input->GetKey(kb_left) == KEY_REPEAT)
	{
		//app->render->camera.x -= floor(100.0f * dt);
		input.x -= 1.f;
	}
	if (app->input->GetKey(kb_down) == KEY_DOWN || app->input->GetKey(kb_down) == KEY_REPEAT)
	{
		//app->render->camera.y += floor(100.0f * dt);
		input.y += 1.f;
	}
	if (app->input->GetKey(kb_right) == KEY_DOWN || app->input->GetKey(kb_right) == KEY_REPEAT)
	{
		//app->render->camera.x += floor(100.0f * dt);
		input.x += 1.f;
	}


}

void Obj_Tank::InputMovementController(fPoint & input)
{
	input = (fPoint)(*controller)->GetJoystick(gamepad_move);
}

bool Obj_Tank::Draw(float dt, Camera * camera)
{
	// Base =========================================
	app->render->Blit(
		curr_tex,
		pos_screen.x - draw_offset.x,
		pos_screen.y - draw_offset.y,
		camera,
		&curr_anim->GetFrame(angle));

	if (show_crosshairs && camera == camera_player)
	{
		float line_length = 5.f;
		//1-- Set a position in the isometric space
		fPoint input_iso_pos(turr_pos.x + shot_dir.x * line_length, turr_pos.y + shot_dir.y * line_length);
		//2-- Transform that poin to screen coordinates
		iPoint input_screen_pos = (iPoint)app->map->MapToScreenF(input_iso_pos);
		app->render->DrawLineSplitScreen(
			pos_screen.x, pos_screen.y - cannon_height,
			input_screen_pos.x, input_screen_pos.y, 0, 0, 255, 123, camera);
	}

	// Turret =======================================
	app->render->Blit(
		turr_tex,
		pos_screen.x - draw_offset.x,
		pos_screen.y - draw_offset.y,
		camera,
		&rotate_turr.GetFrame(turr_angle));

	return true;
}

bool Obj_Tank::DrawShadow(Camera * camera, float dt)
{
	// Revive cycle =================================
	if (draw_revive_cycle_bar)
	{
		SDL_Rect rect = cycle_bar_anim.GetFrame(0);
		app->render->BlitScaled(
			cycle_bar_tex,
			pos_screen.x - rect.w*0.5f * 0.5f,
			pos_screen.y - rect.h*0.5f * 0.5f,
			camera,
			&rect,0.5f);
		cycle_bar_anim.NextFrame(dt);
	}

	// Base =========================================
	app->render->Blit(
		base_shadow_tex,
		pos_screen.x - draw_offset.x,
		pos_screen.y - draw_offset.y,
		camera,
		&curr_anim->GetFrame(angle));

	// Turret =======================================
	app->render->Blit(
		turr_shadow_tex,
		pos_screen.x - draw_offset.x,
		pos_screen.y - draw_offset.y,
		camera,
		&rotate_turr.GetFrame(turr_angle));

	return true;
}

bool Obj_Tank::CleanUp()
{
	return true;
}


void Obj_Tank::OnTrigger(Collider * c1)
{

}

void Obj_Tank::OnTriggerExit(Collider * c1)
{

}

void Obj_Tank::InputShotMouse(const fPoint & turr_map_pos, fPoint & input_dir, fPoint & iso_dir)
{
	iPoint mouse_screen_pos = { 0, 0 };
	app->input->GetMousePosition(mouse_screen_pos.x, mouse_screen_pos.y);

	
	//Add the position of the mouse plus the position of the camera to have the pixel that selects the mouse in the world and then pass it to the map.

	if (camera_player != nullptr)
		mouse_screen_pos += {camera_player->rect.x, camera_player->rect.y};


	input_dir = (fPoint)mouse_screen_pos - app->map->MapToScreenF(turr_map_pos);

	//Transform to map to work all variables in map(blit do MapToWorld automatically)
	iso_dir = app->map->ScreenToMapF(mouse_screen_pos.x, mouse_screen_pos.y) - turr_map_pos;
	iso_dir.Normalize();
}

void Obj_Tank::Shoot(float dt)
{
	//fPoint Obj_Tank::pos is on the center of the base
	//fPoint shot_pos is on the center of the turret (considers the cannon_height)
	turr_pos = pos_map - app->map->ScreenToMapF(  0, cannon_height );
	fPoint iso_dir (0.f, 0.f);
	fPoint input_dir (0.f, 0.f);

	if (shot_input == INPUT_METHOD::KEYBOARD_MOUSE)
	{
		InputShotMouse(turr_pos, input_dir, iso_dir);
	}


	if (!input_dir.IsZero())
	{
		//Angle
		turr_target_angle = atan2(-input_dir.y, input_dir.x) * RADTODEG;
		//- Calculate how many turns has the base angle and apply them to the target angle
		float turns = floor(turr_angle / 360.f);
		turr_target_angle += 360.f * turns;
		//- Check which distance is shorter. Rotating clockwise or counter-clockwise
		if (abs((turr_target_angle + 360.f) - turr_angle) < abs(turr_target_angle - turr_angle))
		{
			turr_target_angle += 360.f;
		}

		//Direction
		shot_dir = iso_dir;//Keep the last direction to shoot bullets if the joystick is not being aimed
	}
	turr_angle = lerp(turr_angle, turr_target_angle, shot_angle_lerp_factor * dt);

}

//Select the input method depending on the last input pressed
//Prioritize controller if both inputs are being pressed at the same time
void Obj_Tank::SelectInputMethod()
{
	//Move input
	if (move_input != INPUT_METHOD::KEYBOARD_MOUSE
		&& (app->input->GetKey(kb_up) != KEY_IDLE
		|| app->input->GetKey(kb_left) != KEY_IDLE
		|| app->input->GetKey(kb_down) != KEY_IDLE
		|| app->input->GetKey(kb_right) != KEY_IDLE))
	{
		move_input = INPUT_METHOD::KEYBOARD_MOUSE;
	}
	if (move_input != INPUT_METHOD::CONTROLLER
		&& (controller != nullptr
		&& !(*controller)->GetJoystick(gamepad_move).IsZero()))
	{
		move_input = INPUT_METHOD::CONTROLLER;
	}

	//Shot input
	if (shot_input != INPUT_METHOD::KEYBOARD_MOUSE
		&& app->input->GetMouseButton(kb_shoot) != KEY_IDLE)
	{
		shot_input = INPUT_METHOD::KEYBOARD_MOUSE;
		SDL_ShowCursor(SDL_ENABLE);
	}
	if (shot_input != INPUT_METHOD::CONTROLLER
		&& (controller != nullptr
		&& (!(*controller)->GetJoystick(gamepad_aim).IsZero()
		|| (*controller)->GetAxis(gamepad_shoot) > 0)))
	{
		shot_input = INPUT_METHOD::CONTROLLER;
		SDL_ShowCursor(SDL_DISABLE);
	}
}


bool Obj_Tank::PressInteract()
{
	return (controller != nullptr && ((*controller)->GetButtonState(gamepad_interact) == KEY_DOWN) || app->input->GetKey(kb_interact) == KeyState::KEY_DOWN);
}

bool Obj_Tank::ReleaseInteract()
{
	return (controller != nullptr && ((*controller)->GetButtonState(gamepad_interact) == KEY_UP) || (app->input->GetKey(kb_interact) == KeyState::KEY_UP));
}

