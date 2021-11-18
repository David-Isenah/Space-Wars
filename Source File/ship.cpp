#include "bullet.h"
#include "shipunit.h"
#include "game.h"
#include "math.h"
#include "iostream"
#include "public_functions.h"
#include "gun_const.h"

const float SPAWN_TRANSPARENCY_TIME = 0.5f;
const float Ship::BAR_DURATION = 5.f;
const float Ship::HIT_EFFECT_DURATION = 0.08f;

sf::VertexArray va_ship_bar_lines;
sf::VertexArray va_ship_bar_quads;
std::vector<sf::Text> text_ship_bar_names;
sf::Text temp_text_name;
int ship_shared_exp[Game::NumOfTeams];
const int Ship::BaseLevelExpShared[4] = {1000, 2500, 4000, 7000};

Ship::Ship() :
    lastHitTime(0.f),
    lastShotTime(0.f),
    lastTargetUpdateTime(0.f),
    targetEntity(nullptr),
    skin(nullptr),
    currentLevel(1),
    exp(nullptr),
    specialMeter(nullptr),
    barTransparencyTime(0),
    spawnTransparencyTime(0),
    isTransmitingSpawn(true),
    health(0),
    hitPoint(0),
    attack(0),
    defense(0),
    speed(0),
    special(0),
    actualAttack(0),
    actualDefense(0),
    actualHitpoint(0),
    actualPowerScore(0),
    targetRange(0),
    fleeDistance(0),
    sizeRadius(0),
    dodgeRadius(0),
    barLength(0),
    barHeight(0),
    shouldDrawBarExp(false),
    hierarchy(-1),
    shipDensityValue(0),
    shipType(Ship::NoShipType),
    isRecruiting(true),
    group(nullptr),
    minBaseCoreAttackHealthFactor(0.f),
    reloadingTime(0.f),
    lastReloadTime(0.f),
    gunMagazine(0),
    gunAmmo(nullptr),
    reloading(false)
{
    minBaseCoreAttackHealthFactor = (50 + GenerateRandomNumber(50)) / 100.f;

    //Remove Later
    currentGun = Gun::LaserGun;
}

void Ship::SetTarget(Ship* target)
{
    targetEntity = target;
}

void Ship::AddSigthTarget(Ship* target)
{
}

void Ship::HandleGunReloading()
{
    if(gunMagazine < Gun::GUN_MAGAZINE_SIZE[currentGun])
    {
        if(reloading == false && (gunMagazine <= 0 || lastShotTime > 1.f) && (currentGun == Gun::LaserGun || (gunAmmo != nullptr && *gunAmmo >= 1)))
        {
            lastReloadTime = reloadingTime = Gun::GUN_RELOAD_TIME[currentGun] * (1 + 0.3f * GenerateRandomNumber(101) / 100.f);
            reloading = true;
        }

        if(reloading && (reloadingTime <= 0 || IsSpecialActive()))
        {
            reloading = false;

            if(currentGun == Gun::LaserGun || shipType == DefenceShip)
                gunMagazine = Gun::GUN_MAGAZINE_SIZE[currentGun];
            else if(gunAmmo != nullptr && *gunAmmo >= 1.f)
            {
                float ammoUsage = (Gun::GUN_MAGAZINE_SIZE[currentGun] - gunMagazine) * (ShipUnit::GetActivePlayerUnit() == this ? 1.f : Gun::GUN_AI_AMMO_USAGE[currentGun]);
                if(*gunAmmo < ammoUsage)
                {
                    gunMagazine += *gunAmmo / ammoUsage * (Gun::GUN_MAGAZINE_SIZE[currentGun] - gunMagazine);
                    *gunAmmo = 0;
                }
                else
                {
                    gunMagazine = Gun::GUN_MAGAZINE_SIZE[currentGun];
                    *gunAmmo -= ammoUsage;
                }
           }
        }
    }
    else reloading = false;
}

void Ship::SwitchGun(short gunType_, unsigned int magazine_, float* ammoRef_)
{
    if(gunType_ >= 0 && gunType_ < Gun::NumOfGuns)
    {
        currentGun = gunType_;
        gunMagazine = magazine_;
        gunAmmo = ammoRef_;
        reloading = false;
    }
}

