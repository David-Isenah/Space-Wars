#include "displayeffect.h"
#include "displayeffect_effects.h"
#include "multidrawer.h"
#include "AssetManager.h"
#include "bullet.h"
#include <list>

std::list<DisplayEffect*> displayEffects;

MultiDrawer multidrawer_shoot_laser[2];
MultiDrawer multidrawer_hit_laser;
MultiDrawer multidrawer_bulletspeed;
MultiDrawer DE_SpecialRadiation::multidrawer;

void DisplayEffect::Initialise()
{
    multidrawer_shoot_laser[0].SetTexture(&AssetManager::GetTexture("DE_Shoot_Laser_1"));
    multidrawer_shoot_laser[1].SetTexture(&AssetManager::GetTexture("DE_Shoot_Laser_2"));

    multidrawer_hit_laser.SetTexture(&AssetManager::GetTexture("DE_Hit_Laser"));
}

void DisplayEffect::AddDisplayEffect(DisplayEffect* effect_)
{
    if(effect_ != nullptr)
        displayEffects.push_back(effect_);
}

void DisplayEffect::ClearDisplayEffects()
{
    for(auto itr = displayEffects.begin(); itr != displayEffects.end(); itr++)
        delete *itr;
    displayEffects.clear();
}

void DisplayEffect::UpdateDisplayEffects(float dt)
{
    for(auto itr = displayEffects.begin(); itr != displayEffects.end();)
    {
        DisplayEffect* effect = (*itr);
        effect->Update(dt);
        if(effect->IsActive() == false)
        {
            delete *itr;
            itr = displayEffects.erase(itr);
        }
        else itr++;
    }
}

void DisplayEffect::RefreshDisplayEffects()
{
    for(auto itr = displayEffects.begin(); itr != displayEffects.end(); itr++)
        (*itr)->RefreshEntities();
}

//DE Effects - MultiDrawer Functions
//DE_Shoot
void DE_Shoot::InitialiseMultiDrawer(short& type)
{
    if(type == Bullet::LaserBullet)
        multiDrawer = &multidrawer_shoot_laser[GenerateRandomNumber(2)];
    //Plus some more...
}

void DE_Shoot::Draw(sf::RenderTarget& target_)
{
    multidrawer_shoot_laser[0].Draw(target_);
    multidrawer_shoot_laser[1].Draw(target_);
}

//DE_Hit
void DE_Hit::InitialiseMultiDrawer(short& type)
{
    if(type == Bullet::LaserBullet)
        multiDrawer = &multidrawer_hit_laser;
    //plus some more
}

void DE_Hit::Draw(sf::RenderTarget& target_)
{
    multidrawer_hit_laser.Draw(target_);
}

//DE_BulletSpeed
void DE_BulletSpeed::InitialiseMultiDrawer()
{
    multiDrawer = &multidrawer_bulletspeed;
}

void DE_BulletSpeed::Draw(sf::RenderTarget& target_)
{
    multidrawer_bulletspeed.Draw(target_);
}
