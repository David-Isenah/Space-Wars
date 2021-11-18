#include "shipdefence.h"
#include "shipunit.h"
#include "bullet.h"
#include "skin.h"
#include "game.h"
#include "AssetManager.h"
#include "maptilemanager.h"
#include "public_functions.h"
#include <iostream>

const int TARGET_RANGE_MINI_DEFENSE = 1200;
const int TARGET_RANGE_MAIN_DEFENSE = 1500;

int shipDefenceBarLength[ShipDefence::NumOfDefenceShipType] = {180, 100};

std::vector<MultiDrawer> multidrawer_shipdefense[ShipDefence::NumOfDefenceShipType][Game::NumOfTeams];

void ShipDefence::InitialiseMultiDrawer()
{
    //Resize
    for(int a = 0; a < Game::NumOfTeams; a++)
        multidrawer_shipdefense[MainDefenseShip][a].resize(Game::GetSkinSize(Game::SkinMainDefence, a));

    for(int a = 0; a < Game::NumOfTeams; a++)
        multidrawer_shipdefense[MiniDefenseShip][a].resize(Game::GetSkinSize(Game::SkinMiniDefence, a));

    //Setting Texture
    for(int a = 0; a < Game::NumOfTeams; a++)
        for(int b = 0; b < Game::GetSkinSize(Game::SkinMainDefence, a); b++)
            multidrawer_shipdefense[MainDefenseShip][a][b].SetTexture(&Game::GetSkin(Game::SkinMainDefence, b, a)->texture);

    for(int a = 0; a < Game::NumOfTeams; a++)
        for(int b = 0; b < Game::GetSkinSize(Game::SkinMiniDefence, a); b++)
            multidrawer_shipdefense[MiniDefenseShip][a][b].SetTexture(&Game::GetSkin(Game::SkinMiniDefence, b, a)->texture);
}

ShipDefence::ShipDefence(DefenceType _type, sf::Vector2f position_, short team_, int hitPoint_, int attack_, int defense_, int skinIndex_, bool isSpawning_) :
    defenceType(_type),
    multiDrawer(nullptr),
    lastBulletFromHigherUnits(nullptr),
    lastBulletFromHigherUnits_time(0)
{
    team = team_;
    shipType = Ship::DefenceShip;
    exp = GetSharedExp(team_);
    health = hitPoint_;

    hitPoint = hitPoint_;
    attack = attack_;
    defense = defense_;
    isTransmitingSpawn = isSpawning_;
    barLength = shipDefenceBarLength[_type];

    if(defenceType == MiniDefenseShip)
    {
        shipDensityValue = 6;
        targetRange = TARGET_RANGE_MINI_DEFENSE;
        dodgeRadius = 69.f;
        skin = Game::GetSkin(Game::SkinMiniDefence, skinIndex_, team_);
    }
    else
    {
        shipDensityValue = 10;
        targetRange = TARGET_RANGE_MAIN_DEFENSE;
        dodgeRadius = 138.f;
        skin = Game::GetSkin(Game::SkinMainDefence, skinIndex_, team_);
    }

    multiDrawer = &multidrawer_shipdefense[_type][team_][skinIndex_];

    if(skin != nullptr && skin->points.size() > 0)
        transformation.setOrigin(skin->points[0]);
    transformation.setScale(0.5f, 0.5f);
    transformation.setPosition(position_);

    if(skin != nullptr)
    {
        sizeRadius = skin->collisionRect.width;
        if(skin->collisionRect.height > skin->collisionRect.width)
            sizeRadius = skin->collisionRect.height;
        sizeRadius *= 0.5f * 0.5f;
    }
    barHeight = sizeRadius + 10.f;

    if(exp != nullptr)
    {
        currentLevel = 1;
        for(int a = 3; a >= 0; a--)
            if(*exp >= BaseLevelExpShared[a])
            {
                currentLevel += a + 1;
                break;
            }
    }
    RefreshRealTimeProperties();
}

