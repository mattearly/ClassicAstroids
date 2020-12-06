#pragma once
#include <glm/glm.hpp>
#include "../../rand/rand/rand.h"

struct Astroid
{
	Astroid(float sp, glm::vec2 dir, glm::vec3 rot, int obj_id, int inst_id, bool fragment)
		: speed(sp), direction(dir), rotation(rot), object_id(obj_id), instance_id(inst_id), isFragment(fragment)
	{};

	float speed;
	glm::vec2 direction;
	glm::vec3 rotation;

	int object_id;
	int instance_id;

	bool isFragment;
};

Astroid createAstroid(int obj_id, int inst_id, bool fragment)
{
	float rand_speed = NTKR(1.f, 5.f);

	glm::vec2 rand_direction;
	rand_direction.x = NTKR(-1.f, 1.f);
	rand_direction.y = NTKR(-1.f, 1.f);

	glm::vec3 rand_rotation;
	rand_rotation.x = NTKR(0.f, 2.6f);
	rand_rotation.y = NTKR(0.f, 2.6f);
	rand_rotation.z = NTKR(0.f, 2.6f);

	return Astroid(rand_speed, rand_direction, rand_rotation, obj_id, inst_id, fragment);
}