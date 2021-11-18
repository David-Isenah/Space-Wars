#include "SFML/Graphics.hpp"
#include <iostream>
#include <iterator>
#include <assert.h>
#include <sstream>
#include <math.h>
#include "game.h"
#include "ship.h"
#include "public_functions.h"
#include "collision.h"
#include "displayeffect.h"
#include "astarpathfinding.h"

const float EntityManager::PL_ZOOM_FACTOR = 1.25f;
const float LV_ZOOM_MAX = 3.f;
const float LV_ZOOM_MIN = 1.f;
const sf::Vector2f EntityManager::PL_AIM_CAMERA_LIMIT_FACTOR = sf::Vector2f(0.8f, 0.8f);
const sf::Vector2f EntityManager::PL_CAMERA_LIMIT_FACTOR = sf::Vector2f(0.6f, 0.8f); //Fix later
const int TARGET_UDATE_PER_FRAME = 15;
const int BULLET_SHIP_COLLISION_LIMIT = 400;
const int CLOSE_MEMBER_RANGE = 4;

Ship* cameraPoint = nullptr;
float cameraScale = EntityManager::PL_ZOOM_FACTOR;
float cameraRealScale = EntityManager::PL_ZOOM_FACTOR;
float cameraLastAimSensitivity = 0.3f;
sf::FloatRect lastCameraRect;
sf::Vector2f cameraEaseInPosition;
sf::Vector2f cameraRealPosition;
sf::Vector2f cameraPeakPosition;
bool cameraQuickPeak = false;
float commandModeWait = 0;
float commandModeLastScale = 0.f;
std::vector<Entity*> entitiesToDelete;
std::vector<sf::Vector2i> shipGridsToClear;
std::vector<sf::Vector2i> positionShipGridsToClear;
std::vector<Ship*> bulletCollideShips;
std::vector<BaseCore*> enemyBaseCore[Game::NumOfTeams];
std::list<ShipUnit*> mmu_units;
sf::Vector2f mmu_destination;
AStar::SearchType mmu_searchType = AStar::NoSearchType;
short mmu_teamUpType = -1;
bool mmu_isActive = false;
unsigned int numOfTeamAliveShipUnits[Game::NumOfTeams] = {};
unsigned int numOfAliveShipUnits = 0;
unsigned int numOfAliveBullets = 0;
unsigned int numOfBulletShipCollisions = 0;
unsigned int lastBulletShipCollisionIndex = 0;
unsigned int lastShipPathFindIndex = 0;
unsigned int lastShipFastPathFindIndex = 0;
unsigned int lastTargetUpdateIndex = 0;

EntityManager* EntityManager::instance = nullptr;
EntityManager entityManagerInstance;

//Useful functions with their variables
int xGridStart = 0;
int xGridEnd = 0;
int yGridStart = 0;
int yGridEnd = 0;
float temp_value = 0;

std::vector<sf::FloatRect> temp_wallrects;

std::vector<sf::Vector2f> temp_wallPoints;
std::vector<sf::Vector2f> temp_globalPoints;
std::vector<sf::Vector2f> temp_sightCollidePoints;
std::vector<sf::Vector2f> temp_shipPoints;
std::vector<sf::Vector2f> temp_checkedGrids;
std::vector<Ship*> temp_checkedShips;
int temp_checkedGridsLastIndex = 0;
sf::Vector2f temp_localPoints[5];
const float tileSizeSqr = MapTileManager::TILE_SIZE * MapTileManager::TILE_SIZE;

void DeduceGridRange(const sf::FloatRect& rect)
{
    temp_value = 0.f;

    temp_value = rect.left / MapTileManager::TILE_SIZE;
    xGridStart = temp_value;
    if(temp_value < 0.f)
        xGridStart--;

    temp_value = (rect.left + rect.width) / MapTileManager::TILE_SIZE;
    xGridEnd = temp_value;
    if(temp_value < 0.f)
        xGridEnd--;

    temp_value = rect.top / MapTileManager::TILE_SIZE;
    yGridStart = temp_value;
    if(temp_value < 0.f)
        yGridStart--;

    temp_value = (rect.top + rect.height) / MapTileManager::TILE_SIZE;
    yGridEnd = temp_value;
    if(temp_value < 0.f)
        yGridEnd--;
}

void DeduceWallsOnBound(const sf::FloatRect& rect, const bool& exceptSmallWalls = false)
{
    DeduceGridRange(rect);
    bool addWall = true;
    temp_wallrects.clear();
    sf::FloatRect tempRect;

    for(int y = yGridStart; y <= yGridEnd; y++)
        for(int x = xGridStart; x <= xGridEnd; x++)
        {
            if(x >= 0 && x < MapTileManager::MAX_MAPTILE && y >= 0 && y < MapTileManager::MAX_MAPTILE)
            {
                if(GetGridInfoTile(y, x).type == MapTileManager::Wall || (GetGridInfoTile(y, x).type == MapTileManager::SmallWall && exceptSmallWalls == false))
                    addWall = true;
                else addWall = false;
            }
            else addWall = true;

            if(addWall)
            {
                tempRect.left = x * MapTileManager::TILE_SIZE;
                tempRect.top = y * MapTileManager::TILE_SIZE;
                tempRect.width = MapTileManager::TILE_SIZE;
                tempRect.height = MapTileManager::TILE_SIZE;

                temp_wallrects.push_back(tempRect);
            }
        }

}