sf::FloatRect ShipDefence::GetBoundingRect()
{
    if(skin != nullptr)
        return transformation.getTransform().transformRect(skin->collisionRect);
    return sf::FloatRect();
}

 void ShipDefence::GetPoints(std::vector<sf::Vector2f>& points)
 {
     if(skin != nullptr)
     {
         points.clear();

         sf::Transform transform_ = transformation.getTransform();
         for(int a = 0; a < skin->points.size(); a++)
            points.push_back(transform_.transformPoint(skin->points[a]));
     }
 }

int ShipDefence::GetLevelBaseExp()
{
    if(currentLevel >= 2 && currentLevel <= 5)
        return BaseLevelExpShared[currentLevel - 2];
    return 0;
}

int ShipDefence::GetNextLevelBaseExp()
{
    if(currentLevel >= 1 && currentLevel < 5)
        return BaseLevelExpShared[currentLevel - 1];
    return 0;
}

void ShipDefence::GetDamagedBy(float damage, sf::Vector2f hitPosition, Ship* shooter)
{
    if(shooter != nullptr && shooter->GetShipType() == UnitShip && static_cast<ShipUnit*>(shooter)->GetUnitType() < ShipUnit::MiniShip)
    {
        lastBulletFromHigherUnits = shooter;
        lastBulletFromHigherUnits_time = 3.f;
    }
    Ship::GetDamagedBy(damage, hitPosition, shooter);
}

void ShipDefence::GainKillPoints(const int& exp_, const float& special_, const float& powerFactor_)
{
    if(exp != nullptr && currentLevel < 5)
        *exp += exp_;
}

void ShipDefence::GiveKillPoints(Ship* shooter)
{
    float gainedExp = 0.f;
    float gainedSpecial = 0.f;
    float powerFactor = actualPowerScore / shooter->GetActualPowerScore();

    if(defenceType == ShipDefence::MainDefenseShip)
    {
        gainedExp = 200.f * (powerFactor / 0.3f);
        gainedSpecial = 10.f * (powerFactor / 0.3f);
    }
    else
    {
        gainedExp = 100.f * (powerFactor / 0.3f);
        gainedSpecial = 5.f * (powerFactor / 0.3f);
    }

    if(lastBulletFromHigherUnits != nullptr)
        lastBulletFromHigherUnits->GainKillPoints(gainedExp, gainedSpecial, powerFactor);
    if(shooter != nullptr)
        *GetSharedExp(shooter->GetTeam()) += gainedExp;
}

 sf::Vector2f ShipDefence::GetPosition()
 {
     return transformation.getPosition();
 }

bool ShipDefence::ResolveWallCollision(sf::FloatRect& wallRect, const float& dt)
{
    return false;
}

void ShipDefence::Shoot()
{
    if(skin != nullptr)
    {
        sf::Vector2f shootPoint = transformation.getTransform().transformPoint(skin->shootPoint);
        float bulletScale = 0.35f;
        if(defenceType == MiniDefenseShip)
            bulletScale = 0.25f;

        //Shooting
        bool justShot = false;
        if(currentGun >= 0 && currentGun < Gun::NumOfGuns && gunMagazine > 0)
            if(Gun::Shoot(currentGun, shootPoint, sf::Vector2f(), team, angle, attack, lastShotTime, bulletScale, gunMagazine, this))
            {
                reloading = false;
                justShot = true;
            }
    }
}

