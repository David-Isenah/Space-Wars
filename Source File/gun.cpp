#include "gun_shipgun.h"
#include "game.h"

std::vector<Ship*> temp_pulseSpreader_ships;
std::vector<sf::Vector2f> temp_pulseSpreader_pushVec;

Gun::ShipGuns* shipGuns[Gun::NumOfGuns] =
{
    new LaserGun_SG(),
    new EnhancedLaserGun_SG(),
    new PulseSpreaderGun_SG(),
    new DisintegratorGun_SG(),
    nullptr
};

bool Gun::Shoot(short gunType, sf::Vector2f shootPosition, sf::Vector2f shipVelocity, short team, float angle, float attack, float &lastShootTime, float bulletScale, unsigned int& gunMagazine, Ship* ship_)
{
    if(gunType >= 0 && gunType < Gun::NumOfGuns)
        return shipGuns[gunType]->Shoot(shootPosition, shipVelocity, team, angle, attack, lastShootTime, bulletScale, gunMagazine, ship_);
    return false;
}

bool PulseSpreaderGun_SG::Shoot(sf::Vector2f shootPosition, sf::Vector2f shipVelocity, short team, float angle, float attack, float &lastShootTime, float bulletScale, unsigned int& gunMagazine, Ship* ship_)
    {
        if(gunMagazine > 0 && lastShootTime >= fireRate * (ship_->IsSpecialActive() ? 0.6f : 1))
        {
            lastShootTime = 0;
            gunMagazine--;

            float velocityMagn = sqrt(shipVelocity.x * shipVelocity.x + shipVelocity.y * shipVelocity.y);
            float deflection = velocityMagn / AimVelocityFactor * angleDeflection * (ship_ == ShipUnit::GetActivePlayerUnit() && ShipUnit::IsActivePlayerManualAiming() ? 0.f : 1.f);
            float shootDamage = attack / AttackFactor * baseAttack;

            sf::Vector2f pos = ship_->GetPosition();
            const sf::Vector2f& dir = shootPosition - ship_->GetPosition();
            sf::Vector2f dirUnitVec = NormalizeVector(dir);
            float refPosRatio = (GenerateRandomNumber(201) - 100) / 100.f;
            sf::Vector2f refPos = shootPosition + sf::Vector2f(dirUnitVec.y, -dirUnitVec.x) * (refPosRatio * POSITION_REF_LENGTH); //- dirUnitVec * ((GenerateRandomNumber(201) - 100) / 100.f * 10.f);
            float shootAngle = angle + (accuracy + deflection) * (-1 - refPosRatio + GenerateRandomNumber(101) / 100.f * 2.f) / 2.f;

            float lineLength = 0.f;
            temp_pulseSpreader_ships.clear();
            temp_pulseSpreader_pushVec.clear();

            /*if(temp_globalPoints.size() != 5)
                temp_globalPoints.resize(5);*/

            sf::Transformable transformable;
            transformable.setRotation(shootAngle);

            sf::Vector2f bulletPathVec = transformable.getTransform().transformPoint(sf::Vector2f(0, -LINE_LENGTH));

            if(EntityManager::isSightOnWall(refPos, refPos + bulletPathVec, nullptr, &lineLength, true, LINE_WIDTH, &temp_pulseSpreader_ships, &temp_pulseSpreader_pushVec, -1, Game::GetTeamUpType(ship_->GetTeam()), false))
                ;
            else lineLength = LINE_LENGTH;

            std::vector<sf::Vector2f> collidePoints;
            collidePoints.push_back(sf::Vector2f(0, -lineLength / 2.f));
            collidePoints.push_back(sf::Vector2f(-LINE_WIDTH / 2.f, 0));
            collidePoints.push_back(sf::Vector2f(-LINE_WIDTH / 2.f, -lineLength));
            collidePoints.push_back(sf::Vector2f(LINE_WIDTH / 2.f, -lineLength));
            collidePoints.push_back(sf::Vector2f(LINE_WIDTH / 2.f, 0));

            transformable.setPosition(refPos);
            for(int c = 0; c < collidePoints.size(); c++)
                collidePoints[c] = transformable.getTransform().transformPoint(collidePoints[c]);

            sf::FloatRect boundRect;
            {
                float minX = collidePoints[0].x;
                float maxX = collidePoints[0].x;
                float minY = collidePoints[0].y;
                float maxY = collidePoints[0].y;

                for(int c = 1; c < collidePoints.size(); c++)
                {
                    if(collidePoints[c].x < minX)
                        minX = collidePoints[c].x;
                    else if(collidePoints[c].x > maxX)
                        maxX = collidePoints[c].x;

                    if(collidePoints[c].y < minY)
                        minY = collidePoints[c].y;
                    else if(collidePoints[c].y > maxY)
                        maxY = collidePoints[c].y;
                }
                boundRect.left = minX;
                boundRect.top = minY;
                boundRect.width = maxX - minX;
                boundRect.height = maxY - minY;
            }

            sf::Vector2f tempHitPos;
            sf::Vector2f tempMoveVec;
            sf::Vector2f baseCoreBulletPathVec = collidePoints[2] - collidePoints[1];
            const std::vector<BaseCore*>& enemyBaseCores = EntityManager::GetEnemyBaseCore(ship_->GetTeam());
            for(auto& bc : enemyBaseCores)
                if(bc->GetAlive() && boundRect.intersects(bc->GetCoreBoundingRect()) && IsColliding(collidePoints, bc->GetPosition(), BaseCore::COLLISION_RADIUS, baseCoreBulletPathVec, tempMoveVec))
                {
                    tempHitPos = refPos + baseCoreBulletPathVec + tempMoveVec;//bc->GetPosition();
                    DisplayEffect::AddDisplayEffect(new DE_Hit(tempHitPos, 0.25f * 0.5f, Bullet::LaserBullet, bc));
                    bc->GetDamagedBy(shootDamage, tempHitPos + sf::Vector2f(GenerateRandomNumber(101) / 100.f, GenerateRandomNumber(101) / 100.f) * 5.f, ship_);
                }

            for(int c = 0; c < temp_pulseSpreader_ships.size(); c++)
            {
                tempHitPos = refPos + bulletPathVec + temp_pulseSpreader_pushVec[c];
                DisplayEffect::AddDisplayEffect(new DE_Hit(tempHitPos, 0.25f * 0.5f, Bullet::LaserBullet, temp_pulseSpreader_ships[c]));
                temp_pulseSpreader_ships[c]->GetDamagedBy(shootDamage, tempHitPos + (float)GenerateRandomNumber(101) / 100.f * 5.f * sf::Vector2f(1.f, 1.f), ship_);
            }

            DisplayEffect::AddDisplayEffect(new DE_PulseSpreaderBullet(transformable.getTransform(), lineLength, LINE_WIDTH));
            //DisplayEffect::AddDisplayEffect(new DE_Hit(refPos + dirUnitVec * lineLength, 0.25f * 0.5f, Bullet::LaserBullet, ship_));
            return true;
        }
        return false;
    }
