#include "public_functions.h"
#include "random"
#include <sstream>

int randomSeed = 0;

void TendTowards(float& value, const float& destValue, const float& tendRate, const float& dt)
{
    if(value != destValue)
    {
        if(dt > 0.0f)
        {
            if(value < destValue)
            {
                value += (tendRate * dt);
                if(value > destValue)
                    value = destValue;
            }
            else if(value > destValue)
            {
                value -= (tendRate * dt);
                if(value < destValue)
                    value = destValue;
            }
        }

        else
        {
            if(value < destValue)
            {
                value += tendRate;
                if(value > destValue)
                    value = destValue;
            }
            else if(value > destValue)
            {
                value -= tendRate;
                if(value < destValue)
                    value = destValue;
            }
        }
    }
}

void TendTowards(float& value, const float& destValue, const float& transmissionFactor, const float& minTransmission, const float& dt)
{
    if(value != destValue)
    {
        float difference = destValue - value;
        if(difference != 0)
        {
            float sign = 1;
            if(difference < 0)
                sign = -1;

            float transmit = difference * sign;
            if(transmissionFactor > 0.f)
            {
                transmit *= dt / transmissionFactor;
                if(transmit < minTransmission)
                    transmit = minTransmission;
            }

            value += transmit * sign;

            difference = destValue - value;
            if(difference * sign < 0)
                value = destValue;
        }
    }
}

int GenerateRandomNumber(int range)
{
    if(range > 0)
        return rand() % range;
    return 0;
}

int GetRandomSeed()
{
    return randomSeed;
}

void SetRandomSeed(int seed)
{
    randomSeed = seed;
    srand(randomSeed);
}

float MagnitudeOfVector(const sf::Vector2f& vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y);
}

float MagnitudeSqureOfVector(const sf::Vector2f& vec)
{
    return vec.x * vec.x + vec.y * vec.y;
}

sf::Vector2f NormalizeVector(sf::Vector2f vec)
{
    float magn = MagnitudeOfVector(vec);
    if(magn != 0)
        return vec / magn;
    return sf::Vector2f();
}

std::string ConvertIntToString(const int& number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

std::string ConvertTimeToString(const int& seconds)
{
    std::stringstream ss;
    if(seconds > 60)
        ss << seconds / 60 << ':' << seconds % 60 << "min";
    else ss << seconds << "sec";
    return ss.str();
}

void ShortenText(sf::Text& text_, const float& width, bool useGlobalBounds)
{
    if((useGlobalBounds ? text_.getGlobalBounds().width : text_.getLocalBounds().width) > width)
        while(text_.getString().getSize() > 0)
        {
            std::string str = text_.getString();
            str.pop_back();
            text_.setString(str + "...");
            if((useGlobalBounds ? text_.getGlobalBounds().width : text_.getLocalBounds().width) > width)
                text_.setString(str);
            else break;
        }
}

sf::Vertex temp_addRect_vertices[4];
void AddRectVerticesToArray(sf::VertexArray& vertexArray_, sf::FloatRect rect_, sf::Color color_, float rotation_, sf::IntRect textureRect_)
{
    temp_addRect_vertices[0].position = sf::Vector2f(rect_.left, rect_.top);
    temp_addRect_vertices[1].position = sf::Vector2f(rect_.left + rect_.width, rect_.top);
    temp_addRect_vertices[2].position = sf::Vector2f(rect_.left + rect_.width, rect_.top + rect_.height);
    temp_addRect_vertices[3].position = sf::Vector2f(rect_.left, rect_.top + rect_.height);

    temp_addRect_vertices[0].texCoords = sf::Vector2f(textureRect_.left, textureRect_.top);
    temp_addRect_vertices[1].texCoords = sf::Vector2f(textureRect_.left + textureRect_.width, textureRect_.top);
    temp_addRect_vertices[2].texCoords = sf::Vector2f(textureRect_.left + textureRect_.width, textureRect_.top + textureRect_.height);
    temp_addRect_vertices[3].texCoords = sf::Vector2f(textureRect_.left, textureRect_.top + textureRect_.height);

    if(rotation_ != 0)
    {
        sf::Transformable transformable;
        transformable.setPosition(rect_.left + rect_.width / 2.f, rect_.top + rect_.height / 2.f);
        transformable.setOrigin(rect_.left + rect_.width / 2.f, rect_.top + rect_.height / 2.f);
        transformable.setRotation(rotation_);

        sf::Transform transform_ = transformable.getTransform();

        for(int a = 0; a < 4; a++)
            temp_addRect_vertices[a].position = transform_.transformPoint(temp_addRect_vertices[a].position);
    }

    for(int a = 0; a < 4; a++)
    {
        temp_addRect_vertices[a].color = color_;
        vertexArray_.append(temp_addRect_vertices[a]);
    }
}

sf::Color ChangeColorHue(const sf::Color& color_, int amount_)
{
    int r = (int)color_.r + amount_;
    int b = (int)color_.b + amount_;
    int g = (int)color_.g + amount_;

    if(r < 0) r = 0;
    if(b < 0) b = 0;
    if(g < 0) g = 0;

    if(r > 255) r = 255;
    if(b > 255) b = 255;
    if(g > 255) g = 255;

    return sf::Color(r, g, b, color_.a);
}
