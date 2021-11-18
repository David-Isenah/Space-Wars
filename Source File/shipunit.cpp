#include "shipunit.h"
#include "bullet.h"
#include "AssetManager.h"
#include "public_functions.h"
#include "astarpathfinding.h"
#include "maptilemanager.h"
#include "skin.h"
#include "game.h"
#include "displayeffect_effects.h"
#include "iostream"
#include "random"

const float AIM_VELOCITY_FACTOR = 0.5f;
const float UNIT_MAX_VELOCITY = 500.f;

const float TARGET_CHASE_TIME = 10.f;

const int TARGET_RANGE_MAIN_SUB_SHIP = 1300;
const int TARGET_RANGE_MINI_SHIP = 700;
const int TARGET_RANGE_MICRO_SHIP = 600;

const float MOVE_REACHED_DISTANCE = 15.f;
const float MOVE_DIR_DISTANCE = 50.f;
const float MOVE_RADUIS = 15.f;
const float MOVE_MAX_PREDICTION = 40.f;
const float BULLET_SCALE_DEFAULT = 0.25f;
const float BULLET_SCALE_MINI_SHIP = 0.2f;
const float BULLET_SCALE_MICRO_SHIP = 0.15f;
const float FLEE_EFFECTIVE_DISTANCE = 5.f;
const int MAX_FLEE_UNITS = 10;
const int MIN_FLEE_UNITS = 3;
const int MAX_SIGHT_UNITS = 8;
const float FG_DISTANCE = 880;
const float FG_DISTANCE_NO_TARGET = 200;
const float PATH_FIND_DELAY = 0.5f;
const float FOLLOWGROUP_DELAY_MIN = 0.25f;
const float FOLLOWGROUP_DELAY_MAX = 1.f;
const float RECOVERY_PER_SEC = 0.04f;
const float RECOVERY_PER_SEC_SMALLUNIT = 0.1f;
const float MIN_COLLISION_VELOCITY = 150.f;
const float SURROUNDING_TROOP_SPAWN_TIME = 5.f;
const float SURROUNDING_TROOP_BUILD_TIME = 2.f;
const float SURROUNDING_TROOP_FAVOUR_TIME = 25.f;
const float SURROUNDING_TROOP_FAVOUR_TIME_CHANGE = 5.f;
const int SURROUNDING_TROOP_MAX = 20;

const float DODGE_DURATION = 5.f;

ShipUnit* activePlayerUnit = nullptr;
bool isActivePlayerAiming = false;
bool isActivePlayerManualAiming = false;
bool isActivePlayerJustShooting = false;
bool blockManualAimForGuiObject = false;
float activePlayerAimDistMagn = 0.f;

const int ShipUnit::BaseLevelExpHigherUnits[4] = {1000, 2500, 4000, 7000};
int shipUnitBarLength[ShipUnit::NumOfUnitType] = {150, 150, 100, 60, 40};

std::vector<MultiDrawer> multidrawer_shipunit_miniship[Game::NumOfTeams];
std::vector<MultiDrawer> multidrawer_shipunit_microship[Game::NumOfTeams];

std::vector<ShipUnit*> shipUnitSpritesToDraw;

//Optimization Variables
std::vector<sf::Vector2i> temp_closestGridPath;
std::vector<sf::Vector2i> temp_GridPath;
std::vector<sf::Vector2f> temp_collideWallPoints_su;
std::vector<sf::Vector2f> temp_collidePoints_su;
std::vector<Ship*> temp_ships_su;
std::vector<int> temp_thinking_id;

//Ship Unit
void ShipUnit::InitialiseMultiDrawer()
{
    //Resize
    for(int a = 0; a < Game::NumOfTeams; a++)
        multidrawer_shipunit_miniship[a].resize(Game::GetSkinSize(Game::SkinMiniShip, a));

    for(int a = 0; a < Game::NumOfTeams; a++)
        multidrawer_shipunit_microship[a].resize(Game::GetSkinSize(Game::SkinMicroShip, a));

    //Setting Texture
    for(int a = 0; a < Game::NumOfTeams; a++)
        for(int b = 0; b < Game::GetSkinSize(Game::SkinMiniShip, a); b++)
            multidrawer_shipunit_miniship[a][b].SetTexture(&Game::GetSkin(Game::SkinMiniShip, b, a)->texture);

    for(int a = 0; a < Game::NumOfTeams; a++)
        for(int b = 0; b < Game::GetSkinSize(Game::SkinMicroShip, a); b++)
            multidrawer_shipunit_microship[a][b].SetTexture(&Game::GetSkin(Game::SkinMicroShip, b, a)->texture);
}

ShipUnit::ShipUnit(UnitType _type, sf::Vector2f position_, short team_, int hitPoint_, int attack_, int defense_, int speed_, int special_, int skinIndex_, short skinType_, bool isSpawning_, ShipIdentity* identity_) :
    unitType(_type),
    velocity(sf::Vector2f(0.f, 0.f)),
    isSpecialActive(false),
    isAngleLocked(true),
    nonTransmittingAngle(0.f),
    attackStartPoint(sf::Vector2f(0.f, 0.f)),
    maxVelocity(speed_ / 100.f * UNIT_MAX_VELOCITY),
    maxForce(maxVelocity * 2.f),
    sightShipChaseTime(0.f),
    numOfFleeingUnits(0),
    surroundingTroops(SURROUNDING_TROOP_MAX),
    surroundTroopsBuildTime(0.f),
    surroundTroopsFavourTime(SURROUNDING_TROOP_FAVOUR_TIME + SURROUNDING_TROOP_FAVOUR_TIME_CHANGE * GenerateRandomNumber(101) / 100.f),
    lastSurroudingTroopSpawn(0.f),
    isMultiDrawerBasedOn(false),
    isCloseLeader(false),
    attackTargetState(true),
    realTimeMaxForce(0),
    realTimeMaxVelocity(0),
    fleeReductionTime(0.f),
    dodgeTime(0.f),
    dodgeAllowance(0.f),
    pathFindDelay(0.f),
    primaryPathIndex(-1),
    moveLineIndex(0),
    moveLineIndexFixTime(0),
    moveLineIndexFixCount(0),
    isThinkingFindingPath(false),
    thinking_searchType(AStar::NoSearchType),
    thinkingWaitDuration(0.f),
    defending_base(0),
    defending_avoidEnemyBases(false),
    defending_duration(0),
    attacking_base(0),
    attacking_tillDeath(false),
    reclaimBase(-1),
    followGroupCloseFactor(0),
    followGroupDelay(0.f),
    traveling_updateTime(0.f),
    isCurentlyUsingAstarProgression(false),
    identity(identity_),
    lastBulletFromHigherUnits(nullptr),
    lastBulletFromHigherUnits_time(0),
    specialScalingFactor(0),
    specialEmitRadiation(0)
{
    team = team_;
    skin = Game::GetSkin((Game::SkinType)skinType_, skinIndex_, team_);
    shipType = UnitShip;
    state = Idle;
    exp = unitType >= MiniShip ? GetSharedExp(team_) : (identity_ != nullptr ? identity_->GetExp() : nullptr);
    shouldDrawBarExp = unitType < MiniShip ? true : false;
    barName = unitType < MiniShip && identity_ != nullptr ? identity_->GetShipInfo()->name : "";
    specialMeter = identity_ != nullptr ? &identity_->GetSpecialMeter() : nullptr;
    isTransmitingSpawn = isSpawning_;
    hierarchy = 4 - _type;
    barLength = shipUnitBarLength[_type];
    minAttackHealthFactor = (10 + GenerateRandomNumber(30)) / 100.f;
    minAttackGroupMembers = 2 + GenerateRandomNumber(MAX_GROUP_MEMBERS - 2 + 1);
    followGroupCloseFactor = (GenerateRandomNumber(2) ? (GenerateRandomNumber(71) + 10) / 100.f * 1.f : (GenerateRandomNumber(71) + 20) / 100.f * -1.f);

    hitPoint = hitPoint_;
    attack = attack_;
    defense = defense_;
    speed = speed_;
    special = special_;

    float t = maxVelocity / maxForce;
    slowingDistance = 0.5f * maxForce * t * t;

    lastSurroudingTroopSpawn = SURROUNDING_TROOP_SPAWN_TIME + 2.f * GenerateRandomNumber(101) / 100.f;

    if(_type == MiniShip)
        targetRange = TARGET_RANGE_MINI_SHIP;
    else if(_type == MicroShip)
        targetRange = TARGET_RANGE_MICRO_SHIP;
    else
        targetRange = TARGET_RANGE_MAIN_SUB_SHIP;

    if(unitType == MiniShip || unitType == MicroShip)
    {
        isMultiDrawerBasedOn = true;
        shipDensityValue = 1;
    }
    else
    {
        isMultiDrawerBasedOn = false;
        shipDensityValue = 4;
    }

    if(isSpawning_)
        angle = GenerateRandomNumber(360);

    if(hierarchy > 1)
        JoinGroup(UnitGroup::CreatUnitGroups(), this);

    if(skin != nullptr && skin->points.size() > 0)
    {
        if(isMultiDrawerBasedOn)
        {
            basedOnProperty.multiDrawerProperties = new MultiDrawerBasedOnProperty();

            if(_type == MiniShip)
                basedOnProperty.multiDrawerProperties->multiDrawer = &multidrawer_shipunit_miniship[team_][skinIndex_];
            else basedOnProperty.multiDrawerProperties->multiDrawer = &multidrawer_shipunit_microship[team_][skinIndex_];

            basedOnProperty.multiDrawerProperties->transformation.setPosition(position_);
            basedOnProperty.multiDrawerProperties->transformation.setOrigin(skin->points[0]);
            basedOnProperty.multiDrawerProperties->transformation.setScale(0.5f, 0.5f);

            sf::Vector2f textrSize;
            if(basedOnProperty.multiDrawerProperties->multiDrawer->GetTexture() != nullptr)
                textrSize = (sf::Vector2f)basedOnProperty.multiDrawerProperties->multiDrawer->GetTexture()->getSize();

            basedOnProperty.multiDrawerProperties->vertices[0].texCoords = sf::Vector2f(0, 0);
            basedOnProperty.multiDrawerProperties->vertices[1].texCoords = sf::Vector2f(textrSize.x, 0);
            basedOnProperty.multiDrawerProperties->vertices[2].texCoords = sf::Vector2f(textrSize.x, textrSize.y);
            basedOnProperty.multiDrawerProperties->vertices[3].texCoords = sf::Vector2f(0, textrSize.y);
        }
        else
        {
            basedOnProperty.sprite = new sf::Sprite(skin->texture, sf::IntRect(0, 0, 180, 180));
            basedOnProperty.sprite->setPosition(position_);
            basedOnProperty.sprite->setOrigin(skin->points[0]);
            basedOnProperty.sprite->setScale(0.5f, 0.5f);
        }

        if(skin != nullptr)
        {
            sizeRadius = skin->collisionRect.width;
            if(skin->collisionRect.height > skin->collisionRect.width)
                sizeRadius = skin->collisionRect.height;

            sizeRadius *= 0.5f * 0.5f;
        }
        dodgeRadius = sizeRadius;
        barHeight = sizeRadius + 10.f;

        fleeDistance = skin->collisionRect.width;
        if(skin->collisionRect.height < skin->collisionRect.width)
            fleeDistance = skin->collisionRect.height;
        fleeDistance *= 0.5f * 0.6f;
        fleeDistance -= FLEE_EFFECTIVE_DISTANCE;
    }

    if(exp != nullptr)
    {
        currentLevel = 1;
        for(int a = 3; a >= 0; a--)
            if(*exp >= (unitType < MiniShip ? BaseLevelExpHigherUnits[a] : BaseLevelExpShared[a]))
            {
                currentLevel += a + 1;
                break;
            }
    }

    if(exp != nullptr)
    {
        currentLevel = 1;
        for(int a = 3; a >= 0; a--)
            if(*exp >= (unitType < MiniShip ? BaseLevelExpHigherUnits[a] : BaseLevelExpShared[a]))
            {
                currentLevel += a + 1;
                break;
            }
        if(currentLevel == 5)
            *exp = unitType < MiniShip ? BaseLevelExpHigherUnits[3] : BaseLevelExpShared[3];
    }
    RefreshRealTimeProperties();
    health = GetActualHitpoint();
}

ShipUnit::~ShipUnit()
{
    if(basedOnProperty.multiDrawerProperties != nullptr)
        delete basedOnProperty.multiDrawerProperties;
    if(basedOnProperty.sprite != nullptr)
        delete basedOnProperty.sprite;

    if(group != nullptr)
        group->LeaveGroup(this);

    if(activePlayerUnit == this)
    {
        activePlayerUnit = nullptr;
        isActivePlayerAiming = false;
        isActivePlayerManualAiming = false;
        isActivePlayerJustShooting = false;
    }
}

void ShipUnit::SetActivePlayerUnit(ShipUnit* ship_)
{
    //if(ship_ != nullptr ? ship_->GetUnitType() == PlayerShip : true)
        activePlayerUnit = ship_;
}

ShipUnit* ShipUnit::GetActivePlayerUnit()
{
    return activePlayerUnit;
}

ShipIdentity* ShipUnit::GetIdentity()
{
    return identity;
}

float ShipUnit::GetSpecialInFactors()
{
    if(specialMeter != nullptr)
        return *specialMeter / 100.f;
    return 0;
}

