#ifndef SHIPDEFENCE_H_INCLUDED
#define SHIPDEFENCE_H_INCLUDED

#include "ship.h"
#include "skin.h"
#include "multidrawer.h"

class ShipDefence : public Ship
{
    public:
        enum DefenceType{MainDefenseShip = 0, MiniDefenseShip, NumOfDefenceShipType};

        ShipDefence(DefenceType _type, sf::Vector2f position_, short team_, int hitPoint_, int attack_, int defense_, int skinIndex_, bool isSpawning_ = true);

        void Update(float dt);
        bool ResolveWallCollision(sf::FloatRect& wallRect, const float& dt);
        void AddToGridBase();
        void JoinGroup(UnitGroup* group_, bool asLeader = false);
        void GainKillPoints(const int& exp_, const float& special_, const float& powerFactor_);
        void GiveKillPoints(Ship* shooter);
        void GetDamagedBy(float damage, sf::Vector2f hitPosition, Ship* shooter = nullptr);
        void Shoot();
        DefenceType GetDefenceType();
        void RefreshEntities();

        int GetLevelBaseExp();
        int GetNextLevelBaseExp();

        void GetPoints(std::vector<sf::Vector2f>& points);
        sf::Vector2f GetPosition();
        sf::FloatRect GetBoundingRect();

        static void InitialiseMultiDrawer();
        static void Draw(sf::RenderTarget& target);

    private:
        Ship* lastBulletFromHigherUnits;
        float lastBulletFromHigherUnits_time;

        DefenceType defenceType;
        sf::Vertex vertices[4];
        sf::Transformable transformation;
        MultiDrawer* multiDrawer;
};

#endif // SHIPDEFENCE_H_INCLUDED
