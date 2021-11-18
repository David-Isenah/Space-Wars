#ifndef ENTITYMANAGER_H_INCLUDED
#define ENTITYMANAGER_H_INCLUDED

#include "list"
#include "bullet.h"
#include "shipunit.h"
#include "defence.h"
#include "maptilemanager.h"
#include "basecore.h"
#include "astarpathfinding.h"

class EntityManager
{
    public:
        static const float PL_ZOOM_FACTOR;
        static const sf::Vector2f PL_CAMERA_LIMIT_FACTOR;
        static const sf::Vector2f PL_AIM_CAMERA_LIMIT_FACTOR;
        static const int MAX_NUM_OF_SHIPS = 400;
        static const int SIGHT_WIDTH = 10;

        EntityManager();
        ~EntityManager();

        static void InitialiseDefenceGrids();

        static void CreateShip(Ship* ship, bool cameraFocus = false);
        static void CreateDefence(sf::Vector2f position_, Defence::DefenceType type_, short team_, short rotation_, int skinIndex_);
        static void CreateBullet(Bullet* bullet);
        static void CreateAllDefenceShips();
        static void CreateBaseCore(short team, sf::Vector2i grid);
        static void RefreshEnemyBaseCores();
        static const std::vector<BaseCore*>& GetEnemyBaseCore(int team_);

        static void DeleteBaseCore(short team);
        static BaseCore* GetBaseCore(short team);

        static void UpdateAll(float dt);
        static void UpdateShipGrid();
        static void UpdateAllShipTarget(float dt);

        static void SetCameraPosition(sf::Vector2f position_, bool doTransmit_ = true);
        static void QuickPeakWithCamera(sf::Vector2f position_);
        static void SetCameraFocus(Ship* ship_);
        static sf::FloatRect GetCameraRect();
        static Ship* GetCameraFocus();
        static sf::View& GetCamera();

        static void RemoveAll();
        static const std::list<Ship*>& GetShipObjects();
        static bool isSightOnWall(const sf::Vector2f& position, const sf::Vector2f& target, const float& sightWidth = SIGHT_WIDTH);
        static bool isSightOnWall(const sf::Vector2f& position, const sf::Vector2f& target, bool* isSightOnSmallWall, float* sightLength = nullptr, const bool& ignoreSmallWallSightLength = true, const float& sightWidth = SIGHT_WIDTH,
                                  std::vector<Ship*>* shipsInSight = nullptr, std::vector<sf::Vector2f>* shipsInSightPushVec = nullptr, const int& shipsInSightLimit = -1, const short& shipsInSightTeamUp = -1, const bool& endOnAnyCondition = false);
        static unsigned int GetNumOfAliveShipUnits();
        static unsigned int GetNumOfTeamAliveShipUnits(short team_);
        static void RecruitGroupMembers(Ship* ship);
        static void MoveMultipleUnits(const std::list<ShipUnit*>& units_, sf::Vector2f destination_, AStar::SearchType searchType_ = AStar::NoSearchType, short teamUpType_ = -1);

    private:
        std::list<Bullet*> bulletObjects;
        std::list<Defence*> defenseObjects;
        std::list<Ship*> shipObjects;

        //std::vector<Ship*> shipGrid[MapTileManager::MAX_MAPTILE][MapTileManager::MAX_MAPTILE];

        sf::View camera;
        BaseCore* basecores[4];

        static EntityManager* instance;
};

#endif // ENTITYVeMANAGER_H_INCLUDED