void ShipUnit::GetDamagedBy(float damage, sf::Vector2f hitPosition, Ship* shooter)
{
    if(shooter != nullptr && shooter->GetShipType() == UnitShip && static_cast<ShipUnit*>(shooter)->GetUnitType() < MiniShip)
    {
        lastBulletFromHigherUnits = shooter;
        lastBulletFromHigherUnits_time = 3.f;
    }

    if(alive && damage > 0)
    {
        float actualDamage = (damage * damage) / (damage + actualDefense);
        if(isSpecialActive)
            *specialMeter -= actualDamage / actualHitpoint * 100;
        else health -= actualDamage;

        if(identity != nullptr)
            identity->GetDamageDE().Add(actualDamage, hitPosition);
        else if(shooter != nullptr && shooter == ShipUnit::GetActivePlayerUnit() && shooter->GetShipType() == Ship::UnitShip)
        {
            ShipUnit* shipUnit = static_cast<ShipUnit*>(shooter);
            if(shipUnit->identity != nullptr)
                shipUnit->GetIdentity()->GetDamageDE().Add(actualDamage, hitPosition);
        }

        if(health <= 0)
        {
            health = 0;
            alive = false;
            LeaveGroup();
            GiveKillPoints(shooter);
        }
    }

    if(barTransparencyTime < 0.5f)
        barTransparencyTime = BAR_DURATION - barTransparencyTime;
    else if(barTransparencyTime < BAR_DURATION - 0.5f)
        barTransparencyTime = BAR_DURATION - 0.5f;

    lastHitTime = HIT_EFFECT_DURATION;
}

void ShipUnit::GainKillPoints(const int& exp_, const float& special_, const float& powerFactor_)
{
    if(exp != nullptr && currentLevel < 5)
        *exp += exp_;
    if(specialMeter != nullptr && isSpecialActive == false)
        *specialMeter += special_;
    if(identity != nullptr)
        identity->RestockGuns(powerFactor_);
}

void ShipUnit::GiveKillPoints(Ship* shooter)
{
    if(shooter != nullptr)
    {
        float gainedExp = 0.f;
        float gainedSpecial = 0.f;
        float powerFactor = actualPowerScore / shooter->GetActualPowerScore();

        float gainedExpShared = 0.f;
        float powerFactorShared = Game::GetDifficultyInfo().miniShip * 3 / shooter->GetActualPowerScore();

        if(unitType == ShipUnit::MicroShip)
        {
            gainedExp = 15.f * (powerFactor / 0.3f);
            gainedExpShared = 15.f * (powerFactorShared / 0.3f);
            gainedSpecial = 2.f * (powerFactor / 0.3f);
        }
        else if(unitType == ShipUnit::MiniShip)
        {
            gainedExp = 30.f * (powerFactor / 0.3f);
            gainedExpShared = 30.f * (powerFactorShared / 0.3f);
            gainedSpecial = 2.f * (powerFactor / 0.3f);
        }
        else
        {
            gainedExp = 300.f * powerFactor;
            gainedExpShared = 300.f * powerFactorShared;
            gainedSpecial = 15.f * powerFactor;
        }

        if(gainedExp < 1)
            gainedExp = 1;
        if(gainedExpShared < 1)
            gainedExpShared = 1;

        if(lastBulletFromHigherUnits != nullptr)
            lastBulletFromHigherUnits->GainKillPoints(gainedExp, gainedSpecial, powerFactor);
        if(shooter != nullptr)
            *GetSharedExp(shooter->GetTeam()) += gainedExpShared;
    }
}

int ShipUnit::GetLevelBaseExp()
{
    if(currentLevel >= 2 && currentLevel <= 5)
        return unitType < MiniShip ? BaseLevelExpHigherUnits[currentLevel - 2] : BaseLevelExpShared[currentLevel - 2];
    return 0;
}

int ShipUnit::GetNextLevelBaseExp()
{
    if(currentLevel >= 1 && currentLevel < 5)
        return unitType < MiniShip ? BaseLevelExpHigherUnits[currentLevel - 1] : BaseLevelExpShared[currentLevel - 1];
    return 0;
}

void ShipUnit::RefreshRealTimeProperties()
{
    Ship::RefreshRealTimeProperties();
    if(isSpecialActive)
    {
        actualDefense *= 2.f;
        //actualAttack *= 1.5f;
    }
}

bool ShipUnit::IsSpecialActive()
{
    return isSpecialActive;
}

sf::Vector2f ShipUnit::GetPosition()
{
    if(isMultiDrawerBasedOn)
        return basedOnProperty.multiDrawerProperties->transformation.getPosition();
    else return basedOnProperty.sprite->getPosition();
}

void ShipUnit::GetPoints(std::vector<sf::Vector2f>& points)
{
    if(skin != nullptr)
     {
         points.clear();

         sf::Transform transform_;
         if(isMultiDrawerBasedOn)
            transform_ = basedOnProperty.multiDrawerProperties->transformation.getTransform();
         else transform_ = basedOnProperty.sprite->getTransform();

         for(int a = 0; a < skin->points.size(); a++)
            points.push_back(transform_.transformPoint(skin->points[a]));
     }
}

sf::FloatRect ShipUnit::GetBoundingRect()
{
    if(skin != nullptr)
    {
        if(isMultiDrawerBasedOn)
            return basedOnProperty.multiDrawerProperties->transformation.getTransform().transformRect(skin->collisionRect);
        else return basedOnProperty.sprite->getTransform().transformRect(skin->collisionRect);
    }
    return sf::FloatRect();
}

bool ShipUnit::GetReclaimingStatus()
{
    if(reclaimBase >= 0)
        return true;
    return false;
}

bool ShipUnit::ResolveWallCollision(sf::FloatRect& wallRect, const float& dt)
{
    if(GetBoundingRect().intersects(wallRect))
    {
        if(temp_collideWallPoints_su.size() != 5)
            temp_collideWallPoints_su.resize(5);

        temp_collideWallPoints_su[0] = sf::Vector2f(wallRect.left + wallRect.width / 2.f, wallRect.top + wallRect.height / 2.f);
        temp_collideWallPoints_su[1] = sf::Vector2f(wallRect.left, wallRect.top);
        temp_collideWallPoints_su[2] = sf::Vector2f(wallRect.left + wallRect.width, wallRect.top);
        temp_collideWallPoints_su[3] = sf::Vector2f(wallRect.left + wallRect.width, wallRect.top + wallRect.height);
        temp_collideWallPoints_su[4] = sf::Vector2f(wallRect.left, wallRect.top + wallRect.height);

        sf::Vector2f moveVec;
        if(unitType < MiniShip && specialScalingFactor > 0)
        {
            float prvScale = basedOnProperty.sprite->getScale().x;
            basedOnProperty.sprite->setScale(0.5f, 0.5f);
            GetPoints(temp_collidePoints_su);
            basedOnProperty.sprite->setScale(prvScale, prvScale);
        }
        else GetPoints(temp_collidePoints_su);

        if(IsColliding(temp_collidePoints_su, temp_collideWallPoints_su, moveVec))
        {
            if(isMultiDrawerBasedOn)
                basedOnProperty.multiDrawerProperties->transformation.move(moveVec);
            else basedOnProperty.sprite->move(moveVec);

            //std::cout << MagnitudeOfVector(moveVec) << "\n";
            float velMagn = MagnitudeOfVector(velocity);
            if(velMagn > MIN_COLLISION_VELOCITY)
            {
                float moveVecMagn = MagnitudeOfVector(moveVec);
                if(moveVecMagn > 0)
                    {
                        sf::Vector2f unitVec = velocity / velMagn;
                        velMagn -= moveVecMagn * 4.5f;
                        if(velMagn < MIN_COLLISION_VELOCITY)
                            velMagn = MIN_COLLISION_VELOCITY;
                        velocity = unitVec * velMagn;
                    }
            }
        }
    }
}

void ShipUnit::AddToGridBase()
{
    const GridInfo& gridInfo = GetGridInfo(GetGridPosition(GetPosition()));
    int baseId = gridInfo.baseId;
    if(baseId >= 0 && baseId < Game::GetNumberOfBases())
        Game::GetBase(baseId)->AddUnit(this);

    //if(activePlayerUnit == this)
        //std::cout << state << " p state\n";

    //Updating other base related information, EntityManager updates this before doing ship updates
    if(state != FollowingGroup && baseId >= 0)
    {
        Base* base = Game::GetBase(baseId);

        int density = shipDensityValue;
        if(group != nullptr && group->leader == this)
            for(auto& member : group->members)
                density += member->GetShipDensityValue();

        if(Game::GetTeamUpType(base->GetTeam()) == Game::GetTeamUpType(team))
            base->AddDensityWantingToDefend(density);
        else base->AddDensityWantingToAttack(density);
    }
    else if(state == Attacking && attacking_base >= 0)
    {
        Base* base = Game::GetBase(attacking_base);

        if(hierarchy > 1)
            base->TeamOfficerIsPlanningAttack(team);

        base->AddDensityWantingToAttack(shipDensityValue);
        if(group != nullptr && group->leader == this)
            for(auto& member : group->members)
                base->AddDensityWantingToAttack(member->GetShipDensityValue());
    }
    else if(state == Defending && defending_base >= 0)
    {
        Base* base = Game::GetBase(defending_base);

        base->AddDensityWantingToDefend(shipDensityValue);
        if(group != nullptr && group->leader == this)
            for(auto& member : group->members)
                base->AddDensityWantingToDefend(member->GetShipDensityValue());
    }
}

void ShipUnit::AddSigthTarget(Ship* target_)
{
    if(this != activePlayerUnit && target_ != nullptr)
    {
        bool addIt = true;

        for(int a = 0; a < sightShips.size(); a++)
        {
            if(sightShips[a] == target_)
            {
                addIt = false;
                break;
            }
        }

        if(addIt)
        {
            if(sightShips.size() >= MAX_SIGHT_UNITS)
            {
                if(GenerateRandomNumber(2))
                    sightShips[GenerateRandomNumber(sightShips.size())] = target_;
            }
            else sightShips.push_back(target_);
            if(targetEntity == nullptr)
                sightShipChaseTime = 0.f;
        }
    }
}

void ShipUnit::SetTarget(Ship* target)
{
    if(target != nullptr || this == activePlayerUnit)
        targetEntity = target;
}

bool ShipUnit::IsMoving()
{
    return movePoints.size() > 0;
}

bool ShipUnit::IsWillingToMoveToEnemyInBase()
{
    sf::Vector2i grid = GetGridPosition(GetPosition());
    int baseId = GetGridInfo(grid.y, grid.x).baseId;
    if(this != activePlayerUnit && pathFindDelay <= 0.f && targetEntity == nullptr && movePoints.size() == 0 && baseId >= 0 && Game::GetBase(baseId)->CanMoveToEnemy(team))
    {
        if((unitType == MiniShip || unitType == MicroShip) && group != nullptr && group->leader != this)
            return false;

        if(state == Idle || state == Attacking || state == Defending)
            return true;
    }

    return false;
}

void ShipUnit::AIUpdate(float dt)
{
    //Action
    bool isShootingTarget = false;
    realTimeMaxVelocity = maxVelocity;
    realTimeMaxForce = maxForce;
    sf::Vector2f force;

    if(IsWillingToMoveToEnemyInBase())
    {
        MoveToEnemiesInBase();
        if((unitType == MiniShip || unitType == MicroShip) && group == nullptr)
        {
            group = UnitGroup::CreatUnitGroups();
            group->leader = this;
            EntityManager::RecruitGroupMembers(this);
        }
    }
    else
    {
        const GridInfo& gridInfo = GetGridInfo(GetGridPosition(GetPosition()));
        if(gridInfo.baseId >= 0 && gridInfo.baseArea != true && pathFindDelay <= 0 && movePoints.size() == 0 && attackTargetState == true && targetEntity == nullptr)
            if(state == Idle || (state == Attacking && attacking_base == gridInfo.baseId))
            {
                Base* base = Game::GetBase(gridInfo.baseId);
                if(base->IsActive() && Game::GetTeamUpType(base->GetTeam()) != Game::GetTeamUpType(team))
                {
                    std::vector<sf::Vector2i> areas;
                    base->GetBaseAreaGrids(areas);
                    MoveTo((static_cast<sf::Vector2f>(areas[GenerateRandomNumber(areas.size())]) + sf::Vector2f(0.5f, 0.5f)) * (float)MapTileManager::TILE_SIZE, AStar::BaseRegionSearchType, false);
                }
            }
    }

    force += AttackTarget(dt, isShootingTarget);
    force += FollowGroup(dt, realTimeMaxVelocity, realTimeMaxForce);

    IdleThinking(dt);
    AttackUpdate();
    DefendUpdate(dt);
    TravelUpdate(dt);

    //Handle Movements
    //____Path Steering
    sf::Vector2f pathSteerForce;
    if(movePoints.size() > 0)
    {
        bool isReached = false;
        pathSteerForce = SteerForPathFollowing(isReached);

        if(isReached)
        {
            movePoints.clear();
            moveLineLength.clear();
            moveLineNormals.clear();
            moveLineIndex = 0;
        }
    }
    force += pathSteerForce;

    //____Flee Steering
    force += DeduceFleeSteering(dt);

    if(MagnitudeSqureOfVector(force) > 0)
    {
        /*if(MagnitudeSqureOfVector(force) > maxForce * maxForce)
            force = NormalizeVector(force) * maxForce;*/

        velocity += force * dt;
        if(MagnitudeSqureOfVector(velocity) > realTimeMaxVelocity * realTimeMaxVelocity)
            velocity = NormalizeVector(velocity) * realTimeMaxVelocity;

        if(isShootingTarget == false)
        {
            float tendAngle = 90 + (atan2f(velocity.y, velocity.x) * 180 / 3.142f);
            TransmitToAngle(tendAngle, dt);
            nonTransmittingAngle = tendAngle;
        }
    }
    else
    {
        if(MagnitudeSqureOfVector(velocity) != 0)
        {
            sf::Vector2f unitVec = NormalizeVector(velocity);
            velocity += -unitVec * realTimeMaxForce * dt;

            if(DotProduct(unitVec, velocity) < 0)
                velocity = sf::Vector2f(0.f, 0.f);
        }
    }

    sightShips.clear();
    AStar::UpdateLimiter();
}

