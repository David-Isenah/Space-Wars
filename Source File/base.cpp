#include "base.h"
#include "game.h"
#include "public_functions.h"
#include "astarpathfinding.h"
#include <iostream>

sf::Texture* Base::outlineTexture = nullptr;
const long MIN_UNITS = 10;
const long MAX_UNITS = 1000;
const int TILES_PER_UNITS = 2;
const int TILES_PER_FREEUNITS = 32;
const float FADE_TIME = 1.f;
const float BUILD_TIME = 1.3f;
const float REINFORCE_TIME = 60.f;
const float ASK_REINFORCE_TIME = 10.f;
const float DENSITY_FAVOUR_TIME = 15.f;
const float DENSITY_FAVOUR_TIME_CHANGE = 10.f;
const int GROUP_UNIT_SPAWN_MAX = 12;
const float GROUP_UNIT_SPAWN_TIME = 10.f;
const int REINFORCE_DIST = 100;
const float BUILD_DELAY = 3.f;
const float SPAWN_DELAY = 2.f;
const float UPDATE_BASE_AREA_TIME = 10.f;
const int MIN_BASE_AREA = 144;

bool shouldUpdateFrontlineBases = true;
int teamActiveBaseCoverage[Game::NumOfTeams] = {};

std::vector<int> frontLineBases[Game::NumOfTeams];
std::vector<int> frontLineBasesEnemy[Game::NumOfTeams];
std::vector<Base::NetworkInfo> baseNetworkInfo;

Base::Base(short _id, short _team, sf::Vector2i _spGrid, sf::IntRect _range) :
    team(_team),
    id(_id),
    baseSize(0),
    range(_range),
    maxNumUnit(0),
    numOfBuildUnits(0),
    maxNumFreeUnits(0),
    fadeFactor(0),
    fadeTime(0),
    spawnTime(0),
    spawnDelay(0.f),
    buildTime(0),
    buildDelay(0),
    reclaimTime(0),
    reinforceTime(0),
    friendlyDensity(0),
    enemyDensity(0),
    totalDensityWantingToAttack(0),
    totalDensityWantingToDefend(0),
    densityFavourTime(0),
    numOfUnitsInBase(0),
    numOfMinorUnitsInBase(0),
    numOfGroupUnitSpawn(0),
    groupUnitsSpawnTime(GROUP_UNIT_SPAWN_TIME),
    moveUnitsTime(0),
    updateBaseAreaTime(UPDATE_BASE_AREA_TIME * GenerateRandomNumber(101) / 100.f),
    updateBaseArea(true),
    isPlayerInBase(false),
    isEnemyInBase(false),
    isTeamUnitInBase(false),
    isAllyInBase(false),
    isBaseActive(false),
    doesFriendlyHaveTarget(false),
    isMainBase(false)
{
     if(_team >= 0 && _team < Game::NumOfTeams)
        isBaseActive = true;

    for(int a = 0; a < Game::NumOfTeams; a++)
    {
        isTeamAttacking[a] = false;
        isTeamOfficerPlanningAttack[a] = false;
        askReinforceTime[a] = 0.f;
    }

    std::cout << "Created Base - " << id << "\n";

    spawnPoint.grid = _spGrid;
    densityFavourTime = DENSITY_FAVOUR_TIME + GenerateRandomNumber(101) / 100.f * DENSITY_FAVOUR_TIME_CHANGE;

    notifyHighlight.setPrimitiveType(sf::Quads);
    highlight.setPrimitiveType(sf::Quads);
    outline.setPrimitiveType(sf::Quads);
    UpdateRegion(_spGrid, _range);
}

Base::~Base()
{
    defencesInBase.clear();

    for(int a = 0; a < Game::NumOfTeams; a++)
        if(EntityManager::GetBaseCore(a) != nullptr && EntityManager::GetBaseCore(a)->GetBaseId() == id)
            EntityManager::DeleteBaseCore(a);

    for(int y = range.top; y < range.top + range.height; y++)
        for(int x = range.left; x < range.left + range.width; x++)
            if(GetGridInfo(y, x).baseId == id)
                GetGridInfo(y, x).baseId = -1;

    if(isBaseActive && team >= 0 && team < Game::NumOfTeams)
        teamActiveBaseCoverage[team] -= spawnPoint.spawnGrids.size();
}

