#include "game.h"
#include "entitymanager.h"
#include "public_functions.h"
#include "gun_const.h"
#include<iostream>

std::string AlphaCharName[] = {"Alex"};
std::string DeltaCharName[] = {"MagmixMMMM"};
std::string VortexCharName[] = {"Vyrim"};
std::string OmegaCharName[] = {"Zendor"};

const long ShipInfo::CLASS_LEVEL_EXP[NumOfShipClasses][MAX_LEVEL] =
{
    {0, 100, 200, 300, 400, 500, 600, 700, 800, 1000},
    {0, 100, 200, 300, 400, 500, 600, 700, 800, 1000},
    {0, 100, 200, 300, 400, 500, 600, 700, 800, 1000},
    {0, 100, 200, 300, 400, 500, 600, 700, 800, 1000},
    {0, 100, 200, 300, 400, 500, 600, 700, 800, 1000},
};

sf::RenderTexture tempCharacterTextr;
sf::RenderTexture tempProfileTextr;
sf::VertexArray lightVertices;
sf::VertexArray darkVertices;
sf::VertexArray barVertices;
sf::VertexArray iconVertices;
sf::VertexArray tempShipVertices;
sf::VertexArray ghostShipVertices;
sf::Sprite skinSprite;
sf::Text text;
sf::Texture* refTexture = nullptr;
std::vector<ShipIdentity*> shipIdentities;
std::vector<FormUnit*> formUnits;

//ShipInfo
ShipInfo::ShipInfo(std::string name_, unsigned short shipClass_, unsigned short uniqueAbility_, unsigned short skinIndex_,
                   unsigned short team_, unsigned short skinType_, unsigned int hitpoint_, unsigned int defense_,
                   unsigned int attack_, unsigned int speed_, unsigned int special_) :
                       name(name_), shipClass(shipClass_), uniqueAbility(uniqueAbility_), skinIndex(skinIndex_), team(team_),
                       skinType(skinType_), hitpoint(hitpoint_), defense(defense_), attack(attack_), speed(speed_), special(special_)
{
    /*if(circuits_ != "")
    {
        unsigned short index = 0;
        std::stringstream ss;
        ss << circuits_;

        while(ss.eof() == false)
        {
            unsigned short temp;
            ss >> temp;
            circuits[index] = temp;
            index++;
        }
    }*/
    PrepareProfileTexture();
}

void ShipInfo::PrepareProfileTexture()
{
    Skin* skin = Game::GetSkin((Game::SkinType)skinType, skinIndex, team);
    if(skin != nullptr)
    {
        tempProfileTextr.clear(sf::Color(255, 255, 255, 0));

        /*sf::Sprite sprite;
        sprite.setTexture(skin->texture, true);
        sprite.setOrigin(sprite.getLocalBounds().width / 2.f, sprite.getLocalBounds().height / 2.f);
        sprite.setRotation(45);
        sprite.setPosition(61, 61);*/

        sf::VertexArray varray;
        varray.setPrimitiveType(sf::Quads);
        varray.append(sf::Vertex(sf::Vector2f(0, 0), sf::Color::White, sf::Vector2f(4, 90)));
        varray.append(sf::Vertex(sf::Vector2f(122, 0), sf::Color::White, sf::Vector2f(90, 4)));
        varray.append(sf::Vertex(sf::Vector2f(122, 122), sf::Color::White, sf::Vector2f(176, 90)));
        varray.append(sf::Vertex(sf::Vector2f(0, 122), sf::Color::White, sf::Vector2f(90, 176)));

        //tempProfileTextr.draw(sprite);
        tempProfileTextr.draw(varray, sf::RenderStates(sf::BlendMode(sf::BlendMode::One, sf::BlendMode::Zero), sf::Transform(), &(skin->texture), nullptr));
        tempProfileTextr.display();
        profileTexture = tempProfileTextr.getTexture();
    }
}

long ShipInfo::CalculateClassLevel(const short& class_, const long& exp_)
{
    if(exp_ >= CLASS_LEVEL_EXP[class_][MAX_LEVEL - 1])
        return MAX_LEVEL;

    for(int a = 1; a < MAX_LEVEL - 1; a++)
        if(exp_ < CLASS_LEVEL_EXP[class_][a])
            return a;

    return 1;
}

//ShipInfoGroup
struct ShipInfoGroup
{
    std::vector<ShipInfo*> shipInfoSet[Game::NumOfTeams];

    ~ShipInfoGroup()
    {
        Clear();
    }

    ShipInfo* AddShipInfo(ShipInfo* shipInfo)
    {
        if(shipInfo->team >=0 && shipInfo->team < Game::NumOfTeams)
        {
            shipInfoSet[shipInfo->team].push_back(shipInfo);
            return shipInfo;
        }
        else delete shipInfo;
        return nullptr;
    }

    ShipInfo* AddShipInfo(std::string name_, unsigned short shipClass_, unsigned short uniqueAbility_, unsigned short skinIndex_,
                          unsigned short team_, unsigned short skinType_, unsigned int hitpoint_, unsigned int defense_,
                          unsigned int attack_, unsigned int speed_, unsigned int special_)
    {
        return AddShipInfo(new ShipInfo(name_, shipClass_, uniqueAbility_, skinIndex_, team_, skinType_, hitpoint_,defense_, attack_, speed_, special_));
    }

    void Remove(unsigned int index, unsigned short team)
    {
        if(index < shipInfoSet[team].size())
        {
            delete shipInfoSet[team][index];
            shipInfoSet[team].erase(shipInfoSet[team].begin() + index);
        }
    }