void ShipUnit::PlayerUpdate(float dt)
{
    isActivePlayerAiming = false;
    isActivePlayerManualAiming = false;
    isActivePlayerJustShooting = false;
    activePlayerAimDistMagn = 0.f;

    if(movePoints.size() > 0)
        movePoints.clear();

    if(GUI::IsCursorOnObject() && Game::GetInput(sf::Mouse::Left, Game::Hold) == false)
        blockManualAimForGuiObject = true;
    else if(Game::GetInput(sf::Mouse::Left, Game::Hold) == false)
        blockManualAimForGuiObject = false;

    bool isInputManualAiming = (blockManualAimForGuiObject == false && (Game::GetInput(sf::Mouse::Left, Game::Hold) || Game::GetInput(sf::Mouse::Right, Game::Hold))) || Game::GetInput(sf::Keyboard::LShift, Game::Hold);
    bool isInputManualShooting = (blockManualAimForGuiObject == false && Game::GetInput(sf::Mouse::Left, Game::Hold)) || Game::GetInput(sf::Keyboard::LShift, Game::Hold);
    bool isInputAutoShooting = Game::GetInput(sf::Keyboard::Space, Game::Hold);

    if(Game::GetInput(sf::Keyboard::W, Game::Hold) ||
       Game::GetInput(sf::Keyboard::S, Game::Hold) ||
       Game::GetInput(sf::Keyboard::A, Game::Hold) ||
       Game::GetInput(sf::Keyboard::D, Game::Hold))
    {
        sf::Vector2f direction(0, 0);
        if(Game::GetInput(sf::Keyboard::W, Game::Hold)) direction.y -= 1;
        if(Game::GetInput(sf::Keyboard::S, Game::Hold)) direction.y += 1;
        if(Game::GetInput(sf::Keyboard::A, Game::Hold)) direction.x -= 1;
        if(Game::GetInput(sf::Keyboard::D, Game::Hold)) direction.x += 1;

        float velMagn = maxVelocity;
        if(isInputManualAiming || isInputAutoShooting)
            velMagn *= AIM_VELOCITY_FACTOR;

        realTimeMaxForce = maxForce;
        realTimeMaxVelocity = velMagn;

        sf::Vector2f desiredVelocity = NormalizeVector(direction) * realTimeMaxVelocity;
        sf::Vector2f steering = desiredVelocity - velocity;
        velocity += NormalizeVector(steering) * realTimeMaxForce * dt;

        if(DotProduct(steering, desiredVelocity - velocity) < 0)
            velocity = desiredVelocity;

        if(MagnitudeSqureOfVector(velocity) > maxVelocity * maxVelocity)
            velocity = NormalizeVector(velocity) * maxVelocity;

        if(MagnitudeSqureOfVector(velocity) != 0 && !isInputManualAiming && !isInputAutoShooting)
        {
            if(isAngleLocked)
            {
                SetAngle(velocity.y, velocity.x);
                nonTransmittingAngle = angle;
            }
            else
            {
                float tendToAngle = 90 + (atan2f(velocity.y, velocity.x) * 180 / 3.142f);
                TransmitToAngle(tendToAngle, dt);
                if(angle == tendToAngle)
                    isAngleLocked = true;
                nonTransmittingAngle = tendToAngle;
            }
        }
    }
    else
    {
        if(velocity != sf::Vector2f(0.f, 0.f))
        {
            sf::Vector2f unitVec = NormalizeVector(velocity);
            velocity += -unitVec * maxForce * dt;

            if(DotProduct(unitVec, velocity) < 0)
                velocity = sf::Vector2f(0.f, 0.f);
        }
        nonTransmittingAngle = angle;
    }

    if(isInputManualAiming || Game::GetInput(sf::Keyboard::C, Game::Hold))
    {
        isAngleLocked = false;

        sf::FloatRect camRect = EntityManager::GetCameraRect();
        sf::Vector2f diff = (sf::Vector2f)Game::GetMousePosition() * (camRect.height / Game::SCREEN_HEIGHT) + sf::Vector2f(camRect.left, camRect.top) - GetPosition();
        float tendAngle = 90 + (atan2f(diff.y, diff.x) * 180 / 3.142f);
        if(isInputManualAiming)
            TransmitToAngle(tendAngle, dt);
        nonTransmittingAngle = tendAngle;
        isActivePlayerAiming = true;
        isActivePlayerManualAiming = true;
        activePlayerAimDistMagn = MagnitudeOfVector(diff);

        if(isInputManualAiming && isInputManualShooting)
            Shoot();
    }
    else if(isInputAutoShooting)
    {
        isActivePlayerAiming = true;
        isAngleLocked = false;
        AimAndShoot(dt);
    }

    int inBaseId = GetGridInfo(GetGridPosition(GetPosition())).baseId;
    if(inBaseId >= 0)
        Game::GetBase(inBaseId)->AskForReinforcement(team);
}

void ShipUnit::Update(float dt)
{
    lastShotTime += dt;
    lastTargetUpdateTime -= dt;
    if(lastHitTime > 0)
        lastHitTime -= dt;
    if(pathFindDelay > 0)
        pathFindDelay -= dt;
    if(barTransparencyTime > 0)
        barTransparencyTime -= dt;
    if(thinkingWaitDuration > 0)
        thinkingWaitDuration -= dt;
    if(followGroupDelay > 0)
        followGroupDelay -= dt;
    if(traveling_updateTime > 0)
        traveling_updateTime -= dt;
    if(reloadingTime > 0)
        reloadingTime -= dt;
    if(movePoints.size() > 0 && moveLineIndexFixTime > 0)
        moveLineIndexFixTime -= dt;
    if(lastSurroudingTroopSpawn > 0 && targetEntity != nullptr)
        lastSurroudingTroopSpawn -= dt;
    if(surroundTroopsFavourTime > 0 && targetEntity != nullptr)
        surroundTroopsFavourTime -= dt;
    if(lastBulletFromHigherUnits_time > 0 || lastBulletFromHigherUnits != nullptr)
    {
        lastBulletFromHigherUnits_time -= dt;
        if(lastBulletFromHigherUnits_time <= 0)
            lastBulletFromHigherUnits = nullptr;
    }

    isCurentlyUsingAstarProgression = false;
    if(AStar::GetProgressionState() && AStar::IsReadyToSearch(false))
    {
        if(alive && this != activePlayerUnit)
        {
            if(state == Idle && thinking_baseId.size() > 0)
            {
                isThinkingFindingPath = true;
                temp_GridPath.clear();
                sf::Vector2i baseGrid = Game::GetBase(*(thinking_baseId.begin()))->GetRandomBaseGrid();
                while(thinking_baseId.size() > 0 && AStar::FindPath(baseGrid, GetGridPosition(GetPosition()), temp_GridPath, (AStar::SearchType)thinking_searchType, Game::GetTeamUpType(team)))
                {
                    if(thinking_closestPath.size() > 0 ? temp_GridPath.size() < thinking_closestPath.size() : true)
                        thinking_closestPath.swap(temp_GridPath);
                    temp_GridPath.clear();

                    thinking_baseId.erase(thinking_baseId.begin());
                    if(thinking_baseId.size() > 0)
                    {
                        baseGrid = Game::GetBase(*(thinking_baseId.begin()))->GetRandomBaseGrid();
                        AStar::ContinueLastTargetSearch(baseGrid);
                    }
                }
            }
            else MoveTo(sf::Vector2f(), AStar::NoSearchType, false);
        }
        else AStar::CancleProgression();

        isCurentlyUsingAstarProgression = AStar::GetProgressionState();
    }

    if(alive)
    {
        if(this == activePlayerUnit)//&& unitType == PlayerShip)
            PlayerUpdate(dt);
        else AIUpdate(dt);

        if(currentLevel < 5 && exp != nullptr && *exp >= GetNextLevelBaseExp())
        {
            if(exp != nullptr)
            {
                currentLevel = 1;
                for(int a = 3; a >= 0; a--)
                    if(*exp >= (unitType < MiniShip ? BaseLevelExpHigherUnits[a] : BaseLevelExpShared[a]))
                    {
                        currentLevel += a + 1;
                        break;
                    }
                if(currentLevel == 5)
                    *exp = unitType < MiniShip ? BaseLevelExpHigherUnits[3] : BaseLevelExpShared[3];
            }
            RefreshRealTimeProperties();
        }

        if(unitType < MiniShip && special > 0)
        {
            if(isSpecialActive)
            {
                *specialMeter -= 1.5f * dt;
                specialEmitRadiation += dt;
                if(*specialMeter <= 0)
                {
                    *specialMeter = 0;
                    isSpecialActive = false;
                    RefreshRealTimeProperties();
                }
                if(specialScalingFactor < 1)
                {
                    specialScalingFactor += 1.f * dt;
                    if(specialScalingFactor > 1)
                        specialScalingFactor = 1;
                }
                if(health < actualHitpoint)
                {
                    health += RECOVERY_PER_SEC / 4.f * actualHitpoint * dt;
                    if(health > actualHitpoint)
                        health = actualHitpoint;
                }
                if(specialEmitRadiation > 3.f)
                {
                    DisplayEffect::AddDisplayEffect(new DE_SpecialRadiation(this));
                    specialEmitRadiation -= 3.f;
                }
            }
            else
            {
                if(*specialMeter >= 100)
                {
                    *specialMeter = special;
                    isSpecialActive = true;
                    specialEmitRadiation = 0.f;
                    RefreshRealTimeProperties();
                    DisplayEffect::AddDisplayEffect(new DE_SpecialRadiation(this, true));
                }
                if(specialScalingFactor > 0)
                {
                    specialScalingFactor -= 1.f * dt;
                    if(specialScalingFactor < 0)
                        specialScalingFactor = 0;
                }
            }

            if(specialScalingFactor > 0)
                basedOnProperty.sprite->setScale(sf::Vector2f(0.5f, 0.5f) + sf::Vector2f(1, 1) * specialScalingFactor * 0.1f * (float)(1 + sin(Game::GetGameExecutionTimeInSec() / 3.f * 2 * 22 / 7)) / 2.f);
            else basedOnProperty.sprite->setScale(0.5f, 0.5f);
        }

        sf::Vector2i grid = GetGridPosition(GetPosition());
        if(grid.x >= 0 && grid.x < MapTileManager::MAX_MAPTILE && grid.y >= 0 && grid.y < MapTileManager::MAX_MAPTILE)
        {
            int baseId = GetGridInfo(grid.y, grid.x).baseId;
            if(baseId >= 0 && baseId < Game::GetNumberOfBases())
            {
                Base* base = Game::GetBase(baseId);

                if(health < GetActualHitpoint() && Game::GetTeamUpType(base->GetTeam()) == Game::GetTeamUpType(team) && base->GetGenerateState())
                {
                    if(barTransparencyTime < 0.5f)
                        barTransparencyTime = BAR_DURATION - barTransparencyTime;
                    else if(barTransparencyTime < BAR_DURATION - 0.5f)
                        barTransparencyTime = BAR_DURATION - 0.5f;

                    if(unitType == MiniShip || unitType  == MicroShip)
                        health += RECOVERY_PER_SEC_SMALLUNIT * GetActualHitpoint() * dt;
                    else health += RECOVERY_PER_SEC * GetActualHitpoint() * dt;

                    if(health > GetActualHitpoint())
                        health = GetActualHitpoint();
                }

                if(hierarchy > 1 && base->GetTeam() == team && base->IsActive() && base->IsEnemyInBase() == false && base->IsFriendlyTargeting() == false)
                {
                    if(surroundTroopsBuildTime > 0)
                        surroundTroopsBuildTime -= dt;
                    if(surroundTroopsBuildTime <= 0 && surroundingTroops < SURROUNDING_TROOP_MAX)
                    {
                        surroundTroopsBuildTime = SURROUNDING_TROOP_BUILD_TIME;
                        surroundingTroops++;
                    }
                }
            }
        }

        /*if(targetEntity != nullptr && (shipType != PlayerShip ? attackTargetState : true))
        {
            sf::Vector2i myGrid = grid;
            sf::Vector2i targetGrid = GetGridPosition(targetEntity->GetPosition());
            int targetGridBase = GetGridInfo(targetGrid.y, targetGrid.x).baseId;
            int myGridBase = GetGridInfo(myGrid.y, myGrid.x).baseId;
            if(targetGridBase >= 0 && myGridBase != targetGridBase)
                Game::GetBase(targetGridBase)->AddUnit(this, false);
        }*/

        unsigned short transparency = 255;
        sf::Color color = sf::Color::White;
        if(reclaimBase >= 0)
        {
            transparency = GetSpawnTransitionFactor(-dt) * 255;
            if(transparency == 0)
            {
                Game::GetBase(reclaimBase)->HasRecaimedUnit();
                alive = false;
            }
        }
        if(isTransmitingSpawn)
            transparency = GetSpawnTransitionFactor(dt) * 255;
        if(lastHitTime > 0)
            color = sf::Color(255, 150, 150);

        if(isMultiDrawerBasedOn)
        {
            basedOnProperty.multiDrawerProperties->transformation.setRotation(angle);
            basedOnProperty.multiDrawerProperties->transformation.move(velocity * dt);

            for(int a = 0; a < 4; a++)
            {
                basedOnProperty.multiDrawerProperties->vertices[a].color = color;
                basedOnProperty.multiDrawerProperties->vertices[a].color.a = transparency;
            }

            sf::Vector2f sizeTextr = (sf::Vector2f)basedOnProperty.multiDrawerProperties->multiDrawer->GetTexture()->getSize();
            basedOnProperty.multiDrawerProperties->vertices[0].position = basedOnProperty.multiDrawerProperties->transformation.getTransform().transformPoint(sf::Vector2f(0, 0));
            basedOnProperty.multiDrawerProperties->vertices[1].position = basedOnProperty.multiDrawerProperties->transformation.getTransform().transformPoint(sf::Vector2f(sizeTextr.x, 0));
            basedOnProperty.multiDrawerProperties->vertices[2].position = basedOnProperty.multiDrawerProperties->transformation.getTransform().transformPoint(sf::Vector2f(sizeTextr.x, sizeTextr.y));
            basedOnProperty.multiDrawerProperties->vertices[3].position = basedOnProperty.multiDrawerProperties->transformation.getTransform().transformPoint(sf::Vector2f(0, sizeTextr.y));

            basedOnProperty.multiDrawerProperties->multiDrawer->AddVertex(basedOnProperty.multiDrawerProperties->vertices, 4);
        }
        else
        {
            sf::Color clr = color;
            clr.a = transparency;

            basedOnProperty.sprite->setColor(clr);
            basedOnProperty.sprite->setRotation(angle);
            basedOnProperty.sprite->move(velocity * dt);
            if(EntityManager::GetCameraRect().intersects(basedOnProperty.sprite->getGlobalBounds()))
                shipUnitSpritesToDraw.push_back(this);
        }

        HandleGunReloading();
        SpawnSurroundingTroops();
        UpdateHealthBar();
    }
}