bool main_IsSightOnWall(const sf::Vector2f& position, const sf::Vector2f& target, const bool& includeSmallWall, bool* isSightOnSmallWall, float* sightLength, const bool& ignoreSmallWallSightLength, const float& sightWidth,
                        std::vector<Ship*>* shipsInSight, std::vector<sf::Vector2f>* shipsInSightPushVec, const int& shipsInSightLimit, const short& shipsInSightTeamUp, const bool& endOnAnyCondition)
{
    sf::Transformable transformation;
    sf::Transform transform_;
    sf::FloatRect localRect(0, 0, sightWidth, MapTileManager::TILE_SIZE);
    sf::FloatRect globalRect;
    sf::Vector2f dist = target - position;
    float distMagnSqr = dist.x * dist.x + dist.y * dist.y;
    float distMagn = distMagnSqr > 0 ? sqrt(distMagnSqr) : 0;

    transformation.setOrigin(sightWidth / 2.f, 0);
    transformation.setPosition(position);
    transformation.setRotation(90.f + (atan2f(dist.y, dist.x) * 180.0 * 7.0 / 22.0));
    transform_ = transformation.getTransform();

    if(sightLength != nullptr)
        *sightLength = -1;

    if(temp_wallPoints.size() != 5)
        temp_wallPoints.resize(5);
    if(temp_globalPoints.size() != 5)
        temp_globalPoints.resize(5);
    if(temp_sightCollidePoints.size() != 5)
        temp_sightCollidePoints.resize(5);
    temp_checkedGrids.clear();
    temp_checkedShips.clear();

    //local points
    temp_localPoints[0].x = sightWidth / 2.f;
    temp_localPoints[0].y = -MapTileManager::TILE_SIZE / 2.f;

    temp_localPoints[1].x = 0.f;
    temp_localPoints[1].y = -MapTileManager::TILE_SIZE;

    temp_localPoints[2].x = sightWidth;
    temp_localPoints[2].y = -MapTileManager::TILE_SIZE;

    temp_localPoints[3].x = sightWidth;
    temp_localPoints[3].y = 0;

    temp_localPoints[4].x = 0;
    temp_localPoints[4].y = 0;

    //collide points
    temp_sightCollidePoints[0].x = sightWidth / 2.f;
    temp_sightCollidePoints[0].y = -distMagn / 2.f;

    temp_sightCollidePoints[1].x = 0.f;
    temp_sightCollidePoints[1].y = -distMagn;

    temp_sightCollidePoints[2].x = sightWidth;
    temp_sightCollidePoints[2].y = -distMagn;

    temp_sightCollidePoints[3].x = sightWidth;
    temp_sightCollidePoints[3].y = 0;

    temp_sightCollidePoints[4].x = 0;
    temp_sightCollidePoints[4].y = 0;

    for(int a = 0; a < 5; a++)
        temp_sightCollidePoints[a] = transform_.transformPoint(temp_sightCollidePoints[a]);

    sf::Vector2f refPos;
    sf::Vector2f shipPushVec;
    sf::Vector2f collisionEntry = temp_sightCollidePoints[1] - temp_sightCollidePoints[4];
    int checkedGridStartIndex = 0;
    int checkedGridEndIndex = 0;
    bool hasReachedSmallWall = false;
    bool skip = false;
    bool collisionState = false;
    bool loopBreaker = true;
    while(loopBreaker)
    {
        localRect.top -= MapTileManager::TILE_SIZE;
        if(localRect.top * localRect.top >= distMagnSqr)
        {
            float diff = -localRect.top - distMagn;

            temp_localPoints[0].y += diff / 2;
            temp_localPoints[1].y += diff;
            temp_localPoints[2].y += diff;

            loopBreaker = false;
        }

        globalRect = transform_.transformRect(localRect);

        for(int a = 0; a < 5; a++)
        {
            temp_globalPoints[a] = transform_.transformPoint(temp_localPoints[a]);

            //Prepare for next loop
            temp_localPoints[a].y -= MapTileManager::TILE_SIZE;
        }

        DeduceGridRange(globalRect);
        GridInfo* gridInfo = nullptr;
        checkedGridStartIndex = checkedGridEndIndex;
        checkedGridEndIndex = temp_checkedGrids.size();

        for(int y = yGridStart; y <= yGridEnd; y++)
            for(int x = xGridStart; x <= xGridEnd; x++)
                if(x >= 0 && x < MapTileManager::MAX_MAPTILE && y >= 0 && y < MapTileManager::MAX_MAPTILE)
                 {
                    skip = false;
                    for(int b = checkedGridStartIndex; b < checkedGridEndIndex; b++)
                        if(temp_checkedGrids[b].x == x && temp_checkedGrids[b].y == y)
                        {
                            skip = true;
                            break;
                        }

                    if(skip == false)
                    {
                        gridInfo = &GetGridInfo(y, x);
                        temp_checkedGrids.push_back(sf::Vector2f(x, y));
                        bool isWall = MapTileManager::GetTile(gridInfo->tileId).type == MapTileManager::Wall;
                        bool isSmallWall = MapTileManager::GetTile(gridInfo->tileId).type == MapTileManager::SmallWall;

                        if(shipsInSight != nullptr && (shipsInSightLimit >= 0 ? shipsInSight->size() < shipsInSightLimit : true))
                        {
                            bool skipShip = false;
                            for(int b = 0; b < gridInfo->ships.size(); b++)
                                if(Game::GetTeamUpType(gridInfo->ships[b]->GetTeam()) != shipsInSightTeamUp)
                                {
                                    skipShip = false;
                                    for(int c = (temp_checkedShips.size() - 15 >= 0 ? temp_checkedShips.size() - 15 : 0); c < temp_checkedShips.size(); c++)
                                        if(gridInfo->ships[b] == temp_checkedShips[c])
                                        {
                                            skipShip = true;
                                            break;
                                        }

                                    if(!skipShip)
                                    {
                                        temp_checkedShips.push_back(gridInfo->ships[b]);

                                        temp_shipPoints.clear();
                                        gridInfo->ships[b]->GetPoints(temp_shipPoints);
                                        if(shipsInSightPushVec != nullptr)
                                        {
                                            if(IsColliding(temp_sightCollidePoints, temp_shipPoints, collisionEntry, shipPushVec))
                                            {
                                                shipsInSight->push_back(gridInfo->ships[b]);
                                                shipsInSightPushVec->push_back(shipPushVec);
                                                if(endOnAnyCondition && shipsInSightLimit >= 0 && shipsInSight->size() >= shipsInSightLimit)
                                                    return false;
                                            }
                                        }
                                        else if(IsColliding(temp_sightCollidePoints, temp_shipPoints))
                                        {
                                            shipsInSight->push_back(gridInfo->ships[b]);
                                            if(endOnAnyCondition && shipsInSightLimit >= 0 && shipsInSight->size() >= shipsInSightLimit)
                                                return false;
                                        }
                                    }
                                }
                        }

                        if(gridInfo != nullptr &&
                           (isWall || (includeSmallWall ? hasReachedSmallWall == false && isSmallWall : false)))
                        {
                            refPos.x = MapTileManager::TILE_SIZE * x;
                            refPos.y = MapTileManager::TILE_SIZE * y;

                            temp_wallPoints[0].x = MapTileManager::TILE_SIZE / 2.f;
                            temp_wallPoints[0].y = temp_wallPoints[0].x;

                            temp_wallPoints[1].x = 0.f;
                            temp_wallPoints[1].y = 0.f;

                            temp_wallPoints[2].x = MapTileManager::TILE_SIZE;
                            temp_wallPoints[2].y = 0.f;

                            temp_wallPoints[3].x = MapTileManager::TILE_SIZE;
                            temp_wallPoints[3].y = MapTileManager::TILE_SIZE;

                            temp_wallPoints[4].x = 0.f;
                            temp_wallPoints[4].y = MapTileManager::TILE_SIZE;

                            for(int a = 0; a < 5; a++)
                                temp_wallPoints[a] += refPos;

                            collisionState = false;
                            if(sightLength != nullptr && *sightLength < 0 && (ignoreSmallWallSightLength ? isSmallWall == false : true))
                            {
                                sf::Vector2f pushVec;
                                collisionState = IsColliding(temp_sightCollidePoints, temp_wallPoints, collisionEntry, pushVec);
                                if(collisionState)
                                    *sightLength = distMagn - MagnitudeOfVector(pushVec);
                            }
                            else collisionState = IsColliding(temp_sightCollidePoints, temp_wallPoints);

                            if(collisionState)
                            {
                                if(isWall)
                                    return true;
                                else if(isSmallWall)
                                {
                                    hasReachedSmallWall = true;
                                    if(isSightOnSmallWall != nullptr)
                                        *isSightOnSmallWall = hasReachedSmallWall;
                                    if(endOnAnyCondition)
                                        return false;
                                }
                            }
                        }
                    }
                }
    }

    return false;
}

bool EntityManager::isSightOnWall(const sf::Vector2f& position, const sf::Vector2f& target, const float& sightWidth)
{
    return main_IsSightOnWall(position, target, false, nullptr, nullptr, true, sightWidth, nullptr, nullptr, -1, -1, false);
}

bool EntityManager::isSightOnWall(const sf::Vector2f& position, const sf::Vector2f& target, bool* isSightOnSmallWall, float* sightLength, const bool& ignoreSmallWallSightLength, const float& sightWidth,
                                  std::vector<Ship*>* shipsInSight, std::vector<sf::Vector2f>* shipsInSightPushVec, const int& shipsInSightLimit, const short& shipsInSightTeamUp, const bool& endOnAnyCondition)
{
    return main_IsSightOnWall(position, target, isSightOnSmallWall != nullptr, isSightOnSmallWall, sightLength, ignoreSmallWallSightLength, sightWidth, shipsInSight, shipsInSightPushVec, shipsInSightLimit, shipsInSightTeamUp, endOnAnyCondition);
}

