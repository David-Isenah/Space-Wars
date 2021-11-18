#ifndef BULLET_H_INCLUDED
#define BULLET_H_INCLUDED

#include "ship.h"
#include "basecore.h"
#include "multidrawer.h"

class Bullet : public Entity
{
    public:
        static void InitializeBullets();
        static void DeleteBullets();
        static void Draw(sf::RenderTarget& target);
        static void ResetBulletShipCollisions();

        enum {LaserBullet = 0, EnhancedLaserBullet, SpreaderBullet, DisintegratorBullet, ThermalBullet, ElectronBullent, NumOfBulletType};

        Bullet(sf::Vector2f strPosition, float strAngle, short type_, short team_, int damage_, float scale_ = 1.f, Ship* shooter_ = nullptr);

        void Update(float dt);
        bool ResolveWallCollision(sf::FloatRect& wallRect);
        bool ResolveShipCollision(Ship* ship, const float& dt);
        bool ResolveBaseCoreCollision(BaseCore* baseCore);
        bool ResolveWallCollision(std::vector<sf::FloatRect>& wallRect);
        void RefreshEntities();
        Ship* GetShooter();
        void GetPoints(std::vector<sf::Vector2f>& points);
        float GetDamage();
        short GetBulletType();
        sf::Vector2f GetPosition();
        sf::Vector2f GetVelocity();
        float GetVelocityMagn();
        sf::FloatRect GetBoundingRect();
        sf::FloatRect GetLocalBoundingRect();
        float GetScale();
        sf::Transform GetTransform();
        sf::Vector2f Kill(sf::Vector2f moveVec_, Entity* killedBy_ = nullptr);

    private:
        sf::Vector2f velocity;
        sf::Transformable transformation;
        sf::FloatRect refBounds;
        float damage;
        float lifeTime;
        short type;
        Ship* shooter;

        MultiDrawer* multiDrawer;
        sf::Vertex vertices[4];
};

#endif // BULLET_H_INCLUDED