void Base::Update(float dt)
{
    if(isBaseActive)
    {
        if(buildTime > 0)
            buildTime -= dt;
        if(buildDelay > 0)
            buildDelay -= dt;
        if(spawnTime > 0)
            spawnTime -= dt;
        if(spawnDelay > 0 && isEnemyInBase)
            spawnDelay -= dt;
        if(moveUnitsTime > 0)
            moveUnitsTime -= dt;
        if(reclaimTime > 0)
            reclaimTime -= dt;
        if(reinforceTime > 0)
            reinforceTime -= dt;
        if(groupUnitsSpawnTime > 0)
            groupUnitsSpawnTime -= dt;
        if(updateBaseAreaTime > 0)
            updateBaseAreaTime -= dt;

        for(int a = 0; a < Game::NumOfTeams; a++)
        {
            if(askReinforceTime[a] <= 0)
                askReinforceTime[a] = ASK_REINFORCE_TIME;
            else if(askReinforceTime[a] > 0)
                askReinforceTime[a] -= dt;
        }

        if(groupUnitsSpawnTime <= 0 && numOfGroupUnitSpawn < GROUP_UNIT_SPAWN_MAX)
        {
            groupUnitsSpawnTime = GROUP_UNIT_SPAWN_TIME;
            numOfGroupUnitSpawn += 4;
            if(numOfGroupUnitSpawn > GROUP_UNIT_SPAWN_MAX)
                numOfGroupUnitSpawn = GROUP_UNIT_SPAWN_MAX;
        }

        if(groupsToAddMembers.size() > 0 && spawnTime <= 0.f && numOfBuildUnits > 0 &&
           ((isEnemyInBase || doesFriendlyHaveTarget) ? numOfGroupUnitSpawn > 0 : true))
        {
            if(isEnemyInBase || doesFriendlyHaveTarget)
                numOfGroupUnitSpawn--;
            Ship::UnitGroup* refGroup = groupsToAddMembers[GenerateRandomNumber(groupsToAddMembers.size())];
            spawnTime = (enemyDensity > friendlyDensity ?  1 - (float)abs(enemyDensity - friendlyDensity) / (enemyDensity + friendlyDensity) : 1.f) * 0.4f * ((GenerateRandomNumber(26) + 75) / 100.f);

            sf::Vector2i leaderGrid = GetGridPosition(refGroup->leader->GetPosition());
            std::vector<sf::Vector2i> spawnGrids;
            for(int y = -1; y <= 1; y++)
                for(int x = -1; x <= 1; x++)
                    if(leaderGrid.y + y >= 0 && leaderGrid.y + y < MapTileManager::MAX_MAPTILE &&
                       leaderGrid.x + x >= 0 && leaderGrid.x + x < MapTileManager::MAX_MAPTILE &&
                       GetGridInfo(leaderGrid.y + y, leaderGrid.x + x).baseId == id)
                    {
                        MapTileManager::TileType type = MapTileManager::GetTile(GetGridInfo(leaderGrid.y + y, leaderGrid.x + x).tileId).type;
                        if(type != MapTileManager::Wall && type != MapTileManager::SmallWall)
                            spawnGrids.push_back(sf::Vector2i(leaderGrid.x + x, leaderGrid.y + y));
                    }

            if(spawnGrids.size() > 0)
            {
                numOfBuildUnits--;
                updateBaseArea = true;
                const Game::DifficultyInfo& difficulty = Game::GetDifficultyInfo();

                ShipUnit::UnitType type = ShipUnit::MiniShip;
                Game::SkinType skinType = Game::SkinMiniShip;
                int attributeValue = difficulty.miniShip;
                if(GenerateRandomNumber(101) <= MICROSHIP_SPAWN_RATIO)
                {
                    type = ShipUnit::MicroShip;
                    skinType = Game::SkinMicroShip;
                    attributeValue = difficulty.microShip;
                }

                sf::Vector2f shipPos = ((sf::Vector2f)(spawnGrids[GenerateRandomNumber(spawnGrids.size())] * MapTileManager::TILE_SIZE) + sf::Vector2f(GenerateRandomNumber(MapTileManager::TILE_SIZE), GenerateRandomNumber(MapTileManager::TILE_SIZE)));
                ShipUnit* unit = new ShipUnit(type, shipPos, team, attributeValue, attributeValue, attributeValue, 50 + GenerateRandomNumber(30), 0,
                                              GenerateRandomNumber(Game::GetSkinSize(skinType, team)), skinType);
                EntityManager::CreateShip(unit, false);
                unit->JoinGroup(refGroup);
            }
        }

        if(isEnemyInBase || doesFriendlyHaveTarget)
        {
            buildDelay = BUILD_DELAY;
            if(densityFavourTime > 0)
                densityFavourTime -= dt;

            if(enemyUnitsInBase.size() == 0)
                spawnDelay = 1.f;

            bool isFavouring = abs(friendlyDensity - enemyDensity) <= 3 && densityFavourTime <= 0 && enemyUnitsInBase.size() > 0;

            if(spawnDelay <= 0 && team >= 0 && team < Game::NumOfTeams && spawnTime <= 0 && numOfMinorUnitsInBase < maxNumFreeUnits && numOfBuildUnits > 0 &&
               (friendlyDensity < enemyDensity || isFavouring))
            {
                spawnTime = (enemyDensity > friendlyDensity ?  1 - (float)abs(enemyDensity - friendlyDensity) / (enemyDensity + friendlyDensity) : 1.f) * 0.4f * ((GenerateRandomNumber(21) + 75) / 100.f);

                if(spawnPoint.spawnGrids.size() > 0)
                {
                    numOfBuildUnits--;
                    updateBaseArea = true;
                    numOfUnitsInBase++;
                    isTeamUnitInBase = true;
                    const Game::DifficultyInfo& difficulty = Game::GetDifficultyInfo();

                    ShipUnit::UnitType type = ShipUnit::MiniShip;
                    Game::SkinType skinType = Game::SkinMiniShip;
                    int attributeValue = difficulty.miniShip;
                    if(GenerateRandomNumber(101) <= MICROSHIP_SPAWN_RATIO)
                    {
                        type = ShipUnit::MicroShip;
                        skinType = Game::SkinMicroShip;
                        attributeValue = difficulty.microShip;
                    }

                    sf::Vector2f shipPos;
                    if(enemyUnitsInBase.size() == 0 || GenerateRandomNumber(7) == 0)
                    {
                        std::vector<sf::Vector2i> areas;
                        GetBaseAreaGrids(areas);
                        shipPos = (sf::Vector2f)(areas[GenerateRandomNumber(areas.size())] * MapTileManager::TILE_SIZE) + sf::Vector2f(GenerateRandomNumber(MapTileManager::TILE_SIZE), GenerateRandomNumber(MapTileManager::TILE_SIZE));
                    }
                    else
                    {
                        std::vector<sf::Vector2i> grids;
                        sf::Vector2i refGrid = GetGridPosition(enemyUnitsInBase[GenerateRandomNumber(enemyUnitsInBase.size())]->GetPosition());

                        int arrayRange = 13;
                        bool isChecked[arrayRange][arrayRange] = {};
                        sf::Vector2i offSetGrid = refGrid - sf::Vector2i(6, 6);
                        std::vector<sf::Vector2i> gridsToCheck;
                        sf::Vector2i offSet;

                        gridsToCheck.push_back(refGrid);
                        offSet = refGrid - offSetGrid;
                        isChecked[offSet.y][offSet.x] = true;
                        if(GetGridInfo(refGrid).baseId == id)
                            grids.push_back(refGrid);

                        for(int i = 0; i < gridsToCheck.size(); i++)
                        {
                            for(int y = gridsToCheck[i].y - 1; y <= gridsToCheck[i].y + 1; y++)
                                for(int x = gridsToCheck[i].x - 1; x <= gridsToCheck[i].x + 1; x++)
                                    if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE &&
                                       GetGridInfoTile(y, x).type != MapTileManager::Wall && GetGridInfoTile(y, x).type != MapTileManager::SmallWall)
                                    {
                                        offSet = sf::Vector2i(x, y) - offSetGrid;
                                        if(offSet.x >= 0 && offSet.y >= 0 && offSet.x < arrayRange && offSet.y < arrayRange && isChecked[offSet.y][offSet.x] == false)
                                        {
                                            gridsToCheck.push_back(sf::Vector2i(x, y));
                                            isChecked[offSet.y][offSet.x] = true;
                                            if(GetGridInfo(y, x).baseId == id)
                                                grids.push_back(sf::Vector2i(x, y));
                                        }
                                    }
                        }

                        /*for(int y = -6; y <= 6; y++)
                            for(int x = -6; x <= 6; x++)
                            {
                                int Y = refGrid.y + y;
                                int X = refGrid.x + x;

                                if(Y >= 0 && Y < MapTileManager::MAX_MAPTILE && X >= 0 && X < MapTileManager::MAX_MAPTILE)
                                {
                                    MapTileManager::TileType tileType = MapTileManager::GetTile(GetGridInfo(Y, X).tileId).type;

                                    if(GetGridInfo(Y, X).baseId == id && tileType != MapTileManager::SmallWall && tileType != MapTileManager::Wall)
                                        grids.push_back(sf::Vector2i(X, Y));
                                }
                            }*/

                        if(grids.size() > 0)
                            shipPos = (sf::Vector2f)(grids[GenerateRandomNumber(grids.size())] * MapTileManager::TILE_SIZE) + sf::Vector2f(GenerateRandomNumber(MapTileManager::TILE_SIZE), GenerateRandomNumber(MapTileManager::TILE_SIZE));
                        else
                        {
                            std::vector<sf::Vector2i> areas;
                            GetBaseAreaGrids(areas);
                            shipPos = (sf::Vector2f)(areas[GenerateRandomNumber(areas.size())] * MapTileManager::TILE_SIZE) + sf::Vector2f(GenerateRandomNumber(MapTileManager::TILE_SIZE), GenerateRandomNumber(MapTileManager::TILE_SIZE));
                        }
                    }
                    ShipUnit* unit = new ShipUnit(type, shipPos, team, attributeValue, attributeValue, attributeValue, 50 + GenerateRandomNumber(30), 0,
                                                  GenerateRandomNumber(Game::GetSkinSize(skinType, team)), skinType);
                    EntityManager::CreateShip(unit, false);

                    //Adding members
                    int numOfMembers = GenerateRandomNumber(Ship::MAX_GROUP_MEMBERS + 1);
                    int limitNum = enemyDensity - friendlyDensity - 1;
                    if(limitNum < 0)
                        limitNum = 0;

                    if(isFavouring)
                    {
                        if(numOfMembers > enemyDensity / 2)
                            numOfMembers = GenerateRandomNumber(enemyDensity / 2 + 1);
                    }
                    else numOfMembers /= 2;

                    if(!isFavouring && numOfMembers > limitNum)
                        numOfMembers = limitNum;
                    if(numOfMembers > numOfBuildUnits)
                        numOfMembers = numOfBuildUnits;

                    if(numOfMembers > 0)
                    {
                        Ship::UnitGroup* refGroup = Ship::UnitGroup::CreatUnitGroups();
                        unit->JoinGroup(refGroup, true);

                        sf::Vector2i leaderGrid = GetGridPosition(refGroup->leader->GetPosition());
                        std::vector<sf::Vector2i> spawnGrids;
                        for(int y = -1; y <= 1; y++)
                            for(int x = -1; x <= 1; x++)
                                if(leaderGrid.y + y >= 0 && leaderGrid.y + y < MapTileManager::MAX_MAPTILE &&
                                   leaderGrid.x + x >= 0 && leaderGrid.x + x < MapTileManager::MAX_MAPTILE &&
                                   GetGridInfo(leaderGrid.y + y, leaderGrid.x + x).baseId == id)
                                {
                                    MapTileManager::TileType type = MapTileManager::GetTile(GetGridInfo(leaderGrid.y + y, leaderGrid.x + x).tileId).type;
                                    if(type != MapTileManager::Wall && type != MapTileManager::SmallWall)
                                        spawnGrids.push_back(sf::Vector2i(leaderGrid.x + x, leaderGrid.y + y));
                                }

                        if(spawnGrids.size() > 0)
                        {
                            numOfBuildUnits -= numOfMembers;
                            numOfUnitsInBase += numOfMembers;
                            const Game::DifficultyInfo& difficulty = Game::GetDifficultyInfo();

                            for(int a = 0; a < numOfMembers; a++)
                            {
                                type = ShipUnit::MiniShip;
                                skinType = Game::SkinMiniShip;
                                attributeValue = difficulty.miniShip;
                                if(GenerateRandomNumber(101) <= MICROSHIP_SPAWN_RATIO)
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
                if(densityFavourTime <= 0)
                    densityFavourTime = DENSITY_FAVOUR_TIME + GenerateRandomNumber(101) / 100.f * DENSITY_FAVOUR_TIME_CHANGE;
            }

            if(numOfBuildUnits <= 0.7f * maxNumUnit)
                AskForReinforcement(team);
        }
        else
        {
            spawnDelay = SPAWN_DELAY;
            if(buildDelay <= 0)
            {
                if(buildTime <= 0 && numOfBuildUnits < maxNumUnit)
                {
                    buildTime = BUILD_TIME;
                    numOfBuildUnits++;
                    updateBaseArea = true;
                }

                if(reclaimTime <= 0.f && numOfMinorUnitsInBase > maxNumFreeUnits * 0.4f && (unitsToReclaim.size() > 0))
                {
                    ShipUnit* unit = unitsToReclaim[GenerateRandomNumber(unitsToReclaim.size())];
                    unit->ReclaimToBase(id);

                    if(unit->GetGroup() != nullptr && unit->GetGroup()->leader == unit)
                        Ship::UnitGroup::DeleteGroup(unit->GetGroup());

                    reclaimTime = 0.05f;
                }
            }
            SendReinforcement();
        }

        /*if(isEnemyInBase && moveUnitsTime <= 0.f && AStar::GetLimitedState() == false && AStar::GetProgressionState() == false)
        {
            ShipUnit* unit = nullptr;
            bool creatGroup = false;

            if(unitToMove_subShip.size() > 0)
                unit = unitToMove_subShip[GenerateRandomNumber(unitToMove_subShip.size())];
            else if(unitToMove_leaders.size() > 0 )//&& (unitToMove_free.size() > 0 ? GenerateRandomNumber(3) : true))
                unit = unitToMove_leaders[GenerateRandomNumber(unitToMove_leaders.size())];
            else if(unitToMove_free.size() > 0)
            {
                unit = unitToMove_free[GenerateRandomNumber(unitToMove_free.size())];
                creatGroup = true;
            }

            std::cout << unitToMove_leaders.size() << ", " << unitToMove_free.size() << "\n";

            if(unit != nullptr)
            {
                if(creatGroup)
                {std::cout << "creating group...\n";
                    Ship::UnitGroup* group = Ship::UnitGroup::CreatUnitGroups();
                    unit->JoinGroup(group);
                    EntityManager::RecruitGroupMembers(unit);
                    if(group->members.size() == 1)
                    {
                        unit->MoveToEnemiesInBase();
                        unit->LeaveGroup();
                    }
                }
                else unit->MoveToEnemiesInBase();;

                moveUnitsTime = float(spawnGrids.size()) / AStar::LIMIT_PER_FRAME / 60.f * 10.f;
                if(moveUnitsTime < 3.f)
                    moveUnitsTime = 3.f;
            }
        }*/

        if(team < 0 && team >= Game::NumOfTeams || (numOfBuildUnits <= 0 && numOfUnitsInBase <= 0 && isTeamUnitInBase == false && isAllyInBase == false && isEnemyInBase && (isMainBase ? EntityManager::GetBaseCore(team)->GetAlive() == false : true)))
        {
            isBaseActive = false;
            if(team >= 0 && team < Game::NumOfTeams)
                teamActiveBaseCoverage[team] -= spawnPoint.spawnGrids.size();
            shouldUpdateFrontlineBases = true;
        }
        UpdateBaseArea();
    }

    if(isPlayerInBase)
    {
        if(fadeTime == 0.f)
            fadeTime = fadeFactor * FADE_TIME;
        fadeTime += dt;
    }
    else
    {
        fadeTime = 0.f;
        fadeFactor -= FADE_TIME * dt / 0.3f;
        if(fadeFactor < 0.f)
            fadeFactor = 0.f;
    }

    if(fadeTime > 0.f)
    {
        if(fadeTime < 0.3f * FADE_TIME)
            fadeFactor = fadeTime / (0.3f * FADE_TIME);
        else if(fadeTime < 0.7f * FADE_TIME)
            fadeFactor = 1.f;
        else if(fadeTime < FADE_TIME)
            fadeFactor = 1.f - ((fadeTime - 0.7f * FADE_TIME) / (0.3f * FADE_TIME));
        else fadeFactor = 0;
    }
}

void Base::Refresh()
{
    for(auto a = 0; a < Game::NumOfTeams; a++)
    {
        unitsInBase[a].clear();
        isTeamAttacking[a] = false;
        isTeamOfficerPlanningAttack[a] = false;
    }
    unitsToReclaim.clear();
    enemyUnitsInBase.clear();
    groupsToAddMembers.clear();
    numOfUnitsInBase = 0;
    numOfMinorUnitsInBase = 0;
    friendlyDensity = enemyDensity = 0;
    totalDensityWantingToDefend = totalDensityWantingToAttack = 0;
    isPlayerInBase = false;
    isEnemyInBase = false;
    isTeamUnitInBase = false;
    isAllyInBase = false;
    doesFriendlyHaveTarget = false;
}

void Base::RefreshMainBaseStatus()
{
    isMainBase = false;
    if(EntityManager::GetBaseCore(team) != nullptr && EntityManager::GetBaseCore(team)->GetBaseId() == id)
    {
        isMainBase = true;
        if(isBaseActive && team >= 0 && team < Game::NumOfTeams)
            teamActiveBaseCoverage[team] -= spawnPoint.spawnGrids.size();
        spawnPoint.spawnGrids.clear();

        sf::Vector2i coreGrid = EntityManager::GetBaseCore(team)->GetGrid();
        bool isChecked[range.height][range.width] = {};
        sf::Vector2i offSetGrid(range.left, range.top);
        std::vector<sf::Vector2i> gridsToCheck;
        sf::Vector2i offSet;

        for(int x = -2; x <= 2; x++)
        {
            //Top
            sf::Vector2i grid = coreGrid + sf::Vector2i(x, -2);
            if(grid.x >= 0 && grid.y >= 0 && grid.x < MapTileManager::MAX_MAPTILE && grid.y < MapTileManager::MAX_MAPTILE &&
               GetGridInfo(grid).baseId == id && GetGridInfoTile(grid.y, grid.x).type != MapTileManager::SmallWall)
            {
                gridsToCheck.push_back(grid);
                offSet = grid - offSetGrid;
                isChecked[offSet.y][offSet.x] = true;
                spawnPoint.spawnGrids.push_back(grid);
            }

            //Bottom
            grid = coreGrid + sf::Vector2i(x, 2);
            if(grid.x >= 0 && grid.y >= 0 && grid.x < MapTileManager::MAX_MAPTILE && grid.y < MapTileManager::MAX_MAPTILE &&
               GetGridInfo(grid).baseId == id && GetGridInfoTile(grid.y, grid.x).type != MapTileManager::SmallWall)
            {
                gridsToCheck.push_back(grid);
                offSet = grid - offSetGrid;
                isChecked[offSet.y][offSet.x] = true;
                spawnPoint.spawnGrids.push_back(grid);
            }
        }
        for(int y = -1; y <= 1; y++)
        {
            //Left
            sf::Vector2i grid = coreGrid + sf::Vector2i(-2, y);
            if(grid.x >= 0 && grid.y >= 0 && grid.x < MapTileManager::MAX_MAPTILE && grid.y < MapTileManager::MAX_MAPTILE &&
               GetGridInfo(grid).baseId == id && GetGridInfoTile(grid.y, grid.x).type != MapTileManager::SmallWall)
            {
                gridsToCheck.push_back(grid);
                offSet = grid - offSetGrid;
                isChecked[offSet.y][offSet.x] = true;
                spawnPoint.spawnGrids.push_back(grid);
            }

            //Right
            grid = coreGrid + sf::Vector2i(2, y);
            if(grid.x >= 0 && grid.y >= 0 && grid.x < MapTileManager::MAX_MAPTILE && grid.y < MapTileManager::MAX_MAPTILE &&
               GetGridInfo(grid).baseId == id && GetGridInfoTile(grid.y, grid.x).type != MapTileManager::SmallWall)
            {
                gridsToCheck.push_back(grid);
                offSet = grid - offSetGrid;
                isChecked[offSet.y][offSet.x] = true;
                spawnPoint.spawnGrids.push_back(grid);
            }
        }

        for(int i = 0; i < gridsToCheck.size(); i++)
        {
            for(int y = gridsToCheck[i].y - 1; y <= gridsToCheck[i].y + 1; y++)
                for(int x = gridsToCheck[i].x - 1; x <= gridsToCheck[i].x + 1; x++)
                    if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE && GetGridInfo(y, x).baseId == id &&
                       GetGridInfoTile(y, x).type != MapTileManager::Wall && GetGridInfoTile(y, x).type != MapTileManager::SmallWall)
                    {
                        offSet = sf::Vector2i(x, y) - offSetGrid;
                        if(offSet.x >= 0 && offSet.y >= 0 && offSet.x < range.width && offSet.y < range.height && isChecked[offSet.y][offSet.x] == false)
                        {
                            gridsToCheck.push_back(sf::Vector2i(x, y));
                            isChecked[offSet.y][offSet.x] = true;
                            spawnPoint.spawnGrids.push_back(sf::Vector2i(x, y));
                        }
                    }
        }

        if(isBaseActive && team >= 0 && team < Game::NumOfTeams)
            teamActiveBaseCoverage[team] += spawnPoint.spawnGrids.size();
    }
}

void Base::UpdateRegion(sf::Vector2i spGrid_, sf::IntRect range_)
{
    if(isBaseActive && team >= 0 && team < Game::NumOfTeams)
        teamActiveBaseCoverage[team] -= spawnPoint.spawnGrids.size();

    for(int y = range.top; y < range.top + range.height; y++)
        for(int x = range.left; x < range.left + range.width; x++)
            if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE)
                if(GetGridInfo(y, x).baseId == id)
                    GetGridInfo(y, x).baseId = -1;

    baseSize = 0;
    spawnPoint.spawnGrids.clear();
    range = range_;
    spawnPoint.grid = spGrid_;

    int resetX = range.left;
    int resetY = range.top;

    bool* isChecked = new bool[range.height * range.width] {};
    std::list<sf::Vector2i> checkList;
    std::vector<sf::Vector2i> regionGrids;

    auto getCheckStatus = [&](sf::Vector2i grid_)->bool
    {
        int x = grid_.x - resetX;
        int y = grid_.y - resetY;
        if(x >= 0 && x < range.width && y >= 0 && y < range.height)
            return isChecked[y * range.width + x];
        return false;
    };

    auto isValid = [&](sf::Vector2i grid_, int baseId_ = -1)->bool
    {
        if(grid_.x >= range.left && grid_.x < range.left + range.width &&
           grid_.y >= range.top && grid_.y < range.top + range.height &&
           GetGridInfo(grid_.y, grid_.x).baseId == baseId_ &&
           MapTileManager::GetTile(GetGridInfo(grid_.y, grid_.x).tileId).type != MapTileManager::Wall)
           return true;
        return false;
    };

    auto addToList = [&](sf::Vector2i grid_, bool onlySmallWalls = false)
    {
        bool addIt = false;
        if(onlySmallWalls && MapTileManager::GetTile(GetGridInfo(grid_.y, grid_.x).tileId).type == MapTileManager::SmallWall && isValid(grid_))
            addIt = true;
        else if(onlySmallWalls == false && isValid(grid_))
            addIt = true;

        if(addIt)
        {
            checkList.push_back(grid_);
            regionGrids.push_back(grid_);
            GetGridInfo(grid_.y, grid_.x).baseId = id;
            baseSize++;

            if(MapTileManager::GetTile(GetGridInfo(grid_.y, grid_.x).tileId).type == MapTileManager::Floor ||
               MapTileManager::GetTile(GetGridInfo(grid_.y, grid_.x).tileId).type == MapTileManager::Empty)
                spawnPoint.spawnGrids.push_back(grid_);
        }
    };

    addToList(spGrid_);
    while(checkList.size() > 0)
    {
        sf::Vector2i grid_ = *checkList.begin();
        int x = grid_.x - resetX;
        int y = grid_.y - resetY;
        if(x >= 0 && x < range.width && y >= 0 && y < range.height)
        {
            isChecked[y * range.width + x] = true;
            bool onlySmallWalls = false;

            if(MapTileManager::GetTile(GetGridInfo(grid_.y, grid_.x).tileId).type == MapTileManager::SmallWall)
                onlySmallWalls = true;

            addToList(grid_ + sf::Vector2i(1, 0), onlySmallWalls);
            addToList(grid_ + sf::Vector2i(-1, 0), onlySmallWalls);
            addToList(grid_ + sf::Vector2i(0, 1), onlySmallWalls);
            addToList(grid_ + sf::Vector2i(0, -1), onlySmallWalls);
        }

        checkList.erase(checkList.begin());
    }

    for(int a = 0; a < regionGrids.size(); a++)
    {
        sf::Vertex vertices[4];
        sf::Color color_;

        if(team == Game::Alpha)
            color_ = Game::GetColor("Alpha_Light");
        else if(team == Game::Delta)
            color_ = Game::GetColor("Delta_Light");
        else if(team == Game::Vortex)
            color_ = Game::GetColor("Vortex_Light");
        else if(team == Game::Omega)
            color_ = Game::GetColor("Omega_Light");
        else color_ = sf::Color(170, 170, 170);
        color_.a = 100;

        sf::Vector2f refPos(regionGrids[a].x * MapTileManager::TILE_SIZE, regionGrids[a].y * MapTileManager::TILE_SIZE);
        vertices[0].position = sf::Vector2f(0, 0) + refPos;
        vertices[1].position = sf::Vector2f(MapTileManager::TILE_SIZE, 0) + refPos;
        vertices[2].position = sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE) + refPos;
        vertices[3].position = sf::Vector2f(0, MapTileManager::TILE_SIZE) + refPos;

        for(int b = 0; b < 4; b++)
            vertices[b].color = color_;
        for(int b = 0; b < 4; b++)
            highlight.append(vertices[b]);
        for(int b = 0; b < 4; b++)
            vertices[b].color = sf::Color::White;

        vertices[0].texCoords = sf::Vector2f(0, 0);
        vertices[1].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE, 0);
        vertices[2].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE);
        vertices[3].texCoords = sf::Vector2f(0, MapTileManager::TILE_SIZE);

        bool l = false;
        bool r = false;
        bool t = false;
        bool b = false;

        if(isValid(regionGrids[a] + sf::Vector2i(-1, 0), id)) l = true;
        if(isValid(regionGrids[a] + sf::Vector2i(1, 0), id)) r = true;
        if(isValid(regionGrids[a] + sf::Vector2i(0, -1), id)) t = true;
        if(isValid(regionGrids[a] + sf::Vector2i(0, 1), id)) b = true;

        if(l == false)
        {
            sf::Vector2f txRefPos(0, MapTileManager::TILE_SIZE * 0);

            vertices[0].texCoords = sf::Vector2f(0, 0) + txRefPos;
            vertices[1].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE, 0) + txRefPos;
            vertices[2].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE) + txRefPos;
            vertices[3].texCoords = sf::Vector2f(0, MapTileManager::TILE_SIZE) + txRefPos;

            for(int c = 0; c < 4; c++)
                outline.append(vertices[c]);
        }
        if(t == false)
        {
            sf::Vector2f txRefPos(0, MapTileManager::TILE_SIZE * 1);

            vertices[0].texCoords = sf::Vector2f(0, 0) + txRefPos;
            vertices[1].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE, 0) + txRefPos;
            vertices[2].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE) + txRefPos;
            vertices[3].texCoords = sf::Vector2f(0, MapTileManager::TILE_SIZE) + txRefPos;

            for(int c = 0; c < 4; c++)
                outline.append(vertices[c]);
        }
        if(r == false)
        {
            sf::Vector2f txRefPos(0, MapTileManager::TILE_SIZE * 2);

            vertices[0].texCoords = sf::Vector2f(0, 0) + txRefPos;
            vertices[1].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE, 0) + txRefPos;
            vertices[2].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE) + txRefPos;
            vertices[3].texCoords = sf::Vector2f(0, MapTileManager::TILE_SIZE) + txRefPos;

            for(int c = 0; c < 4; c++)
                outline.append(vertices[c]);
        }
        if(b == false)
        {
            sf::Vector2f txRefPos(0, MapTileManager::TILE_SIZE * 3);

            vertices[0].texCoords = sf::Vector2f(0, 0) + txRefPos;
            vertices[1].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE, 0) + txRefPos;
            vertices[2].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE) + txRefPos;
            vertices[3].texCoords = sf::Vector2f(0, MapTileManager::TILE_SIZE) + txRefPos;

            for(int c = 0; c < 4; c++)
                outline.append(vertices[c]);
        }
    }

    if(maxNumUnit <= 0)
        SetDefaultMaxUnits();

    maxNumFreeUnits = baseSize / TILES_PER_FREEUNITS;
    //if(maxNumFreeUnits > maxNumUnit)
        //maxNumFreeUnits = maxNumUnit;
    delete[] isChecked;

    if(isBaseActive && team >= 0 && team < Game::NumOfTeams)
        teamActiveBaseCoverage[team] += spawnPoint.spawnGrids.size();
}