//Entity Manager
EntityManager::EntityManager()
{
    assert(instance == nullptr);
    instance = this;

    //tileSheetTexture = &AssetManager::GetTexture("TileSheet");

    instance->camera.reset(sf::FloatRect(0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT));
    instance->camera.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
    instance->camera.zoom(PL_ZOOM_FACTOR);

    for(int a = 0; a < 4; a++)
        instance->basecores[a] = nullptr;

    for(int a = 0; a < 0; a++) //Remove later
    {
        sf::Vector2f pos = sf::Vector2f(GenerateRandomNumber(10), GenerateRandomNumber(10)) * (float)MapTileManager::TILE_SIZE + sf::Vector2f(GenerateRandomNumber(MapTileManager::TILE_SIZE), GenerateRandomNumber(MapTileManager::TILE_SIZE));
        ShipUnit* unit = new ShipUnit(ShipUnit::MiniShip, pos, 1, 1, 40, 1, 50 + GenerateRandomNumber(30) , 1, GenerateRandomNumber(Game::GetSkinSize(Game::SkinMiniShip, 1)), Game::SkinMiniShip);
        CreateShip(unit, false);
    }
}

EntityManager::~EntityManager()
{
    std::cout << instance->shipObjects.size() << " alive entities\n";
    std::cout << instance->bulletObjects.size() << " alive bullet(s)\n";
    RemoveAll();
}

unsigned int EntityManager::GetNumOfAliveShipUnits()
{
    return numOfAliveShipUnits;
}

unsigned int EntityManager::GetNumOfTeamAliveShipUnits(short team_)
{
    if(team_ >= 0 && team_ < Game::NumOfTeams)
        return numOfTeamAliveShipUnits[team_];
    return 0;
}

void EntityManager::CreateShip(Ship* ship, bool cameraFocus)
{
    if(ship != nullptr)
    {
        instance->shipObjects.push_back(ship);
        if(ship->GetShipType() == Ship::UnitShip)
        {
            numOfAliveShipUnits++;
            numOfTeamAliveShipUnits[ship->GetTeam()]++;
        }

        if(cameraFocus == true)
        {
            cameraPoint = ship;
            instance->camera.setCenter(cameraPoint->GetPosition());
        }
    }
}

void EntityManager::CreateDefence(sf::Vector2f position_, Defence::DefenceType type_, short team_, short rotation_, int skinIndex_)
{
    Defence* defence = new Defence(position_, type_, team_, rotation_, skinIndex_);
    instance->defenseObjects.push_back(defence);
}

void EntityManager::CreateBullet(Bullet* bullet)
{
    if(bullet != nullptr)
    {
        instance->bulletObjects.push_back(bullet);
        numOfAliveBullets++;
    }
}

void EntityManager::CreateAllDefenceShips()
{
    for(Defence*& defense : instance->defenseObjects)
        defense->BuildEntity(false);
}

void EntityManager::RemoveAll()
{
    for(auto& subject : instance->shipObjects)
        delete subject;
    for(auto& subject : instance->defenseObjects)
        delete subject;
    for(auto& subject : instance->bulletObjects)
        delete subject;
    for(int a = 0; a < Game::NumOfTeams; a++)
        numOfTeamAliveShipUnits[a] = 0;

    numOfAliveShipUnits = numOfAliveBullets = 0;
}

const std::list<Ship*>& EntityManager::GetShipObjects()
{
    return instance->shipObjects;
}

void EntityManager::UpdateShipGrid()
{
    //Reset grid
    for(int a = 0; a < shipGridsToClear.size(); a++)
        if(shipGridsToClear[a].x >= 0 && shipGridsToClear[a].x < MapTileManager::MAX_MAPTILE &&
           shipGridsToClear[a].y >= 0 && shipGridsToClear[a].y < MapTileManager::MAX_MAPTILE)
        {
            GetGridInfo(shipGridsToClear[a].y, shipGridsToClear[a].x).ships.clear();
        }
    shipGridsToClear.clear();

    for(int a = 0; a < positionShipGridsToClear.size(); a++)
        if(positionShipGridsToClear[a].x >= 0 && positionShipGridsToClear[a].x < MapTileManager::MAX_MAPTILE &&
           positionShipGridsToClear[a].y >= 0 && positionShipGridsToClear[a].y < MapTileManager::MAX_MAPTILE)
        {
            GetGridInfo(positionShipGridsToClear[a].y, positionShipGridsToClear[a].x).positionShips.clear();
        }
    positionShipGridsToClear.clear();

    //Update grid
    GridInfo* gridInfo = nullptr;
    for(Ship*& ship : instance->shipObjects)
    {
        DeduceGridRange(ship->GetBoundingRect());

        for(int y = yGridStart; y <= yGridEnd; y++)
            for(int x = xGridStart; x <= xGridEnd; x++)
            {
                if(x >= 0 && x < MapTileManager::MAX_MAPTILE && y >= 0 && y < MapTileManager::MAX_MAPTILE)
                {
                    gridInfo = &GetGridInfo(y, x);
                    if(gridInfo->ships.size() == 0)
                        shipGridsToClear.push_back(sf::Vector2i(x, y));
                    gridInfo->ships.push_back(ship);
                }
            }

        sf::Vector2i grid = GetGridPosition(ship->GetPosition());
        GetGridInfo(grid).positionShips.push_back(ship);
        positionShipGridsToClear.push_back(grid);
    }
}

