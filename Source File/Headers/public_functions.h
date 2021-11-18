#ifndef PUBLIC_FUNCTIONS_H_INCLUDED
#define PUBLIC_FUNCTIONS_H_INCLUDED

#include "SFML/Graphics.hpp"
#include <string>

void TendTowards(float& value, const float& destValue, const float& tendRate, const float& dt = 0.f);
void TendTowards(float& value, const float& destValue, const float& transmissionFactor, const float& minTransmission, const float& dt);

int GenerateRandomNumber(int range);
void SetRandomSeed(int seed);
int GetRandomSeed();

float MagnitudeOfVector(const sf::Vector2f& vec);
float MagnitudeSqureOfVector(const sf::Vector2f& vec);
sf::Vector2f NormalizeVector(sf::Vector2f vec);

std::string ConvertIntToString(const int& number);
std::string ConvertTimeToString(const int& seconds);
void ShortenText(sf::Text& text_, const float& width, bool useGlobalBounds = false);
void AddRectVerticesToArray(sf::VertexArray& vertexArray_, sf::FloatRect rect_, sf::Color color_ = sf::Color::White, float rotation_ = 0, sf::IntRect textureRect_ = sf::IntRect());
sf::Color ChangeColorHue(const sf::Color& color_, int amount_);

#endif // PUBLIC_FUNCTIONS_H_INCLUDED
