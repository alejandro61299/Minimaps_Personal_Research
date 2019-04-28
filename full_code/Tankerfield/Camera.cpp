#include "Camera.h"
#include "Obj_Tank.h"
#include "M_Map.h"

Camera::Camera()
{
	max_shake_offset = 30.f;
	trauma_decay = 0.9f;
	random_generator.seed(random_device());
	aim_distance = 2.5f;
	lerp_factor = 7.f;
}

void Camera::FollowPlayer(float dt, Obj_Tank * player)
{
	fPoint source_pos(camera_pos);

	fPoint target_pos( app->map->MapToScreenF(player->pos_map).x - (float)rect.w * 0.5f, app->map->MapToScreenF(player->pos_map).y - (float)rect.h * 0.5f);

	fPoint lerp_pos = lerp(source_pos, target_pos, dt * lerp_factor);

	camera_pos = lerp_pos;

	rect.x = (float)camera_pos.x;
	rect.y = (float)camera_pos.y;
}


void Camera::MoveToScreenPoint(float dt, fPoint point)
{
	fPoint source_pos = camera_pos;

	fPoint target_pos = point;

	fPoint lerp_pos = lerp(source_pos, target_pos, dt * lerp_factor);

	camera_pos = lerp_pos;

	rect.x = (float)camera_pos.x;
	rect.y = (float)camera_pos.y;
}


void Camera::AddTrauma(float value)
{
	if (trauma + value >= 1.f)
	{
		trauma = 1.f;
	}
	else
	{
		trauma += value;
	}
}

void Camera::ResetCamera()
{
	//camera_pos = (fPoint)unaltered_pos;
	//rect.x = unaltered_pos.x;
	//rect.y = unaltered_pos.y;
}

void Camera::ShakeCamera(float dt)
{
	////Keep the original position
	//unaltered_pos.x = rect.x;
	//unaltered_pos.y = rect.y;

	////Apply the camera shake
	//if (trauma > 0.f)
	//{
	//	float shake = GetShakeAmount();
	//	rect.x += max_shake_offset * shake * GetRandomValue(-1.f, 1.f);
	//	rect.y += max_shake_offset * shake * GetRandomValue(-1.f, 1.f);

	//	//Reduce trauma
	//	trauma -= trauma_decay * dt;
	//}
}

//Shake amout is based on trauma
//It is not linearly dependant on trauma, and instead its trauma^2 because it feels better that with more trauma there is more shake
float Camera::GetShakeAmount() const
{
	return trauma * trauma;
}


float Camera::GetRandomValue(float min_value, float max_value)
{
	std::uniform_real_distribution<float> dis(min_value, std::nextafter(max_value, FLT_MAX));

	return dis(random_generator);
}