    void Remove(ShipInfo* shipInfo)
    {
        for(int a = 0; a < Game::NumOfTeams; a++)
            for(int b = 0; b < shipInfoSet[a].size(); b++)
                if(shipInfoSet[a][b] == shipInfo)
                    Remove(b, a);
    }

    void Clear()
    {
        for(int a = 0; a < Game::NumOfTeams; a++)
            for(int b = 0; b < shipInfoSet[a].size(); b++)
                delete shipInfoSet[a][b];

        for(int a = 0; a < Game::NumOfTeams; a++)
            shipInfoSet[a].clear();
    }
};

ShipInfoGroup characterInfoGroup[ShipInfo::NumOfCharacterType];

ShipInfo* Game::GetCharacterInfo(unsigned short characterType, unsigned short index, unsigned short team)
{
    if(characterType < ShipInfo::NumOfCharacterType && index < characterInfoGroup[characterType].shipInfoSet[team].size())
        return characterInfoGroup[characterType].shipInfoSet[team][index];
    return nullptr;
}

int Game::GetCharacterTypeInfoSize(unsigned short characterType, unsigned short team)
{
    return characterInfoGroup[characterType].shipInfoSet[team].size();
}

ShipInfo* Game::AddCharacterInfo(ShipInfo* shipInfo, unsigned short characterType)
{
    return characterInfoGroup[characterType].AddShipInfo(shipInfo);
}

//Ship Identity
ShipIdentity::ShipIdentity():
    shipInfo(nullptr), rotation(0), isMainShip(false), isPlayable(false), shipEntity(nullptr), exp(0), specialMeter(0), spawnTime(0), spawnTime_last(0), gunChangeTime(0), shipInfoLevel(0), unitLevel(1), unitExpFactor(0), lastExp(0), lastGun(0)
{
    if(shipInfo != nullptr)
        shipInfoLevel = ShipInfo::CalculateClassLevel(shipInfo->shipClass, shipInfo->experiencePoint);

    gunAmmo[0] = 0;
    gunLastMagazine[0] = Gun::GUN_MAGAZINE_SIZE[0];
    for(int a = 0; a < Gun::NumOfGuns; a++)
    {
        if(Gun::GUN_LEVEL_START_AMMO[a] > Gun::GUN_MAGAZINE_SIZE[a])
            gunAmmo[a] = Gun::GUN_LEVEL_START_AMMO[a] - Gun::GUN_MAGAZINE_SIZE[a];
        else gunAmmo[a] = 0;
        gunLastMagazine[a] = Gun::GUN_LEVEL_START_AMMO[a] % Gun::GUN_MAGAZINE_SIZE[a];
        gunAmmoRestock[a] = 0;
        gunAmmoRestockTreshold[a] = Gun::GUN_MIN_AMMO_RESTOCK[a] + (Gun::GUN_MAX_AMMO_RESTOCK[a] - Gun::GUN_MIN_AMMO_RESTOCK[a]) * (GenerateRandomNumber(101) / 100.f);
    }
}

ShipIdentity::ShipIdentity(ShipInfo* shipInfo_, sf::Vector2i startGrid_, bool isMainShip_, bool isPlayable_, short rotation_):
    shipInfo(shipInfo_), isMainShip(isMainShip_), isPlayable(isPlayable_), rotation(rotation_), startGrid(startGrid_), shipEntity(nullptr), exp(0), specialMeter(0), spawnTime(0), gunChangeTime(0), shipInfoLevel(0), unitLevel(1), unitExpFactor(0), lastExp(0), lastGun(0)
{
    if(shipInfo != nullptr)
        shipInfoLevel = ShipInfo::CalculateClassLevel(shipInfo->shipClass, shipInfo->experiencePoint);

    for(int a = 0; a < Gun::NumOfGuns; a++)
    {
        gunAmmo[a] = Gun::GUN_LEVEL_START_AMMO[a];
        gunAmmoRestock[a] = 0;
        gunAmmoRestockTreshold[a] = Gun::GUN_MIN_AMMO_RESTOCK[a] + (Gun::GUN_MAX_AMMO_RESTOCK[a] - Gun::GUN_MIN_AMMO_RESTOCK[a]) * (GenerateRandomNumber(101) / 100.f);
        gunLastMagazine[a] = gunAmmo[a] != 0 || a == Gun::LaserGun ? Gun::GUN_MAGAZINE_SIZE[a] : 0;
    }
}

ShipIdentity* Game::AddShipIdentity(ShipInfo* shipInfo, sf::Vector2i grid_, bool isMainShip_, bool isPlayable_, short rotation_)
{
    if(shipInfo != nullptr)
    {
        shipIdentities.push_back(new ShipIdentity(shipInfo, grid_, isMainShip_, isPlayable_, rotation_));
        return shipIdentities[shipIdentities.size() - 1];
    }
    return nullptr;
}

ShipIdentity* Game::GetShipIdentity(Ship* ship_)
{
    for(int a = 0; a < shipIdentities.size(); a++)
        if(shipIdentities[a]->GetShipEntity() == ship_)
            return shipIdentities[a];
    return nullptr;
}

ShipIdentity* Game::GetShipIdentity(int index)
{
    if(index < shipIdentities.size())
        return shipIdentities[index];
    return nullptr;
}

