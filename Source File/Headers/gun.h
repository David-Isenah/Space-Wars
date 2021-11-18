#ifndef GUN_H_INCLUDED
#define GUN_H_INCLUDED

#include "SFML/System/Vector2.hpp"
#include "entity.h"

class Ship;

namespace Gun
{
    enum {NoGun = -1, LaserGun, EnhancedLaserGun, PulseSpreaderGun, DisintegratorGun, ThermalBlaster, ElectronGun, NumOfGuns};

    bool Shoot(short gunType, sf::Vector2f shootPosition, sf::Vector2f shipVelocity, short team, float angle, float attack, float &lastShootTime, float bulletScale, unsigned int& gunMagazine, Ship* ship_ = nullptr);

    class ShipGuns
    {
    protected:
        int baseAttack;
        int accuracy;
        int angleDeflection;
        float fireRate;

    public:
        ShipGuns() :
            baseAttack(0),
            accuracy(0),
            fireRate(0),
            angleDeflection(0)
        {
        }
        virtual ~ShipGuns()
        {
        }
        virtual bool Shoot(sf::Vector2f shootPosition, sf::Vector2f shipVelocity, short team, float angle, float attack, float &lastShootTime, float bulletScale, unsigned int& gunMagazine, Ship* ship_ = nullptr) = 0;
    };
}

#endif // GUN_H_INCLUDED