short Ship::GetCurrentGun()
{
    return currentGun;
}

unsigned int Ship::GetGunMagazine()
{
    return gunMagazine;
}

float* Ship::GetGunAmmo()
{
    return gunAmmo;
}

float Ship::GetReloadingFactor()
{
    if(reloading)
        return (lastReloadTime > 0 ? 1.f - reloadingTime / lastReloadTime : 1.f);
    return 1;
}

bool Ship::IsReloading()
{
    return reloading;
}

float Ship::GetBaseCoreAttackHealthFactor()
{
    return minBaseCoreAttackHealthFactor;
}

short Ship::GetShipDensityValue()
{
    return shipDensityValue;
}

float Ship::GetSizeRadius()
{
    return sizeRadius;
}

 float Ship::GetDodgeRadius()
 {
     return dodgeRadius;
 }

Ship::ShipType Ship::GetShipType()
{
    return shipType;
}

Ship* Ship::GetShipTarget()
{
    return targetEntity;
}

float Ship::GetTargetRange()
{
    return targetRange;
}

void Ship::RefreshRealTimeProperties()
{
    actualAttack = attack * (1.f + (0.1f * (currentLevel - 1)));
    actualDefense = defense * (1.f + (0.1f * (currentLevel - 1)));
    actualHitpoint = hitPoint * (1.f + (0.1f * (currentLevel - 1)));
    actualPowerScore = actualAttack + actualDefense + actualHitpoint;
}

const float& Ship::GetActualAttack()
{
    return actualAttack;
}

const float& Ship::GetActualDefence()
{
    return actualDefense;
}

const float& Ship::GetActualHitpoint()
{
    return actualHitpoint;
}

const float& Ship::GetActualPowerScore()
{
    return actualPowerScore;
}

const short& Ship::GetCurrentLevel()
{
    return currentLevel;
}

int* Ship::GetExp()
{
    return exp;
}

float Ship::GetFleeDistance()
{
    return fleeDistance;
}

short Ship::GetHierarchy()
{
    return hierarchy;
}

float Ship::GetHealth()
{
    return health;
}

float Ship::GetHealthInFactors()
{
    return health / GetActualHitpoint();
}

float Ship::GetSpecialInFactors()
{
    return 0;
}

bool Ship::IsSpecialActive()
{
    return false;
}

float Ship::GetSpawnTransitionFactor(float dt)
{
    spawnTransparencyTime += dt;
    if(spawnTransparencyTime < 0)
        spawnTransparencyTime = 0;
    if(spawnTransparencyTime >= SPAWN_TRANSPARENCY_TIME)
    {
        isTransmitingSpawn = false;
        spawnTransparencyTime = SPAWN_TRANSPARENCY_TIME;
        return 1.f;
    }

    return spawnTransparencyTime / SPAWN_TRANSPARENCY_TIME;
}

void Ship::RefreshEntities()
{
    if(targetEntity != nullptr)
        if(targetEntity->GetAlive() == false)
            targetEntity = nullptr;
}

void Ship::LevelUp()
{
}