void EntityManager::UpdateAllShipTarget(float dt)
{
    std::vector<Ship*> foundShips;

    auto findShipsOnGrid = [&](const sf::Vector2i& grid_, short ignoreTeam_, sf::Vector2f position)
    {
        if(grid_.y >= 0 && grid_.y < MapTileManager::GetMapColoums() &&
           grid_.x >= 0 && grid_.x < MapTileManager::GetMapRows())
        {
            ignoreTeam_ = Game::GetTeamUpType(ignoreTeam_);

            for(int b = 0; b < GetGridInfo(grid_.y, grid_.x).ships.size(); b++)
            {
                Ship* ship = GetGridInfo(grid_.y, grid_.x).ships[b];

                if(Game::GetTeamUpType(ship->GetTeam()) != ignoreTeam_)
                    if(isSightOnWall(position, ship->GetPosition()) == false)
                        foundShips.push_back(GetGridInfo(grid_.y, grid_.x).ships[b]);
            }
        }
    };

    if(lastTargetUpdateIndex >= instance->shipObjects.size())
        lastTargetUpdateIndex = 0;

    //Declaring loop variables
    sf::Vector2f position;
    sf::Vector2i gridPosition;
    short ignoreTeam = -1;
    int targetRangeGrid = 0;
    float targetRange = 0.f;
    float sightRangeGrid = 0.f;
    bool ignoreSightShips = false;
    float sightRangeSqr = Ship::SHIP_SIGTH_RANGE * Ship::SHIP_SIGTH_RANGE;
    float targetRangeSqr = 0.f;
    Ship* ship = nullptr;
    int numOfTargetsUpdated = 0;

    int actualTargetUpdatePerFrame = (dt * 60.f > 1 ? 1 : dt * 60.f) * TARGET_UDATE_PER_FRAME;
    if(actualTargetUpdatePerFrame < 0)
        actualTargetUpdatePerFrame = 1;

    for(auto itr = std::next(instance->shipObjects.begin(), lastTargetUpdateIndex); itr != instance->shipObjects.end(); itr++)
    {
        ship = *itr;
        RecruitGroupMembers(ship);

        position = ship->GetPosition();
        gridPosition = GetGridPosition(position);
        ignoreTeam = ship->GetTeam();
        targetRange = ship->GetTargetRange();
        targetRangeSqr = targetRange * targetRange;
        ignoreSightShips = false;
        targetRangeGrid = targetRange / MapTileManager::TILE_SIZE + 1;


        if(ship->GetShipType() == Ship::DefenceShip)
            sightRangeGrid = targetRangeGrid;
        else sightRangeGrid = Ship::SHIP_SIGTH_RANGE / MapTileManager::TILE_SIZE + 1;

        const std::vector<BaseCore*>& enemyBaseCores = EntityManager::GetEnemyBaseCore(ship->GetTeam());
        float minBCDistSqr = 0.f;
        BaseCore* baseCore = nullptr;
        for(auto& bc : enemyBaseCores)
        {
            float distSqr = MagnitudeSqureOfVector(bc->GetPosition() - ship->GetPosition());
            if(bc->GetAlive() && distSqr <= targetRangeSqr && (baseCore == nullptr || distSqr < minBCDistSqr))
                if(isSightOnWall(ship->GetPosition(), bc->GetPosition()) == false)
                {
                    baseCore = bc;
                    minBCDistSqr = distSqr;
                }
        }

        //if(baseCore != nullptr ? ship->GetHealth() / ship->GetActualHitpoint() < ship->GetBaseCoreAttackHealthFactor() : true)
        findShipsOnGrid(gridPosition, ignoreTeam, ship->GetPosition());

        if(foundShips.size() == 0)
        {
            for(int range = 1; range <= sightRangeGrid; range++)
            {
                for(int topX = -range; topX <= range; topX++)
                    findShipsOnGrid(sf::Vector2i(topX + gridPosition.x, range + gridPosition.y), ignoreTeam, ship->GetPosition());

                for(int bottomX = -range; bottomX <= range; bottomX++)
                    findShipsOnGrid(sf::Vector2i(bottomX + gridPosition.x, -range + gridPosition.y), ignoreTeam, ship->GetPosition());

                for(int leftY = -range + 1; leftY <= range - 1; leftY++)
                    findShipsOnGrid(sf::Vector2i(range + gridPosition.x, leftY + gridPosition.y), ignoreTeam, ship->GetPosition());

                for(int rightY = -range + 1; rightY <= range - 1; rightY++)
                    findShipsOnGrid(sf::Vector2i(-range + gridPosition.x, rightY + gridPosition.y), ignoreTeam, ship->GetPosition());

                if(foundShips.size() > 0)
                break;
            }
        }

        if(foundShips.size() == 0 && baseCore != nullptr)
            ship->SetTarget(baseCore);
        else
        {
            if(foundShips.size() > 0)
            {
                int closestShip = -1;
                float closestDistSqr = -1;

                sf::Vector2f distVec;
                float distSqr = 0.f;
                bool isTargetedBySightUnit = false;

                for(int b = 0; b < foundShips.size(); b++)
                {
                    distVec = foundShips[b]->GetPosition() - ship->GetPosition();
                    distSqr = distVec.x * distVec.x + distVec.y * distVec.y;

                    if(distSqr <= targetRangeSqr)
                    {
                        if(closestDistSqr < 0 || distSqr < closestDistSqr)
                        {
                            closestDistSqr = distSqr;
                            closestShip = b;
                        }
                    }
                    else if(ship->GetShipType() != Ship::DefenceShip && distSqr <= sightRangeSqr)
                    {
                        ship->AddSigthTarget(foundShips[b]);
                        if(baseCore != nullptr && isTargetedBySightUnit == false && foundShips[b]->GetShipTarget() == ship)
                            isTargetedBySightUnit = true;
                    }
                }

                if(baseCore != nullptr && (closestShip >= 0 ? minBCDistSqr < closestDistSqr : true) && (ship->GetHealth() / ship->GetActualHitpoint() > ship->GetBaseCoreAttackHealthFactor() ? true : closestShip < 0 && isTargetedBySightUnit == false))
                    ship->SetTarget(baseCore);
                else if(closestShip >= 0)
                    ship->SetTarget(foundShips[closestShip]);
            }
            else ship->SetTarget(nullptr);
        }
        foundShips.clear();

        numOfTargetsUpdated++;
        if(numOfTargetsUpdated >= actualTargetUpdatePerFrame)
            break;
    };
    lastTargetUpdateIndex += numOfTargetsUpdated;
}