ShipIdentity* Game::GetShipIdentity(sf::Vector2i startGridPos)
{
    for(int a = 0; a < shipIdentities.size(); a++)
        if(shipIdentities[a]->GetStartGrid() == startGridPos)
            return shipIdentities[a];
    return nullptr;
}


void Game::RemoveShipIdentity(ShipIdentity* identity)
{
    for(int a = 0; a < ShipInfo::NumOfCharacterType; a++)
    {
        if(a != ShipInfo::MainCharacterType)
            characterInfoGroup[a].Remove(identity->GetShipInfo());
    }
    for(int a = 0; a < shipIdentities.size(); a++)
        if(shipIdentities[a] == identity)
        {
            delete identity;
            shipIdentities.erase(shipIdentities.begin() + a);
        }
}

unsigned int Game::GetShipIdentitySize()
{
    return shipIdentities.size();
}

void ShipIdentity::RefreshShipEntity()
{
    if(shipEntity != nullptr)
    {
        if(shipEntity->GetAlive() == false)
        {
            gunLastMagazine[shipEntity->GetCurrentGun()] = shipEntity->GetGunMagazine();
            shipEntity = nullptr;
            spawnTime = spawnTime_last = 10.f;
        }
    }
}

void ShipIdentity::RefreshShipIdentities()
{
    for(auto& identity : shipIdentities)
        identity->RefreshShipEntity();
}

void ShipIdentity::Update(float dt)
{
    if(spawnTime > 0)
        spawnTime -= dt;

    if(shipEntity == nullptr && spawnTime <= 0)
        CreateShip();

    //modify later, its too simplified
    if(shipEntity != nullptr && shipEntity->GetAlive() && ShipUnit::GetActivePlayerUnit() != shipEntity)
    {
        gunChangeTime -= dt;
        if(gunChangeTime <= 0 || shipEntity->GetGunMagazine() + gunAmmo[shipEntity->GetCurrentGun()] == 0)
        {
            gunChangeTime = (0.3f + GenerateRandomNumber(101) / 100.f * 0.7f) * 60;

            short numOfAvailableGuns = 0;
            for(int i = 1; i < Gun::NumOfGuns; i++)
                if(gunAmmo[i] + gunLastMagazine[i] > 0)
                    numOfAvailableGuns++;
            short gunCountToSwitch = 0;
            if(numOfAvailableGuns == 1)
                gunCountToSwitch = 1;
            else if(numOfAvailableGuns > 1)
                gunCountToSwitch = 1 + GenerateRandomNumber(numOfAvailableGuns);
            short tempCount = 0;
            for(int i = 0; i < Gun::NumOfGuns; i++)
                if(i == 0 || gunAmmo[i] + gunLastMagazine[i] > 0)
                {
                    if(tempCount == gunCountToSwitch)
                    {
                        SwitchUnitGun(i);
                        break;
                    }
                    else tempCount++;
                }
        }
    }

    if(lastExp != exp)
    {
        lastExp = exp;

        unitLevel = 1;
        for(int a = 3; a >= 0; a--)
            if(exp >= ShipUnit::BaseLevelExpHigherUnits[a])
            {
                unitLevel += a + 1;
                break;
            }

        int baseLvlExp = unitLevel == 1 ? 0 : ShipUnit::BaseLevelExpHigherUnits[unitLevel - 2];
        int nextBaselvlExp = unitLevel == 5 ? 0 : ShipUnit::BaseLevelExpHigherUnits[unitLevel - 1];

        if(nextBaselvlExp > 0)
            unitExpFactor = (float)(exp - baseLvlExp) / (nextBaselvlExp - baseLvlExp);
        else unitExpFactor = 0.f;
    }

    damageDisplayEffect.Update(dt);
}

void ShipIdentity::UpdateShipIdentities(float dt)
{
    for(auto& identity : shipIdentities)
        identity->Update(dt);
}