/*void Ship::Draw(sf::RenderTarget& target)
{
    if(lastHitTime > 0)
    {
        GetSprite().setColor(sf::Color(255, 150, 150, sprite.getColor().a));
    }
    else if(GetSprite().getColor() != sf::Color::White)
        GetSprite().setColor(sf::Color(255, 255, 255, sprite.getColor().a));

    target.draw(GetSprite());
}

void Ship::DrawHealth(sf::RenderTarget& target)
{
    if(lastHitTime > 0)
    {
        float alpha = 0.f;
        if(lastHitTime > 2.5f)
            alpha = (lastHitTime - 2.5f) / 0.5f;
        alpha = 1.f - alpha;

        sf::RectangleShape healthBar;
        healthBar.setSize(sf::Vector2f(hitPoint, 3.f));
        healthBar.setOutlineColor(sf::Color(0, 0, 0, 225 * alpha));
        healthBar.setOutlineThickness(1.f);
        healthBar.setFillColor(sf::Color(0, 0, 0, 5 * alpha));
        healthBar.setOrigin(healthBar.getSize().x / 2, healthBar.getSize().y / 2);
        healthBar.setPosition(GetPosition() + sf::Vector2f(0, -50));

        sf::RectangleShape healthRec;
        healthRec.setSize(sf::Vector2f(health, 3.f));
        if(health / hitPoint < 0.3)
            healthRec.setFillColor(sf::Color(220, 0, 30, 150 * alpha));
        else
            healthRec.setFillColor(sf::Color(0, 220, 30, 150 * alpha));
        healthRec.setOrigin(0, healthRec.getSize().y / 2);
        healthRec.setPosition(GetPosition() + sf::Vector2f(-healthBar.getSize().x  / 2, -50));

        float expLenght = *exp / 1 * hitPoint;

        sf::RectangleShape expBar;
        expBar.setSize(sf::Vector2f(expLenght, 3.f));
        expBar.setFillColor((sf::Color(200, 50, 50, 150 * alpha)));
        expBar.setOrigin(0, expBar.getSize().y / 2);
        expBar.setPosition(GetPosition() + sf::Vector2f(-healthBar.getSize().x  / 2, -45));

        target.draw(healthBar);
        target.draw(healthRec);
        target.draw(expBar);
    }
}*/

/* DONT REMOVE YET
bool Ship::ResolveEntityCollision(Entity* entity)
{
    if(entity != nullptr)
        if(Game::GetTeamUpType(entity->GetTeam()) != Game::GetTeamUpType(team) && entity->GetAlive())
            if(GetBoundingRect().intersects(entity->GetBoundingRect()))
                if(IsColliding(entity->GetPoints(), GetPoints()))
                    return true;
    return false;
}
*/

/*bool Ship::ResolveWallCollision(sf::FloatRect& wallRect)
{
    if(GetBoundingRect().intersects(wallRect))
    {
        collideObjectPoints.push_back(sf::Vector2f(wallRect.left + wallRect.width / 2.f, wallRect.top + wallRect.height / 2.f));
        collideObjectPoints.push_back(sf::Vector2f(wallRect.left, wallRect.top));
        collideObjectPoints.push_back(sf::Vector2f(wallRect.left + wallRect.width, wallRect.top));
        collideObjectPoints.push_back(sf::Vector2f(wallRect.left + wallRect.width, wallRect.top + wallRect.height));
        collideObjectPoints.push_back(sf::Vector2f(wallRect.left, wallRect.top + wallRect.height));

        sf::Vector2f pushVec(0, 0);
        GetPoints(collidePoints);
        if(IsColliding(collidePoints, collideObjectPoints, pushVec))
        {
            sprite.move(pushVec);
            return true;
        }
    }
    return false;
}*/

bool Ship::ResolveWallCollision(std::vector<sf::FloatRect>& wallRect, const float& dt)
{
    bool collided = false;
    for(int a = 0; a < wallRect.size(); a++)
        if(ResolveWallCollision(wallRect[a], dt))
            collided = true;

    return collided;
}

/*std::vector<sf::Vector2f> Ship::GetPoints()
{
    std::vector<sf::Vector2f> points;

    if(skin != nullptr)
    {
        sf::Transform transformation = sprite.getTransform();

        for(int a = 0; a < skin->points.size(); a++)
            points.push_back(transformation.transformPoint(skin->points[a]));
    }

    return points;
}*/

void Ship::AimAndShoot(float dt)
{
    if(targetEntity != nullptr)
    {
        if(Game::GetTeamUpType(targetEntity->GetTeam()) != Game::GetTeamUpType(team))
        {
            sf::Vector2f dist = targetEntity->GetPosition() - GetPosition();
            float distSqr = dist.x * dist.x + dist.y * dist.y;

            if(distSqr <= targetRange * targetRange)
                TransmitToAngle(90 + (atan2f(dist.y, dist.x) * 180 / 3.142f), dt);
            else targetEntity = nullptr;
        }
    }
    Shoot();
}

void Ship::JoinGroup(UnitGroup* group_, bool asLeader)
{
    if(group != nullptr)
    {
        group->LeaveGroup(this);
        group = nullptr;
    }

    if(group_ != nullptr)
    {
        if(asLeader)
        {
            if(group_->leader != nullptr)
                group_->leader->LeaveGroup();
            group_->leader = this;
        }
        else if(group_->members.size() < MAX_GROUP_MEMBERS)
            group_->AddMember(this);
        group = group_;
    }
}