void Base::UpdateRegion()
{
    UpdateRegion(spawnPoint.grid, range);
}

void Base::AddDefence(Defence* defence)
{
    if(defence != nullptr)
        defencesInBase.push_back(defence);
}

bool Base::IsActive()
{
    return isBaseActive;
}

bool Base::IsMainBase()
{
    return isMainBase;
}

sf::IntRect Base::GetRange()
{
    return range;
}

short Base::GetTeam()
{
    return team;
}

bool Base::GetGenerateState()
{
    if(isBaseActive && buildDelay <= 0 && isEnemyInBase == false && doesFriendlyHaveTarget == false)
        return true;
    return false;
}

long Base::GetBaseSize()
{
    return baseSize;
}

int Base::GetMaxUnits()
{
    return maxNumUnit;
}

int Base::GetNumOfBuildUnits()
{
    return numOfBuildUnits;
}

int Base::GetNumOfInactiveDefences()
{
    int num = 0;
    for(Defence* defence : defencesInBase)
        if(defence->IsShipDefenceAlive() == false)
            num++;
    return num;
}

Base::SpawnPointObject* Base::GetSpawnPoint()
{
    return &spawnPoint;
}

bool Base::IsEnemyInBase()
{
    return isEnemyInBase;
}

bool Base::IsFriendlyTargeting()
{
    return doesFriendlyHaveTarget;
}