void Game::UpdateCharacterTexture(ShipInfo* shipInfo, sf::Color color)
{
    if(shipInfo != nullptr)
    {
        for(int a = 0; a < darkVertices.getVertexCount(); a++)
            darkVertices[a].color = sf::Color(color.r, color.g, color.b, 255);
        for(int a = 0; a < lightVertices.getVertexCount(); a++)
            lightVertices[a].color = sf::Color(color.r, color.g, color.b, 150);
        for(int a = 0; a < 4; a++)
            tempShipVertices[a].color = sf::Color(color.r, color.g, color.b, 150);

        float minValue = 0;
        float maxValue = 0;

        if(shipInfo->shipClass == ShipInfo::D)
        {
            minValue = ShipInfo::Class_D_Min;
            maxValue = ShipInfo::Class_D_Max;
        }
        else if(shipInfo->shipClass == ShipInfo::C)
        {
            minValue = ShipInfo::Class_C_Min;
            maxValue = ShipInfo::Class_C_Max;
        }
        else if(shipInfo->shipClass == ShipInfo::B)
        {
            minValue = ShipInfo::Class_B_Min;
            maxValue = ShipInfo::Class_B_Max;
        }
        else if(shipInfo->shipClass == ShipInfo::A)
        {
            minValue = ShipInfo::Class_A_Min;
            maxValue = ShipInfo::Class_A_Max;
        }
        else if(shipInfo->shipClass == ShipInfo::S)
        {
            minValue = ShipInfo::Class_S_Min;
            maxValue = ShipInfo::Class_S_Max;
        }

        //HitPoint
        barVertices[1].position.x = 27 + (shipInfo->hitpoint - minValue) / (maxValue - minValue) * 128;
        barVertices[2].position.x = 27 + (shipInfo->hitpoint - minValue) / (maxValue - minValue) * 128;

        //Attack
        barVertices[5].position.x = 27 + (shipInfo->attack - minValue) / (maxValue - minValue) * 128;
        barVertices[6].position.x = 27 + (shipInfo->attack - minValue) / (maxValue - minValue) * 128;

        //Defense
        barVertices[9].position.x = 27 + (shipInfo->defense - minValue) / (maxValue - minValue) * 128;
        barVertices[10].position.x = 27 + (shipInfo->defense - minValue) / (maxValue - minValue) * 128;

        //Speed
        barVertices[13].position.x = 27 + shipInfo->speed / 100.f * 128;
        barVertices[14].position.x = 27 + shipInfo->speed / 100.f * 128;

        //Special
        barVertices[17].position.x = 27 + shipInfo->special / 100.f * 128;
        barVertices[18].position.x = 27 + shipInfo->special / 100.f * 128;

        //____Drawing
        //Clear Texture
        tempCharacterTextr.clear(sf::Color(255, 255, 255, 0));

        //Vertex
        tempCharacterTextr.draw(lightVertices);
        tempCharacterTextr.draw(darkVertices);
        tempCharacterTextr.draw(barVertices);
        tempCharacterTextr.draw(iconVertices, refTexture);

        //Skin
        if(Game::GetSkin((Game::SkinType)(shipInfo->skinType),
                         shipInfo->skinIndex,
                         shipInfo->team) != nullptr)
        {
            skinSprite.setTexture(Game::GetSkin((Game::SkinType)(shipInfo->skinType),
                                            shipInfo->skinIndex,
                                            shipInfo->team)->texture);
            tempCharacterTextr.draw(skinSprite);
        }

        //Name
        text.setFillColor(sf::Color::White);
        float textPosRest = 5;

        text.setString(shipInfo->name);
        text.setOrigin(text.getGlobalBounds().width / 2.f, text.getGlobalBounds().height / 2.f);
        text.setPosition(80, 11 - textPosRest);
        tempCharacterTextr.draw(text);

        text.setString("10");
        text.setOrigin(text.getGlobalBounds().width / 2.f, text.getGlobalBounds().height / 2.f);
        text.setPosition(149, 11 - textPosRest);
        tempCharacterTextr.draw(text);

        if(shipInfo->shipClass == ShipInfo::D)
            text.setString("D");
        else if(shipInfo->shipClass == ShipInfo::C)
            text.setString("C");
        else if(shipInfo->shipClass == ShipInfo::B)
            text.setString("B");
        else if(shipInfo->shipClass == ShipInfo::A)
            text.setString("A");
        else if(shipInfo->shipClass == ShipInfo::S)
            text.setString("S");
        text.setOrigin(text.getGlobalBounds().width / 2.f, text.getGlobalBounds().height / 2.f);
        text.setPosition(149, 38 - textPosRest);
        text.setFillColor(sf::Color(color.r, color.g, color.b, 255));
        tempCharacterTextr.draw(text);

        //Set texture
        tempCharacterTextr.display();
        shipInfo->texture = tempCharacterTextr.getTexture();
    }
}

void ShipIdentity::RestockGuns(const float& powerFactor_)
{
    for(int a = 1; a < Gun::NumOfGuns; a++)
    {
        gunAmmoRestock[a] += Gun::GUN_RESTOCK_PER_POWER_FACTOR[a] * powerFactor_;
        if(gunAmmoRestock[a] >= gunAmmoRestockTreshold[a])
        {
            gunAmmo[a] += gunAmmoRestock[a] * (1.f + (GenerateRandomNumber(201) - 100) / 100.f * 0.5f);
            if(gunAmmo[a] > Gun::GUN_MAX_AMMO[a])
                gunAmmo[a] = Gun::GUN_MAX_AMMO[a];

            gunAmmoRestock[a] = 0;
            gunAmmoRestockTreshold[a] = Gun::GUN_MIN_AMMO_RESTOCK[a] + (Gun::GUN_MAX_AMMO_RESTOCK[a] - Gun::GUN_MIN_AMMO_RESTOCK[a]) * (GenerateRandomNumber(101) / 100.f);
        }
    }
}

