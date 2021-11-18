#ifndef BASECORE_H_INCLUDED
#define BASECORE_H_INCLUDED

#include "ship.h"

class BaseCore : public Ship
{
public:
    const int WHEEL_ROTATION_SPEED = 15;
    static const int COLLISION_RADIUS = 83.5f;

    BaseCore(short team_, sf::Vector2i grid_);
    ~BaseCore();

    static void DrawUpper(sf::RenderTarget& target);
    static void DrawLower(sf::RenderTarget& target);
    static void ResetBaseCoresToDraw();

    void Update(float dt);
    void Update(float dt, sf::FloatRect cameraRect);
    sf::Vector2i GetGrid();
    sf::FloatRect GetBoundingRect();
    sf::FloatRect GetCoreBoundingRect();
    void GetPoints(std::vector<sf::Vector2f>& points);
    sf::Vector2f GetPosition();
    int GetBaseId();
    void GiveKillPoints(Ship* shooter);
    void GetDamagedBy(float damage, sf::Vector2f hitPosition, Ship* shooter = nullptr);
    void InitializeForLevel();

    //Useless ship functions not needed
    void Shoot();
    int GetLevelBaseExp();
    int GetNextLevelBaseExp();
    bool ResolveWallCollision(sf::FloatRect& wallRect, const float& dt);

    void SetTexture(sf::Texture* texture);

private:
    sf::Vector2i grid;
    sf::Vector2f position;
    sf::Sprite sprite;
    sf::Sprite wheel;
    sf::Sprite wheelCover;
    sf::Sprite spriteUpper;
    sf::FloatRect rect;
    sf::FloatRect coreRect;

    float wheelAngle;
    int baseId;
    sf::Vector2f globalPoints[5];
};

#endif // BASECORE_H_INCLUDED