bool Base::IsBaseActive()
{
    return isBaseActive;
}

void Base::SetDefaultMaxUnits()
{
    maxNumUnit = baseSize / TILES_PER_UNITS;

    if(maxNumUnit < MIN_UNITS)
        maxNumUnit = MIN_UNITS;
    if(maxNumUnit > MAX_UNITS)
        maxNumUnit = MAX_UNITS;

    numOfBuildUnits = maxNumUnit * 1.f;
}

void Base::SetMaxUnits(long units)
{
    maxNumUnit = units;

    if(maxNumUnit < MIN_UNITS)
        maxNumUnit = MIN_UNITS;
    if(maxNumUnit > MAX_UNITS)
        maxNumUnit = MAX_UNITS;

    if(isBaseActive)
        numOfBuildUnits = maxNumUnit * 1.f;
}

void Base::DrawNotifyHighlight(sf::RenderTarget& target_, sf::FloatRect screenRect_)
{
    if(fadeFactor > 0.f)
    {
        notifyHighlight.clear();

        sf::IntRect screenGridRect(screenRect_.left / MapTileManager::TILE_SIZE,
                                   screenRect_.top / MapTileManager::TILE_SIZE,
                                   screenRect_.width / MapTileManager::TILE_SIZE,
                                   screenRect_.height / MapTileManager::TILE_SIZE);
        sf::Vector2f refPos[4] =
        {
            sf::Vector2f(0, 0),
            sf::Vector2f(MapTileManager::TILE_SIZE, 0),
            sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE),
            sf::Vector2f(0, MapTileManager::TILE_SIZE)
        };

        sf::Color color_;
        if(team == Game::Alpha)
            color_ = Game::GetColor("Alpha_Light");
        else if(team == Game::Delta)
            color_ = Game::GetColor("Delta_Light");
        else if(team == Game::Vortex)
            color_ = Game::GetColor("Vortex_Light");
        else if(team == Game::Omega)
            color_ = Game::GetColor("Omega_Light");
        else color_ = sf::Color(170, 170, 170);

        sf::Vertex vertex;
        vertex.color = color_;
        vertex.color.a = 120.f * fadeFactor;

        sf::Vector2f realPos;
        for(int y = screenGridRect.top; (y <= range.top + range.height) && (y <= screenGridRect.top + screenGridRect.height + 1); y++)
            for(int x = screenGridRect.left; (x <= range.left + range.width) && (x <= screenGridRect.left + screenGridRect.width + 1); x++)
                if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE)
                    if(GetGridInfo(y, x).baseId == id)
                    {
                        realPos.x = x * MapTileManager::TILE_SIZE;
                        realPos.y = y * MapTileManager::TILE_SIZE;

                        for(int a = 0; a < 4; a++)
                        {
                            vertex.position = realPos + refPos[a];
                            notifyHighlight.append(vertex);
                        }
                    }
        target_.draw(notifyHighlight);
    }
}