void Ship::LeaveGroup()
{
    if(group != nullptr)
        group->LeaveGroup(this);
    group = nullptr;
}

Ship::UnitGroup* Ship::GetGroup()
{
    return group;
}

bool Ship::GetRecruitingState()
{
    return isRecruiting;
}

sf::Vector2f Ship::GetVelocity()
{
    return sf::Vector2f(0, 0);
}

bool Ship::IsMoving()
{
    return false;
}

void Ship::AddToGridBase()
{
}

void Ship::GetVelocityDetails(float& maxVelocity_, float& maxForce_)
{
    maxVelocity_ = 0;
    maxForce_ = 0;
}

void Ship::UpdateHealthBar()
{
    if(barTransparencyTime > 0.f)
    {
        if(va_ship_bar_lines.getPrimitiveType() != sf::Lines)
            va_ship_bar_lines.setPrimitiveType(sf::Lines);
        if(va_ship_bar_quads.getPrimitiveType() != sf::Quads)
            va_ship_bar_quads.setPrimitiveType(sf::Quads);

        float actualBarLength = barLength * (1.f + 0.1f * (currentLevel - 1));
        float healthLength = (IsSpecialActive() ? GetSpecialInFactors() : health / actualHitpoint) * actualBarLength;
        sf::Vector2f refPos = GetPosition() - sf::Vector2f(actualBarLength / 2.f, barHeight);
        sf::Color color = sf::Color::White;
        sf::Color quadColor = sf::Color::White;
        if(healthLength / actualBarLength <= 0.25f && IsSpecialActive() == false)
            quadColor = sf::Color(203, 101, 101);
        if(barTransparencyTime < 0.5f)
        {
            color.a = 255.f * barTransparencyTime / 0.5f;
            quadColor.a = color.a;
        }
        else if(barTransparencyTime > BAR_DURATION - 0.5f)
        {
            color.a = 255.f * (0.5f - (barTransparencyTime - (BAR_DURATION - 0.5f))) / 0.5f;
            quadColor.a = color.a;
        }

        //Drawing Lines
        if(shouldDrawBarExp && exp != nullptr && GetLevelBaseExp() >= 0 && GetNextLevelBaseExp() > 0)
        {
            float expLength = (float)(*exp - GetLevelBaseExp()) / (GetNextLevelBaseExp() - GetLevelBaseExp()) * actualBarLength;
            va_ship_bar_lines.append(sf::Vertex(refPos, color));
            va_ship_bar_lines.append(sf::Vertex(refPos + sf::Vector2f(expLength, 0.f), color));
        }

        va_ship_bar_lines.append(sf::Vertex(refPos + sf::Vector2f(0, -9), color));
        va_ship_bar_lines.append(sf::Vertex(refPos + sf::Vector2f(actualBarLength, -9), color));
        va_ship_bar_lines.append(sf::Vertex(refPos + sf::Vector2f(actualBarLength, -9), color));
        va_ship_bar_lines.append(sf::Vertex(refPos + sf::Vector2f(actualBarLength, -4), color));
        va_ship_bar_lines.append(sf::Vertex(refPos + sf::Vector2f(actualBarLength, -4), color));
        va_ship_bar_lines.append(sf::Vertex(refPos + sf::Vector2f(0, -4), color));
        va_ship_bar_lines.append(sf::Vertex(refPos + sf::Vector2f(0, -4), color));
        va_ship_bar_lines.append(sf::Vertex(refPos + sf::Vector2f(0, -9), color));

        //Drawing Quads
        if(IsSpecialActive())
        {
            va_ship_bar_quads.append(sf::Vertex(refPos + sf::Vector2f(0, -9), sf::Color(255, 255, 255, (float)quadColor.a * 0.5f)));
            va_ship_bar_quads.append(sf::Vertex(refPos + sf::Vector2f(GetHealthInFactors() * actualBarLength, -9), sf::Color(255, 255, 255, (float)quadColor.a * 0.5f)));
            va_ship_bar_quads.append(sf::Vertex(refPos + sf::Vector2f(GetHealthInFactors() * actualBarLength, -4), sf::Color(255, 255, 255, (float)quadColor.a * 0.5f)));
            va_ship_bar_quads.append(sf::Vertex(refPos + sf::Vector2f(0, -4), sf::Color(255, 255, 255, (float)quadColor.a * 0.5f)));
        }

        va_ship_bar_quads.append(sf::Vertex(refPos + sf::Vector2f(0, -9), quadColor));
        va_ship_bar_quads.append(sf::Vertex(refPos + sf::Vector2f(healthLength, -9), quadColor));
        va_ship_bar_quads.append(sf::Vertex(refPos + sf::Vector2f(healthLength, -4), quadColor));
        va_ship_bar_quads.append(sf::Vertex(refPos + sf::Vector2f(0, -4), quadColor));

        //Drawing Name
        if(barName.size() > 0)
        {
            if(temp_text_name.getFont() == nullptr)
            {
                temp_text_name.setFont(*Game::GetFont("Conthrax"));
                temp_text_name.setCharacterSize(14 * 2);
                temp_text_name.setScale(0.5f, 0.5f);
            }

            temp_text_name.setString(barName);
            temp_text_name.setFillColor(color);
            temp_text_name.setPosition(refPos - sf::Vector2f(0, 30));
            ShortenText(temp_text_name, actualBarLength, true);
            text_ship_bar_names.push_back(temp_text_name);
        }
    }
}