void Game::UpdateCharacterTexture(unsigned short characterType, unsigned short index, unsigned short team, sf::Color color)
{
    if(characterType < ShipInfo::NumOfCharacterType && index < characterInfoGroup[characterType].shipInfoSet[team].size())
    {
        ShipInfo* shipInfo = characterInfoGroup[characterType].shipInfoSet[team][index];

        if(shipInfo != nullptr)
        {
            for(int a = 0; a < darkVertices.getVertexCount(); a++)
                darkVertices[a].color = sf::Color(color.r, color.g, color.b, 255);
            for(int a = 0; a < lightVertices.getVertexCount(); a++)
                lightVertices[a].color = sf::Color(color.r, color.g, color.b, 150);
            for(int a = 0; a < 4; a++)
                tempShipVertices[a].color = sf::Color(color.r, color.g, color.b, 150);

            float minValue = 0;
            float maxValue = 0;

            if(shipInfo->shipClass == ShipInfo::D)
            {
                minValue = ShipInfo::Class_D_Min;
                maxValue = ShipInfo::Class_D_Max;
            }
            else if(shipInfo->shipClass == ShipInfo::C)
            {
                minValue = ShipInfo::Class_C_Min;
                maxValue = ShipInfo::Class_C_Max;
            }
            else if(shipInfo->shipClass == ShipInfo::B)
            {
                minValue = ShipInfo::Class_B_Min;
                maxValue = ShipInfo::Class_B_Max;
            }
            else if(shipInfo->shipClass == ShipInfo::A)
            {
                minValue = ShipInfo::Class_A_Min;
                maxValue = ShipInfo::Class_A_Max;
            }
            else if(shipInfo->shipClass == ShipInfo::S)
            {
                minValue = ShipInfo::Class_S_Min;
                maxValue = ShipInfo::Class_S_Max;
            }

            if(characterType == ShipInfo::MainCharacterType || characterType == ShipInfo::CustomCharactertype)
            {
                //HitPoint
                barVertices[1].position.x = 27 + (shipInfo->hitpoint - minValue) / (maxValue - minValue) * 128;
                barVertices[2].position.x = 27 + (shipInfo->hitpoint - minValue) / (maxValue - minValue) * 128;

                //Attack
                barVertices[5].position.x = 27 + (shipInfo->attack - minValue) / (maxValue - minValue) * 128;
                barVertices[6].position.x = 27 + (shipInfo->attack - minValue) / (maxValue - minValue) * 128;

                //Defense
                barVertices[9].position.x = 27 + (shipInfo->defense - minValue) / (maxValue - minValue) * 128;
                barVertices[10].position.x = 27 + (shipInfo->defense - minValue) / (maxValue - minValue) * 128;
            }

            //Speed
            barVertices[13].position.x = 27 + shipInfo->speed / 100.f * 128;
            barVertices[14].position.x = 27 + shipInfo->speed / 100.f * 128;

            //Special
            barVertices[17].position.x = 27 + shipInfo->special / 100.f * 128;
            barVertices[18].position.x = 27 + shipInfo->special / 100.f * 128;

            //____Drawing
            //Clear Texture
            tempCharacterTextr.clear(sf::Color(255, 255, 255, 0));

            //Vertex
            if(characterType == ShipInfo::MainCharacterType || characterType == ShipInfo::CustomCharactertype)
            {
                tempCharacterTextr.draw(lightVertices);
                tempCharacterTextr.draw(darkVertices);
                tempCharacterTextr.draw(barVertices);
                tempCharacterTextr.draw(iconVertices, refTexture);
            }
            else if(characterType == ShipInfo::TemporaryCharacterType)
            {
                sf::VertexArray va;
                va.setPrimitiveType(sf::Quads);

                for(int a = 0; a < 8 && a < lightVertices.getVertexCount(); a++)
                    va.append(lightVertices[a]);
                for(int a = 20; a < lightVertices.getVertexCount(); a++)
                    va.append(lightVertices[a]);
                tempCharacterTextr.draw(va);
                va.clear();

                for(int a = 12; a < 20 && a < darkVertices.getVertexCount(); a++)
                    va.append(darkVertices[a]);
                for(int a = 36; a < darkVertices.getVertexCount(); a++)
                    va.append(darkVertices[a]);
                tempCharacterTextr.draw(va);
                va.clear();

                for(int a = 12; a < barVertices.getVertexCount(); a++)
                    va.append(barVertices[a]);
                tempCharacterTextr.draw(va);
                va.clear();

                for(int a = 12; a < 20 && a < iconVertices.getVertexCount(); a++)
                    va.append(iconVertices[a]);
                tempCharacterTextr.draw(va, refTexture);

                tempCharacterTextr.draw(tempShipVertices);
            }

            //Skin
            if(Game::GetSkin((Game::SkinType)(shipInfo->skinType),
                             shipInfo->skinIndex,
                             shipInfo->team) != nullptr)
            {
                skinSprite.setTexture(Game::GetSkin((Game::SkinType)(shipInfo->skinType),
                                            shipInfo->skinIndex,
                                            shipInfo->team)->texture);
                tempCharacterTextr.draw(skinSprite);
            }

            //Name
            text.setFillColor(sf::Color::White);
            float textPosRest = 5;

            text.setString(shipInfo->name);
            if(text.getLocalBounds().width > 100.f)
                while(text.getString().getSize() > 0)
                {
                    std::string str = text.getString();
                    str.pop_back();
                    text.setString(str + "...");
                    if(text.getLocalBounds().width > 100.f)
                        text.setString(str);
                    else break;
                }
            text.setOrigin(text.getLocalBounds().width / 2.f, text.getLocalBounds().height / 2.f);
            text.setPosition(80, 8);
            tempCharacterTextr.draw(text);

            if(characterType == ShipInfo::MainCharacterType || characterType == ShipInfo::CustomCharactertype)
            {
                text.setString("10");
                text.setOrigin(text.getLocalBounds().width / 2.f, text.getLocalBounds().height / 2.f);
                text.setPosition(149, 11 - textPosRest);
                tempCharacterTextr.draw(text);

                if(shipInfo->shipClass == ShipInfo::D)
                    text.setString("D");
                else if(shipInfo->shipClass == ShipInfo::C)
                    text.setString("C");
                else if(shipInfo->shipClass == ShipInfo::B)
                    text.setString("B");
                else if(shipInfo->shipClass == ShipInfo::A)
                    text.setString("A");
                else if(shipInfo->shipClass == ShipInfo::S)
                    text.setString("S");
                text.setOrigin(text.getGlobalBounds().width / 2.f, text.getGlobalBounds().height / 2.f);
                text.setPosition(149, 38 - textPosRest);
                text.setFillColor(sf::Color(color.r, color.g, color.b, 255));
                tempCharacterTextr.draw(text);
            }

            //Set texture
            tempCharacterTextr.display();
            shipInfo->texture = tempCharacterTextr.getTexture();
        }
    }
}

