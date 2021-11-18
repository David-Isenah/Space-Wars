#ifndef ENTITY_H_INCLUDED
#define ENTITY_H_INCLUDED

#include "string"
#include "SFML/Graphics.hpp"
#include "collision.h"

class Entity //Base class for ships and bullets
{
    public:
        Entity();
        virtual ~Entity(); //must declare destructor for all inherited classes

        //virtual void SetPosition(sf::Vector2f& position) = 0;
        void SetTeam(short team_);
        void SetAngle(float Y, float X);
        void SetAngle(float _angle);

        float GetAngle();
        virtual sf::Vector2f GetPosition() = 0;
        bool GetAlive();
        short GetTeam();

        virtual void GetPoints(std::vector<sf::Vector2f>& points) = 0;
        virtual sf::FloatRect GetBoundingRect() = 0;
        virtual void Update(float dt) = 0;
        virtual void Kill();

    protected:
        short team;
        float angle;
        bool alive;
};

#endif // ENTITY_H_INCLUDED