void ShipUnit::RefreshEntities()
{
    Ship::RefreshEntities();
    if(lastBulletFromHigherUnits != nullptr && lastBulletFromHigherUnits->GetAlive() == false)
    {
        lastBulletFromHigherUnits = nullptr;
        lastBulletFromHigherUnits_time = 0;
    }
}

ShipUnit::UnitType ShipUnit::GetUnitType()
{
    return unitType;
}

void ShipUnit::AimAndShoot(float dt)
{
    if(targetEntity != nullptr)
    {
        if(Game::GetTeamUpType(targetEntity->GetTeam()) != Game::GetTeamUpType(team))
        {
            sf::Vector2f dist = targetEntity->GetPosition() - GetPosition();
            float distSqr = dist.x * dist.x + dist.y * dist.y;

            if(distSqr <= targetRange * targetRange)
            {
                float tendAngle = 90 + (atan2f(dist.y, dist.x) * 180 / 3.142f);
                TransmitToAngle(tendAngle, dt);
                nonTransmittingAngle = tendAngle;
                if(activePlayerUnit == this)
                {
                    isActivePlayerAiming = true;
                    activePlayerAimDistMagn = MagnitudeOfVector(dist);
                }
            }
            else targetEntity = nullptr;
        }
    }
    Shoot();
}

void ShipUnit::Shoot()
{
    if(skin != nullptr)
    {
        float bulletScale = 0;
        sf::Vector2f shootPoint;
        if(isMultiDrawerBasedOn)
            shootPoint = basedOnProperty.multiDrawerProperties->transformation.getTransform().transformPoint(skin->shootPoint);
        else shootPoint = basedOnProperty.sprite->getTransform().transformPoint(skin->shootPoint);

        if(unitType == MicroShip || unitType == MiniShip) //remove later
            bulletScale = BULLET_SCALE_MICRO_SHIP;
        else if(unitType == MiniShip)
            bulletScale = BULLET_SCALE_MINI_SHIP;
        else bulletScale = BULLET_SCALE_DEFAULT;

        //Shooting
        bool justShot = false;
        if(currentGun >= 0 && currentGun < Gun::NumOfGuns && gunMagazine > 0)
            if(Gun::Shoot(currentGun, shootPoint, velocity, team, angle, attack, lastShotTime, bulletScale, gunMagazine, this))
            {
                reloading = false;
                reloadingTime = 0.f;
                justShot = true;
            }

        if(activePlayerUnit == this)
            isActivePlayerJustShooting = justShot;
    }
}

bool ShipUnit::FleeFromUnit(Ship* unit_)
{
    float factor = 0.f;
    float distMagn = MagnitudeOfVector(unit_->GetPosition() - GetPosition()) - unit_->GetFleeDistance();
    if(distMagn < 0)
        distMagn = -distMagn;
    if(distMagn < fleeDistance + FLEE_EFFECTIVE_DISTANCE && distMagn != 0)
    {
        factor = 1.f - (distMagn - fleeDistance) / FLEE_EFFECTIVE_DISTANCE;
        //if(factor > 2.f) factor = 2.f;

        fleeSteering += NormalizeVector((GetPosition() - unit_->GetPosition()) / distMagn * realTimeMaxForce - velocity) * realTimeMaxForce * (factor > 3 ? 3 : factor);
        numOfFleeingUnits++;
        return true;
    }

    return false;
}