void Base::DrawHighlight(sf::RenderTarget& target_)
{
    target_.draw(highlight);
}

void Base::DrawOutline(sf::RenderTarget& target_)
{
    target_.draw(outline, outlineTexture);
}

void Base::SetId(short id_)
{
    id = id_;
}

bool Base::CanMoveToEnemy(const short& team_)
{
    if(moveUnitsTime <= 0.f)
    {
        for(int a = 0; a < Game::NumOfTeams; a++)
            if(Game::GetTeamUpType(a) != Game::GetTeamUpType(team_) && unitsInBase[a].size() > 0)
                return true;
    }
    return false;
}

void Base::ResetMoveToEnemy()
{
    moveUnitsTime = float(spawnPoint.spawnGrids.size()) / AStar::LIMIT_PER_FRAME / 60.f * 10.f;
    if(moveUnitsTime < 1.f)
        moveUnitsTime = 1.f;
}

void Base::AddUnit(ShipDefence* unit_)
{
    if(unit_ != nullptr && isBaseActive)
    {
        if(GetGridInfo(GetGridPosition(unit_->GetPosition())).baseId == id)
        {
            unitsInBase[unit_->GetTeam()].push_back(unit_);
            if(Game::GetTeamUpType(unit_->GetTeam()) != Game::GetTeamUpType(team))
            {
                isEnemyInBase = true;
                if(unit_->GetShipTarget() != nullptr)
                {
                    enemyDensity += unit_->GetShipDensityValue();
                    enemyUnitsInBase.push_back(unit_);
                }
                isTeamAttacking[unit_->GetTeam()] = true;
            }
            else
            {
                isAllyInBase = true;
                if(unit_->GetShipTarget() != nullptr)
                {
                    doesFriendlyHaveTarget = true;
                    isTeamAttacking[unit_->GetShipTarget()->GetTeam()] = true;
                    friendlyDensity += unit_->GetShipDensityValue();
                }
            }

            if(unit_->GetTeam() == team)
            {
                isTeamUnitInBase = true;
                numOfUnitsInBase++;
            }
        }
    }
}

void Base::AddUnit(ShipUnit* unit_)
{
    if(unit_ != nullptr && isBaseActive)
    {
        if(GetGridInfo(GetGridPosition(unit_->GetPosition())).baseId == id)
        {
            bool isInBaseArea = GetGridInfo(GetGridPosition(unit_->GetPosition())).baseArea;

            unitsInBase[unit_->GetTeam()].push_back(unit_);
            if(ShipUnit::GetActivePlayerUnit() == unit_)
                isPlayerInBase = true;

            if(Game::GetTeamUpType(unit_->GetTeam()) != Game::GetTeamUpType(team))
            {
                isEnemyInBase = true;
                if(isInBaseArea)
                {
                    enemyDensity += unit_->GetShipDensityValue();
                    enemyUnitsInBase.push_back(unit_);
                }
                isTeamAttacking[unit_->GetTeam()] = true;
            }
            else
            {
                isAllyInBase = true;
                if(isInBaseArea)
                    friendlyDensity += unit_->GetShipDensityValue();
                if(unit_->GetShipTarget() != nullptr)
                {
                    doesFriendlyHaveTarget = true;
                    isTeamAttacking[unit_->GetShipTarget()->GetTeam()] = true;
                }
            }

            if(unit_->GetTeam() == team)
            {
                if((unit_->GetUnitType() == ShipUnit::MiniShip || unit_->GetUnitType() == ShipUnit::MicroShip) && unit_->GetReclaimingStatus() == false)
                {
                    if(unit_->GetGroup() != nullptr ? (unit_->GetGroup()->leader != nullptr && unit_->GetGroup()->leader->GetHierarchy() <= 1) : true)
                        numOfMinorUnitsInBase++;

                    if(isEnemyInBase == false  && unit_->GetShipTarget() == nullptr && unit_->GetState() == ShipUnit::Idle)
                    {
                        unitsToReclaim.push_back(unit_);
                        if(unit_->GetGroup() != nullptr && unit_->GetGroup()->leader == unit_)
                            Ship::UnitGroup::DeleteGroup(unit_->GetGroup());
                    }
                }

                isTeamUnitInBase = true;
                numOfUnitsInBase++;

                if(unit_->GetGroup() != nullptr && unit_->GetHierarchy() > 1 && unit_->GetGroup()->leader == unit_ && unit_->GetGroup()->members.size() < Ship::MAX_GROUP_MEMBERS)
                {
                    if(((isEnemyInBase || doesFriendlyHaveTarget) && GetEnemyDensityDifference() == 0 && GetGridInfo(GetGridPosition(unit_->GetPosition())).baseArea) ? unit_->GetGroup()->members.size() < 2 : true)
                        groupsToAddMembers.push_back(unit_->GetGroup());
                }
            }
        }
    }
}

void Base::AddShipDensity(Ship* ship_)
{
    if(ship_ != nullptr)
    {
        const GridInfo& gridInfo = GetGridInfo(GetGridPosition(ship_->GetPosition()));

        if(gridInfo.baseId != id)
            unitsInBase[ship_->GetTeam()].push_back(ship_);
        if(gridInfo.baseId == id ? gridInfo.baseArea != true : true)
        {
            if(Game::GetTeamUpType(ship_->GetTeam()) != Game::GetTeamUpType(team))
            {
                enemyDensity += ship_->GetShipDensityValue();
                enemyUnitsInBase.push_back(ship_);
            }
            else friendlyDensity += ship_->GetShipDensityValue();
        }
    }
}

void Base::HasRecaimedUnit()
{
    numOfBuildUnits++;
}

void Base::InitializeBaseAreas()
{
    for(int a = 0; a < Game::GetNumberOfBases(); a++)
    {
        Base* base = Game::GetBase(a);
        for(sf::Vector2i grid : base->spawnPoint.spawnGrids)
            GetGridInfo(grid).baseArea = true;

        for(Defence* defence : base->GetDefensesInBase())
        {
            sf::FloatRect rect = defence->GetDefenceSpotRect();
            sf::Vector2i refGrid(rect.left /= MapTileManager::TILE_SIZE, rect.top / MapTileManager::TILE_SIZE);

            for(int y = refGrid.y; y < refGrid.y + 2; y++)
                for(int x = refGrid.x; x < refGrid.x + 2; x++)
                    if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE)
                        GetGridInfo(y, x).baseArea = true;

        }
    }
}

const std::vector<Defence*>& Base::GetDefensesInBase()
{
    return defencesInBase;
}