void EntityManager::UpdateAll(float dt)
{
    //Refreshing all entities
    for(auto& ship : instance->shipObjects)
        ship->RefreshEntities();
    for(auto& defense : instance->defenseObjects)
        defense->RefreshEntities();
    for(auto& bullet : instance->bulletObjects)
        bullet->RefreshEntities();
    ShipIdentity::RefreshShipIdentities();
    DisplayEffect::RefreshDisplayEffects();
    Ship::UnitGroup::RefreshUnitGroups();

    for(auto itr = mmu_units.begin(); itr != mmu_units.end();)
    {
        if((*itr)->GetAlive() == false)
            itr = mmu_units.erase(itr);
        else itr++;
    }

    //Deleting all dead entities
    for(auto& entity : entitiesToDelete)
        delete entity;
    entitiesToDelete.clear();

    //Updating ship grids
    UpdateShipGrid();

    //Updating all ship targets
    UpdateAllShipTarget(dt);

    //Updating all entities
    //____Bullets
    unsigned int bulletIndex = 0;
    unsigned int newBulletLast = 0;
    std::list<Bullet*>::iterator lastItr = instance->bulletObjects.end();

    int actualBulletShipCollisionLimit = (dt * 60.f > 1 ? 1 : dt * 60.f) * BULLET_SHIP_COLLISION_LIMIT;
    if(actualBulletShipCollisionLimit < 0)
        actualBulletShipCollisionLimit = 1;

    for(auto itr = instance->bulletObjects.begin(); itr != instance->bulletObjects.end(); itr++)
    {
        Bullet* bullet = *itr;
        bullet->Update(dt);

        //Resolve Wall Collision
        DeduceWallsOnBound(bullet->GetBoundingRect(), true);
        bullet->ResolveWallCollision(temp_wallrects);

        //Resolve Ship and BaseCore Collision
        if(bulletIndex >= lastBulletShipCollisionIndex && numOfBulletShipCollisions < actualBulletShipCollisionLimit)
        {
            //Ship
            bool checkCollision = true;
            bool didCollision = false;
            bool bulletCollided = false;
            bulletCollideShips.clear();
            int teamUp = Game::GetTeamUpType(bullet->GetTeam());

            DeduceGridRange(bullet->GetBoundingRect());
            for(int y = yGridStart; y <= yGridEnd && bulletCollided == false; y++)
                for(int x = xGridStart; x <= xGridEnd && bulletCollided == false; x++)
                    if(x >= 0 && x < MapTileManager::MAX_MAPTILE && y >= 0 && y < MapTileManager::MAX_MAPTILE)
                        if(GetGridInfo(y, x).ships.size() > 0)
                        {
                            for(auto& ship : GetGridInfo(y, x).ships)
                            {
                                if(Game::GetTeamUpType(ship->GetTeam()) != teamUp)
                                {
                                    didCollision = true;
                                    checkCollision = true;
                                    for(int a = 0; a < bulletCollideShips.size() && checkCollision; a++)
                                        if(bulletCollideShips[a] == ship)
                                        {
                                            checkCollision = false;
                                            break;
                                        }

                                    if(checkCollision)
                                    {
                                        if(bullet->ResolveShipCollision(ship, dt))
                                        {
                                            bulletCollided = true;
                                            break;
                                        }
                                        bulletCollideShips.push_back(ship);
                                    }
                                }
                            }
                        }
            if(didCollision)
            {
                newBulletLast = bulletIndex + 1;
                numOfBulletShipCollisions++;
            }

            //BaseCore
            const std::vector<BaseCore*>& enemyBaseCores = EntityManager::GetEnemyBaseCore((*itr)->GetTeam());
            for(auto& bc : enemyBaseCores)
                bullet->ResolveBaseCoreCollision(bc);
        }

        if(bulletIndex == lastBulletShipCollisionIndex)
            lastItr = itr;

        bulletIndex++;
    }

    if(numOfBulletShipCollisions < actualBulletShipCollisionLimit)
    {
        bulletIndex = 0;
        for(auto itr = instance->bulletObjects.begin(); itr != instance->bulletObjects.end() && itr != lastItr; itr++)
        {
            Bullet* bullet = *itr;
            //Resolve Ship Collision
            if(numOfBulletShipCollisions < actualBulletShipCollisionLimit)
            {
                bool checkCollision = true;
                bool didCollision = false;
                bool bulletCollided = false;
                bulletCollideShips.clear();
                int teamUp = Game::GetTeamUpType(bullet->GetTeam());

                DeduceGridRange(bullet->GetBoundingRect());
                for(int y = yGridStart; y <= yGridEnd && bulletCollided == false; y++)
                    for(int x = xGridStart; x <= xGridEnd && bulletCollided == false; x++)
                        if(x >= 0 && x < MapTileManager::MAX_MAPTILE && y >= 0 && y < MapTileManager::MAX_MAPTILE && GetGridInfo(y, x).ships.size() > 0)
                            for(auto& ship : GetGridInfo(y, x).ships)
                            {
                                if(Game::GetTeamUpType(ship->GetTeam()) != teamUp)
                                {
                                    didCollision = true;
                                    checkCollision = true;
                                    for(int a = 0; a < bulletCollideShips.size() && checkCollision; a++)
                                        if(bulletCollideShips[a] == ship)
                                            checkCollision = false;

                                    if(checkCollision)
                                    {
                                        if(bullet->ResolveShipCollision(ship, dt))
                                        {
                                            bulletCollided = true;
                                            break;
                                        }
                                        bulletCollideShips.push_back(ship);
                                    }
                                }
                            }
                if(didCollision)
                {
                    newBulletLast = bulletIndex + 1;
                    numOfBulletShipCollisions++;
                }
            }
        }
    }

    //Reset Limiter
    if(newBulletLast >= numOfAliveBullets)
        newBulletLast = 0;
    lastBulletShipCollisionIndex = newBulletLast;
    numOfBulletShipCollisions = 0;

    //Checking alive state
    bulletIndex = 0;
    for(auto itr = instance->bulletObjects.begin(); itr != instance->bulletObjects.end();)
    {
        Bullet* bullet = *itr;
        if(bullet->GetAlive() == false)
        {
            entitiesToDelete.push_back(bullet);
            itr = instance->bulletObjects.erase(itr);
            numOfAliveBullets--;

            if(bulletIndex < lastBulletShipCollisionIndex)
                lastBulletShipCollisionIndex--;
        }
        else
        {
            itr++;
            bulletIndex++;
        }
    }

    //____Ships
    AStar::SetProgressiveSearchStatus(false);
    AStar::SetFastSearchStatus(false);
    unsigned int shipIndex = 0;
    int newLastPathFindIndex = -1;
    int newLastFastPathFindIndex = -1;

    //Updating base units
    for(Ship*& unit : instance->shipObjects)
        unit->AddToGridBase();

    for(auto itr = instance->shipObjects.begin(); itr != instance->shipObjects.end();)
    {
        Ship* ship = *itr;
        bool prevAstarLimitedState = AStar::GetLimitedState();

        if(shipIndex == lastShipPathFindIndex)
            AStar::SetProgressiveSearchStatus(true);
        if(shipIndex == lastShipFastPathFindIndex)
            AStar::SetFastSearchStatus(true);

        if(shipIndex >= lastShipPathFindIndex && prevAstarLimitedState == false)
            newLastPathFindIndex = shipIndex + 1;
        if(shipIndex >= lastShipFastPathFindIndex && AStar::GetFastSearchLimitedState() == false)
            newLastFastPathFindIndex = shipIndex + 1;

        if(prevAstarLimitedState == false && mmu_units.size() > 0 && (AStar::GetProgressionState() ? mmu_isActive : true))
        {
            mmu_isActive = true;
            while(mmu_units.size() > 0 && AStar::IsReadyToSearch(false))
            {
                std::vector<sf::Vector2i> path_;
                if(AStar::FindPath(GetGridPosition((*mmu_units.begin())->GetPosition()), GetGridPosition(mmu_destination), path_, mmu_searchType, mmu_teamUpType))
                {
                    if(path_.size() > 0)
                        (*mmu_units.begin())->MoveTo(path_);
                    mmu_units.erase(mmu_units.begin());
                    if(mmu_units.size() > 0)
                        AStar::ContinueLastTargetSearch(GetGridPosition((*mmu_units.begin())->GetPosition()));
                }
                else if(AStar::GetProgressionState() == false)
                {
                    mmu_units.erase(mmu_units.begin());
                    if(mmu_units.size() > 0)
                        AStar::ContinueLastTargetSearch(GetGridPosition((*mmu_units.begin())->GetPosition()));
                }
            }
        }
        else if(mmu_units.size() <= 0)
            mmu_isActive = false;

        ship->Update(dt);

        //Resolve Collision
        if(ship->GetShipType() != Ship::DefenceShip)
        {
            DeduceWallsOnBound(ship->GetBoundingRect());
            ship->ResolveWallCollision(temp_wallrects, dt);
        }

        //Checking alive state
        if(ship->GetAlive() == false)
        {
            if(ship->GetShipType() == Ship::UnitShip)
            {
                numOfAliveShipUnits--;
                numOfTeamAliveShipUnits[ship->GetTeam()]--;
            }

            if(shipIndex < lastShipPathFindIndex)
                lastShipPathFindIndex--;
            else if(shipIndex == lastShipPathFindIndex)
                AStar::CancleProgression();

            if(shipIndex < lastShipFastPathFindIndex)
                lastShipFastPathFindIndex--;
            if(shipIndex < newLastPathFindIndex)
                newLastPathFindIndex--;
            if(shipIndex < newLastFastPathFindIndex)
                newLastFastPathFindIndex--;
            if(shipIndex < lastTargetUpdateIndex)
                lastTargetUpdateIndex--;

            entitiesToDelete.push_back(ship);
            itr = instance->shipObjects.erase(itr);
        }

        if(prevAstarLimitedState == false && AStar::GetLimitedState())
            newLastPathFindIndex = shipIndex;

        if(ship->GetAlive())
        {
            itr++;
            shipIndex++;
        }
    }
    if(newLastPathFindIndex < 0 || newLastPathFindIndex >= instance->shipObjects.size())
        lastShipPathFindIndex = 0;
    else lastShipPathFindIndex = newLastPathFindIndex;

    if(newLastFastPathFindIndex < 0 || newLastFastPathFindIndex >= instance->shipObjects.size())
        lastShipFastPathFindIndex = 0;
    else lastShipFastPathFindIndex = newLastFastPathFindIndex;

    //____Defences
    for(auto& defense : instance->defenseObjects)
        defense->Update(dt);

    //____Base Cores
    for(int a = 0; a < Game::NumOfTeams; a++)
        if(instance->basecores[a] != nullptr)
            instance->basecores[a]->Update(dt);

    //____Ship Identities
    ShipIdentity::UpdateShipIdentities(dt);

    //Updating camera
    if(Game::isMouseScrolled() && GUI::GetMouseScrollDelta() != 0 && cameraPoint == nullptr && commandModeWait <= 0.f)
        cameraScale += -Game::GetMouseScrollDelta() * 0.4f;
    if(cameraPoint != nullptr)
    {
        cameraScale = PL_ZOOM_FACTOR;
        if(cameraRealScale <= PL_ZOOM_FACTOR + (LV_ZOOM_MAX - PL_ZOOM_FACTOR) * 0.2f)
        {
            commandModeWait = 0.5f;
            commandModeLastScale = cameraRealScale;
        }
    }
    else if(commandModeWait > 0)
    {
        if(commandModeWait >= 0.2f)
            cameraScale = (1.f - sin(((commandModeWait - 0.2f) / 0.3f) * (22.f / 7.f / 2.f))) * (2.0f - commandModeLastScale) + commandModeLastScale;
        else cameraScale = 2.0f;
        commandModeWait -= dt;
    }

    if(cameraScale > LV_ZOOM_MAX)
        cameraScale = LV_ZOOM_MAX;
    if(cameraScale < LV_ZOOM_MIN)
        cameraScale = LV_ZOOM_MIN;
    TendTowards(cameraRealScale, cameraScale, 0.1, 0.0001, dt);
    instance->camera.setSize(Game::SCREEN_WIDTH * cameraRealScale, Game::SCREEN_HEIGHT * cameraRealScale);

    if(cameraQuickPeak)
        cameraRealPosition = cameraEaseInPosition = cameraPeakPosition;
    else if(cameraPoint != nullptr)
    {
        if(cameraPoint->GetAlive() != false)
        {
            //float heightFactor = CAMERA_FACTOR * PL_ZOOM_FACTOR * Game::SCREEN_HEIGHT / 2.f;
            //sf::Vector2f factorY(0.f, heightFactor * sin((90 - cameraPoint->GetAngle()) * 3.142f / 180.f));

                sf::Vector2f limitFactor = ShipUnit::IsActivePlayerAiming() ? PL_AIM_CAMERA_LIMIT_FACTOR : PL_CAMERA_LIMIT_FACTOR;
                sf::Vector2f camDisplacement = PL_ZOOM_FACTOR * sf::Vector2f(Game::SCREEN_WIDTH * limitFactor.x / 2.f, Game::SCREEN_HEIGHT * limitFactor.y / 2.f);
                sf::Vector2f camFactor(cos((90 - cameraPoint->GetNonTransmittingAngle()) * 3.142f / 180.f), -sin((90 - cameraPoint->GetNonTransmittingAngle()) * 3.142f / 180.f));
                cameraRealPosition = cameraPoint->GetPosition() + sf::Vector2f(camFactor.x * camDisplacement.x, camFactor.y * camDisplacement.y);
        }
        else cameraPoint = nullptr;
    }
    else if(commandModeWait <= 0)
    {
        sf::Vector2f camDir;
        if(Game::GetInput(sf::Keyboard::W, Game::Hold))
            camDir.y -= 1;
        if(Game::GetInput(sf::Keyboard::S, Game::Hold))
            camDir.y += 1;
        if(Game::GetInput(sf::Keyboard::A, Game::Hold))
            camDir.x -= 1;
        if(Game::GetInput(sf::Keyboard::D, Game::Hold))
            camDir.x += 1;

        cameraRealPosition += NormalizeVector(camDir) * 2000.f * dt;
    }

    if(cameraRealPosition.x - instance->camera.getSize().x / 2.f < 0)
        cameraRealPosition.x = instance->camera.getSize().x / 2.f;
    if(cameraRealPosition.x + instance->camera.getSize().x / 2.f > MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE)
        cameraRealPosition.x = MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - instance->camera.getSize().x / 2.f;
    if(cameraRealPosition.y - instance->camera.getSize().y / 2.f < 0)
        cameraRealPosition.y = instance->camera.getSize().y / 2.f;
    if(cameraRealPosition.y + instance->camera.getSize().y / 2.f > MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE)
        cameraRealPosition.y = MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - instance->camera.getSize().y / 2.f;

    sf::Vector2f dist = cameraRealPosition - cameraEaseInPosition;
    float distLenght = sqrt(dist.x * dist.x + dist.y * dist.y);
    if(distLenght > (Game::SCREEN_WIDTH > Game::SCREEN_HEIGHT ? Game::SCREEN_WIDTH : Game::SCREEN_HEIGHT) * cameraRealScale * 1.5f)
    {
        cameraEaseInPosition = cameraRealPosition;
        distLenght = 0;
        dist = sf::Vector2f();
    }
    sf::Vector2f unitVec(0.f, 0.f);
    if(distLenght > 0)
        unitVec = dist / distLenght;
    if(cameraPoint != nullptr && ShipUnit::IsActivePlayerManualAiming() == false)
        cameraEaseInPosition += unitVec * (distLenght * dt / 0.1f);
    else cameraEaseInPosition = cameraRealPosition;

    //dist = cameraRealPosition  - instance->camera.getCenter();
    dist = cameraEaseInPosition - instance->camera.getCenter();
    distLenght = sqrt(dist.x * dist.x + dist.y * dist.y);
    if(distLenght > (Game::SCREEN_WIDTH > Game::SCREEN_HEIGHT ? Game::SCREEN_WIDTH : Game::SCREEN_HEIGHT) * cameraRealScale * 1.5f)
    {
        cameraEaseInPosition = cameraRealPosition;
        instance->camera.setCenter(cameraRealPosition);
        distLenght = 0;
        dist = sf::Vector2f();
    }
    unitVec = sf::Vector2f(0.f, 0.f);
    if(distLenght > 0)
        unitVec = dist / distLenght;

    float sensitivity = 0.3f;
    float aimMagn = 0.f;
    if(cameraQuickPeak)
    {
        cameraQuickPeak = false;
        sensitivity = 0.06f;
    }
    else if(cameraPoint != nullptr && ShipUnit::IsActivePlayerManualAiming() && ShipUnit::IsActivePlayerAiming(aimMagn))
    {
        float factor = 1 - aimMagn / (Game::SCREEN_HEIGHT < Game::SCREEN_WIDTH ? (float)Game::SCREEN_HEIGHT * PL_AIM_CAMERA_LIMIT_FACTOR.y : (float)Game::SCREEN_WIDTH * PL_AIM_CAMERA_LIMIT_FACTOR.x);
        if(factor < 0)
            factor = 0.f;
        if(factor > 1)
            factor = 1.f;
        sensitivity += (10.f - 0.3f) * factor * factor * factor;

        //Mouse should move outside view for smooth aiming
        float scaling = instance->camera.getSize().y / Game::SCREEN_HEIGHT;
        sf::Vector2f topLeft = instance->camera.getCenter() - instance->camera.getSize() / 2.f;
        sf::Vector2f mouseWorldPosition = topLeft + Game::GetMousePosition() * scaling;
        sf::Vector2f diff = (mouseWorldPosition - cameraPoint->GetPosition());
        sf::Vector2f normVec = NormalizeVector(sf::Vector2f(diff.x / Game::GetScreenStretchFactor().x, diff.y / Game::GetScreenStretchFactor().y));
        sf::Vector2f areaMagn(instance->GetCamera().getSize().x * (1 + PL_AIM_CAMERA_LIMIT_FACTOR.x) * 0.5f, instance->GetCamera().getSize().y * (1 + PL_AIM_CAMERA_LIMIT_FACTOR.y) * 0.5f);
        sf::Vector2f limit(normVec.x * areaMagn.x, normVec.y * areaMagn.y);

        if(MagnitudeSqureOfVector(diff) > MagnitudeSqureOfVector(limit))
        {
            //mouseWorldPosition = cameraPoint->GetPosition() + limit;
            sf::Vector2f halfCamSize = instance->GetCamera().getSize() / 2.f;
            sf::Vector2f camPos = cameraRealPosition;

            sf::Vector2f moveVec;
            sf::Vector2f refPos = mouseWorldPosition;
            if(mouseWorldPosition.x > camPos.x + halfCamSize.x)
                refPos.x = camPos.x + halfCamSize.x;
            else if(mouseWorldPosition.x < camPos.x - halfCamSize.x)
                refPos.x = camPos.x - halfCamSize.x;

            if(mouseWorldPosition.y > camPos.y + halfCamSize.y)
                refPos.y = camPos.y + halfCamSize.y;
            else if(mouseWorldPosition.y < camPos.y - halfCamSize.y)
                refPos.y = camPos.y - halfCamSize.y;

            moveVec = refPos - mouseWorldPosition;
            mouseWorldPosition += moveVec * dt * 10.f;
            if(DotProduct(moveVec, refPos - mouseWorldPosition) < 0)
                mouseWorldPosition = refPos;
        }

        if(mouseWorldPosition.x < 0)
            mouseWorldPosition.x = 0;
        else if(mouseWorldPosition.x > MapTileManager::TILE_SIZE * MapTileManager::MAX_MAPTILE)
            mouseWorldPosition.x = MapTileManager::TILE_SIZE * MapTileManager::MAX_MAPTILE;
        if(mouseWorldPosition.y < 0)
            mouseWorldPosition.y = 0;
        else if(mouseWorldPosition.y > MapTileManager::TILE_SIZE * MapTileManager::MAX_MAPTILE)
            mouseWorldPosition.y = MapTileManager::TILE_SIZE * MapTileManager::MAX_MAPTILE;

        Game::SetMousePosition((mouseWorldPosition - topLeft) / scaling);
    }
    else if(cameraPoint != nullptr && ShipUnit::IsActivePlayerManualAiming() == false && ShipUnit::IsActivePlayerAiming(aimMagn))
    {
        if(cameraPoint->GetShipTarget() != nullptr)
        {
            float factor = 1 - aimMagn / (Game::SCREEN_HEIGHT < Game::SCREEN_WIDTH ? (float)Game::SCREEN_HEIGHT * PL_AIM_CAMERA_LIMIT_FACTOR.y : (float)Game::SCREEN_WIDTH * PL_AIM_CAMERA_LIMIT_FACTOR.x);
            if(factor < 0)
                factor = 0.f;
            if(factor > 1)
                factor = 1.f;
            sensitivity += (20.f - 0.3f) * factor * factor * factor;
            cameraLastAimSensitivity = sensitivity;
        }
        else
        {
            TendTowards(cameraLastAimSensitivity, 0.3f, 0.5f, 0.0001f, dt);
            sensitivity = cameraLastAimSensitivity;
        }
    }
    else if(cameraPoint == nullptr)
        sensitivity = 0.15f;
    float moveAmount = distLenght * dt / (sensitivity > 0 ? sensitivity : 1.f);
    //if(moveAmount < 0.5f) moveAmount = 0.5f;

    lastCameraRect = sf::FloatRect(instance->camera.getCenter().x - instance->camera.getSize().x / 2.f,
                                   instance->camera.getCenter().y - instance->camera.getSize().y / 2.f,
                                   instance->camera.getSize().x, instance->camera.getSize().y);

    sf::Vector2f cursorWorldPosition = sf::Vector2f(lastCameraRect.left, lastCameraRect.top) + (sf::Vector2f)Game::GetMousePosition() * (lastCameraRect.height / Game::SCREEN_HEIGHT);
    sf::Vector2f moveVec(0.f, 0.f);
    moveVec = unitVec * moveAmount;
    instance->camera.setCenter(instance->camera.getCenter() + moveVec);

    if(instance->camera.getCenter().x - instance->camera.getSize().x / 2.f < 0)
        instance->camera.setCenter(instance->camera.getSize().x / 2.f, instance->camera.getCenter().y);
    if(instance->camera.getCenter().x + instance->camera.getSize().x / 2.f > MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE)
        instance->camera.setCenter(MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - instance->camera.getSize().x / 2.f, instance->camera.getCenter().y);
    if(instance->camera.getCenter().y - instance->camera.getSize().y / 2.f < 0)
        instance->camera.setCenter(instance->camera.getCenter().x, instance->camera.getSize().y / 2.f);
    if(instance->camera.getCenter().y + instance->camera.getSize().y / 2.f > MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE)
        instance->camera.setCenter(instance->camera.getCenter().x, MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - instance->camera.getSize().y / 2.f);

    //dist = cameraRealPosition - (instance->camera.getCenter() + moveVec);
    dist = cameraEaseInPosition - (instance->camera.getCenter() + moveVec);
    if(DotProduct(unitVec, dist) < 0)
        instance->camera.setCenter(cameraRealPosition);

    if(ShipUnit::IsActivePlayerManualAiming() && ShipUnit::IsActivePlayerAiming())
    {
        lastCameraRect = sf::FloatRect(instance->camera.getCenter().x - instance->camera.getSize().x / 2.f,
                                        instance->camera.getCenter().y - instance->camera.getSize().y / 2.f,
                                        instance->camera.getSize().x, instance->camera.getSize().y);

        sf::Vector2f newCursorPosition = (cursorWorldPosition - sf::Vector2f(lastCameraRect.left, lastCameraRect.top)) * (Game::SCREEN_HEIGHT / lastCameraRect.height);
        Game::SetMousePosition(newCursorPosition);
    }

    //Reset Limiters
    AStar::ResetLimiter();
}

