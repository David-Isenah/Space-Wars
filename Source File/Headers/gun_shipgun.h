#ifndef GUN_SHIPGUN_H_INCLUDED
#define GUN_SHIPGUN_H_INCLUDED

#include "gun.h"
#include "entitymanager.h"
#include "public_functions.h"
#include "displayeffect_effects.h"
#include "shipunit.h"
#include <iostream>

float AimVelocityFactor = 200;
float AttackFactor = 2000;

class LaserGun_SG : public Gun::ShipGuns
{
protected:
    short bulletType;
public:
    LaserGun_SG() :
        bulletType(Bullet::LaserBullet)
    {
        baseAttack = 400;
        accuracy = 4;
        fireRate = 0.3f;
        angleDeflection = 10;
    }

    bool Shoot(sf::Vector2f shootPosition, sf::Vector2f shipVelocity, short team, float angle, float attack, float &lastShootTime, float bulletScale, unsigned int& gunMagazine, Ship* ship_ = nullptr)
    {
        if(gunMagazine > 0 && lastShootTime >= fireRate  * (ship_->IsSpecialActive() ? 0.6 : 1))
        {
            lastShootTime = 0;
            gunMagazine--;

            float velocityMagn = sqrt(shipVelocity.x * shipVelocity.x + shipVelocity.y * shipVelocity.y);
            float deflection = velocityMagn / AimVelocityFactor * angleDeflection * (ship_ == ShipUnit::GetActivePlayerUnit() && ShipUnit::IsActivePlayerManualAiming() ? 0.f : 1.f);
            float shootDamage = attack / AttackFactor * baseAttack;
            float shootAngle = GenerateRandomNumber(101) / 100.f * (accuracy + deflection);

            shootAngle = angle - (accuracy + deflection) / 2 + shootAngle;

            EntityManager::CreateBullet(new Bullet(shootPosition, shootAngle, bulletType, team, shootDamage, bulletScale, ship_));
            DisplayEffect::AddDisplayEffect(new DE_Shoot(shootPosition, shootAngle, bulletScale * 0.6f, Bullet::LaserBullet, ship_));
            return true;
        }
        return false;
    }
};

class EnhancedLaserGun_SG : public LaserGun_SG
{
public:
    EnhancedLaserGun_SG()
    {
        fireRate = 0.15f;
        accuracy = 6.f;
        bulletType = Bullet::EnhancedLaserBullet;
    }
};

class DisintegratorGun_SG : public Gun::ShipGuns
{
public:
    const int BULLETS_PER_SHOT = 10;
    const int POSITION_REF_LENGTH = 40;

    DisintegratorGun_SG()
    {
        baseAttack = 400;
        accuracy = 8.f;
        fireRate = 0.7f;
        angleDeflection = 8;
    }

    bool Shoot(sf::Vector2f shootPosition, sf::Vector2f shipVelocity, short team, float angle, float attack, float &lastShootTime, float bulletScale, unsigned int& gunMagazine, Ship* ship_ = nullptr)
    {
        if(gunMagazine > 0 && lastShootTime >= fireRate * (ship_->IsSpecialActive() ? 0.6f : 1))
        {
            lastShootTime = 0;

            float velocityMagn = sqrt(shipVelocity.x * shipVelocity.x + shipVelocity.y * shipVelocity.y);
            float deflection = velocityMagn / AimVelocityFactor * angleDeflection;//* (ship_ == ShipUnit::GetActivePlayerUnit() && ShipUnit::IsActivePlayerManualAiming() ? 0.f : 1.f);
            float shootDamage = attack / AttackFactor * baseAttack;
            float shootAngle = 0.f;

            int numOfShotBullets = 0;
            if(gunMagazine > BULLETS_PER_SHOT)
            {
                numOfShotBullets = BULLETS_PER_SHOT;
                gunMagazine -= BULLETS_PER_SHOT;
            }
            else
            {
                numOfShotBullets = gunMagazine;
                gunMagazine = 0;
            }

            const sf::Vector2f& dir = shootPosition - ship_->GetPosition();
            sf::Vector2f dirUnitVec = NormalizeVector(dir);
            float refPosRatio = 0;
            sf::Vector2f refPos;

            for(int a = 0; a < numOfShotBullets; a++)
            {
                refPosRatio = (GenerateRandomNumber(201) - 100) / 100.f;
                refPos = shootPosition + sf::Vector2f(dirUnitVec.y, -dirUnitVec.x) * (refPosRatio * POSITION_REF_LENGTH) - dirUnitVec * ((GenerateRandomNumber(201) - 100) / 100.f * 40.f);

                shootAngle = angle + (accuracy + deflection) * (-1 - refPosRatio + GenerateRandomNumber(101) / 100.f * 2.f) / 2.f;
                EntityManager::CreateBullet(new Bullet(refPos, shootAngle, Bullet::DisintegratorBullet, team, shootDamage, bulletScale, ship_));
            }

            DisplayEffect::AddDisplayEffect(new DE_Shoot(shootPosition, shootAngle, bulletScale * 0.8f, Bullet::LaserBullet, ship_));
            return true;
        }
        return false;
    }
};

//Pulse Spreader
class PulseSpreaderGun_SG : public Gun::ShipGuns
{
public:
    const int POSITION_REF_LENGTH = 50;
    const int LINE_LENGTH = 3000;
    const int LINE_WIDTH = 10;

    PulseSpreaderGun_SG()
    {
        baseAttack = 300;
        accuracy = 3;
        fireRate = 0.1f;
        angleDeflection = 7.f;
    }

    void ShootShipsInLine(sf::Vector2f position, float angle, float damage, float& lineLength);

    bool Shoot(sf::Vector2f shootPosition, sf::Vector2f shipVelocity, short team, float angle, float attack, float &lastShootTime, float bulletScale, unsigned int& gunMagazine, Ship* ship_ = nullptr);

};

#endif // GUN_SHIPGUN_H_INCLUDED
