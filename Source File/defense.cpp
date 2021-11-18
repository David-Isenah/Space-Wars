#include "defence.h"
#include "AssetManager.h"
#include "entitymanager.h"
#include "public_functions.h"
#include "game.h"
#include <iostream>

MultiDrawer multidrawer_defence;

void Defence::InitialiseMultiDrawer()
{
    multidrawer_defence.SetTexture(&AssetManager::GetTexture("Defence_Spot"));
}

void Defence::Draw(sf::RenderTarget& target)
{
    multidrawer_defence.Draw(target);
}

Defence::Defence(sf::Vector2f locationPos, DefenceType type_, short team_, short rotation_, int skinIndex_) :
    location(locationPos),
    type(type_),
    team(team_),
    rotation(rotation_),
    shipDefence(nullptr),
    baseId(-1),
    skinIndex(skinIndex_)
{
    sf::Vector2f textrSize = (sf::Vector2f)multidrawer_defence.GetTexture()->getSize();
    transformation.setOrigin(textrSize / 2.f);
    transformation.setPosition(locationPos);

    if(type == MainDefence)
        transformation.setScale(0.5f, 0.5f);
    else if(type == MiniDefence)
        transformation.setScale(0.3f, 0.3f);

    sf::Vector2i grid = GetGridPosition(locationPos);
    baseId = GetGridInfo(grid.y, grid.x).baseId;

    float xStart = locationPos.x - textrSize.x * transformation.getScale().x / 2.f;
    float xEnd = locationPos.x + textrSize.x * transformation.getScale().x / 2.f;
    float yStart = locationPos.y - textrSize.y * transformation.getScale().x / 2.f;
    float yEnd = locationPos.y + textrSize.y * transformation.getScale().x / 2.f;
    for(int y = yStart; y <= yEnd && baseId >= 0; y++)
        for(int x = xStart; x <= xEnd && baseId >= 0; x++)
            if(x >= 0 && x < MapTileManager::MAX_MAPTILE && y >= 0 && y < MapTileManager::MAX_MAPTILE)
                if(GetGridInfo(y, x).baseId != baseId)
                    baseId = -1;

    if(baseId >= 0)
        Game::GetBase(baseId)->AddDefence(this);

    vertices[0].position = sf::Vector2f(xStart, yStart);
    vertices[1].position = sf::Vector2f(xEnd, yStart);
    vertices[2].position = sf::Vector2f(xEnd, yEnd);
    vertices[3].position = sf::Vector2f(xStart, yEnd);

    vertices[0].texCoords = sf::Vector2f(0, 0);
    vertices[1].texCoords = sf::Vector2f(textrSize.x, 0);
    vertices[2].texCoords = sf::Vector2f(textrSize.x, textrSize.y);
    vertices[3].texCoords = sf::Vector2f(0, textrSize.y);

    spotRect = transformation.getTransform().transformRect(sf::FloatRect(0, 0, textrSize.x, textrSize.y));
}

ShipDefence* Defence::BuildEntity(bool isSpawning_)
{
    if(shipDefence == nullptr)
    {
        const Game::DifficultyInfo& difficulty = Game::GetDifficultyInfo();
        int attributeValue = difficulty.mainDefence;

        ShipDefence::DefenceType dType = ShipDefence::MainDefenseShip;
        if(type == MiniDefence)
        {
            dType = ShipDefence::MiniDefenseShip;
            attributeValue = difficulty.miniDefence;
        }

        if(skinIndex >= 0)
            shipDefence = new ShipDefence(dType, location, team, attributeValue, attributeValue * 0.75f, attributeValue * 0.75f, skinIndex, isSpawning_);
        else if(isSpawning_)
        {
            Game::SkinType skinType = Game::SkinMainDefence;
            if(type == MiniDefence)
                skinType = Game::SkinMiniDefence;

            shipDefence = new ShipDefence(dType, location, team, attributeValue, attributeValue * 0.75f, attributeValue * 0.75f, GenerateRandomNumber(Game::GetSkinSize(skinType, team)), isSpawning_);
        }

        if(shipDefence != nullptr)
        {
            shipDefence->SetAngle(rotation * 45);
            EntityManager::CreateShip(shipDefence);
        }
        return shipDefence;
    }

    return nullptr;
}

void Defence::RefreshEntities()
{
    if(shipDefence != nullptr)
        if(shipDefence->GetAlive() == false)
            shipDefence = nullptr;
}

bool Defence::IsShipDefenceAlive()
{
    if(shipDefence != nullptr)
        if(shipDefence->GetAlive() == true)
            return true;

    return false;
}

sf::FloatRect Defence::GetDefenceSpotRect()
{
    return spotRect;
}

void Defence::Update(float dt)
{
    multidrawer_defence.AddVertex(vertices, 4);
}
