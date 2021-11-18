#include "bullet.h"
#include "displayeffect_effects.h"
#include "public_functions.h"
#include "math.h"
#include "multidrawer.h"
#include "game.h"
#include <iostream>

MultiDrawer multidrawer_bullet[Bullet::NumOfBulletType];

//Optimization Variables
std::vector<sf::Vector2f> temp_collideWallPoints;
std::vector<sf::Vector2f> temp_collidePoints;
std::vector<sf::Vector2f> temp_collideObjPoints;

struct BulletInfo
{
    std::vector<sf::Vector2f> points;
    sf::Texture texture;
    float lifeTime = 0;
    float velocityMagn = 0;
    sf::Color fadeColor;
    float fadeLength = 0;
};

BulletInfo* bulletInfo[Bullet::NumOfBulletType];

void Bullet::InitializeBullets()
{
    for(int a = 0; a < NumOfBulletType; a++)
        bulletInfo[a] = new BulletInfo();

    //Points
    bulletInfo[LaserBullet]->points.push_back(sf::Vector2f(20, 230));
    bulletInfo[LaserBullet]->points.push_back(sf::Vector2f(0, 0));
    bulletInfo[LaserBullet]->points.push_back(sf::Vector2f(40, 0));
    bulletInfo[LaserBullet]->points.push_back(sf::Vector2f(40, 230));
    bulletInfo[LaserBullet]->points.push_back(sf::Vector2f(0, 230));

    bulletInfo[DisintegratorBullet]->points.push_back(sf::Vector2f(36, 276));
    bulletInfo[DisintegratorBullet]->points.push_back(sf::Vector2f(0, 0));
    bulletInfo[DisintegratorBullet]->points.push_back(sf::Vector2f(72, 0));
    bulletInfo[DisintegratorBullet]->points.push_back(sf::Vector2f(72, 276));
    bulletInfo[DisintegratorBullet]->points.push_back(sf::Vector2f(0, 276));

    //Textures
    bulletInfo[LaserBullet]->texture.loadFromFile("Resources/Images/Bullets/laserbullet.png");
    bulletInfo[DisintegratorBullet]->texture.loadFromFile("Resources/Images/Bullets/disintegratorbullet.png");

    //Life Time
    bulletInfo[LaserBullet]->lifeTime = 2;
    bulletInfo[DisintegratorBullet]->lifeTime = 2;

    //Velocity Magnitude
    bulletInfo[LaserBullet]->velocityMagn = 1500;
    bulletInfo[DisintegratorBullet]->velocityMagn = 1200;

    //Fade Color
    bulletInfo[LaserBullet]->fadeColor = sf::Color(255, 255, 255, 150);
    bulletInfo[DisintegratorBullet]->fadeColor = sf::Color(255, 255, 255, 150);

    //Fade Length
    bulletInfo[LaserBullet]->fadeLength = 200;
    bulletInfo[DisintegratorBullet]->fadeLength = 150;

    //Others
    *bulletInfo[EnhancedLaserBullet] = *bulletInfo[Bullet::LaserBullet];
    bulletInfo[EnhancedLaserBullet]->velocityMagn = 2000;

    //MultiDrawer
    for(int a = 0; a < NumOfBulletType; a++)
        multidrawer_bullet[a].SetTexture(&bulletInfo[a]->texture);
}

void Bullet::DeleteBullets()
{
    for(int a = 0; a < NumOfBulletType; a++)
    {
        delete bulletInfo[a];
        bulletInfo[a] = nullptr;
    }
}

void Bullet::Draw(sf::RenderTarget& target)
{
    for(int a = 0; a < NumOfBulletType; a++)
        multidrawer_bullet[a].Draw(target);
}

Bullet::Bullet(sf::Vector2f strPosition, float strAngle, short type_, short team_, int damage_, float scale_, Ship* shooter_) :
    damage(damage_),
    type(type_),
    lifeTime(bulletInfo[type_]->lifeTime),
    shooter(shooter_)
{
    team = team_;
    angle = strAngle;
    multiDrawer = &multidrawer_bullet[type_];

    velocity.x = bulletInfo[type_]->velocityMagn * cos((90 - strAngle) * 3.142f / 180.f);
    velocity.y = -bulletInfo[type_]->velocityMagn * sin((90 - strAngle) * 3.142f / 180.f);

    if(multiDrawer != nullptr)
    {
        sf::Vector2f textrSize;
        if(multiDrawer->GetTexture() != nullptr)
            textrSize = (sf::Vector2f)multiDrawer->GetTexture()->getSize();

        if(bulletInfo[type_]->points.size() > 0)
            transformation.setOrigin(bulletInfo[type_]->points[0]);
        transformation.setScale(scale_, scale_);
        transformation.setPosition(strPosition);
        transformation.setRotation(angle);

        vertices[0].texCoords = sf::Vector2f(0, 0);
        vertices[1].texCoords = sf::Vector2f(textrSize.x, 0);
        vertices[2].texCoords = sf::Vector2f(textrSize.x, textrSize.y);
        vertices[3].texCoords = sf::Vector2f(0, textrSize.y);

        refBounds.width = textrSize.x;
        refBounds.height = textrSize.y;
    }

    DisplayEffect::AddDisplayEffect(new DE_BulletSpeed(this, bulletInfo[type_]->fadeColor, bulletInfo[type_]->fadeLength));
}

void Bullet::RefreshEntities()
{
    if(shooter != nullptr && shooter->GetAlive() == false)
        shooter = nullptr;
}

Ship* Bullet::GetShooter()
{
    return shooter;
}

float Bullet::GetDamage()
{
    return damage;
}

short Bullet::GetBulletType()
{
    return type;
}

sf::Vector2f Bullet::GetPosition()
{
    return transformation.getPosition();
}