void Base::InitializeNetworkInfo()
{
    baseNetworkInfo.clear();
    baseNetworkInfo.resize(Game::GetNumberOfBases());

    for(int a = 0; a < Game::GetNumberOfBases(); a++)
    {
        Base* base = Game::GetBase(a);
        std::vector<sf::Vector2i> tempPath;
        std::vector<long> tempDist;
        //Connected Bases
        for(int b = 0; b < Game::GetNumberOfBases(); b++)
        {
            if(b != a)
            {
                tempPath.clear();
                AStar::FindPath(base->GetSpawnPoint()->grid, Game::GetBase(b)->GetSpawnPoint()->grid, tempPath, AStar::NoSearchType, -1, true);
                for(int c = 0; c < tempPath.size(); c++)
                {
                    int gridId = GetGridInfo(tempPath[c].y, tempPath[c].x).baseId;
                    if(gridId == b)
                    {
                        int pushBackId = b;
                        long dist = c;

                        for(int d = 0; d < baseNetworkInfo[a].connectedBases.size(); d++)
                        {
                            unsigned int& id = baseNetworkInfo[a].connectedBases[d];

                            if(dist < tempDist[d])
                            {
                                int tempId = id;
                                long tempDistValue = tempDist[d];
                                id = pushBackId;
                                tempDist[d] = dist;
                                pushBackId = tempId;
                                dist = tempDistValue;
                            }
                        }
                        baseNetworkInfo[a].connectedBases.push_back(pushBackId);
                        tempDist.push_back(dist);
                        break;
                    }
                    else if(gridId >= 0 && gridId != b && gridId != a)
                        break;
                }
            }
        }

        //Network Value
        for(int b = 0; b < Game::NumOfTeams; b++)
            if(EntityManager::GetBaseCore(b) != nullptr)
            {
                sf::Vector2i coreGrid = EntityManager::GetBaseCore(b)->GetGrid();
                sf::Vector2i mainBaseGrid = Game::GetBase(GetGridInfo(coreGrid.y, coreGrid.x).baseId)->GetSpawnPoint()->grid;

                if(Game::GetBase(GetGridInfo(coreGrid.y, coreGrid.x).baseId) == base)
                    baseNetworkInfo[a].networkValue[b] = 0;
                else
                {
                    tempPath.clear();
                    AStar::FindPath(mainBaseGrid, base->GetSpawnPoint()->grid, tempPath, AStar::NoSearchType, -1, true);
                    for(int c = 0; c < tempPath.size(); c++)
                    {
                        if(GetGridInfo(tempPath[c].y, tempPath[c].x).baseId == a)
                        {
                            baseNetworkInfo[a].networkValue[b] = c;
                            break;
                        }
                    }
                }
            }

        //Reinforcement Bases
        for(int b = 0; b < Game::GetNumberOfBases(); b++)
            if(a != b)
            {
                tempPath.clear();
                AStar::FindPath(Game::GetBase(b)->GetSpawnPoint()->grid, base->GetSpawnPoint()->grid, tempPath, AStar::AvoidEnemySearchType, Game::GetTeamUpType(Game::GetBase(b)->GetTeam()), true);
                for(int c = 0; c < tempPath.size() && c <= REINFORCE_DIST; c++)
                    if(GetGridInfo(tempPath[c].y, tempPath[c].x).baseId == a)
                    {
                        baseNetworkInfo[a].reinforcementBases[Game::GetBase(b)->GetTeam()].push_back(b);
                        break;
                    }
            }

        for(auto id : baseNetworkInfo[a].connectedBases)
        {
            int tempTeam = Game::GetBase(id)->GetTeam();
            bool addId = true;
            for(int b = 0; b < baseNetworkInfo[a].reinforcementBases[tempTeam].size(); b++)
                if(id == baseNetworkInfo[a].reinforcementBases[tempTeam][b])
                {
                    addId = false;
                    break;
                }
            if(addId)
                baseNetworkInfo[a].reinforcementBases[tempTeam].push_back(id);
        }
    }
}

void Base::GetBaseAreaGrids(std::vector<sf::Vector2i>& grids)
{
    int gridArea = ((float)numOfBuildUnits / maxNumUnit * 0.7f + 0.3f) * (spawnPoint.spawnGrids.size() - MIN_BASE_AREA) + MIN_BASE_AREA;
    if(gridArea < MIN_BASE_AREA)
        gridArea = MIN_BASE_AREA;

    for(int a = 0; a < gridArea && a < spawnPoint.spawnGrids.size(); a++)
        grids.push_back(spawnPoint.spawnGrids[a]);
}

sf::Vector2i Base::GetRandomBaseGrid()
{
    if(spawnPoint.spawnGrids.size() > 0)
        return spawnPoint.spawnGrids[GenerateRandomNumber(spawnPoint.spawnGrids.size())];

    return sf::Vector2i();
}

const Base::NetworkInfo& Base::GetNetworkInfo(unsigned int id)
{
    return baseNetworkInfo[id];
}

std::vector<Ship*>* Base::GetUnitsInBase(int team_)
{
    return &unitsInBase[team_];
}

//UpdateFrontLineBases() useful stuffs
std::vector<bool> hasChecked[Game::NumOfTeams];

void AddBaseToFrontLine(int id, int enemyTeam)
{
    //Front line of base
    int team = Game::GetBase(id)->GetTeam();
    bool add_id = true;
    int push_back_id = id;

    if(team >= 0 && team < Game::NumOfTeams)
    {
        for(auto& baseId : frontLineBases[team])
            if(baseId == id)
            {
                add_id = false;
                break;
            }

        if(add_id)
        {
            for(auto& baseId : frontLineBases[team])
            {
                if(baseNetworkInfo[push_back_id].networkValue < baseNetworkInfo[baseId].networkValue)
                {
                    int temp_id = baseId;
                    baseId = push_back_id;
                    push_back_id = temp_id;
                }
            }

            frontLineBases[team].push_back(push_back_id);
        }
    }

    //Front line of enemy
    add_id = true;
    push_back_id = id;

    if(enemyTeam >= 0 && enemyTeam < Game::NumOfTeams)
    {
        for(auto& baseId : frontLineBasesEnemy[enemyTeam])
            if(baseId == id)
            {
                add_id = false;
                break;
            }

        if(add_id)
        {
            for(auto& baseId : frontLineBasesEnemy[enemyTeam])
            {
                if(baseNetworkInfo[push_back_id].networkValue < baseNetworkInfo[baseId].networkValue)
                {
                    int temp_id = baseId;
                    baseId = push_back_id;
                    push_back_id = temp_id;
                }
            }

            frontLineBasesEnemy[enemyTeam].push_back(push_back_id);
        }
    }
}

void FindEnemiesOnBase(int id, int team)
{
    if(hasChecked[team][id] == false)
    {
        hasChecked[team][id] = true;
        for(auto& cBaseId : baseNetworkInfo[id].connectedBases)
        {
            Base* cBase = Game::GetBase(cBaseId);
            if(cBase->IsActive() && Game::GetTeamUpType(cBase->GetTeam()) != Game::GetTeamUpType(team))
                AddBaseToFrontLine(cBaseId, team);
            else if(cBase->IsActive() == false)
                FindEnemiesOnBase(cBaseId, team);
        }
    }
}

