#pragma once

#include "Entity.h"
#include "..\Physics\ColliderType.h"
#include "..\Math\Vector2.h"

struct Rigidbody
{
	Entity entity;
	ColliderType colliderType;
	Vector2 velocity;
	float angularVelocity;
};