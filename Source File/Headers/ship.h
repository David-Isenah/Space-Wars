#ifndef SHIP_H_INCLUDED
#define SHIP_H_INCLUDED

#include "entity.h"
#include "skin.h"
#include "gun.h"

class Ship : public Entity
{
    public:
        enum ShipType{NoShipType, UnitShip, DefenceShip};

        static const int SHIP_SIGTH_RANGE = 1500;
        static const int MAX_GROUP_MEMBERS = 8;
        static const float BAR_DURATION;
        static const float HIT_EFFECT_DURATION;
        static const int BaseLevelExpShared[4];

        struct UnitGroup
        {
            std::vector<Ship*> members;
            Ship* leader = nullptr;

            void AddMember(Ship* unit);
            void LeaveGroup(Ship* unit);

            static UnitGroup* CreatUnitGroups(/*Ship* leader_ = nullptr*/);
            static void DeleteGroup(UnitGroup* group_);
            static void RefreshUnitGroups();
        };

        Ship();

        //void Draw(sf::RenderTarget& target);
        //void DrawHealth(sf::RenderTarget& target);

        void UpdateHealthBar();
        virtual void SetTarget(Ship* target);
        virtual void AddSigthTarget(Ship* target);
        float GetSpawnTransitionFactor(float dt);
        float GetBaseCoreAttackHealthFactor();
        ShipType GetShipType();
        Ship* GetShipTarget();
        float GetTargetRange();
        float GetFleeDistance();
        const float& GetActualHitpoint();
        const float& GetActualDefence();
        const float& GetActualAttack();
        const float& GetActualPowerScore();
        const short& GetCurrentLevel();
        int* GetExp();
        virtual void RefreshRealTimeProperties();
        virtual void Shoot() = 0;
        virtual void AimAndShoot(float dt);
        virtual void HandleGunReloading();
        void SwitchGun(short gunType_, unsigned int magazine_, float* ammoRef_);
        short GetCurrentGun();
        unsigned int GetGunMagazine();
        float* GetGunAmmo();
        float GetReloadingFactor();
        bool IsReloading();
        virtual void GainKillPoints(const int& exp_, const float& special_, const float& powerFactor_);
        virtual void GiveKillPoints(Ship* shooter) = 0;
        virtual void GetDamagedBy(float damage, sf::Vector2f hitPosition, Ship* shooter = nullptr);
        virtual void GetPoints(std::vector<sf::Vector2f>& points) = 0;
        virtual int GetLevelBaseExp() = 0;
        virtual int GetNextLevelBaseExp() = 0;
        short GetShipDensityValue();
        sf::Vector2f GetPosition() = 0;
        virtual sf::FloatRect GetBoundingRect() = 0;
        virtual float GetNonTransmittingAngle();
        float GetSizeRadius();
        float GetDodgeRadius();
        float GetHealth();
        float GetHealthInFactors();
        virtual float GetSpecialInFactors();
        virtual bool IsSpecialActive();
        short GetHierarchy();
        virtual void GetVelocityDetails(float& maxVelocity_, float& maxForce_);
        virtual sf::Vector2f GetVelocity();
        virtual bool IsMoving();
        virtual void AddToGridBase();
        void TransmitToAngle(float angle_, float dt_);

        virtual void JoinGroup(UnitGroup* group_, bool asLeader = false);
        void LeaveGroup();
        UnitGroup* GetGroup();
        bool GetRecruitingState();

        //bool ResolveBulletCollision(Bullet* bullet);
        virtual bool ResolveWallCollision(sf::FloatRect& wallRect, const float& dt) = 0;
        bool ResolveWallCollision(std::vector<sf::FloatRect>& wallRect, const float& dt);
        //bool ResolveEntityCollision(Entity* entity);

        virtual void RefreshEntities();
        void LevelUp();

        static void DrawHealthBars(sf::RenderTarget& target_);
        static int* GetSharedExp(short team_);

    protected:
        ShipType shipType;
        float lastShotTime;
        float lastHitTime;
        float lastTargetUpdateTime;
        float lastReloadTime;

        int hitPoint;
        int attack;
        int defense;
        int speed;
        int special;

        float actualHitpoint;
        float actualAttack;
        float actualDefense;
        float actualPowerScore;

        float health;
        float fleeDistance;
        short currentLevel;
        short currentGun;
        float reloadingTime;
        bool reloading;
        unsigned int gunMagazine;
        float* gunAmmo;
        float targetRange;
        std::string barName;
        float barTransparencyTime;
        float spawnTransparencyTime;
        float minBaseCoreAttackHealthFactor;
        float sizeRadius;
        float dodgeRadius;
        float barLength;
        float barHeight;
        bool shouldDrawBarExp;
        bool isTransmitingSpawn;
        short hierarchy;
        short shipDensityValue;
        UnitGroup* group;
        bool isRecruiting;
        int* exp;
        float* specialMeter;

        Ship* targetEntity;
        Skin* skin;
        sf::Vertex vertices_healthbar[4];

        std::vector<sf::Vector2f> collidePoints;
        std::vector<sf::Vector2f> collideObjectPoints;
};

#endif // SHIP_H_INCLUDED