void Base::UpdateFrontLineBases(bool ignoreCheck)
{
    if(ignoreCheck || shouldUpdateFrontlineBases)
    {
        shouldUpdateFrontlineBases = false;

        for(int a = 0; a < Game::NumOfTeams; a++)
        {
            frontLineBases[a].clear();
            frontLineBasesEnemy[a].clear();
        }

        if(Game::GetLevelMode() == Game::BossMode)
            ;
        else if(Game::GetLevelMode() == Game::DefendMode)
        {
            if(Game::GetNumberOfBases() >= 1)
            {
                int teamUp = Game::GetTeamUpType(Game::GetBase(0)->GetTeam());
                for(int a = 0; a < Game::NumOfTeams; a++)
                {
                    if(Game::GetTeamUpType(a) == teamUp)
                        frontLineBases[a].push_back(0);
                    else frontLineBasesEnemy[a].push_back(0);
                }
            }
        }
        else
        {
            for(int a = 0; a < Game::NumOfTeams; a++)
            {
                hasChecked[a].clear();
                hasChecked[a].resize(Game::GetNumberOfBases(), false);
            }

            for(int a = 0; a < Game::GetNumberOfBases(); a++)
            {
                if(Game::GetBase(a)->IsActive())
                    FindEnemiesOnBase(a, Game::GetBase(a)->GetTeam());
            }

            std::vector<int> tempIncludeBasesEnemy[4];
            std::vector<int> tempIncludeBases[4];
            for(int a = 0; a < Game::NumOfTeams; a++)
                if(frontLineBasesEnemy[a].size() == 0)
                    for(int b = 0; b < Game::NumOfTeams; b++)
                        if(b != a && Game::GetTeamUpType(a) == Game::GetTeamUpType(b))
                        {
                            //Enemy front-line
                            for(int c = 0; c < frontLineBasesEnemy[b].size(); c++)
                            {
                                bool addIt = true;

                                for(int d = 0; d < tempIncludeBasesEnemy[a].size(); d++)
                                    if(tempIncludeBasesEnemy[a][d] == frontLineBasesEnemy[b][c])
                                    {
                                        addIt = false;
                                        break;
                                    }

                                if(addIt)
                                    tempIncludeBasesEnemy[a].push_back(frontLineBasesEnemy[b][c]);
                            }

                            //Friendly front-line
                            for(int c = 0; c < frontLineBases[b].size(); c++)
                            {
                                bool addIt = true;

                                for(int d = 0; d < tempIncludeBases[a].size(); d++)
                                    if(tempIncludeBases[a][d] == frontLineBases[b][c])
                                    {
                                        addIt = false;
                                        break;
                                    }

                                if(addIt)
                                    tempIncludeBases[a].push_back(frontLineBases[b][c]);
                            }
                        }

            //If no front-line bases, include ally bases(tempIncludeBases)
            for(int a = 0; a < Game::NumOfTeams; a++)
                if(tempIncludeBasesEnemy[a].size() > 0)
                    for(int b = 0; b < tempIncludeBasesEnemy[a].size(); b++)
                        frontLineBasesEnemy[a].push_back(tempIncludeBasesEnemy[a][b]);

            for(int a = 0; a < Game::NumOfTeams; a++)
                if(tempIncludeBases[a].size() > 0)
                    for(int b = 0; b < tempIncludeBases[a].size(); b++)
                        frontLineBases[a].push_back(tempIncludeBases[a][b]);

            //If still no front line, include all bases
            for(int a = 0; a < Game::NumOfTeams; a++)
                if(frontLineBasesEnemy[a].size() == 0)
                    for(int b = 0; b < Game::GetNumberOfBases(); b++)
                    {
                        Base* tempBase = Game::GetBase(b);
                        if(tempBase->IsActive() && Game::GetTeamUpType(a) != Game::GetTeamUpType(tempBase->GetTeam()))
                            frontLineBasesEnemy[a].push_back(b);
                    }

            for(int a = 0; a < Game::NumOfTeams; a++)
                if(frontLineBases[a].size() == 0)
                    for(int b = 0; b < Game::GetNumberOfBases(); b++)
                    {
                        Base* tempBase = Game::GetBase(b);
                        if(tempBase->IsActive() && Game::GetTeamUpType(a) == Game::GetTeamUpType(tempBase->GetTeam()))
                            frontLineBases[a].push_back(b);
                    }
        }


        /*Displaying details
        for(int a = 0; a < Game::GetNumberOfBases(); a++)
        {
            if(true)
            {
                std::cout << "#" << a << " Base" << "\nNetwork Value :\n";
                for(int b = 0; b < 4; b++)
                    std::cout << baseNetworkInfo[a].networkValue[b] << "\n";
                std::cout << "Connected Bases:\n";
                for(auto& cBase : baseNetworkInfo[a].connectedBases)
                    std::cout << cBase << "\n";
            }

            std::cout << "\n";
        }

        for(int a = 0; a < 4; a++)
        {
            std::cout << "#" << a << " Team\n";
            std::cout << "Front Line Bases:\n";
            for(auto& id : frontLineBases[a])
                std::cout << id << "\n";
            std::cout << "Front Line Bases Enemy:\n";
            for(auto& id : frontLineBasesEnemy[a])
                std::cout << id << "\n";
            std::cout << "\n";
        }*/
    }
}

const std::vector<int>& Base::GetFrontLineBases(unsigned short team)
{
    if(team >=0 && team < Game::NumOfTeams)
        return frontLineBases[team];
    return frontLineBases[0];
}

const std::vector<int>& Base::GetFrontLineBasesEnemy(unsigned short team)
{
    if(team >=0 && team < Game::NumOfTeams)
        return frontLineBasesEnemy[team];
    return frontLineBasesEnemy[0];
}

bool Base::IsTeamAttacking(short team)
{
    if(team >= 0 && team < Game::NumOfTeams)
        return isTeamAttacking[team];
    return false;
}

bool Base::IsTeamOfficerPlanningAttack(short team)
{
    if(team >= 0 && team < Game::NumOfTeams)
        return isTeamOfficerPlanningAttack[team];
    return false;
}

void Base::TeamOfficerIsPlanningAttack(short team)
{
    if(team >= 0 && team < Game::NumOfTeams)
        isTeamOfficerPlanningAttack[team] = true;
}

int Base::GetNumOfBasesReinforcing()
{
    return basesToReinforce.size();
}

void Base::AskForReinforcement(short team_)
{
    int densityDiff = 0;
    if(Game::GetTeamUpType(team_) != Game::GetTeamUpType(team))
        densityDiff = totalDensityWantingToDefend - totalDensityWantingToAttack;
    else densityDiff = totalDensityWantingToAttack - totalDensityWantingToDefend;

    if(team_ >= 0 && team_ < Game::NumOfTeams && askReinforceTime[team_] <= 0 && densityDiff >= -4)
    {
        std::vector<int> reachableReinforcementBases;
        std::vector<int> tempBases;

        for(auto baseId : baseNetworkInfo[id].reinforcementBases[team_])
            if(Game::GetBase(baseId)->IsActive())
                reachableReinforcementBases.push_back(baseId);

        if(reachableReinforcementBases.size() == 0)
        {
            tempBases.push_back(id);

            for(int a = 0; a < tempBases.size() && reachableReinforcementBases.size() == 0; a++)
                for(auto cBaseId : baseNetworkInfo[tempBases[a]].connectedBases)
                    if(Game::GetBase(cBaseId)->IsActive() == false)
                    {
                        for(auto rBaseId : baseNetworkInfo[cBaseId].reinforcementBases[team_])
                            if(Game::GetBase(rBaseId)->IsActive())
                                reachableReinforcementBases.push_back(rBaseId);

                        for(auto ccBaseId : baseNetworkInfo[cBaseId].connectedBases)
                            if(Game::GetBase(ccBaseId)->IsActive() == false)
                            {
                                bool include = true;
                                for(auto tBaseId : tempBases)
                                    if(tBaseId == ccBaseId)
                                    {
                                        include = false;
                                        break;
                                    }
                                if(include)
                                    tempBases.push_back(ccBaseId);
                            }
                    }
        }


        int baseId = -1;
        int lessBusyBaseId = -1;
        int lessBusyBaseRBases = -1;
        for(auto reiforceBase : reachableReinforcementBases)
        {
            if(Game::GetBase(reiforceBase)->IsEnemyInBase() == false && Game::GetBase(reiforceBase)->IsActive())
            {
                /*if(Game::GetBase(reiforceBase)->GetNumOfBasesReinforcing() == 0)
                {
                    baseId = reiforceBase;
                    break;
                }
                else */if(lessBusyBaseRBases < 0 || Game::GetBase(reiforceBase)->GetNumOfBasesReinforcing() < lessBusyBaseRBases)
                    lessBusyBaseId = reiforceBase;
            }
        }
        if(baseId < 0 && lessBusyBaseId >= 0)
            baseId = lessBusyBaseId;

        if(baseId >= 0)
        {
            Game::GetBase(baseId)->Reinforce(id);
            askReinforceTime[team_] = ASK_REINFORCE_TIME;
            std::cout<< id << "# asking\n";
        }
        else askReinforceTime[team_] = 3.f;
    }
}

void Base::Reinforce(short id_)
{
    if(id_ >= 0 && id_ < Game::GetNumberOfBases() && id_ != id)
    {
        bool addIt = true;
        for(auto rBaseId: basesToReinforce)
            if(rBaseId == id_)
            {
                addIt = false;
                break;
            }

        if(addIt)
            basesToReinforce.push_back(id_);
    }
}