sf::View& EntityManager::GetCamera()
{
    return instance->camera;
}

sf::FloatRect EntityManager::GetCameraRect()
{
    return lastCameraRect;
}

void EntityManager::SetCameraFocus(Ship* ship_)
{
    cameraPoint = ship_;
}

Ship* EntityManager::GetCameraFocus()
{
    return cameraPoint;
}

void EntityManager::SetCameraPosition(sf::Vector2f position_, bool doTransmit_)
{
    if(position_.x - instance->camera.getSize().x / 2.f < 0)
        position_.x = instance->camera.getSize().x;
    else if(position_.x + instance->camera.getSize().x / 2.f > MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE)
        position_.x = MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - instance->camera.getSize().x / 2.f;

    if(position_.y - instance->camera.getSize().y / 2.f < 0)
        position_.y = instance->camera.getSize().y;
    else if(position_.y + instance->camera.getSize().y / 2.f > MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE)
        position_.y = MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - instance->camera.getSize().y / 2.f;

    cameraRealPosition = position_;
    if(doTransmit_ == false)
    {
        cameraEaseInPosition = position_;
        instance->camera.setCenter(position_);
    }
}

void EntityManager::QuickPeakWithCamera(sf::Vector2f position_)
{
    if(position_.x - instance->camera.getSize().x / 2.f < 0)
        position_.x = instance->camera.getSize().x / 2.f;
    else if(position_.x + instance->camera.getSize().x / 2.f > MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE)
        position_.x = MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - instance->camera.getSize().x / 2.f;

    if(position_.y - instance->camera.getSize().y / 2.f < 0)
        position_.y = instance->camera.getSize().y / 2.f;
    else if(position_.y + instance->camera.getSize().y / 2.f > MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE)
        position_.y = MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - instance->camera.getSize().y / 2.f;

    cameraQuickPeak = true;
    cameraPeakPosition = position_;
}