sf::Vector2f Bullet::GetVelocity()
{
    return velocity;
}

float Bullet::GetVelocityMagn()
{
    return bulletInfo[type]->velocityMagn;
}

sf::FloatRect Bullet::GetBoundingRect()
{
    return transformation.getTransform().transformRect(refBounds);
}

void Bullet::Update(float dt)
{
    if(GetAlive() == true)
    {
        lifeTime -= dt;

        if(lifeTime <= 0)
        {
            lifeTime = 0;
            alive = false;
        }
        else  transformation.move(velocity * dt);

        if(multiDrawer != nullptr && multiDrawer->GetTexture() != nullptr)
        {
            sf::Vector2f sizeTextr = (sf::Vector2f)multiDrawer->GetTexture()->getSize();
            vertices[0].position = transformation.getTransform().transformPoint(sf::Vector2f(0, 0));
            vertices[1].position = transformation.getTransform().transformPoint(sf::Vector2f(sizeTextr.x, 0));
            vertices[2].position = transformation.getTransform().transformPoint(sf::Vector2f(sizeTextr.x, sizeTextr.y));
            vertices[3].position = transformation.getTransform().transformPoint(sf::Vector2f(0, sizeTextr.y));

            multiDrawer->AddVertex(vertices, 4);
        }
    }
}

sf::Vector2f Bullet::Kill(sf::Vector2f moveVec_, Entity* killedBy_)
{
    alive = false;

    if(moveVec_ != sf::Vector2f() && velocity != sf::Vector2f())
    {
        sf::Vector2f unitVec = velocity / GetVelocityMagn();
        moveVec_ = -unitVec * (float)sqrt(moveVec_.x * moveVec_.x + moveVec_.y * moveVec_.y);
    }

    transformation.move(moveVec_);

    sf::Vector2f hitPoint = bulletInfo[type]->points[0];
    hitPoint.y = 0;
    hitPoint = transformation.getTransform().transformPoint(hitPoint);
    DisplayEffect::AddDisplayEffect(new DE_Hit(hitPoint, transformation.getScale().x * 0.6f, type, killedBy_));

    return hitPoint + sf::Vector2f(GenerateRandomNumber(201) - 100, GenerateRandomNumber(201) - 100) / 100.f * 5.f;
}

bool Bullet::ResolveWallCollision(sf::FloatRect& wallRect)
{
    if(GetBoundingRect().intersects(wallRect))
    {
        if(temp_collideWallPoints.size() != 5)
            temp_collideWallPoints.resize(5);

        temp_collideWallPoints[0] = sf::Vector2f(wallRect.left + wallRect.width / 2.f, wallRect.top + wallRect.height / 2.f);
        temp_collideWallPoints[1] = sf::Vector2f(wallRect.left, wallRect.top);
        temp_collideWallPoints[2] = sf::Vector2f(wallRect.left + wallRect.width, wallRect.top);
        temp_collideWallPoints[3] = sf::Vector2f(wallRect.left + wallRect.width, wallRect.top + wallRect.height);
        temp_collideWallPoints[4] = sf::Vector2f(wallRect.left, wallRect.top + wallRect.height);

        sf::Vector2f moveVec;
        GetPoints(temp_collidePoints);
        if(IsColliding(temp_collidePoints, temp_collideWallPoints, moveVec))
            Kill(moveVec);
    }
}

bool Bullet::ResolveShipCollision(Ship* ship, const float& dt)
{
    if(ship->GetAlive() && GetAlive() && Game::GetTeamUpType(team) != Game::GetTeamUpType(ship->GetTeam()) &&
       GetBoundingRect().intersects(ship->GetBoundingRect()))
    {
        GetPoints(temp_collidePoints);
        ship->GetPoints(temp_collideObjPoints);

        sf::Vector2f moveVec;
        if(IsColliding(temp_collidePoints, temp_collideObjPoints, moveVec))
        {
            ship->GetDamagedBy(damage, Kill(moveVec, ship), shooter);

            return true;
        }
    }
    return false;
}

bool Bullet::ResolveBaseCoreCollision(BaseCore* baseCore)
{
    if(baseCore != nullptr)
    {
        sf::FloatRect coreRect = baseCore->GetCoreBoundingRect();
        if(baseCore->GetAlive() && GetAlive() && Game::GetTeamUpType(team) != Game::GetTeamUpType(baseCore->GetTeam()) &&
           GetBoundingRect().intersects(coreRect))
        {
            GetPoints(temp_collidePoints);

            sf::Vector2f moveVec;
            if(IsColliding(temp_collidePoints, sf::Vector2f(coreRect.left + coreRect.width / 2.f, coreRect.top + coreRect.height / 2.f), BaseCore::COLLISION_RADIUS, moveVec))
            {
                baseCore->GetDamagedBy(damage, Kill(moveVec, baseCore));

                return true;
            }
        }
    }
    return false;
}

bool Bullet::ResolveWallCollision(std::vector<sf::FloatRect>& wallRect)
{
    for(int a = 0; a < wallRect.size(); a++)
        if(ResolveWallCollision(wallRect[a]))
            return true;

    return false;
}

void Bullet::GetPoints(std::vector<sf::Vector2f>& points)
{
    if(points.size() != bulletInfo[type]->points.size())
        points.resize(bulletInfo[type]->points.size());

    sf::Transform transform_ = transformation.getTransform();
    for(int a = 0; a < bulletInfo[type]->points.size(); a++)
        points[a] = transform_.transformPoint(bulletInfo[type]->points[a]);
}

sf::FloatRect Bullet::GetLocalBoundingRect()
{
    return refBounds;
}

float Bullet::GetScale()
{
    return transformation.getScale().x;
}

sf::Transform Bullet::GetTransform()
{
    return transformation.getTransform();
}