void ShipDefence::Update(float dt)
{
    lastShotTime += dt;
    lastTargetUpdateTime -= dt;
    if(lastHitTime > 0)
        lastHitTime -= dt;
    if(reloadingTime > 0)
        reloadingTime -= dt;
    if(barTransparencyTime > 0)
        barTransparencyTime -= dt;
    if(lastBulletFromHigherUnits_time > 0 || lastBulletFromHigherUnits != nullptr)
    {
        lastBulletFromHigherUnits_time -= dt;
        if(lastBulletFromHigherUnits_time <= 0)
            lastBulletFromHigherUnits = nullptr;
    }

    if(alive)
    {
        if(targetEntity != nullptr)
            AimAndShoot(dt);

        if(targetEntity != nullptr)// targetEntity != nullptr is necessary since AimAndShoot(dt) can change targetEntity
        {
            sf::Vector2i myGrid = GetGridPosition(GetPosition());
            sf::Vector2i targetGrid = GetGridPosition(targetEntity->GetPosition());
            int targetGridBase = GetGridInfo(targetGrid.y, targetGrid.x).baseId;
            int myGridBase = GetGridInfo(myGrid.y, myGrid.x).baseId;
            bool myInBaseArea = GetGridInfo(myGrid.y, myGrid.x).baseArea;
            bool targetInBaseArea = GetGridInfo(targetGrid.y, targetGrid.x).baseArea;
            if(targetGridBase >= 0 && targetInBaseArea && (myGridBase != targetGridBase || myInBaseArea == false))
                Game::GetBase(targetGridBase)->AddShipDensity(this);
        }

        if(currentLevel < 5 && exp != nullptr && *exp >= GetNextLevelBaseExp())
        {
            if(exp != nullptr)
            {
                currentLevel = 1;
                for(int a = 3; a >= 0; a--)
                    if(*exp >= BaseLevelExpShared[a])
                    {
                        currentLevel += a + 1;
                        break;
                    }

                if(currentLevel == 5)
                    *exp = BaseLevelExpShared[3];
            }
            RefreshRealTimeProperties();
        }

        HandleGunReloading();
    }
    transformation.setRotation(GetAngle());

    unsigned short transparency = 255;
    sf::Color color = sf::Color::White;
    if(isTransmitingSpawn)
        transparency = GetSpawnTransitionFactor(dt) * 255;
    if(lastHitTime > 0)
        color = sf::Color(255, 150, 150);

    if(multiDrawer != nullptr && multiDrawer->GetTexture() != nullptr)
    {
        for(int a = 0; a < 4; a++)
        {
            vertices[a].color = color;
            vertices[a].color.a = transparency;
        }

        sf::Vector2f sizeTextr = (sf::Vector2f)multiDrawer->GetTexture()->getSize();
        vertices[0].position = transformation.getTransform().transformPoint(sf::Vector2f(0, 0));
        vertices[1].position = transformation.getTransform().transformPoint(sf::Vector2f(sizeTextr.x, 0));
        vertices[2].position = transformation.getTransform().transformPoint(sf::Vector2f(sizeTextr.x, sizeTextr.y));
        vertices[3].position = transformation.getTransform().transformPoint(sf::Vector2f(0, sizeTextr.y));

        vertices[0].texCoords = sf::Vector2f(0, 0);
        vertices[1].texCoords = sf::Vector2f(sizeTextr.x, 0);
        vertices[2].texCoords = sf::Vector2f(sizeTextr.x, sizeTextr.y);
        vertices[3].texCoords = sf::Vector2f(0, sizeTextr.y);

        multiDrawer->AddVertex(vertices, 4);
    }

    /*sf::Vector2i grid = GetGridPosition(GetPosition());
    int baseId = GetGridInfo(grid.y, grid.x).baseId;
    if(baseId >= 0 && baseId < Game::GetNumberOfBases())
        Game::GetBase(baseId)->AddUnit(this);*/
    UpdateHealthBar();
}

void ShipDefence::Draw(sf::RenderTarget& target)
{
    for(int a = 0; a < NumOfDefenceShipType; a++)
        for(int b = 0; b < Game::NumOfTeams; b++)
            for(int c = 0; c < multidrawer_shipdefense[a][b].size(); c++)
                multidrawer_shipdefense[a][b][c].Draw(target);
}

 void ShipDefence::AddToGridBase()
{
    sf::Vector2i grid = GetGridPosition(GetPosition());
    int baseId = GetGridInfo(grid.y, grid.x).baseId;
    if(baseId >= 0 && baseId < Game::GetNumberOfBases())
        Game::GetBase(baseId)->AddUnit(this);
}

void ShipDefence::JoinGroup(UnitGroup* group_, bool asLeader)
{
}

ShipDefence::DefenceType ShipDefence::GetDefenceType()
{
    return defenceType;
}

void ShipDefence::RefreshEntities()
{
    Ship::RefreshEntities();
    if(lastBulletFromHigherUnits != nullptr && lastBulletFromHigherUnits->GetAlive() == false)
    {
        lastBulletFromHigherUnits = nullptr;
        lastBulletFromHigherUnits_time = 0;
    }
}