void EntityManager::CreateBaseCore(short team, sf::Vector2i grid)
{
    if(team >= 0 && team < 4)
    {
        if(instance->basecores[team] != nullptr)
            DeleteBaseCore(team);

        instance->basecores[team] = new BaseCore(team, grid);
    }
}

void EntityManager::DeleteBaseCore(short team)
{
    if(team >= 0 && team < 4)
        if(instance->basecores[team] != nullptr)
        {
            delete instance->basecores[team];
            instance->basecores[team] = nullptr;
        }
}

BaseCore* EntityManager::GetBaseCore(short team)
{
    if(team >= 0 && team < 4)
        return instance->basecores[team];
    return nullptr;
}

void EntityManager::RecruitGroupMembers(Ship* ship)
{
    if(ship->GetGroup() != nullptr && ship->GetGroup()->leader == ship && ship->GetGroup()->members.size() < Ship::MAX_GROUP_MEMBERS && ship->GetRecruitingState())
    {
        sf::Vector2i grid = GetGridPosition(ship->GetPosition());
        std::vector<Ship*> foundShips;
        short team = ship->GetTeam();
        int x = grid.x;
        int y = grid.y;

        if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE)
            for(Ship*& unit : GetGridInfo(y, x).positionShips)
            {
               if(unit != ship && team == unit->GetTeam() && unit->GetHierarchy() >= 0 && unit->GetHierarchy() <= 1)
                {
                    if(unit->GetGroup() != nullptr)
                    {
                        if(ship->GetHierarchy() > 1 && unit->GetGroup() != ship->GetGroup())
                        {
                            if(unit->GetGroup()->leader != nullptr && unit->GetGroup()->leader->GetShipType() == Ship::UnitShip)
                            {
                                if(((ShipUnit*)unit)->GetState() == ShipUnit::Idle)
                                    foundShips.push_back(unit);
                            }
                            else foundShips.push_back(unit);
                        }
                    }
                    else foundShips.push_back(unit);
                }
            }

        for(int range = 1; range < CLOSE_MEMBER_RANGE + 1 && foundShips.size() + ship->GetGroup()->members.size() < Ship::MAX_GROUP_MEMBERS; range++)
        {

            //Top
            y = grid.y - range;
            for(int a = -range; a < range; a++)
            {
                x = grid.x + a;
                if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE)
                    for(Ship*& unit : GetGridInfo(y, x).positionShips)
                    {
                        if(unit != ship && team == unit->GetTeam() && unit->GetHierarchy() >= 0 && unit->GetHierarchy() <= 1)
                        {
                            if(unit->GetGroup() != nullptr)
                            {
                                if(ship->GetHierarchy() > 1 && unit->GetGroup() != ship->GetGroup())
                                {
                                    if(unit->GetGroup()->leader != nullptr && unit->GetGroup()->leader->GetShipType() == Ship::UnitShip)
                                    {
                                        if(((ShipUnit*)unit)->GetState() == ShipUnit::Idle)
                                            foundShips.push_back(unit);
                                    }
                                    else foundShips.push_back(unit);
                                }
                            }
                            else foundShips.push_back(unit);
                        }
                    }
            }

            //Bottom
            y = grid.y + range;
            for(int a = -range; a < range; a++)
            {
                x = grid.x + a;
                if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE)
                    for(Ship*& unit : GetGridInfo(y, x).positionShips)
                    {
                        if(unit != ship && team == unit->GetTeam() && unit->GetHierarchy() >= 0 && unit->GetHierarchy() <= 1)
                        {
                            if(unit->GetGroup() != nullptr)
                            {
                                if(ship->GetHierarchy() > 1 && unit->GetGroup() != ship->GetGroup())
                                {
                                    if(unit->GetGroup()->leader != nullptr && unit->GetGroup()->leader->GetShipType() == Ship::UnitShip)
                                    {
                                        if(((ShipUnit*)unit)->GetState() == ShipUnit::Idle)
                                            foundShips.push_back(unit);
                                    }
                                    else foundShips.push_back(unit);
                                }
                            }
                            else foundShips.push_back(unit);
                        }
                    }
            }

            //Left
            x = grid.x - range;
            for(int a = -range + 1; a < range - 1; a++)
            {
                y = grid.y + a;
                if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE)
                    for(Ship*& unit : GetGridInfo(y, x).positionShips)
                    {
                        if(unit != ship && team == unit->GetTeam() && unit->GetHierarchy() >= 0 && unit->GetHierarchy() <= 1)
                        {
                            if(unit->GetGroup() != nullptr)
                            {
                                if(ship->GetHierarchy() > 1 && unit->GetGroup() != ship->GetGroup())
                                {
                                    if(unit->GetGroup()->leader != nullptr && unit->GetGroup()->leader->GetShipType() == Ship::UnitShip)
                                    {
                                        if(((ShipUnit*)unit)->GetState() == ShipUnit::Idle)
                                            foundShips.push_back(unit);
                                    }
                                    else foundShips.push_back(unit);
                                }
                            }
                            else foundShips.push_back(unit);
                        }
                    }
            }

            //Right
            x = grid.x + range;
            for(int a = -range + 1; a < range - 1; a++)
            {
                y = grid.y + a;
                if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE)
                    for(Ship*& unit : GetGridInfo(y, x).positionShips)
                    {
                        if(unit != ship && team == unit->GetTeam() && unit->GetHierarchy() >= 0 && unit->GetHierarchy() <= 1)
                        {
                            if(unit->GetGroup() != nullptr)
                            {
                                if(ship->GetHierarchy() > 1 && unit->GetGroup() != ship->GetGroup())
                                {
                                    if(unit->GetGroup()->leader != nullptr && unit->GetGroup()->leader->GetShipType() == Ship::UnitShip)
                                    {
                                        if(((ShipUnit*)unit)->GetState() == ShipUnit::Idle)
                                            foundShips.push_back(unit);
                                    }
                                    else foundShips.push_back(unit);
                                }
                            }
                            else foundShips.push_back(unit);
                        }
                    }
            }
        }

        int numOfMembers = ship->GetGroup()->members.size();
        for(Ship*& unit : foundShips)
        {
            unit->JoinGroup(ship->GetGroup());
            numOfMembers++;
            if(numOfMembers >= Ship::MAX_GROUP_MEMBERS)
                break;
        }
    }
}

