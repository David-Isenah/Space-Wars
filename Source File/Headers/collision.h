#ifndef COLLSION_H_INCLUDED
#define COLLSION_H_INCLUDED

#include<SFML/Graphics.hpp>

float DotProduct(const sf::Vector2f& vec1, const sf::Vector2f& vec2);

//Polygon to Polygon
bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos ,const std::vector<sf::Vector2f>& obj2DotsPos);

bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const std::vector<sf::Vector2f>& obj2DotsPos, sf::Vector2f& pushVector);

bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const std::vector<sf::Vector2f>& obj2DotsPos, sf::Vector2f& obj1EntryDir, sf::Vector2f& pushVector);

//Polygon to circle
bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const sf::Vector2f& obj2CircleCenter, const float& obj2CircleRadius, sf::Vector2f& pushVector);

bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const sf::Vector2f& obj2CircleCenter, const float& obj2CircleRadius, sf::Vector2f& obj1EntryDir, sf::Vector2f& pushVector);

#endif // COLLSION_H_INCLUDED