sf::Vector2f ShipUnit::DeduceFleeSteering(float dt)
{
    sf::Vector2f steering;
    if(this != activePlayerUnit)//unitType != PlayerShip)
    {
        int numOfUnitsFled = 0;
        int maxFleeBasedOnSize = (1.f - (EntityManager::GetNumOfAliveShipUnits() / (float)EntityManager::MAX_NUM_OF_SHIPS)) * (MAX_FLEE_UNITS - MIN_FLEE_UNITS) + MIN_FLEE_UNITS;
        if(maxFleeBasedOnSize < MIN_FLEE_UNITS)
            maxFleeBasedOnSize = MIN_FLEE_UNITS;

        sf::Vector2f unitPos = GetPosition();
        temp_ships_su.clear();

        float fleeSize = fleeDistance + FLEE_EFFECTIVE_DISTANCE;
        int xGridStart = (unitPos.x - fleeSize) / MapTileManager::TILE_SIZE;
        int xGridEnd = (unitPos.x + fleeSize) / MapTileManager::TILE_SIZE;
        int yGridStart = (unitPos.y - fleeSize) / MapTileManager::TILE_SIZE;
        int yGridEnd = (unitPos.y + fleeSize) / MapTileManager::TILE_SIZE;

        for(int y = yGridStart; y <= yGridEnd && numOfUnitsFled < maxFleeBasedOnSize; y++)
        {
            for(int x = xGridStart; x <= xGridEnd && numOfUnitsFled < maxFleeBasedOnSize; x++)
            {
                if(x >= 0 && x < MapTileManager::MAX_MAPTILE && y >= 0 && y < MapTileManager::MAX_MAPTILE)
                {
                    GridInfo& gridInfo = GetGridInfo(y, x);
                    if(gridInfo.ships.size() > 0)
                    {
                        for(int a = 0; a < gridInfo.ships.size() && numOfUnitsFled < maxFleeBasedOnSize; a++)
                        {
                            if(gridInfo.ships[a] != this)
                            {
                                bool isSkipping = false;

                                for(int b = 0; b < temp_ships_su.size(); b++)
                                {
                                    if(gridInfo.ships[a] == temp_ships_su[b])
                                    {
                                        isSkipping = true;
                                        break;
                                    }
                                }

                                if(isSkipping == false)
                                {
                                    if(FleeFromUnit(gridInfo.ships[a]))
                                    {
                                        temp_ships_su.push_back(gridInfo.ships[a]);
                                        numOfUnitsFled++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if(numOfFleeingUnits > 0)
            steering = fleeSteering / (float)numOfFleeingUnits * 1.f;

        fleeSteering = sf::Vector2f(0, 0);
        numOfFleeingUnits = 0;
    }

    float strMagn = steering != sf::Vector2f(0, 0) ? MagnitudeOfVector(steering) : 0;
    if(IsMoving() && strMagn / realTimeMaxForce > 0.7f)
    {
        fleeReductionTime += dt;
        if(fleeReductionTime > 1.f)
            fleeReductionTime = 1.f;

        float rdtForce = 0.7f * realTimeMaxForce;
        steering = steering / strMagn * (rdtForce + (strMagn - rdtForce) * (1.f - fleeReductionTime));
    }
    else if(fleeReductionTime > 0.f)
    {
        fleeReductionTime -= dt * 0.5f;
        if(fleeReductionTime < 0.f)
            fleeReductionTime = 0.f;
    }

    return steering;
}

void ShipUnit::MoveTo(const std::vector<sf::Vector2i>& path_)
{
    if(path_.size() > 0)
    {
        movePoints.clear();
        moveLineLength.clear();
        moveLineNormals.clear();
        moveLineIndex = 0;

        sf::Vector2f halfTileSize(MapTileManager::TILE_SIZE / 2.f, MapTileManager::TILE_SIZE / 2.f);
        pathFindDelay = (path_.size() / (float)AStar::LIMIT_FAST_SEARCH) * PATH_FIND_DELAY;

        for(int a = 0; a < path_.size(); a++)
            movePoints.push_back((sf::Vector2f)(path_[a] * MapTileManager::TILE_SIZE) + halfTileSize);

        sf::Vector2f diff;
        float magn = 0;
        for(int a = 0; a < movePoints.size() - 1; a++)
        {
            diff = movePoints[a + 1] - movePoints[a];
            magn = MagnitudeOfVector(diff);
            moveLineLength.push_back(magn);
            moveLineNormals.push_back(diff / magn);
        }
    }
}

void ShipUnit::MoveTo(sf::Vector2f location, short searchType, bool isFastSearch, bool useReachSearch)
{
    if(AStar::IsReadyToSearch(isFastSearch) && pathFindDelay <= 0.f)
    {
        temp_GridPath.clear();
        if(isFastSearch)
        {
            if(useReachSearch)
                AStar::FindFastPath(GetGridPosition(location), GetGridPosition(GetPosition()), temp_GridPath, (AStar::SearchType)searchType, Game::GetTeamUpType(GetTeam()), false, useReachSearch);
            else AStar::FindFastPath(GetGridPosition(GetPosition()), GetGridPosition(location), temp_GridPath, (AStar::SearchType)searchType, Game::GetTeamUpType(GetTeam()), false, false);
        }
        else
        {
            if(useReachSearch)
                AStar::FindPath(GetGridPosition(location), GetGridPosition(GetPosition()), temp_GridPath, (AStar::SearchType)searchType, Game::GetTeamUpType(GetTeam()), false, useReachSearch);
            else AStar::FindPath(GetGridPosition(GetPosition()), GetGridPosition(location), temp_GridPath, (AStar::SearchType)searchType, Game::GetTeamUpType(GetTeam()), false, false);
        }

        if(temp_GridPath.size() > 0)
        {
            movePoints.clear();
            moveLineLength.clear();
            moveLineNormals.clear();
            moveLineIndex = 0;

            sf::Vector2f halfTileSize(MapTileManager::TILE_SIZE / 2.f, MapTileManager::TILE_SIZE / 2.f);
            pathFindDelay = (temp_GridPath.size() / (float)AStar::LIMIT_FAST_SEARCH) * PATH_FIND_DELAY;

            for(int a = 0; a < temp_GridPath.size(); a++)
                movePoints.push_back((sf::Vector2f)(temp_GridPath[a] * MapTileManager::TILE_SIZE) + halfTileSize);

            sf::Vector2f diff;
            float magn = 0;
            for(int a = 0; a < movePoints.size() - 1; a++)
            {
                diff = movePoints[a + 1] - movePoints[a];
                magn = MagnitudeOfVector(diff);
                moveLineLength.push_back(magn);
                moveLineNormals.push_back(diff / magn);
            }
        }
    }
}

sf::Vector2f ShipUnit::SteerForSeek(const sf::Vector2f& target)
{
    if(isMultiDrawerBasedOn)
        return NormalizeVector(target - basedOnProperty.multiDrawerProperties->transformation.getPosition()) * realTimeMaxVelocity - velocity;
    else return NormalizeVector(target - basedOnProperty.sprite->getPosition()) * realTimeMaxVelocity - velocity;
}

sf::Vector2f ShipUnit::SteerForFlee(const sf::Vector2f& target)
{
    if(isMultiDrawerBasedOn)
        return NormalizeVector(basedOnProperty.multiDrawerProperties->transformation.getPosition() - target) * realTimeMaxVelocity - velocity;
    else return NormalizeVector(basedOnProperty.sprite->getPosition() - target) * realTimeMaxVelocity - velocity;
}

sf::Vector2f ShipUnit::SteerForArrive(const sf::Vector2f& target, bool& isReached, float minVelocityFactor)
{
    sf::Vector2f position;
    if(isMultiDrawerBasedOn)
        position = basedOnProperty.multiDrawerProperties->transformation.getPosition();
    else position = basedOnProperty.sprite->getPosition();

    float dist = MagnitudeOfVector(target - position);

    if(dist < MOVE_REACHED_DISTANCE)
        isReached = true;
    else isReached = false;

    if(dist < slowingDistance)
        return NormalizeVector((NormalizeVector(target - position) * realTimeMaxVelocity * (dist / slowingDistance * (1 - minVelocityFactor) + minVelocityFactor)) - velocity) * realTimeMaxForce;
    return NormalizeVector(SteerForSeek(target)) * realTimeMaxForce;
}

sf::Vector2f ShipUnit::SteerForPathFollowing(bool& isReached)
{
    sf::Vector2f position;
    if(isMultiDrawerBasedOn)
        position = basedOnProperty.multiDrawerProperties->transformation.getPosition();
    else position = basedOnProperty.sprite->getPosition();

    isReached = false;
    /*if(movePoints.size() == 2 && MagnitudeSqureOfVector(movePoints[1] - position) < slowingDistance * slowingDistance)
    {
        movePoints.erase(movePoints.begin());
        moveLineLength.clear();
        moveLineNormals.clear();
    }*/

    if(movePoints.size() == 1)
    {
        return SteerForArrive(movePoints[0], isReached);
    }
    else if(movePoints.size() > 1)
    {
        float velocityMagn = MagnitudeOfVector(velocity);
        float predictMagn = 0.f;
        if(realTimeMaxForce != 0)
            predictMagn = velocityMagn * velocityMagn / realTimeMaxForce;
        //if(predictMagn > MOVE_MAX_PREDICTION)
            //predictMagn = MOVE_MAX_PREDICTION;

        sf::Vector2f predictedPosition = position + (velocityMagn != 0 && predictMagn != 0 ? (velocity / velocityMagn * predictMagn) : sf::Vector2f());

        bool isSightOnSmallWall = false;
        float sightLenght = 0.f;
        if(EntityManager::isSightOnWall(position, predictedPosition, &isSightOnSmallWall, &sightLenght, false, sizeRadius * 2) || isSightOnSmallWall)
        {
            predictMagn = sightLenght;
            predictedPosition = position + (velocityMagn != 0 && predictMagn != 0 ? (velocity / velocityMagn * predictMagn) : sf::Vector2f());
        }

        sf::Vector2f target;
        float smallestDistSqr = -1;
        bool isOnRightDir = false;
        int lineIndex = -1;

        if(moveLineIndex > movePoints.size() - 2)
            moveLineIndex = movePoints.size() - 2;

        //Declaring loop variables for optimization
        float projectionOnLine = 0;
        float outsideDistSqr = 0;
        sf::Vector2f lineTarget;
        bool rightDir = false;
        for(int a = (moveLineIndex < 3 ? 0 : moveLineIndex - 3); a < movePoints.size() - 1 && a <= moveLineIndex + 3; a++)
        {
            projectionOnLine = DotProduct(moveLineNormals[a], predictedPosition - movePoints[a]);

            if(projectionOnLine < 0)
                lineTarget = movePoints[a];
            else if(projectionOnLine > moveLineLength[a])
                lineTarget = movePoints[a + 1];
            else lineTarget = movePoints[a] + moveLineNormals[a] * projectionOnLine;

            outsideDistSqr = MagnitudeSqureOfVector(lineTarget - predictedPosition);
            rightDir = DotProduct(movePoints[a + 1] - position, predictedPosition - position) > 0;

            if(rightDir == false)
                lineTarget = movePoints[a + 1];
            else
            {
                sf::Vector2f plusVec = moveLineNormals[a] * MOVE_DIR_DISTANCE;
                isSightOnSmallWall = false;
                if(EntityManager::isSightOnWall(position, lineTarget + plusVec, sizeRadius * 2) == false)
                    lineTarget += plusVec;
            }

            if(outsideDistSqr <= smallestDistSqr || smallestDistSqr < 0)
            {
                smallestDistSqr = outsideDistSqr;
                target = lineTarget;
                isOnRightDir = rightDir;
                lineIndex = a;
            }
        }
        moveLineIndex = lineIndex;

        //Check for index error
        if(moveLineIndexFixTime <= 0)
        {
            bool isLineOnSmallWall = false;
            if(EntityManager::isSightOnWall(GetPosition(), movePoints[moveLineIndex], isLineOnSmallWall) || isLineOnSmallWall)
                moveLineIndexFixCount++;
            else moveLineIndexFixCount = 0;

            if(moveLineIndexFixCount >= 50)
            {
                moveLineIndexFixCount = 0;
                movePoints.clear();
            }

            if(moveLineIndexFixCount > 0)
                moveLineIndexFixTime = 0.1f;
            else moveLineIndexFixTime = 5.f;
        }

        if(lineIndex >= movePoints.size() - 2 && MagnitudeSqureOfVector(movePoints[movePoints.size() - 1] - position) < slowingDistance * slowingDistance)
            return SteerForArrive(movePoints[movePoints.size() - 1], isReached);
        else if(lineIndex >= 0)
        {
            if(smallestDistSqr >= MOVE_RADUIS * MOVE_RADUIS || isOnRightDir == false || velocityMagn < realTimeMaxVelocity * 0.9f)
            {
                /*if(lineIndex > 0 && false)
                {
                    movePoints.erase(movePoints.begin(), movePoints.begin() + lineIndex);
                    moveLineLength.erase(moveLineLength.begin(), moveLineLength.begin() + lineIndex);
                    moveLineNormals.erase(moveLineNormals.begin(), moveLineNormals.begin() + lineIndex);
                }*/
                //return NormalizeVector(SteerForSeek(target)) * realTimeMaxForce;
                bool reached = false;
                sf::Vector2f return_ = SteerForArrive(target, reached, 0.1f);

                if(isReached && lineIndex < movePoints.size() - 2)
                    lineIndex++;
                return return_;
            }
        }
    }

    return sf::Vector2f();
}

void ShipUnit::Draw(sf::RenderTarget& target)
{
    for(ShipUnit*& unit : shipUnitSpritesToDraw)
    {
        if(unit->unitType < MiniShip && unit->specialScalingFactor > 0)
        {
            if(unit->specialScalingFactor < 1 || unit->GetSpecialInFactors() <= 0.25f)
                target.draw(*unit->basedOnProperty.sprite);
            if(unit->specialScalingFactor > 0)
            {
                unit->basedOnProperty.sprite->setTextureRect(sf::IntRect(180, 0, 180, 180));
                if(unit->GetSpecialInFactors() <= 0.25f)
                    unit->basedOnProperty.sprite->setColor(sf::Color(255, 255, 255, 255.f * unit->specialScalingFactor * (1.f + sin(Game::GetGameExecutionTimeInSec() * 2.f * 22.f / 7.f)) / 2.f));
                target.draw(*unit->basedOnProperty.sprite);
                unit->basedOnProperty.sprite->setTextureRect(sf::IntRect(0, 0, 180, 180));
                unit->basedOnProperty.sprite->setColor(sf::Color::White);
            }
        }
        else target.draw(*unit->basedOnProperty.sprite);
    }
    shipUnitSpritesToDraw.clear();

    for(int a = 0; a < Game::NumOfTeams; a++)
    {
        for(int b = 0; b < multidrawer_shipunit_miniship[a].size(); b++)
            multidrawer_shipunit_miniship[a][b].Draw(target);

        for(int b = 0; b < multidrawer_shipunit_microship[a].size(); b++)
            multidrawer_shipunit_microship[a][b].Draw(target);
    }
}

sf::Vector2f ShipUnit::AttackTarget(float dt, bool& isShootingTarget)
{
    isShootingTarget = false;
    sf::Vector2f force;

    if(attackTargetState)
    {
        if(targetEntity != nullptr)
        {
            sf::Vector2f dist = targetEntity->GetPosition() - GetPosition();
            float distMagn = MagnitudeOfVector(dist);
            bool isOnSight = !EntityManager::isSightOnWall(GetPosition(), targetEntity->GetPosition());

            sf::Vector2i myGrid = GetGridPosition(GetPosition());
            sf::Vector2i targetGrid = GetGridPosition(targetEntity->GetPosition());
            int targetGridBase = GetGridInfo(targetGrid.y, targetGrid.x).baseId;
            int myGridBase = GetGridInfo(myGrid.y, myGrid.x).baseId;
            bool myInBaseArea = GetGridInfo(myGrid.y, myGrid.x).baseArea;
            bool targetInBaseArea = GetGridInfo(targetGrid.y, targetGrid.x).baseArea;
            if(targetGridBase >= 0 && targetInBaseArea && (myGridBase != targetGridBase || myInBaseArea == false))
                Game::GetBase(targetGridBase)->AddShipDensity(this);
            if(targetEntity->GetShipTarget() != nullptr)
            {
                sf::Vector2i targetTargetGrid = GetGridPosition(targetEntity->GetShipTarget()->GetPosition());
                int targetTargetGridBase = GetGridInfo(targetTargetGrid.y, targetTargetGrid.x).baseId;
                bool targetTargetInBaseArea = GetGridInfo(targetTargetGrid.y, targetTargetGrid.x).baseArea;

                if(targetTargetGridBase >= 0 && targetTargetInBaseArea && (myGridBase != targetTargetGridBase || myInBaseArea == false) && (targetGridBase != targetTargetGridBase || targetInBaseArea == false))
                    Game::GetBase(targetTargetGridBase)->AddShipDensity(this);
            }

            if(isOnSight && dist.x * dist.x + dist.y * dist.y <= targetRange * targetRange)
            {
                AimAndShoot(dt);
                isShootingTarget = true;
                movePoints.clear();

                //Dodging
                if(MagnitudeSqureOfVector(dodgeDir) > 0)
                {
                    if(distMagn > targetRange * 0.9f)
                        dodgeDir.y = 1;
                    else if(distMagn < dodgeRadius + targetEntity->GetDodgeRadius() + 5.f)
                        dodgeDir.y = -2;

                    sf::Transform dirTransform;
                    dirTransform.rotate(angle);
                    sf::Vector2f dir = dirTransform.transformPoint(dodgeDir.x, 0);

                    force = NormalizeVector(dir) * realTimeMaxVelocity * AIM_VELOCITY_FACTOR;

                    sf::Vector2f inForce = NormalizeVector(targetEntity->GetPosition() - GetPosition() + force);
                    inForce *= MagnitudeOfVector(targetEntity->GetPosition() - GetPosition() + force) - MagnitudeOfVector(dist);
                    force += inForce;
                    sf::Vector2f desiredVelocity = force + NormalizeVector(dist) * realTimeMaxVelocity * 0.15f * dodgeDir.y;
                    desiredVelocity = NormalizeVector(desiredVelocity) * realTimeMaxVelocity * AIM_VELOCITY_FACTOR;
                    force = NormalizeVector(desiredVelocity - velocity) * realTimeMaxForce;

                    sf::Vector2f normDisiredVel = NormalizeVector(desiredVelocity);
                    float velMagn = MagnitudeOfVector(velocity);
                    float t = velMagn / realTimeMaxForce;
                    sf::Vector2f checkVec =  normDisiredVel * (velMagn * t + 0.5f * -realTimeMaxForce * t * t + dodgeRadius + dodgeAllowance);
                    sf::Vector2f checkVelDistVec = (velMagn != 0 ? velocity / velMagn : sf::Vector2f()) * 25.f;

                    bool stopDodging = false;
                    if(group != nullptr && group->leader != nullptr && group->leader != this &&
                       MagnitudeSqureOfVector(group->leader->GetPosition() - (GetPosition() + checkVec)) > FG_DISTANCE * FG_DISTANCE)
                        stopDodging = true;
                    else if(skin != nullptr && EntityManager::isSightOnWall(GetPosition(), GetPosition() + checkVec) == true)
                        stopDodging = true;
                    else if(EntityManager::isSightOnWall(GetPosition() + checkVelDistVec, targetEntity->GetPosition()) == true)
                        stopDodging = true;

                    if(stopDodging)
                    {
                        force = sf::Vector2f();
                        dodgeDir = sf::Vector2f();
                    }
                }

                dodgeTime -= dt;
                if(dodgeTime <= 0.f)
                {
                    dodgeTime = 0.5f + DODGE_DURATION * GenerateRandomNumber(100) / 100.f;
                    dodgeAllowance = 5.f + (GenerateRandomNumber(2) ? (GenerateRandomNumber(151) / 100.f) * MapTileManager::TILE_SIZE + 5.f : 5.f);
                    dodgeDir = sf::Vector2f();

                    if(GenerateRandomNumber(2))
                    {
                        sf::Vector2f unitVec = dist / distMagn;
                        bool isWallLeft = EntityManager::isSightOnWall(GetPosition(), GetPosition() + sf::Vector2f(unitVec.y, -unitVec.x) * (dodgeRadius + dodgeAllowance + 10.f));
                        bool isWallRight = EntityManager::isSightOnWall(GetPosition(), GetPosition() + sf::Vector2f(-unitVec.y, unitVec.x) * (dodgeRadius + dodgeAllowance + 10.f));

                        if(isWallLeft == true && isWallRight == false)
                            dodgeDir.x = 1 + (GenerateRandomNumber(3) > 0 ? 0 : -1);
                        else if(isWallLeft == false && isWallRight == true)
                            dodgeDir.x = -1 + (GenerateRandomNumber(3) > 0 ? 0 : 1);
                        else if(isWallLeft == false && isWallRight == false)
                            dodgeDir.x = GenerateRandomNumber(2) ? 1 : -1;

                        if(dodgeDir.x != 0)
                        {
                            dodgeDir.y = -1 + GenerateRandomNumber(3);
                            if(GenerateRandomNumber(5) == 0)
                                dodgeDir.y *= 2.5f;
                        }
                    }
                }
            }
            else
            {
                sightShipChaseTime += dt;
                dodgeDir.x = 0;
                dodgeTime = 0.5f;

                bool isAstarReady = AStar::IsReadyToSearch(true);
                MoveTo(targetEntity->GetPosition(), AStar::NoSearchType, true, true);
                if((movePoints.size() == 0 && isAstarReady) || (movePoints.size() > (float)SHIP_SIGTH_RANGE / MapTileManager::TILE_SIZE * 1.5f + 1 && isAstarReady) || sightShipChaseTime >= TARGET_CHASE_TIME)
                {
                    targetEntity = nullptr;
                    sightShipChaseTime = 0;
                    movePoints.clear();
                }
            }
        }
        else if(sightShips.size() > 0 && targetEntity == nullptr && AStar::IsReadyToSearch(true) && pathFindDelay <= 0)
        {
            int closestShip = -1;
             pathFindDelay += 1.f;

            for(int a = 0; a < sightShips.size(); a++)
            {
                temp_GridPath.clear();

                if(AStar::FindFastPath(GetGridPosition(sightShips[a]->GetPosition()), GetGridPosition(GetPosition()), temp_GridPath, AStar::NoSearchType, -1, false, true) &&
                   (closestShip == -1 || temp_GridPath.size() < temp_closestGridPath.size()))
                {
                    closestShip = a;
                    temp_closestGridPath.swap(temp_GridPath);
                }
                if(a + 1 < sightShips.size())
                    AStar::ContinueLastTargetFastSearch(GetGridPosition(sightShips[a + 1]->GetPosition()));
            }

            if(closestShip >= 0)
            {
                targetEntity = sightShips[closestShip];
                sightShipChaseTime = 0.f;

                movePoints.clear();
                moveLineLength.clear();
                moveLineNormals.clear();
                moveLineIndex = 0;

                if(temp_closestGridPath.size() > 0)
                {
                    sf::Vector2f halfTileSize(MapTileManager::TILE_SIZE / 2.f, MapTileManager::TILE_SIZE / 2.f);
                    for(int a = 0; a < temp_closestGridPath.size(); a++)
                        movePoints.push_back((sf::Vector2f)(temp_closestGridPath[a] * MapTileManager::TILE_SIZE) + halfTileSize);

                    sf::Vector2f diff;
                    float magn = 0;
                    if(movePoints.size() > 0)
                    {
                        for(int a = 0; a < movePoints.size() - 1; a++)
                        {
                            diff = movePoints[a + 1] - movePoints[a];
                            magn = MagnitudeOfVector(diff);
                            moveLineLength.push_back(magn);
                            moveLineNormals.push_back(diff / magn);
                        }
                    }
                }
            }
            /*else
            {
                movePoints.clear();
                moveLineLength.clear();
                moveLineNormals.clear();
                moveLineIndex = 0;
            }*/
        }
    }
    else targetEntity = nullptr;
    return force;
}

sf::Vector2f ShipUnit::GetVelocity()
{
    return velocity;
}

void ShipUnit::MoveToEnemiesInBase()
{
    if(pathFindDelay <= 0.f && AStar::IsReadyToSearch(false))
    {
        sf::Vector2i gridPos = GetGridPosition(GetPosition());
        if(GetGridInfo(gridPos.y, gridPos.x).baseId >= 0)
        {
            Base* base = Game::GetBase(GetGridInfo(gridPos.y, gridPos.x).baseId);
            if(base != nullptr && base->CanMoveToEnemy(team))
            {
                if(Game::GetTeamUpType(team) != Game::GetTeamUpType(base->GetTeam()) && base == Game::GetMainBase(base->GetTeam()) && Game::GetMainBase(base->GetTeam())->IsActive())
                {
                    MoveTo(EntityManager::GetBaseCore(base->GetTeam())->GetPosition(), AStar::BaseRegionSearchType, false);
                    base->ResetMoveToEnemy();
                }
                else
                {
                    std::vector<Ship*>* unitsInBase;
                    std::vector<Ship*> getToUnits;
                    for(int b = 0; b < Game::NumOfTeams; b++)
                        if(b != team)
                        {
                            unitsInBase = base->GetUnitsInBase(b);
                            if(unitsInBase->size() > 0)
                                getToUnits.push_back((*unitsInBase)[GenerateRandomNumber(unitsInBase->size())]);
                        }
                    if(getToUnits.size() > 0)
                    {
                        Ship* tempShip = getToUnits[GenerateRandomNumber(getToUnits.size())];
                        sf::Vector2i tempGrid = GetGridPosition(tempShip->GetPosition());
                        bool isInSameBase = GetGridInfo(gridPos.y, gridPos.x).baseId == GetGridInfo(tempGrid.y, tempGrid.x).baseId;

                        MoveTo(tempShip->GetPosition(), isInSameBase ? AStar::BaseRegionSearchType : AStar::NoSearchType, false);
                        base->ResetMoveToEnemy();
                    }
                }
            }
        }
    }
}

ShipUnit::ShipState ShipUnit::GetState()
{
    return state;
}

void ShipUnit::GetVelocityDetails(float& maxVelocity_, float& maxForce_)
{
    maxVelocity_ = maxVelocity;
    maxForce_ = maxForce;
}

void ShipUnit::JoinGroup(UnitGroup* group_, bool asLeader )
{
    Ship::JoinGroup(group_, asLeader);
    if(asLeader == false && group != nullptr)
        state = FollowingGroup;
}

sf::Vector2f ShipUnit::FollowGroup(float dt, float& maxVelocity_, float& maxForce_)
{
    sf::Vector2f force;

    if(state == FollowingGroup)
    {//std::cout << "...following group\n";
        if(group == nullptr || (group != nullptr && group->leader == this))
        {
            state = Idle;
            return sf::Vector2f();
        }

        if(group != nullptr && group->leader != nullptr)
        {
            float distMagn = MagnitudeOfVector(group->leader->GetPosition() - GetPosition());//std::cout << "debugging\n";
            float mV;
            float mF;
            group->leader->GetVelocityDetails(mV, mF);

            float boostDir = -1;
            float boostFactor = 1;

            if(targetEntity != nullptr && attackTargetState && followGroupDelay < FOLLOWGROUP_DELAY_MIN)
                followGroupDelay = (FOLLOWGROUP_DELAY_MAX - FOLLOWGROUP_DELAY_MIN) * GenerateRandomNumber(101) / 100.f;

            if(MagnitudeSqureOfVector(group->leader->GetVelocity()) > (mV * mV) * 0.2f)
            {
                boostFactor = DotProduct(NormalizeVector(group->leader->GetVelocity()), GetPosition() - group->leader->GetPosition());
                boostFactor /= FG_DISTANCE_NO_TARGET;
                if(boostFactor > followGroupCloseFactor + 0.1f)
                    boostFactor = followGroupCloseFactor + 0.1f;
                if(boostFactor < followGroupCloseFactor - 0.1f)
                    boostFactor = followGroupCloseFactor - 0.1f;
                if(boostFactor < followGroupCloseFactor)
                {
                    boostDir = 1;
                    boostFactor = (followGroupCloseFactor + 0.1f) - (boostFactor - (followGroupCloseFactor - 0.1f));
                }
                boostFactor = 1.f + ((boostFactor - (followGroupCloseFactor)) / 0.1f * 0.25f * boostDir);
            }
            mV *= boostFactor;
            mF *= boostFactor;

            if(isCloseLeader == false && distMagn < FG_DISTANCE_NO_TARGET && EntityManager::isSightOnWall(GetPosition(), group->leader->GetPosition()) == false)
            {
                isCloseLeader = true;
            }
            else if(isCloseLeader)
            {
                if((targetEntity == nullptr && distMagn > FG_DISTANCE_NO_TARGET && followGroupDelay <= 0) ||
                   (/*targetEntity != nullptr &&*/ distMagn > FG_DISTANCE) ||
                   (targetEntity == nullptr && EntityManager::isSightOnWall(GetPosition(), group->leader->GetPosition())))
                {
                    isCloseLeader = false;
                    targetEntity = nullptr;
                }
            }

            if(isCloseLeader == false)
            {
                attackTargetState = false;
                if(AStar::IsReadyToSearch(true) && pathFindDelay <= 0.f)
                {
                    MoveTo(group->leader->GetPosition());
                    if(movePoints.size() == 0 && GetGridPosition(GetPosition()) != GetGridPosition(group->leader->GetPosition()))
                    {
                        state = Idle;
                        LeaveGroup();
                        attackTargetState = true;
                        movePoints.clear();
                    }
                }

                if(mV > maxVelocity)
                    maxVelocity_ = mV;
                if(mF > maxForce)
                    maxForce_ = mF;
            }
            else
            {
                maxVelocity_ = mV;
                maxForce_ = mF;
                attackTargetState = true;

                if(targetEntity == nullptr)
                {
                    if(group->leader->GetShipTarget() != nullptr)
                        targetEntity = group->leader->GetShipTarget();
                    else
                    {
                        movePoints.clear();
                        if(MagnitudeSqureOfVector(group->leader->GetVelocity()) > (mV * mV) * 0.2f && MagnitudeSqureOfVector(velocity) <= MagnitudeSqureOfVector(group->leader->GetVelocity()) * boostFactor)
                            force = NormalizeVector(group->leader->GetVelocity()) * mF;
                    }
                }
            }
        }
    }
    return force;
}

void ShipUnit::ClearThinking()
{
    thinking_baseId.clear();
    thinking_closestPath.clear();
    isThinkingFindingPath = false;
    thinking_searchType = AStar::NoSearchType;
    thinking_state = -1;
}

void ShipUnit::IdleThinking(float dt)
{
    if(state == Idle && activePlayerUnit != this && thinkingWaitDuration <= 0.f)
    {
        sf::Vector2i grid = GetGridPosition(GetPosition());
        int gridBaseId = GetGridInfo(grid.y, grid.x).baseId;
        Base* gridBase = Game::GetBase(gridBaseId);

        if(hierarchy <= 1 && group != nullptr && group->leader != this)
            state = FollowingGroup;
        else if(hierarchy <= 1 ? (group != nullptr ? group->leader == this : false) && (gridBaseId >= 0 && gridBase->IsActive() ? gridBase->GetTeam() != team : true) : true)
        {
            if(thinking_baseId.size() > 0) //Initiate search
            {
                if(AStar::IsReadyToSearch(false) && pathFindDelay <= 0)
                {
                    isThinkingFindingPath = true;
                    temp_GridPath.clear();
                    sf::Vector2i baseGrid = Game::GetBase(*(thinking_baseId.begin()))->GetRandomBaseGrid();
                    while(thinking_baseId.size() > 0 && AStar::FindPath(baseGrid, GetGridPosition(GetPosition()), temp_GridPath, (AStar::SearchType)thinking_searchType, Game::GetTeamUpType(team)))
                    {
                        if(thinking_closestPath.size() > 0 ? temp_GridPath.size() < thinking_closestPath.size() : true)
                            thinking_closestPath.swap(temp_GridPath);
                        temp_GridPath.clear();

                        thinking_baseId.erase(thinking_baseId.begin());
                        if(thinking_baseId.size() > 0)
                        {
                            baseGrid = Game::GetBase(*(thinking_baseId.begin()))->GetRandomBaseGrid();
                            AStar::ContinueLastTargetSearch(baseGrid);
                        }
                    }
                }
            }
            else if(isThinkingFindingPath) //Handle found base if any
            {
                if(thinking_closestPath.size() > 0)
                {
                    short baseId = GetGridInfo(*(thinking_closestPath.begin())).baseId;
                    Base* base = Game::GetBase(baseId);
                    if(base != nullptr && base->IsActive())
                    {
                        temp_GridPath.clear();
                        for(int b = thinking_closestPath.size() - 1; b >= 0; b--)
                            temp_GridPath.push_back(thinking_closestPath[b]);

                        MoveTo(temp_GridPath);
                        primaryPath = movePoints;

                        if(thinking_state == Defending)
                        {
                            state = Defending;
                            defending_base = baseId;
                        }
                        else
                        {
                            state = Attacking;
                            attacking_base = baseId;
                        }
                    }
                    pathFindDelay = 1.5f;
                    ClearThinking();
                }
                else if(thinking_state == Defending && thinking_searchType == AStar::AvoidEnemySearchType)
                {
                    pathFindDelay = 1.5f;
                    isThinkingFindingPath = false;
                    thinking_searchType = AStar::NoSearchType;
                    defending_avoidEnemyBases = false;

                    for(const int& temp : Base::GetFrontLineBases(team))
                        thinking_baseId.push_back(temp);
                }
                else
                {
                    ClearThinking();
                    pathFindDelay = 1.5f;
                }
            }
            else //Make a decision
            {
                ClearThinking();
                bool isThereBasesToAttack = Base::GetFrontLineBasesEnemy(team).size() > 0;
                bool isThereBasesToDefend = Base::GetFrontLineBases(team).size() > 0;
                bool isOkayToAttack = (health >= GetActualHitpoint() * minAttackHealthFactor) && (group != nullptr ? group->members.size() >= minAttackGroupMembers : true);

                bool isInFrontLineDefendBase = false;
                for(auto& id : Base::GetFrontLineBases(team))
                    if(id == gridBaseId)
                    {
                        isInFrontLineDefendBase = true;
                        break;
                    }

                //Attack enemy base if in one and ready to attack
                if(gridBaseId >= 0 && gridBase->IsActive() && Game::GetTeamUpType(team) != Game::GetTeamUpType(gridBase->GetTeam()) && isOkayToAttack)
                {
                    attacking_tillDeath = GenerateRandomNumber(4);
                    attacking_base = gridBaseId;
                    state = Attacking;
                }
                //Attack front line enemy base if ready to attack
                else if(isThereBasesToAttack && (isOkayToAttack || isThereBasesToDefend == false))
                {
                    thinking_state = Attacking;
                    attacking_tillDeath = GenerateRandomNumber(4);
                    for(const int& temp : Base::GetFrontLineBasesEnemy(team))
                        thinking_baseId.push_back(temp);
                }
                //Defend a base and recover if in one
                else if(gridBaseId >= 0 && gridBase->IsActive() && isInFrontLineDefendBase)
                {
                    defending_avoidEnemyBases = false;
                    defending_base = gridBaseId;
                    defending_duration = 5.f;
                    state = Defending;
                }
                //Find a base to defend
                else if(isThereBasesToDefend)
                {
                    thinking_state = Defending;
                    defending_duration = 5.f;
                    defending_avoidEnemyBases = false;
                    if(health / GetActualHitpoint() < 0.3f)
                    {
                        defending_avoidEnemyBases = true;
                        thinking_searchType = AStar::AvoidEnemySearchType;
                    }

                    for(const int& temp : Base::GetFrontLineBases(team))
                        thinking_baseId.push_back(temp);
                }
                else thinkingWaitDuration = 3.f;
            }
        }
    }
}

int ShipUnit::GetAttackingBase()
{
    if(state == Attacking)
        return attacking_base;
    return -1;
}

void ShipUnit::AttackUpdate()
{
    if(state == Attacking)
    {
        sf::Vector2i gridPos = GetGridPosition(GetPosition());
        int gridBaseId = GetGridInfo(gridPos.y, gridPos.x).baseId;
        Base* base = Game::GetBase(attacking_base);
        Base* gridBase = Game::GetBase(gridBaseId);
        attackTargetState = true;

        if(attacking_base < 0 || base->IsActive() == false || Game::GetTeamUpType(base->GetTeam()) == Game::GetTeamUpType(team))
        {
            state = Idle;
            ClearThinking();
        }
        else if(gridBase != nullptr && gridBase->IsActive() && Game::GetTeamUpType(gridBase->GetTeam()) != Game::GetTeamUpType(team))
            attacking_base = gridBaseId;
        else if(gridBaseId != attacking_base)
        {
            if(AStar::IsReadyToSearch(false) && pathFindDelay <= 0 && (attackTargetState ? targetEntity == nullptr : true))
            {
                sf::Vector2i endGrid;
                if(movePoints.size() > 0)
                    endGrid = GetGridPosition(movePoints[movePoints.size() - 1]);

                if(movePoints.size() > 0 ? GetGridInfo(endGrid.y, endGrid.x).baseId != attacking_base : true)
                {
                    sf::Vector2f halfTileSize(MapTileManager::TILE_SIZE / 2.f, MapTileManager::TILE_SIZE / 2.f);
                    const std::vector<sf::Vector2i>& baseGrids = base->GetSpawnPoint()->spawnGrids;
                    MoveTo((sf::Vector2f)baseGrids[GenerateRandomNumber(baseGrids.size())] * (float)MapTileManager::TILE_SIZE + halfTileSize, AStar::NoSearchType, false);
                }
            }
        }
        else
        {
            if(attacking_tillDeath == false && health < GetActualHitpoint() * minAttackHealthFactor)
                state = Idle;
        }

        if(hierarchy > 1 && state == Attacking)
            Game::GetBase(attacking_base)->AskForReinforcement(team);
    }
}

void ShipUnit::DefendUpdate(float dt)
{
    if(state == Defending)
    {
        sf::Vector2i gridPos = GetGridPosition(GetPosition());
        int gridBaseId = GetGridInfo(gridPos.y, gridPos.x).baseId;
        Base* base = Game::GetBase(defending_base);

        if(base->IsActive() == false || (gridBaseId >= 0 && Game::GetTeamUpType(base->GetTeam()) != Game::GetTeamUpType(team)))
        {
            state = Idle;
        }
        else if(gridBaseId != defending_base)
        {
            if(AStar::IsReadyToSearch(false) && pathFindDelay <= 0 && (attackTargetState ? targetEntity == nullptr : true))
            {
                if(movePoints.size() > 0)
                {
                    sf::Vector2i endGrid = GetGridPosition(movePoints[movePoints.size() - 1]);
                    if(GetGridInfo(endGrid.y, endGrid.x).baseId != defending_base)
                    {
                        sf::Vector2f halfTileSize(MapTileManager::TILE_SIZE / 2.f, MapTileManager::TILE_SIZE / 2.f);
                        const std::vector<sf::Vector2i>& baseGrids = base->GetSpawnPoint()->spawnGrids;
                        MoveTo((sf::Vector2f)baseGrids[GenerateRandomNumber(baseGrids.size())] * (float)MapTileManager::TILE_SIZE + halfTileSize, defending_avoidEnemyBases ? AStar::AvoidEnemySearchType : AStar::NoSearchType, false);
                    }
                }
                else
                {
                    sf::Vector2f halfTileSize(MapTileManager::TILE_SIZE / 2.f, MapTileManager::TILE_SIZE / 2.f);
                    const std::vector<sf::Vector2i>& baseGrids = base->GetSpawnPoint()->spawnGrids;
                    MoveTo((sf::Vector2f)baseGrids[GenerateRandomNumber(baseGrids.size())] * (float)MapTileManager::TILE_SIZE + halfTileSize, defending_avoidEnemyBases ? AStar::AvoidEnemySearchType : AStar::NoSearchType, false);
                }

                if(movePoints.size() == 0 && AStar::GetProgressionState() == false)
                    state = Idle;
            }
        }
        else
        {
            attackTargetState = true;

            if(defending_duration > 0 && base->IsEnemyInBase() == false)
                defending_duration -= dt;

            if(movePoints.size() > 0)
            {
                sf::Vector2i endGrid = GetGridPosition(movePoints[movePoints.size() - 1]);
                if(GetGridInfo(endGrid.y, endGrid.x).baseId != defending_base)
                    movePoints.clear();
            }

            if(hierarchy <= 1)
            {
                if(defending_duration <= 0)
                    state = Idle;
            }
            else if(defending_duration <= 0 && base->IsEnemyInBase() == false && (group != nullptr ? group->members.size() >= MAX_GROUP_MEMBERS : true) &&
               health >= GetActualHitpoint() && Base::GetFrontLineBasesEnemy(team).size() > 0)
            {
                state = Idle;
            }
        }
    }
}

void ShipUnit::TravelUpdate(float dt)
{
    if(state == Traveling)
    {
        if(targetEntity == nullptr)
        {
            bool isSightOnSmallWall = false;
            float distMagnSqr = MagnitudeSqureOfVector(GetPosition() - traveling_destination);
            if(traveling_updateTime <= 0.f && EntityManager::isSightOnWall(GetPosition(), traveling_destination, isSightOnSmallWall) == false && isSightOnSmallWall == false)
            {
                traveling_updateTime = 0.1f;

                if(distMagnSqr <= traveling_stopRadius * traveling_stopRadius)
                {
                    traveling_stopDuration -= dt;
                    if(traveling_stopDuration <= 0.f)
                    {
                        state = Idle;
                        thinkingWaitDuration = 1.f;
                    }
                }
                if(distMagnSqr <= sizeRadius * sizeRadius)
                {
                    state = Idle;
                    thinkingWaitDuration = 1.f;
                }
            }

            if(movePoints.size() > 0 && GetGridPosition(movePoints[movePoints.size() - 1]) == GetGridPosition(traveling_destination) && movePoints[movePoints.size() - 1] != traveling_destination)
                movePoints[movePoints.size() - 1] = traveling_destination;

            if(AStar::IsReadyToSearch(false) && distMagnSqr > sizeRadius * sizeRadius && (movePoints.size() > 0 ? GetGridPosition(movePoints[movePoints.size() - 1]) != GetGridPosition(traveling_destination) : true))
                MoveTo(traveling_destination, AStar::NoSearchType, false, false);
        }
    }
}

sf::Vector2i ShipUnit::GetTravellingGrid()
{
    return GetGridPosition(traveling_destination);
}

void ShipUnit::ReclaimToBase(int baseId)
{
    reclaimBase = baseId;
    isTransmitingSpawn = false;
    if(group != nullptr && group->leader == this)
        UnitGroup::DeleteGroup(group);
}

void ShipUnit::Attack(int base_)
{
    ClearThinking();

    state = Attacking;
    attacking_base = base_;
}

void ShipUnit::Defend(int base_, bool avoidEnemyBases_, float defendDuration_)
{
    ClearThinking();

    state = Defending;
    defending_base = base_;
    defending_avoidEnemyBases = avoidEnemyBases_;
    defending_duration = defendDuration_;
}

void ShipUnit::Travel(sf::Vector2f destination_, bool shouldAttackInTravel_, float stopRadius_)
{
    sf::Vector2i destGrid = GetGridPosition(destination_);
    if(destGrid.x >= 0 && destGrid.y >= 0 && destGrid.x < MapTileManager::MAX_MAPTILE && destGrid.y < MapTileManager::MAX_MAPTILE &&
       true)//GetGridInfoTile(destGrid.y, destGrid.x).type <= MapTileManager::Floor)
    {
        ClearThinking();
        state = Traveling;

        //Left
        float minX = MapTileManager::TILE_SIZE * destGrid.x;
        sf::Vector2i tempGrid(destGrid.x - 1, destGrid.y);
        if(tempGrid.x >= 0 && tempGrid.y >= 0 && tempGrid.x < MapTileManager::MAX_MAPTILE && tempGrid.y < MapTileManager::MAX_MAPTILE &&
           GetGridInfoTile(tempGrid.y, tempGrid.x).type > MapTileManager::Floor)
           minX += sizeRadius / 2.f;

        //Right
        float maxX = MapTileManager::TILE_SIZE * destGrid.x + MapTileManager::TILE_SIZE;
        tempGrid = sf::Vector2i(destGrid.x + 1, destGrid.y);
        if(tempGrid.x >= 0 && tempGrid.y >= 0 && tempGrid.x < MapTileManager::MAX_MAPTILE && tempGrid.y < MapTileManager::MAX_MAPTILE &&
           GetGridInfoTile(tempGrid.y, tempGrid.x).type > MapTileManager::Floor)
           maxX +=  -sizeRadius / 2.f;

        //Top
        float minY = MapTileManager::TILE_SIZE * destGrid.y;
        tempGrid = sf::Vector2i(destGrid.x, destGrid.y - 1);
        if(tempGrid.x >= 0 && tempGrid.y >= 0 && tempGrid.x < MapTileManager::MAX_MAPTILE && tempGrid.y < MapTileManager::MAX_MAPTILE &&
           GetGridInfoTile(tempGrid.y, tempGrid.x).type > MapTileManager::Floor)
           minY += sizeRadius / 2.f;

        //Bottom
        float maxY = MapTileManager::TILE_SIZE * destGrid.y + MapTileManager::TILE_SIZE;
        tempGrid = sf::Vector2i(destGrid.x, destGrid.y + 1);
        if(tempGrid.x >= 0 && tempGrid.y >= 0 && tempGrid.x < MapTileManager::MAX_MAPTILE && tempGrid.y < MapTileManager::MAX_MAPTILE &&
           GetGridInfoTile(tempGrid.y, tempGrid.x).type > MapTileManager::Floor)
           maxY += -sizeRadius / 2.f;

        if(destination_.x < minX)
            destination_.x = minX;
        else if(destination_.x > maxX)
            destination_.x = maxX;
        if(destination_.y < minY)
            destination_.y = minY;
        else if(destination_.y > maxY)
            destination_.y = maxY;

        attackTargetState = shouldAttackInTravel_;
        traveling_destination = destination_;
        traveling_stopRadius = stopRadius_;
        traveling_stopDuration = 2.f;
    }
}

void ShipUnit::CancleCurrentAction()
{
    ClearThinking();
    state = Idle;
    movePoints.clear();
}

float ShipUnit::GetNonTransmittingAngle()
{
    return nonTransmittingAngle;
}

bool ShipUnit::IsActivePlayerAiming()
{
    if(activePlayerUnit != nullptr)
        return isActivePlayerAiming;
    return false;
}

bool ShipUnit::IsActivePlayerAiming(float& aimDistMagn)
{
    if(activePlayerUnit != nullptr)
    {
        aimDistMagn = activePlayerAimDistMagn;
        return isActivePlayerAiming;
    }
    aimDistMagn = 0.f;
    return false;
}

bool ShipUnit::IsActivePlayerManualAiming()
{
    if(activePlayerUnit != nullptr)
        return isActivePlayerManualAiming;
    return false;
}

bool ShipUnit::IsActivePlayerJustShooting()
{
    if(activePlayerUnit != nullptr)
        return isActivePlayerJustShooting;
    return false;
}

const int& ShipUnit::GetSurroundingTroops()
{
    return surroundingTroops;
}

void ShipUnit::SpawnSurroundingTroops()
{
    if(hierarchy > 1 && (lastSurroudingTroopSpawn <= 0 || surroundTroopsFavourTime <= 0) && surroundingTroops > 0 && targetEntity != nullptr)
    {
        //Get ready, cause this shit is about to twist your brains like a freshly cooked hot noodles
        int enemyDensityDiff = 0;
        int actualEnemyDensityDiff = 0;
        sf::Vector2i spawnGrid;
        sf::Vector2i grid = GetGridPosition(GetPosition());
        int gridBaseId = GetGridInfo(grid).baseId;
        Ship* startTarget = nullptr;
        bool isFavouring = surroundTroopsFavourTime <= 0;
        int enemiesAround = 0;

        /*if(gridBaseId >= 0 && Game::GetBase(gridBaseId)->IsActive() && GetGridInfo(grid).baseArea)
        {
            if(Game::GetTeamUpType(Game::GetBase(gridBaseId)->GetTeam()) == Game::GetTeamUpType(team))
            {
                enemyDensityDiff = Game::GetBase(gridBaseId)->GetEnemyDensityDifference();
                actualEnemyDensityDiff = Game::GetBase(gridBaseId)->GetEnemyDensityDifference(false);
                enemiesAround = Game::GetBase(gridBaseId)->GetEnemyDensity();
            }
            else
            {
                enemyDensityDiff = Game::GetBase(gridBaseId)->GetFriendlyDensityDifference();
                actualEnemyDensityDiff = Game::GetBase(gridBaseId)->GetFriendlyDensityDifference(false);
                enemiesAround = Game::GetBase(gridBaseId)->GetFriendlyDensity();
            }

            Base* base = Game::GetBase(gridBaseId);
            std::vector<Ship*> enemiesInBase;
            for(int a = 0; a < Game::NumOfTeams; a++)
                if(Game::GetTeamUpType(team) != Game::GetTeamUpType(a))
                {
                    const std::vector<Ship*>* unitsInBase = base->GetUnitsInBase(a);
                    enemiesInBase.insert(enemiesInBase.begin(), unitsInBase->begin(), unitsInBase->end());
                }

            if(enemiesInBase.size() > 0)
            {
                sf::Vector2i refGrid = GetGridPosition(enemiesInBase[GenerateRandomNumber(enemiesInBase.size())]->GetPosition());

                int arrayRange = 7;
                bool isChecked[arrayRange][arrayRange] = {};
                sf::Vector2i offSetGrid = refGrid - sf::Vector2i(3, 3);
                std::vector<sf::Vector2i> gridsToCheck;
                std::vector<sf::Vector2i> spawnGrids;
                sf::Vector2i offSet;

                gridsToCheck.push_back(refGrid);
                offSet = refGrid - offSetGrid;
                isChecked[offSet.y][offSet.x] = true;
                spawnGrids.push_back(refGrid);

                for(int i = 0; i < gridsToCheck.size(); i++)
                {
                    for(int y = gridsToCheck[i].y - 1; y <= gridsToCheck[i].y + 1; y++)
                        for(int x = gridsToCheck[i].x - 1; x <= gridsToCheck[i].x + 1; x++)
                            if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE && GetGridInfo(y, x).baseId == gridBaseId &&
                               GetGridInfoTile(y, x).type != MapTileManager::Wall && GetGridInfoTile(y, x).type != MapTileManager::SmallWall)
                            {
                                offSet = sf::Vector2i(x, y) - offSetGrid;
                                if(offSet.x >= 0 && offSet.y >= 0 && offSet.x < arrayRange && offSet.y < arrayRange && isChecked[offSet.y][offSet.x] == false)
                                {
                                    gridsToCheck.push_back(sf::Vector2i(x, y));
                                    isChecked[offSet.y][offSet.x] = true;
                                    spawnGrids.push_back(sf::Vector2i(x, y));
                                }
                            }
                }

                if(spawnGrids.size() > 0)
                    spawnGrid = spawnGrids[GenerateRandomNumber(spawnGrids.size())];
            }
            else
            {
                const std::vector<sf::Vector2i>& spawnGrids = base->GetSpawnPoint()->spawnGrids;
                if(spawnGrids.size() > 0)
                    spawnGrid = spawnGrids[GenerateRandomNumber(spawnGrids.size())];
            }
        }
        else*/
        {
            int range = SHIP_SIGTH_RANGE / MapTileManager::TILE_SIZE;
            int arrayRange = range * 2 + 1;
            bool isChecked[arrayRange][arrayRange] = {};
            std::vector<sf::Vector2i> gridsToCheck;
            std::vector<Ship*> shipUnits;
            sf::Vector2i offSetGrid = grid - sf::Vector2i(range, range);
            short teamUp = Game::GetTeamUpType(team);
            int mainFriendlyDensity = 0;
            int mainEnemyDensity = 0;

            gridsToCheck.push_back(grid);
            isChecked[grid.y - offSetGrid.y][grid.x - offSetGrid.x] = true;

            for(int i = 0; i < gridsToCheck.size(); i++)
            {
                for(int y = gridsToCheck[i].y - 1; y <= gridsToCheck[i].y + 1; y++)
                    for(int x = gridsToCheck[i].x - 1; x <= gridsToCheck[i].x + 1; x++)
                        if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE && GetGridInfoTile(y, x).type != MapTileManager::Wall)
                        {
                            sf::Vector2i offSet = sf::Vector2i(x, y) - offSetGrid;
                            if(offSet.x >= 0 && offSet.y >= 0 && offSet.x < arrayRange && offSet.y < arrayRange &&
                               (i == 0 && gridsToCheck[i] == sf::Vector2i(x, y) ? true : isChecked[offSet.y][offSet.x] == false))
                            {
                                if(i == 0 && gridsToCheck[i] == sf::Vector2i(x, y) ? false : true)
                                {
                                    gridsToCheck.push_back(sf::Vector2i(x, y));
                                    isChecked[offSet.y][offSet.x] = true;
                                }
                                const std::vector<Ship*>& tempShips = GetGridInfo(y, x).positionShips;

                                for(auto& ship : tempShips)
                                {
                                    bool hasTarget = ship->GetShipTarget() != nullptr;
                                    if(hasTarget == false && ship->GetHierarchy() <= 1 && ship->GetGroup() != nullptr && ship->GetGroup()->leader != nullptr)
                                        hasTarget = ship->GetGroup()->leader->GetShipTarget() != nullptr;

                                    if(hasTarget)
                                    {
                                        if(teamUp != Game::GetTeamUpType(ship->GetTeam()))
                                        {
                                            shipUnits.push_back(ship);
                                            mainEnemyDensity += ship->GetShipDensityValue();
                                        }
                                        else mainFriendlyDensity += ship->GetShipDensityValue();
                                    }
                                }
                            }
                        }
            }

            enemiesAround = shipUnits.size();

            sf::Vector2i gridOfInterest;
            int minDiffDensity = -1;
            for(int tries = 0; tries < 10 && shipUnits.size() > 0; tries++)
            {
                int index = GenerateRandomNumber(shipUnits.size());
                int tempEnemyDensity = 0;
                int tempFriendlyDensity = 0;
                int tempDiffDensity = 0;
                Ship* tempShip = shipUnits[index];
                sf::Vector2i tempGrid = GetGridPosition(tempShip->GetPosition());
                std::vector<sf::Vector2i> tempSpawnGrids;
                shipUnits.erase(shipUnits.begin() + index);

                sf::Vector2i tempOffSetGrid = tempGrid - sf::Vector2i(range / 2, range / 2);
                int tempArrayRange = (range * 0.3f) * 2 + 1;
                for(int y = 0; y < tempArrayRange; y++)
                    for(int x = 0; x < tempArrayRange; x++)
                        isChecked[y][x] = false;

                gridsToCheck.clear();
                gridsToCheck.push_back(tempGrid);
                isChecked[tempGrid.y - tempOffSetGrid.y][tempGrid.x - tempOffSetGrid.x] = true;
                for(int i = 0; i < gridsToCheck.size(); i++)
                {
                    //tempSpawnGrids.clear();

                    for(int y = gridsToCheck[i].y - 1; y <= gridsToCheck[i].y + 1; y++)
                        for(int x = gridsToCheck[i].x - 1; x <= gridsToCheck[i].x + 1; x++)
                            if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE && GetGridInfoTile(y, x).type != MapTileManager::Wall)
                            {
                                sf::Vector2i offSet = sf::Vector2i(x, y) - tempOffSetGrid;
                                if(offSet.x >= 0 && offSet.y >= 0 && offSet.x < tempArrayRange && offSet.y < tempArrayRange &&
                                   (i == 0 && gridsToCheck[i] == sf::Vector2i(x, y) ? true : isChecked[offSet.y][offSet.x] == false))
                                {
                                    if(GetGridInfoTile(y, x).type != MapTileManager::SmallWall)
                                        tempSpawnGrids.push_back(sf::Vector2i(x, y));
                                    if(i == 0 && gridsToCheck[i] == sf::Vector2i(x, y) ? false : true)
                                    {
                                        gridsToCheck.push_back(sf::Vector2i(x, y));
                                        isChecked[offSet.y][offSet.x] = true;
                                    }
                                    const std::vector<Ship*>& tempShips = GetGridInfo(y, x).positionShips;

                                    for(auto& ship : tempShips)
                                    {
                                        if(teamUp == Game::GetTeamUpType(ship->GetTeam()))
                                            tempFriendlyDensity += ship->GetShipDensityValue();
                                        else tempEnemyDensity += ship->GetShipDensityValue();
                                    }
                                }
                            }

                }

                tempDiffDensity = tempEnemyDensity - tempFriendlyDensity;
                /*if(tempDiffDensity < 0)
                    tempDiffDensity = 0;*/

                if(tempDiffDensity < minDiffDensity || minDiffDensity == -1)
                {
                    minDiffDensity = tempDiffDensity;
                    gridOfInterest = tempGrid;
                    startTarget = tempShip;
                    if(tempSpawnGrids.size() > 0)
                        spawnGrid = tempSpawnGrids[GenerateRandomNumber(tempSpawnGrids.size())];
                    else spawnGrid = tempGrid;
                }
            }

            enemyDensityDiff = mainEnemyDensity - mainFriendlyDensity;
            actualEnemyDensityDiff = enemyDensityDiff;
            if(enemyDensityDiff < 0)
                enemyDensityDiff = 0;
        }

        if(isFavouring)
            surroundTroopsFavourTime = SURROUNDING_TROOP_FAVOUR_TIME + SURROUNDING_TROOP_FAVOUR_TIME_CHANGE * GenerateRandomNumber(101) / 100.f;
        if(isFavouring && (enemiesAround >= 10 || actualEnemyDensityDiff < 0))
            surroundTroopsFavourTime /= 2.f;

        //Spawning time
        if(enemyDensityDiff > 0 || (isFavouring && enemiesAround < 10 && actualEnemyDensityDiff >= 0))
        {
            const Game::DifficultyInfo& difficulty = Game::GetDifficultyInfo();

            ShipUnit::UnitType type = ShipUnit::MiniShip;
            Game::SkinType skinType = Game::SkinMiniShip;
            int attributeValue = difficulty.miniShip;
            if(GenerateRandomNumber(101) <= Base::MICROSHIP_SPAWN_RATIO)
            {
                type = ShipUnit::MicroShip;
                skinType = Game::SkinMicroShip;
                attributeValue = difficulty.microShip;
            }

            sf::Vector2f shipPos = (sf::Vector2f)(spawnGrid * MapTileManager::TILE_SIZE) + sf::Vector2f(GenerateRandomNumber(MapTileManager::TILE_SIZE), GenerateRandomNumber(MapTileManager::TILE_SIZE));
            ShipUnit* unit = new ShipUnit(type, shipPos, team, attributeValue, attributeValue, attributeValue, 50 + GenerateRandomNumber(30), 0,
                                          GenerateRandomNumber(Game::GetSkinSize(skinType, team)), skinType);
            if(startTarget != nullptr)
                unit->SetTarget(startTarget);
            EntityManager::CreateShip(unit, false);

            //Group members
            int numOfMembers = GenerateRandomNumber(MAX_GROUP_MEMBERS + 1);
            if(isFavouring == false)
            {
                if(numOfMembers > enemyDensityDiff - 1)
                    numOfMembers = enemyDensityDiff - 1;
                if(numOfMembers > surroundingTroops - 1)
                    numOfMembers = surroundingTroops - 1;
            }

            if(team == 0) std::cout << "isFavouring\n";

            surroundingTroops -= numOfMembers + 1;
            if(surroundingTroops < 0)
                surroundingTroops = 0;

            if(team == 0)
                std::cout << surroundingTroops << " troops\n";

            if(numOfMembers > 0)
            {
                Ship::UnitGroup* refGroup = Ship::UnitGroup::CreatUnitGroups();
                unit->JoinGroup(refGroup, true);

                sf::Vector2i leaderGrid = GetGridPosition(refGroup->leader->GetPosition());
                std::vector<sf::Vector2i> spawnGrids;
                for(int y = -1; y <= 1; y++)
                    for(int x = -1; x <= 1; x++)
                        if(leaderGrid.y + y >= 0 && leaderGrid.y + y < MapTileManager::MAX_MAPTILE &&
                           leaderGrid.x + x >= 0 && leaderGrid.x + x < MapTileManager::MAX_MAPTILE)
                        {
                            MapTileManager::TileType type = MapTileManager::GetTile(GetGridInfo(leaderGrid.y + y, leaderGrid.x + x).tileId).type;
                            if(type != MapTileManager::Wall && type != MapTileManager::SmallWall)
                                spawnGrids.push_back(sf::Vector2i(leaderGrid.x + x, leaderGrid.y + y));
                        }

                if(spawnGrids.size() > 0)
                {
                    for(int a = 0; a < numOfMembers; a++)
                    {
                        type = ShipUnit::MiniShip;
                        skinType = Game::SkinMiniShip;
                        attributeValue = difficulty.miniShip;
                        if(GenerateRandomNumber(101) <= Base::MICROSHIP_SPAWN_RATIO)
                        {
                            type = ShipUnit::MicroShip;
                            skinType = Game::SkinMicroShip;
                            attributeValue = difficulty.microShip;
                        }

                        shipPos = ((sf::Vector2f)(spawnGrids[GenerateRandomNumber(spawnGrids.size())] * MapTileManager::TILE_SIZE) + sf::Vector2f(GenerateRandomNumber(MapTileManager::TILE_SIZE), GenerateRandomNumber(MapTileManager::TILE_SIZE)));
                        unit = new ShipUnit(type, shipPos, team, attributeValue, attributeValue, attributeValue, 50 + GenerateRandomNumber(30), 0,
                                                      GenerateRandomNumber(Game::GetSkinSize(skinType, team)), skinType);
                        EntityManager::CreateShip(unit, false);
                        unit->JoinGroup(refGroup);
                    }
                }
            }
        }
    }

    if(lastSurroudingTroopSpawn <= 0)
        lastSurroudingTroopSpawn = SURROUNDING_TROOP_SPAWN_TIME + 2.f * GenerateRandomNumber(101) / 100.f;
}