void EntityManager::RefreshEnemyBaseCores()
{
    for(int a = 0; a < Game::NumOfTeams; a++)
    {
        enemyBaseCore[a].clear();

        for(int b = 0; b < Game::NumOfTeams; b++)
            if(Game::GetTeamUpType(a) != Game::GetTeamUpType(b))
                if(instance->basecores[b] != nullptr)
                    enemyBaseCore[a].push_back(instance->basecores[b]);
    }
}

const std::vector<BaseCore*>& EntityManager::GetEnemyBaseCore(int team_)
{
    return enemyBaseCore[team_];
}

void EntityManager::MoveMultipleUnits(const std::list<ShipUnit*>& units_, sf::Vector2f destination_, AStar::SearchType searchType_, short teamUpType_)
{
    mmu_units = units_;
    mmu_destination = destination_;
    mmu_searchType = searchType_;
    mmu_teamUpType = teamUpType_;
    mmu_isActive = false;
}

void EntityManager::InitialiseDefenceGrids()
{
    for(Defence*& defence : instance->defenseObjects)
    {
        DeduceGridRange(defence->GetDefenceSpotRect());
        for(int y = yGridStart; y <= yGridEnd; y++)
            for(int x = xGridStart; x <= xGridEnd; x++)
                if(x >= 0 && x < MapTileManager::MAX_MAPTILE && y >= 0 && y < MapTileManager::MAX_MAPTILE)
                    GetGridInfo(y, x).defence = defence;
    }
}
