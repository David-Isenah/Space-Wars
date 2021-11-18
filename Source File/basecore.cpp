#include "basecore.h"
#include "AssetManager.h"
#include "maptilemanager.h"
#include "entitymanager.h"
#include "game.h"
#include "displayeffect_effects.h"

std::vector<BaseCore*> baseCoreToDraw;
DE_DamageDisplay* bc_damageDisplayEffect = nullptr;
BaseCore* bc_damageDisplayEffect_updater = nullptr;

BaseCore::BaseCore(short team_, sf::Vector2i grid_) :
    grid(grid_),
    wheelAngle(0.f),
    baseId(0)
{
    sf::Vector2f refPos = sf::Vector2f((grid_.x - 1) * MapTileManager::TILE_SIZE, (grid_.y - 1) * MapTileManager::TILE_SIZE);
    position = (sf::Vector2f)(grid_ * MapTileManager::TILE_SIZE) + sf::Vector2f(MapTileManager::TILE_SIZE / 2.f, MapTileManager::TILE_SIZE / 2.f);

    team = team_;
    shipType = Ship::NoShipType;
    hitPoint = health = actualHitpoint = 10000;
    defense = 1.f;
    barLength = 400.f;
    barHeight = 175.f;
    sizeRadius = 75.f;
    dodgeRadius = 206.f;
    baseId = GetGridInfo(grid_.y, grid_.x).baseId;

    sprite.setPosition(refPos + sf::Vector2f(0, 3));
    spriteUpper.setPosition(refPos + sf::Vector2f(0, -12));
    wheel.setPosition(refPos + sf::Vector2f(149, 152));
    wheelCover.setPosition(refPos + sf::Vector2f(62, 55));

    rect.left = (grid_.x - 1) * MapTileManager::TILE_SIZE;
    rect.top = (grid_.y - 1) * MapTileManager::TILE_SIZE;
    rect.width = rect.height = MapTileManager::TILE_SIZE * 3;

    coreRect.width = coreRect.height = 167.f;
    coreRect.left = refPos.x + 62.f;
    coreRect.top = refPos.y + 55.f;

    if(team_ == 0)
        SetTexture(&AssetManager::GetTexture("BaseCore_Alpha"));
    else if(team_ == 1)
        SetTexture(&AssetManager::GetTexture("BaseCore_Delta"));
    else if(team_ == 2)
        SetTexture(&AssetManager::GetTexture("BaseCore_Vortex"));
    else if(team_ == 3)
        SetTexture(&AssetManager::GetTexture("BaseCore_Omega"));

    if(bc_damageDisplayEffect == nullptr)
        bc_damageDisplayEffect = new DE_DamageDisplay();
    bc_damageDisplayEffect_updater = this;
}

BaseCore::~BaseCore()
{
    for(int a = 0; a < baseCoreToDraw.size(); a++)
        if(this == baseCoreToDraw[a])
        {
            baseCoreToDraw.erase(baseCoreToDraw.begin() + a);
            a--;
        }
    if(bc_damageDisplayEffect_updater == this)
    {
        if(bc_damageDisplayEffect != nullptr)
            delete bc_damageDisplayEffect;
        bc_damageDisplayEffect_updater = nullptr;
        bc_damageDisplayEffect = nullptr;
    }
}

void BaseCore::InitializeForLevel()
{
    hitPoint = Game::GetDifficultyInfo().baseCore;
    health = actualHitpoint = hitPoint;
    alive = true;
}

void BaseCore::GetPoints(std::vector<sf::Vector2f>& points)
{
    points.clear();

    for(int a = 0; a < 5; a++)
        points.push_back(globalPoints[a]);
}

sf::Vector2f BaseCore::GetPosition()
{
    return (sf::Vector2f)(grid * MapTileManager::TILE_SIZE) + sf::Vector2f(MapTileManager::TILE_SIZE / 2.f, MapTileManager::TILE_SIZE / 2.f);
}

void BaseCore::GiveKillPoints(Ship* shooter)
{
    //code to give to all team up
}