void Game::UpdateCharacterTexture(sf::Color color)
{
    for(int a = 0; a < ShipInfo::NumOfCharacterType; a++)
        for(int b = 0; b < Game::NumOfTeams; b++)
            for(int c = 0; c < characterInfoGroup[a].shipInfoSet[b].size(); c++)
                UpdateCharacterTexture(a, c, b, color);
}

void Game::InitializeCharacterInfo()
{
    //____Texture Data Preparation
    refTexture = &AssetManager::GetTexture("ShipInfo_Data");

    skinSprite.setScale(0.5f, 0.5f);
    skinSprite.setTextureRect(sf::IntRect(0, 0, 194, 194));
    skinSprite.setOrigin(97, 97);
    skinSprite.setPosition(80, 76);

    tempCharacterTextr.create(160, 260);
    tempCharacterTextr.setSmooth(true);
    tempCharacterTextr.clear(sf::Color(255, 255, 255, 0));
    tempCharacterTextr.display();

    tempProfileTextr.create(122, 122);
    tempProfileTextr.setSmooth(true);
    tempProfileTextr.clear(sf::Color(255, 255, 255, 0));
    tempProfileTextr.display();

    lightVertices.setPrimitiveType(sf::Quads);
    darkVertices.setPrimitiveType(sf::Quads);
    barVertices.setPrimitiveType(sf::Quads);
    iconVertices.setPrimitiveType(sf::Quads);
    tempShipVertices.setPrimitiveType(sf::Quads);
    ghostShipVertices.setPrimitiveType(sf::Quads);

    text.setFont(*Game::GetFont("Conthrax"));
    text.setFillColor(sf::Color::White);
    text.setCharacterSize(13);

    auto addConnerVetices = [](sf::FloatRect rect, sf::VertexArray& va)
    {
        va.append(sf::Vertex(sf::Vector2f(rect.left, rect.top)));
        va.append(sf::Vertex(sf::Vector2f(rect.left + rect.width, rect.top)));
        va.append(sf::Vertex(sf::Vector2f(rect.left + rect.width, rect.top + rect.height)));
        va.append(sf::Vertex(sf::Vector2f(rect.left, rect.top + rect.height)));
    };

    auto addTextrConnerVetices = [](sf::FloatRect rect, sf::FloatRect textrRect, sf::VertexArray& va)
    {
        va.append(sf::Vertex(sf::Vector2f(rect.left, rect.top), sf::Vector2f(textrRect.left, textrRect.top)));
        va.append(sf::Vertex(sf::Vector2f(rect.left + rect.width, rect.top), sf::Vector2f(textrRect.left + textrRect.width, textrRect.top)));
        va.append(sf::Vertex(sf::Vector2f(rect.left + rect.width, rect.top + rect.height), sf::Vector2f(textrRect.left + textrRect.width, textrRect.top + textrRect.height)));
        va.append(sf::Vertex(sf::Vector2f(rect.left, rect.top + rect.height), sf::Vector2f(textrRect.left, textrRect.top + textrRect.height)));
    };

    //Light Vertex
    addConnerVetices(sf::FloatRect(0, 0, 160, 22), lightVertices);
    addConnerVetices(sf::FloatRect(0, 27, 160, 97), lightVertices);
    addConnerVetices(sf::FloatRect(22, 130, 138, 22), lightVertices);
    addConnerVetices(sf::FloatRect(22, 157, 138, 22), lightVertices);
    addConnerVetices(sf::FloatRect(22, 184, 138, 22), lightVertices);
    addConnerVetices(sf::FloatRect(22, 211, 138, 22), lightVertices);
    addConnerVetices(sf::FloatRect(22, 238, 138, 22), lightVertices);

    //Dark Vertex
    addConnerVetices(sf::FloatRect(27, 135, 128, 11), darkVertices);
    addConnerVetices(sf::FloatRect(27, 162, 128, 11), darkVertices);
    addConnerVetices(sf::FloatRect(27, 189, 128, 11), darkVertices);
    addConnerVetices(sf::FloatRect(27, 217, 128, 11), darkVertices);
    addConnerVetices(sf::FloatRect(27, 244, 128, 11), darkVertices);
    addConnerVetices(sf::FloatRect(138, 0, 22, 22), darkVertices);
    addConnerVetices(sf::FloatRect(0, 130, 22, 22), darkVertices);
    addConnerVetices(sf::FloatRect(0, 157, 22, 22), darkVertices);
    addConnerVetices(sf::FloatRect(0, 184, 22, 22), darkVertices);
    addConnerVetices(sf::FloatRect(0, 211, 22, 22), darkVertices);
    addConnerVetices(sf::FloatRect(0, 238, 22, 22), darkVertices);

    //TempShip Vertex
    addConnerVetices(sf::FloatRect(0, 130, 160, 76), tempShipVertices);
    addConnerVetices(sf::FloatRect(21, 147, 118, 5), tempShipVertices);
    addConnerVetices(sf::FloatRect(21, 165, 118, 5), tempShipVertices);
    addConnerVetices(sf::FloatRect(21, 183, 118, 5), tempShipVertices);
    for(int a = 4; a < tempShipVertices.getVertexCount(); a++)
        tempShipVertices[a].color = sf::Color::White;

    //Bar Vertex
    addConnerVetices(sf::FloatRect(27, 135, 128, 11), barVertices);
    addConnerVetices(sf::FloatRect(27, 162, 128, 11), barVertices);
    addConnerVetices(sf::FloatRect(27, 189, 128, 11), barVertices);
    addConnerVetices(sf::FloatRect(27, 217, 128, 11), barVertices);
    addConnerVetices(sf::FloatRect(27, 244, 128, 11), barVertices);
    for(int a = 0; a < barVertices.getVertexCount(); a++)
            barVertices[a].color = sf::Color::White;

    //Icon Vertex
    addTextrConnerVetices(sf::FloatRect(0, 130, 22, 22), sf::FloatRect(0, 100, 100, 100), iconVertices);
    addTextrConnerVetices(sf::FloatRect(0, 157, 22, 22), sf::FloatRect(0, 200, 100, 100), iconVertices);
    addTextrConnerVetices(sf::FloatRect(0, 184, 22, 22), sf::FloatRect(0, 0, 100, 100), iconVertices);
    addTextrConnerVetices(sf::FloatRect(0, 211, 22, 22), sf::FloatRect(0, 300, 100, 100), iconVertices);
    addTextrConnerVetices(sf::FloatRect(0, 238, 22, 22), sf::FloatRect(0, 400, 100, 100), iconVertices);
    addTextrConnerVetices(sf::FloatRect(138, 27, 22, 23), sf::FloatRect(0, 500, 100, 108), iconVertices);

    //____Main Characters
    //Alpha
    characterInfoGroup[ShipInfo::MainCharacterType].AddShipInfo(AlphaCharName[ShipInfo::Alex], ShipInfo::C, 0, 0, Game::Alpha, Game::SkinMainShip, 420, 400, 300, 100, 70);

    //Delta
    characterInfoGroup[ShipInfo::MainCharacterType].AddShipInfo(DeltaCharName[ShipInfo::Magmix], ShipInfo::B, 0, 0, Game::Delta, Game::SkinMainShip, 600, 800, 650, 50, 50);

    //Vortex
    characterInfoGroup[ShipInfo::MainCharacterType].AddShipInfo(VortexCharName[ShipInfo::Vyrim], ShipInfo::B, 0, 0, Game::Vortex, Game::SkinMainShip, 860, 550, 700, 80, 80);

    //Omega
    characterInfoGroup[ShipInfo::MainCharacterType].AddShipInfo(OmegaCharName[ShipInfo::Zendor], ShipInfo::S, 0, 0, Game::Omega, Game::SkinMainShip, 1740, 1900, 1800, 60, 90);
}