void Base::SendReinforcement()
{
    if(isEnemyInBase == false && numOfBuildUnits > 0.4f * maxNumUnit && numOfBuildUnits >= 9 && reinforceTime <= 0 && basesToReinforce.size() >= 1)
    {
        Base* base = Game::GetBase(*basesToReinforce.begin());

        int densityDiff = 0;
        if(Game::GetTeamUpType(base->GetTeam()) != Game::GetTeamUpType(team))
            densityDiff = base->totalDensityWantingToDefend - base->GetTotalDensityWantingToAttack();
        else densityDiff = base->GetTotalDensityWantingToAttack() - base->totalDensityWantingToDefend;
        std::cout<< densityDiff << " density\n";

        if((Game::GetTeamUpType(base->GetTeam()) != Game::GetTeamUpType(team) ? (base->IsTeamAttacking(team) || base->IsTeamOfficerPlanningAttack(team)) : true) && densityDiff >= -4)
        {
            std::cout << id << "# Sending\n";
            sf::Vector2i grid = spawnPoint.spawnGrids[GenerateRandomNumber(spawnPoint.spawnGrids.size())];
            std::vector<sf::Vector2i> spawnGrids;

            spawnGrids.push_back(grid);
            sf::Vector2i tempGrid = grid + sf::Vector2i(-1, 0);
            if(tempGrid.x >= 0 && tempGrid.x < MapTileManager::MAX_MAPTILE && tempGrid.y >= 0 && tempGrid.y < MapTileManager::MAX_MAPTILE &&
               GetGridInfo(tempGrid.y, tempGrid.x).baseId == id && MapTileManager::GetTile(GetGridInfo(tempGrid.y, tempGrid.x).tileId).type != MapTileManager::SmallWall)
                spawnGrids.push_back(tempGrid);

            tempGrid = grid + sf::Vector2i(1, 0);
            if(tempGrid.x >= 0 && tempGrid.x < MapTileManager::MAX_MAPTILE && tempGrid.y >= 0 && tempGrid.y < MapTileManager::MAX_MAPTILE &&
               GetGridInfo(tempGrid.y, tempGrid.x).baseId == id && MapTileManager::GetTile(GetGridInfo(tempGrid.y, tempGrid.x).tileId).type != MapTileManager::SmallWall)
                spawnGrids.push_back(tempGrid);

            for(int a = -1; a <= 1; a++)
            {
                tempGrid = grid + sf::Vector2i(a, -1);
                if(tempGrid.x >= 0 && tempGrid.x < MapTileManager::MAX_MAPTILE && tempGrid.y >= 0 && tempGrid.y < MapTileManager::MAX_MAPTILE &&
                   GetGridInfo(tempGrid.y, tempGrid.x).baseId == id && MapTileManager::GetTile(GetGridInfo(tempGrid.y, tempGrid.x).tileId).type != MapTileManager::SmallWall)
                    spawnGrids.push_back(tempGrid);

                tempGrid = grid + sf::Vector2i(a, 1);
                if(tempGrid.x >= 0 && tempGrid.x < MapTileManager::MAX_MAPTILE && tempGrid.y >= 0 && tempGrid.y < MapTileManager::MAX_MAPTILE &&
                   GetGridInfo(tempGrid.y, tempGrid.x).baseId == id && MapTileManager::GetTile(GetGridInfo(tempGrid.y, tempGrid.x).tileId).type != MapTileManager::SmallWall)
                    spawnGrids.push_back(tempGrid);
            }

            const Game::DifficultyInfo& difficulty = Game::GetDifficultyInfo();

            Ship::UnitGroup* group = nullptr;
            ShipUnit* tempShip = nullptr;
            ShipUnit::UnitType type = ShipUnit::MiniShip;
            Game::SkinType skinType = Game::SkinMiniShip;
            int attributeValue = difficulty.miniShip;

            sf::Vector2f shipPos = (sf::Vector2f)(grid * MapTileManager::TILE_SIZE) + sf::Vector2f(GenerateRandomNumber(MapTileManager::TILE_SIZE), GenerateRandomNumber(MapTileManager::TILE_SIZE));
            tempShip = new ShipUnit(type, shipPos, team, attributeValue, attributeValue, attributeValue, 50 + GenerateRandomNumber(30), 0, GenerateRandomNumber(Game::GetSkinSize(skinType, team)), skinType);
            EntityManager::CreateShip(tempShip, false);
            group = Ship::UnitGroup::CreatUnitGroups();
            tempShip->JoinGroup(group, true);
            if(base->GetTeam() == team)
                tempShip->Defend(*basesToReinforce.begin(), true, 5.f);
            else tempShip->Attack(*basesToReinforce.begin());

            for(int a = 0; a < Ship::MAX_GROUP_MEMBERS; a++)
            {
                type = ShipUnit::MiniShip;
                skinType = Game::SkinMiniShip;
                attributeValue = difficulty.miniShip;
                if(GenerateRandomNumber(101) <= MICROSHIP_SPAWN_RATIO)
                {
                    type = ShipUnit::MicroShip;
                    skinType = Game::SkinMicroShip;
                    attributeValue = difficulty.microShip;
                }

                shipPos = (sf::Vector2f)(spawnGrids[GenerateRandomNumber(spawnGrids.size())] * MapTileManager::TILE_SIZE) + sf::Vector2f(GenerateRandomNumber(MapTileManager::TILE_SIZE), GenerateRandomNumber(MapTileManager::TILE_SIZE));
                tempShip = new ShipUnit(type, shipPos, team, attributeValue, attributeValue, attributeValue, 50 + GenerateRandomNumber(30), 0, GenerateRandomNumber(Game::GetSkinSize(skinType, team)), skinType);
                EntityManager::CreateShip(tempShip, false);
                tempShip->JoinGroup(group);
            }

            numOfBuildUnits -= Ship::MAX_GROUP_MEMBERS + 1;
            reinforceTime = REINFORCE_TIME;
        }
        basesToReinforce.erase(basesToReinforce.begin());
    }
}

int Base::GetEnemyDensityDifference(bool nonNegative)
{
    int diff = enemyDensity - friendlyDensity;
    if(nonNegative && diff < 0)
        diff = 0;
    return diff;
}

int Base::GetFriendlyDensityDifference(bool nonNegative)
{
    int diff = friendlyDensity - enemyDensity;
    if(nonNegative && diff < 0)
        diff = 0;
    return diff;
}

int Base::GetEnemyDensity()
{
    return enemyDensity;
}

int Base::GetFriendlyDensity()
{
    return friendlyDensity;
}

void Base::AddDensityWantingToAttack(int density)
{
    totalDensityWantingToAttack += density;
}

void Base::AddDensityWantingToDefend(int density)
{
    totalDensityWantingToDefend += density;
}

int Base::GetTotalDensityWantingToAttack()
{
    return totalDensityWantingToAttack;
}

int Base::GetTotalDensityWantingToDefend()
{
    return totalDensityWantingToDefend;
}

void Base::UpdateBaseArea()
{
    if(updateBaseArea && updateBaseAreaTime <= 0)
    {
        std::cout << id << "# updating area\n";
        updateBaseArea = false;
        updateBaseAreaTime = UPDATE_BASE_AREA_TIME;
        float buildUnitsFactor = (float)numOfBuildUnits / maxNumUnit;
        if(buildUnitsFactor < 0.3f)
            buildUnitsFactor = 0.3f;

        int gridArea = (buildUnitsFactor - 0.3f) / 0.7f * (spawnPoint.spawnGrids.size() - MIN_BASE_AREA) + MIN_BASE_AREA;
        if(gridArea < MIN_BASE_AREA)
            gridArea = MIN_BASE_AREA;

        for(int a = 0; a < spawnPoint.spawnGrids.size(); a++)
        {
            if(a < gridArea)
                GetGridInfo(spawnPoint.spawnGrids[a]).baseArea = true;
            else GetGridInfo(spawnPoint.spawnGrids[a]).baseArea = false;
        }

        for(Defence*& defence : defencesInBase)
            if(defence->IsShipDefenceAlive())
            {
                sf::FloatRect rect(defence->GetDefenceSpotRect().left / MapTileManager::TILE_SIZE - 5, defence->GetDefenceSpotRect().top / MapTileManager::TILE_SIZE - 5, 12, 12);

                int arrayRange = 13;
                bool* isChecked = new bool[arrayRange * arrayRange] {};
                sf::Vector2i refGrid(rect.left, rect.top);
                sf::Vector2i offSetGrid = refGrid - sf::Vector2i(6, 6);
                std::vector<sf::Vector2i> gridsToCheck;
                sf::Vector2i offSet;

                gridsToCheck.push_back(refGrid);
                offSet = refGrid - offSetGrid;
                isChecked[offSet.y * arrayRange + offSet.x] = true;
                if(GetGridInfo(refGrid).baseId == id && GetGridInfoTile(refGrid.y, refGrid.x).type != MapTileManager::SmallWall)
                    GetGridInfo(refGrid).baseArea = true;

                for(int i = 0; i < gridsToCheck.size(); i++)
                {
                    for(int y = gridsToCheck[i].y - 1; y <= gridsToCheck[i].y + 1; y++)
                        for(int x = gridsToCheck[i].x - 1; x <= gridsToCheck[i].x + 1; x++)
                            if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE && GetGridInfo(refGrid).baseId == id)
                            {
                                offSet = sf::Vector2i(x, y) - offSetGrid;
                                if(offSet.x >= 0 && offSet.y >= 0 && offSet.x < arrayRange && offSet.y < arrayRange && isChecked[offSet.y * arrayRange + offSet.x] == false)
                                {
                                    gridsToCheck.push_back(sf::Vector2i(x, y));
                                    isChecked[offSet.y * arrayRange + offSet.x] = true;
                                    if(GetGridInfoTile(y, x).type != MapTileManager::SmallWall)
                                        GetGridInfo(y, x).baseArea = true;
                                }
                            }
                }
                delete[] isChecked;

                /*for(int y = rect.top; y < rect.top + rect.height; y++)
                    for(int x = rect.left; x < rect.left + rect.width; x++)
                        if(y >= 0 && y < MapTileManager::MAX_MAPTILE && x >= 0 && x < MapTileManager::MAX_MAPTILE)
                            if(GetGridInfo(y, x).baseId == id)
                                if(GetGridInfoTile(y, x).type != MapTileManager::Wall)//&& GetGridInfoTile(y, x).type != MapTileManager::SmallWall)
                                    GetGridInfo(y, x).baseArea = true;*/
            }
    }
}
 const int& Base::GetTeamActiveBaseCoverage(const short& team_)
 {
    if(team_ >= 0 && team_ < Game::NumOfTeams)
        return teamActiveBaseCoverage[team_];
    return 0;
 }