void Ship::DrawHealthBars(sf::RenderTarget& target_)
{
    target_.draw(va_ship_bar_lines);
    target_.draw(va_ship_bar_quads);
    for(auto& text_ : text_ship_bar_names)
        target_.draw(text_);

    va_ship_bar_lines.clear();
    va_ship_bar_quads.clear();
    text_ship_bar_names.clear();
}

int* Ship::GetSharedExp(short team_)
{
    if(team_ >= 0 && team_ < Game::NumOfTeams)
        return &ship_shared_exp[team_];
    return nullptr;
}

void Ship::GainKillPoints(const int& exp_, const float& special_, const float& powerFactor_)
{
    if(exp != nullptr)
        *exp += exp_;
    if(specialMeter != nullptr)
        *specialMeter += special_;
}

void Ship::GetDamagedBy(float damage, sf::Vector2f hitPosition, Ship* shooter)
{
    if(alive && damage > 0)
    {
        float actualDamage = (damage * damage) / (damage + GetActualDefence());

        health -= actualDamage;
        if(health <= 0)
        {
            health = 0;
            alive = false;
            LeaveGroup();
            GiveKillPoints(shooter);
        }

        if(shooter != nullptr && shooter == ShipUnit::GetActivePlayerUnit())
            static_cast<ShipUnit*>(shooter)->GetIdentity()->GetDamageDE().Add(actualDamage, hitPosition);
    }

    if(barTransparencyTime < 0.5f)
        barTransparencyTime = BAR_DURATION - barTransparencyTime;
    else if(barTransparencyTime < BAR_DURATION - 0.5f)
        barTransparencyTime = BAR_DURATION - 0.5f;

    lastHitTime = HIT_EFFECT_DURATION;
}

void Ship::TransmitToAngle(float angle_, float dt_)
{
    angle += (int)(-angle / 360.f) * 360;
    if(angle < 0)
        angle += 360;

    angle_ += (int)(-angle_ / 360.f) * 360;
    if(angle_ < 0)
        angle_ += 360;

    float entityAmgle[3];
    entityAmgle[0] = angle_;
    entityAmgle[1] = entityAmgle[0] + 360;
    entityAmgle[2] = entityAmgle[0] - 360;

    float tendToAngle = 0.f;
    float minDiff = 0.f;
    for(int a = 0; a < 3; a++)
    {
        float diff = entityAmgle[a] - angle;
        if(diff < 0)
            diff = -diff;

        if(a == 0 || diff < minDiff)
        {
            tendToAngle = entityAmgle[a];
            minDiff = diff;
        }
    }

    TendTowards(angle, tendToAngle, 0.1f, 0.5f, dt_);
}

float Ship::GetNonTransmittingAngle()
{
    return angle;
}