void ShipIdentity::DrawInfo(sf::RenderTarget& rw, sf::FloatRect viewArea)
{
    sf::Sprite sp(Game::GetSkin((Game::SkinType)shipInfo->skinType, shipInfo->skinIndex, shipInfo->team)->texture, sf::IntRect(0, 0, 180, 180));
    sp.setScale(0.5f, 0.5f);
    sp.setOrigin(90, 90);
    sp.setPosition((sf::Vector2f)(startGrid * MapTileManager::TILE_SIZE) + sf::Vector2f(48.5, 48.5));
    sp.setRotation(rotation * 45.f);

    if(sp.getGlobalBounds().intersects(viewArea))
        rw.draw(sp);
}

void FormUnit::Draw(sf::RenderTarget& target_, sf::FloatRect viewArea)
{
    if(formType == FormMainDefence)
    {
        sf::Sprite sp(AssetManager::GetTexture("Defence_Spot"));
        sp.setScale(0.5f, 0.5f);
        sp.setOrigin(100, 100);
        sp.setPosition((sf::Vector2f)(startGrid * MapTileManager::TILE_SIZE) + sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE) - sf::Vector2f(0, 10));

        if(sp.getGlobalBounds().intersects(viewArea))
            target_.draw(sp);
    }

    if(formType == FormMiniDefence)
    {
        sf::Sprite sp(AssetManager::GetTexture("Defence_Spot"));
        sp.setScale(0.3f, 0.3f);
        sp.setOrigin(100, 100);
        sp.setPosition((sf::Vector2f)(startGrid * MapTileManager::TILE_SIZE) + sf::Vector2f(48.5, 38.5));

        if(sp.getGlobalBounds().intersects(viewArea))
            target_.draw(sp);
    }

    if(skinIndex >= 0)
    {
        short skinType = -1;
        if(formType == FormMainDefence)
            skinType = Game::SkinMainDefence;
        else if(formType == FormMiniDefence)
            skinType = Game::SkinMiniDefence;
        else if(formType == FormMiniShip)
            skinType = Game::SkinMiniShip;
        else if(formType == FormMicroShip)
            skinType = Game::SkinMicroShip;

        if(formType == FormMainDefence)
        {
            sf::Sprite sp(Game::GetSkin((Game::SkinType)skinType, skinIndex, team)->texture, sf::IntRect(0, 0, 360, 360));
            sp.setScale(0.5f, 0.5f);
            sp.setOrigin(180, 180);
            sp.setPosition((sf::Vector2f)(startGrid * MapTileManager::TILE_SIZE) + sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE) - sf::Vector2f(0, 10));
            sp.setRotation(rotation * 45.f);

            if(sp.getGlobalBounds().intersects(viewArea))
                target_.draw(sp);
        }
        else
        {
            sf::Sprite sp(Game::GetSkin((Game::SkinType)skinType, skinIndex, team)->texture, sf::IntRect(0, 0, 180, 180));
            sp.setScale(0.5f, 0.5f);
            sp.setOrigin(90, 90);
            sp.setPosition((sf::Vector2f)(startGrid * MapTileManager::TILE_SIZE) + sf::Vector2f(48.5, 38.5));
            sp.setRotation(rotation * 45.f);

            if(sp.getGlobalBounds().intersects(viewArea))
                target_.draw(sp);
        }
    }
}