void BaseCore::GetDamagedBy(float damage, sf::Vector2f hitPosition, Ship* shooter)
{
    if(alive && damage > 0)
    {
        float actualDamage = (damage * damage) / (damage + GetActualDefence());

        health -= actualDamage;
        if(health <= 0)
        {
            health = 0;
            alive = false;
            LeaveGroup();
            GiveKillPoints(shooter);
        }
        if(bc_damageDisplayEffect != nullptr)
            bc_damageDisplayEffect->Add(actualDamage, hitPosition);
    }

    if(barTransparencyTime < 0.5f)
        barTransparencyTime = BAR_DURATION - barTransparencyTime;
    else if(barTransparencyTime < BAR_DURATION - 0.5f)
        barTransparencyTime = BAR_DURATION - 0.5f;

    lastHitTime = HIT_EFFECT_DURATION;
}

void BaseCore::SetTexture(sf::Texture* texture)
{
    if(texture != nullptr)
    {
        sprite.setTexture(*texture);
        sprite.setTextureRect(sf::IntRect(0, 15, 294, 294));
        //sprite.setOrigin(sf::Vector2f(147, 147));

        spriteUpper.setTexture(*texture);
        spriteUpper.setTextureRect(sf::IntRect(0, 0, 294, 15));
        //spriteUpper.setOrigin(sf::Vector2f(147, 159));

        wheel.setTexture(*texture);
        wheel.setTextureRect(sf::IntRect(294, 0, 150, 150));
        wheel.setOrigin(75, 75);

        wheelCover.setTexture(*texture);
        wheelCover.setTextureRect(sf::IntRect(444, 0, 174, 184));
        //wheelCover.setOrigin(87, 103);
    }
}

void BaseCore::Update(float dt, sf::FloatRect cameraRect)
{
    if(lastHitTime > 0)
        lastHitTime -= dt;
    if(barTransparencyTime > 0)
        barTransparencyTime -= dt;

    wheelAngle += WHEEL_ROTATION_SPEED * dt;
    if(wheelAngle > 360)
        wheelAngle -= 360;

    wheel.setRotation(wheelAngle);

    sf::Color color(255, 255, 255);
    if(lastHitTime > 0)
        color = sf::Color(255, 200, 200);

    wheelCover.setColor(color);
    wheel.setColor(color);
    sprite.setColor(color);
    spriteUpper.setColor(color);

    if(rect.intersects(cameraRect))
        baseCoreToDraw.push_back(this);

    if(bc_damageDisplayEffect_updater == this && bc_damageDisplayEffect != nullptr)
        bc_damageDisplayEffect->Update(dt);

    UpdateHealthBar();
}

void BaseCore::Update(float dt)
{
    Update(dt, EntityManager::GetCameraRect());
}

void BaseCore::DrawUpper(sf::RenderTarget& target)
{
    for(BaseCore*& baseCore : baseCoreToDraw)
        target.draw(baseCore->spriteUpper);
}

void BaseCore::DrawLower(sf::RenderTarget& target)
{
    for(BaseCore*& baseCore : baseCoreToDraw)
    {
        target.draw(baseCore->sprite);
        target.draw(baseCore->wheel);
        target.draw(baseCore->wheelCover);
    }
}

void BaseCore::ResetBaseCoresToDraw()
{
    baseCoreToDraw.clear();
}

sf::Vector2i BaseCore::GetGrid()
{
    return grid;
}

sf::FloatRect BaseCore::GetBoundingRect()
{
    return rect;
}

sf::FloatRect BaseCore::GetCoreBoundingRect()
{
    return coreRect;
}

int BaseCore::GetBaseId()
{
    return baseId;
}

void BaseCore::Shoot()
{
}

int BaseCore::GetLevelBaseExp()
{
    return 0;
}

int BaseCore::GetNextLevelBaseExp()
{
    return 0;
}

bool BaseCore::ResolveWallCollision(sf::FloatRect& wallRect, const float& dt)
{
    return false;
}
