#ifndef SHIPIDENTY_H_INCLUDED
#define SHIPIDENTY_H_INCLUDED

#include <string>
#include <sstream>
#include <vector>
#include "SFML/Graphics.hpp"
#include "shipunit.h"
#include "displayeffect_effects.h"

struct ShipInfo
{
    enum GameCharacters
    {
        //Alpha
        Alex = 0, NumOfAlphaCharacters,

        //Delta
        Magmix = 0, NumOfDeltaCharacters,

        //Vortex
        Vyrim = 0, NumOfVortexCharacters,

        //Omega
        Zendor = 0, NumOfOmegaCharacters
    };

    enum ShipClasses {NoShipClass = -1, D, C, B, A, S, NumOfShipClasses};
    enum CharacterType {MainCharacterType = 0, CustomCharactertype, TemporaryCharacterType, NumOfCharacterType};
    enum ClassMaxAttribute {Class_D_Max = 200, Class_C_Max = 500, Class_B_Max = 900, Class_A_Max = 1400, Class_S_Max = 2000};
    enum ClassMinAttribute {Class_D_Min = 0, Class_C_Min = 200, Class_B_Min = 500, Class_A_Min = 900, Class_S_Min = 1400};

    static const int MAX_CIRCUITS = 5;
    static const int MAX_LEVEL = 10;
    static const long CLASS_LEVEL_EXP[NumOfShipClasses][MAX_LEVEL];
    static long CalculateClassLevel(const short& class_, const long& exp_);

    std::string name;
    long experiencePoint = 1000;
    unsigned short shipClass = -1;
    unsigned short uniqueAbility = -1;
    unsigned short skinIndex = -1;
    unsigned short team = -1;
    unsigned short skinType = -1;
    unsigned short circuits[MAX_CIRCUITS] = {};
    sf::Texture texture;
    sf::Texture profileTexture;

    unsigned int hitpoint = 0;
    unsigned int defense = 0;
    unsigned int attack = 0;
    unsigned int speed = 0;
    unsigned int special = 0;

    unsigned short unlock_circuit[MAX_CIRCUITS] = {0};
    unsigned short unlock_gun[Gun::NumOfGuns] = {0};

    ShipInfo(std::string name_, unsigned short shipClass_, unsigned short uniqueAbility_, unsigned short skinIndex_,
             unsigned short team_, unsigned short skinType_, unsigned int hitpoint_, unsigned int defense_,
             unsigned int attack_, unsigned int speed_, unsigned int special_);
    void PrepareProfileTexture();
};

class ShipIdentity
{
private:
    ShipInfo* shipInfo;
    sf::Vector2i startGrid;
    unsigned short rotation;
    ShipUnit* shipEntity;
    DE_DamageDisplay damageDisplayEffect;
    float spawnTime;
    float spawnTime_last;
    bool isMainShip;
    bool isPlayable;
    int shipInfoLevel;
    float gunAmmo[Gun::NumOfGuns];
    float gunAmmoRestock[Gun::NumOfGuns];
    float gunAmmoRestockTreshold[Gun::NumOfGuns];
    unsigned int gunLastMagazine[Gun::NumOfGuns];
    short unitLevel;
    float unitExpFactor;
    int lastExp;
    int exp;
    float specialMeter;
    float gunChangeTime;
    short lastGun;

public:
    static void UpdateShipIdentities(float dt);
    static void RefreshShipIdentities();

    ShipIdentity();
    ShipIdentity(ShipInfo* shipInfo_, sf::Vector2i startGrid_, bool isMainShip_ = false, bool isPlayable_ = false, short rotation_ = 0);

    void CreateShip(bool atStartPos = false);
    void RefreshShipEntity();
    void RestockGuns(const float& powerFactor_);
    void Update(float dt);
    void DrawInfo(sf::RenderTarget& rw, sf::FloatRect viewArea);
    ShipInfo* GetShipInfo()
    {
        return shipInfo;
    }

    const int& GetShipInfoLevel()
    {
        return shipInfoLevel;
    }

    sf::Vector2i GetStartGrid()
    {
        return startGrid;
    }

    ShipUnit* GetShipEntity()
    {
        return shipEntity;
    }

    int* GetExp()
    {
        return &exp;
    }

    const float& GetSpawnTime()
    {
        return spawnTime;
    }

    const short& GetUnitLevel()
    {
        return unitLevel;
    }

    const float& GetUnitExpFactor()
    {
        return unitExpFactor;
    }

    float& GetSpecialMeter()
    {
        return specialMeter;
    }

    DE_DamageDisplay& GetDamageDE()
    {
        return damageDisplayEffect;
    }

    float GetSpawnTimeFactor(bool delayByOneSec = false)
    {
        if(spawnTime_last <= 0)
            return 0.f;

        float factor = 0;
        if(delayByOneSec)
            factor = (spawnTime - 1) / (spawnTime_last - 1);
        else factor = spawnTime / spawnTime_last;
        if(factor < 0)
            factor = 0;

        return 1.f - factor;
    }

    bool GetPlayableStatus()
    {
        return isPlayable;
    }

    float GetGunAmmo(short gun_)
    {
        if(gun_ >= 0 && gun_ < Gun::NumOfGuns)
            return gunAmmo[gun_];
        return 0;
    }

    unsigned int GetGunLastMagazine(short gun_)
    {
        if(gun_ >= 0 && gun_ < Gun::NumOfGuns)
            return gunLastMagazine[gun_];
        return 0;
    }

    void SwitchUnitGun(short gun_)
    {
        if(gun_ >= 0 && gun_ < Gun::NumOfGuns && shipEntity != nullptr && shipEntity->GetCurrentGun() != gun_)
        {
            lastGun = gun_;
            gunLastMagazine[shipEntity->GetCurrentGun()] = shipEntity->GetGunMagazine();
            shipEntity->SwitchGun(gun_, gunLastMagazine[gun_], &gunAmmo[gun_]);
        }
    }

    void Rotate()
    {
        rotation++;
        if(rotation >= 8)
            rotation = 0;
    }
};

class FormUnit
{
private:
    short formType;
    sf::Vector2i startGrid;
    unsigned short rotation;
    short team;
    short skinIndex;

public:
    enum {NoForm = -1, FormMainDefence, FormMiniDefence, FormMiniShip, FormMicroShip};

    FormUnit():
        formType(NoForm),
        rotation(0),
        team(-1),
        skinIndex(-1)
    {
    }

    FormUnit(short formType_, sf::Vector2i startGrid_, short team_, short skinIndex_, unsigned short rotation_ = 0) :
        formType(formType_),
        startGrid(startGrid_),
        rotation(rotation_),
        skinIndex(skinIndex_),
        team(team_)
    {
    }

    short GetFormType()
    {
        return formType;
    }

    short GetSkinIndex()
    {
        return skinIndex;
    }

    sf::Vector2i GetStartGrid()
    {
        return startGrid;
    }

    short GetTeam()
    {
        return team;
    }

    void SetSkinIndex(short index)
    {
        skinIndex = index;
    }

    void Rotate()
    {
        rotation++;
        if(rotation >= 8)
            rotation = 0;
    }

    virtual void Draw(sf::RenderTarget& target, sf::FloatRect viewArea);
    void Create();
};

#endif // SHIPIDENTY_H_INCLUDED