FormUnit* Game::AddFormUnit(FormUnit* formUnit)
{
    if(formUnit != nullptr)
    {
        formUnits.push_back(formUnit);
        return formUnits[formUnits.size() - 1];
    }

    return nullptr;
}

void Game::RemoveFormUnit(FormUnit* formUnit)
{
    for(int a = 0; a < formUnits.size(); a++)
        if(formUnits[a] == formUnit)
        {
            delete formUnits[a];
            formUnits.erase(formUnits.begin() + a);
        }
}

FormUnit* Game::GetFormUnit(int index)
{
    if(index >= 0 && index < formUnits.size())
        return formUnits[index];
    return nullptr;
}

unsigned int Game::GetFormUnitSize()
{
    return formUnits.size();
}

FormUnit* Game::GetFormUnit(sf::Vector2i grid)
{
    for(int a =  0; a < formUnits.size(); a++)
    {
        if(formUnits[a]->GetFormType() == FormUnit::FormMainDefence)
        {
            if(grid == formUnits[a]->GetStartGrid() ||
               grid == formUnits[a]->GetStartGrid() + sf::Vector2i(1, 0) ||
               grid == formUnits[a]->GetStartGrid() + sf::Vector2i(1, 1) ||
               grid == formUnits[a]->GetStartGrid() + sf::Vector2i(0, 1))
                return formUnits[a];
        }
        else if(grid == formUnits[a]->GetStartGrid())
            return formUnits[a];
    }

    return nullptr;
}

bool Game::isOnFormUnit(sf::IntRect rect)
{
    for(int a = 0; a < formUnits.size(); a++)
    {
        if(formUnits[a]->GetFormType() == FormUnit::FormMainDefence &&
           rect.intersects(sf::IntRect(formUnits[a]->GetStartGrid().x, formUnits[a]->GetStartGrid().y, 2, 2)))
           return true;
        else if(rect.intersects(sf::IntRect(formUnits[a]->GetStartGrid().x, formUnits[a]->GetStartGrid().y, 1, 1)))
            return true;
    }
    return false;
}

void ShipIdentity::CreateShip(bool atStartPos)
{
    if(shipInfo != nullptr && shipEntity == nullptr)
    {
        bool createIt = true;

        sf::Vector2f pos;
        if(atStartPos)
            pos = (sf::Vector2f)(startGrid * MapTileManager::TILE_SIZE) + sf::Vector2f(48.5, 48.5);
        else
        {
            std::vector<Base*> basesToSpawn;
            for(int a = 0; a < Game::GetNumberOfBases(); a++)
            {
                Base* tempBase = Game::GetBase(a);
                if(tempBase->IsActive() && tempBase->GetTeam() == shipInfo->team)
                    basesToSpawn.push_back(tempBase);
            }

            if(basesToSpawn.size() > 0)
            {
                const std::vector<sf::Vector2i>& baseGrids = basesToSpawn[GenerateRandomNumber(basesToSpawn.size())]->GetSpawnPoint()->spawnGrids;
                pos = (sf::Vector2f)baseGrids[GenerateRandomNumber(baseGrids.size())] * (float)MapTileManager::TILE_SIZE + sf::Vector2f(GenerateRandomNumber(MapTileManager::TILE_SIZE), GenerateRandomNumber(MapTileManager::TILE_SIZE));
            }
            else createIt = false;
        }

        if(createIt)
        {
            ShipUnit::UnitType type_ = isMainShip ? ShipUnit::MainShip : ShipUnit::SubShip;
            if(isPlayable && isMainShip)
                type_ = ShipUnit::PlayerShip;

            shipEntity = new ShipUnit(type_, pos, shipInfo->team, shipInfo->hitpoint, shipInfo->attack, shipInfo->defense, shipInfo->speed, shipInfo->special, shipInfo->skinIndex, shipInfo->skinType, false, this);
            shipEntity->SwitchGun(lastGun, gunLastMagazine[lastGun], &gunAmmo[lastGun]);
            if(atStartPos)
                shipEntity->SetAngle(rotation * 45);
            EntityManager::CreateShip(shipEntity);

            /*if(type_ == ShipUnit::PlayerShip && ShipUnit::GetActivePlayerUnit() == nullptr)
            {
                ShipUnit::SetActivePlayerUnit(shipEntity);
                EntityManager::SetCameraFocus(shipEntity);
            }*/
        }
    }
}

void FormUnit::Create()
{
    if(formType == FormMainDefence)
        EntityManager::CreateDefence((sf::Vector2f)(startGrid * MapTileManager::TILE_SIZE) + sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE) - sf::Vector2f(0, 10), Defence::MainDefence, team, rotation, skinIndex);
    else if(formType == FormMiniDefence)
        EntityManager::CreateDefence((sf::Vector2f)(startGrid * MapTileManager::TILE_SIZE) + sf::Vector2f(48.5, 38.5), Defence::MiniDefence, team, rotation, skinIndex);
}
