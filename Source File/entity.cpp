#include "entity.h"
#include "game.h"
#include "collision.h"
#include "math.h"

Entity::Entity() :
    angle(0.f),
    alive(true),
    team(-1)
{
}

Entity::~Entity()
{
}

void Entity::SetAngle(float Y, float X)
{
    float rad = atan2f(Y, X);

    angle = 90 + (rad * 180 / 3.142f);
}

void Entity::SetAngle(float _angle)
{
    angle = _angle;
}


void Entity::SetTeam(short team_)
{
    team = team_;
}

float Entity::GetAngle()
{
    return angle;
}

bool Entity::GetAlive()
{
    return alive;
}

short Entity::GetTeam()
{
    return team;
}

void Entity::Kill()
{
    alive = false;
}

