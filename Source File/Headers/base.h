#ifndef BASE_H_INCLUDED
#define BASE_H_INCLUDED

#include "defence.h"
#include "entity.h"
#include "shipunit.h"
#include <list>

class Base
{
public:
    struct SpawnPointObject
    {
        sf::Vector2i grid;
        float lastSpawnTime = 0.f;
        std::vector<sf::Vector2i> spawnGrids;
    };

    struct NetworkInfo
    {
        std::vector<unsigned int> connectedBases;
        std::vector<unsigned int> reinforcementBases[4];
        int networkValue[4] = {-1, -1, -1, -1};
    };

    static const int MICROSHIP_SPAWN_RATIO = 40;

    static void InitializeNetworkInfo();
    static void InitializeBaseAreas();
    static void UpdateFrontLineBases(bool ignoreCheck = false);
    static const NetworkInfo& GetNetworkInfo(unsigned int id);
    static const std::vector<int>& GetFrontLineBases(unsigned short team);
    static const std::vector<int>& GetFrontLineBasesEnemy(unsigned short team);
    static const int& GetTeamActiveBaseCoverage(const short& team_);

    Base(short _id, short _team, sf::Vector2i _spGrid, sf::IntRect _range);
    ~Base();

    void Update(float dt);
    void UpdateRegion(sf::Vector2i spGrid_, sf::IntRect range_);
    void UpdateRegion();
    void UpdateBaseArea();

    sf::IntRect GetRange();
    short GetTeam();
    bool IsTeamAttacking(short team);
    bool IsTeamOfficerPlanningAttack(short team);
    void TeamOfficerIsPlanningAttack(short team);
    void AddDensityWantingToAttack(int density);
    void AddDensityWantingToDefend(int density);
    int GetTotalDensityWantingToAttack();
    int GetTotalDensityWantingToDefend();
    long GetBaseSize();
    int GetMaxUnits();
    int GetNumOfBuildUnits();
    SpawnPointObject* GetSpawnPoint();
    void GetBaseAreaGrids(std::vector<sf::Vector2i>& grids);
    sf::Vector2i GetRandomBaseGrid();
    std::vector<Ship*>* GetUnitsInBase(int team_);
    const std::vector<Defence*>& GetDefensesInBase();
    int GetEnemyDensityDifference(bool nonNegative = true);
    int GetFriendlyDensityDifference(bool nonNegative = true);
    int GetEnemyDensity();
    int GetFriendlyDensity();
    bool IsEnemyInBase();
    bool IsFriendlyTargeting();
    bool IsBaseActive();

    void SetMaxUnits(long units);
    void SetDefaultMaxUnits();

    void AddDefence(Defence* defence);
    void AddUnit(ShipDefence* unit_);
    void AddUnit(ShipUnit* unit_);
    void AddShipDensity(Ship* ship_);
    bool IsActive();
    bool IsMainBase();
    int GetNumOfInactiveDefences();
    void DrawHighlight(sf::RenderTarget& target_);
    void DrawOutline(sf::RenderTarget& target_);
    void DrawNotifyHighlight(sf::RenderTarget& target_, sf::FloatRect screenRect_);
    bool CanMoveToEnemy(const short& team_);
    bool GetGenerateState();
    void SendReinforcement();
    void AskForReinforcement(short team_);
    void Reinforce(short id_);
    int GetNumOfBasesReinforcing();
    void ResetMoveToEnemy();
    void SetId(short id_);
    void HasRecaimedUnit();
    void Refresh();
    void RefreshMainBaseStatus();

    static sf::Texture* outlineTexture;

private:
    sf::IntRect range;
    int maxNumUnit;
    int maxNumFreeUnits;
    int numOfBuildUnits;
    int numOfUnitsInBase;
    int numOfMinorUnitsInBase;
    int numOfGroupUnitSpawn;
    int friendlyDensity;
    int enemyDensity;
    int totalDensityWantingToAttack;
    int totalDensityWantingToDefend;
    float fadeFactor;
    float fadeTime;
    float buildTime;
    float buildDelay;
    float spawnTime;
    float spawnDelay;
    float moveUnitsTime;
    float reinforceTime;
    float groupUnitsSpawnTime;
    float reclaimTime;
    float densityFavourTime;
    float updateBaseAreaTime;
    bool updateBaseArea;
    bool isPlayerInBase;
    bool isEnemyInBase;
    bool doesFriendlyHaveTarget;
    bool isTeamUnitInBase;
    bool isAllyInBase;
    bool isBaseActive;
    bool isMainBase;

    std::vector<Defence*> defencesInBase;
    std::vector<Ship*> unitsInBase[4];
    std::vector<Ship*> enemyUnitsInBase;
    std::vector<ShipUnit*> unitsToReclaim;
    std::vector<Ship::UnitGroup*> groupsToAddMembers;
    std::list<int> basesToReinforce;
    float askReinforceTime[4];
    bool isTeamAttacking[4];
    bool isTeamOfficerPlanningAttack[4];
    SpawnPointObject spawnPoint;
    sf::VertexArray notifyHighlight;
    sf::VertexArray highlight;
    sf::VertexArray outline;
    long baseSize;
    short team;
    int id;
};

#endif // BASE_H_INCLUDED
