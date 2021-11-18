#ifndef SKIN_H_INCLUDED
#define SKIN_H_INCLUDED

#include "SFML/Graphics.hpp"

struct Skin
{
    std::vector<sf::Vector2f> points;
    sf::Texture texture;
    sf::Vector2f shootPoint;
    sf::FloatRect collisionRect;
};

#endif // SKIN_H_INCLUDED
