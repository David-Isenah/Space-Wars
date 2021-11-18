#ifndef SHIPUNIT_H_INCLUDED
#define SHIPUNIT_H_INCLUDED

#include "ship.h"
#include "skin.h"
#include "multidrawer.h"
#include <list>

class ShipIdentity;

class ShipUnit : public Ship
{
    public:
        struct MultiDrawerBasedOnProperty
        {
            sf::Transformable transformation;
            sf::Vertex vertices[4];
            MultiDrawer* multiDrawer;
        };

        struct BasedOnProperty
        {
            sf::Sprite* sprite = nullptr;
            MultiDrawerBasedOnProperty* multiDrawerProperties = nullptr;
        };

        static const int BaseLevelExpHigherUnits[4];

        enum UnitType{PlayerShip = 0, MainShip, SubShip, MiniShip, MicroShip, NumOfUnitType};
        enum ShipState{Idle, Attacking, Defending, FollowingGroup, Traveling};

        ShipUnit(UnitType _type, sf::Vector2f position_, short team_, int hitPoint_, int attack_, int defense_, int speed_, int special_, int skinIndex_, short skinType_, bool isSpawning_ = true, ShipIdentity* identity_ = nullptr);
        ~ShipUnit();

        void PlayerUpdate(float dt);
        void AIUpdate(float dt);
        void Update(float dt);

        int GetLevelBaseExp();
        int GetNextLevelBaseExp();

        virtual void RefreshEntities();
        void RefreshRealTimeProperties();
        bool IsSpecialActive();
        ShipIdentity* GetIdentity();
        void GainKillPoints(const int& exp_, const float& special_, const float& powerFactor_);
        void GiveKillPoints(Ship* shooter);
        float GetSpecialInFactors();
        void JoinGroup(UnitGroup* group_, bool asLeader = false);
        UnitType GetUnitType();
        sf::Vector2f GetPosition();
        void GetPoints(std::vector<sf::Vector2f>& points);
        sf::FloatRect GetBoundingRect();
        float GetNonTransmittingAngle();
        bool GetReclaimingStatus();
        void ReclaimToBase(int baseId);
        ShipState GetState();
        const int& GetSurroundingTroops();
        void AddToGridBase();
        void AddSigthTarget(Ship* target_);
        void SetTarget(Ship* target);
        void MoveTo(sf::Vector2f location, short searchType = -1, bool isFastSearch = true, bool useReachSearch = false);
        void MoveTo(const std::vector<sf::Vector2i>& path_);
        void MoveToEnemiesInBase();
        bool IsWillingToMoveToEnemyInBase();
        bool IsMoving();
        bool ResolveWallCollision(sf::FloatRect& wallRect, const float& dt);
        void SpawnSurroundingTroops();
        void GetDamagedBy(float damage, sf::Vector2f hitPosition, Ship* shooter = nullptr);
        void GunSwitching(float dt);
        void AimAndShoot(float dt);
        void Shoot();

        sf::Vector2f AttackTarget(float dt, bool& isShootingTarget);
        sf::Vector2f FollowGroup(float dt, float& maxVelocity_, float& maxForce_);
        int GetAttackingBase();
        sf::Vector2i GetTravellingGrid();
        void AttackUpdate();
        void DefendUpdate(float dt);
        void TravelUpdate(float dt);
        void IdleThinking(float dt);
        void ClearThinking();

        void Attack(int base_);
        void Defend(int base_, bool avoidEnemyBases_, float defendDuration_ = 0.f);
        void Travel(sf::Vector2f destination_, bool shouldAttackInTravel_ = true, float stopRadius_ = 0.f);
        void CancleCurrentAction();

        bool FleeFromUnit(Ship* unit_);
        sf::Vector2f DeduceFleeSteering(float dt);

        sf::Vector2f GetVelocity();
        void GetVelocityDetails(float& maxVelocity_, float& maxForce_);

        sf::Vector2f SteerForSeek(const sf::Vector2f& target);
        sf::Vector2f SteerForFlee(const sf::Vector2f& target);
        sf::Vector2f SteerForArrive(const sf::Vector2f& target, bool& isReached, float minVelocityFactor = 0.f);
        sf::Vector2f SteerForPathFollowing(bool& isReached);

        static void SetActivePlayerUnit(ShipUnit* ship_);
        static ShipUnit* GetActivePlayerUnit();
        static bool IsActivePlayerAiming();
        static bool IsActivePlayerAiming(float& aimDistMagn);
        static bool IsActivePlayerManualAiming();
        static bool IsActivePlayerJustShooting();
        static void Draw(sf::RenderTarget& target);
        static void InitialiseMultiDrawer();

    private:
        float maxVelocity;
        float maxForce;
        float slowingDistance;
        float minAttackHealthFactor;
        int minAttackGroupMembers;

        sf::Vector2f velocity;
        bool isSpecialActive;
        bool isAngleLocked;
        float nonTransmittingAngle;
        sf::Vector2f fleeVelocity;
        sf::Vector2f fleeSteering;
        float fleeReductionTime;
        int numOfFleeingUnits;
        int surroundingTroops;
        float surroundTroopsBuildTime;
        float surroundTroopsFavourTime;
        float lastSurroudingTroopSpawn;
        sf::Vector2f attackStartPoint;
        int moveLineIndex;
        float moveLineIndexFixTime;
        int moveLineIndexFixCount;
        std::vector<sf::Vector2f> movePoints;
        std::vector<sf::Vector2f> moveLineNormals;
        std::vector<float> moveLineLength;
        std::vector<sf::Vector2f> primaryPath;
        int primaryPathIndex;
        std::vector<Ship*> sightShips;
        float sightShipChaseTime;
        float pathFindDelay;
        bool attackTargetState;
        float realTimeMaxVelocity;
        float realTimeMaxForce;
        float followGroupCloseFactor;
        float followGroupDelay;
        sf::Vector2f dodgeDir;
        float dodgeTime;
        float dodgeAllowance;
        int reclaimBase;
        Ship* lastBulletFromHigherUnits;
        float lastBulletFromHigherUnits_time;
        bool isCurentlyUsingAstarProgression;
        float specialScalingFactor;
        float specialEmitRadiation;

        float thinkingWaitDuration;
        short thinking_searchType;
        short thinking_state = -1;
        bool isThinkingFindingPath;
        std::list<int> thinking_baseId;
        std::vector<sf::Vector2i> thinking_closestPath;

        //Defending variables
        int defending_base;
        bool defending_avoidEnemyBases;
        float defending_duration;

        //Attacking variables
        int attacking_base;
        bool attacking_tillDeath;

        //Traveling variables
        sf::Vector2f traveling_destination;
        float traveling_stopDuration;
        float traveling_stopRadius;
        float traveling_updateTime;

        //Follow group variables
        bool isCloseLeader;

        ShipState state;
        UnitType unitType;
        BasedOnProperty basedOnProperty;
        bool isMultiDrawerBasedOn;
        ShipIdentity* identity;
};



#endif // SHIPUNIT_H_INCLUDED
