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
MultiDrawer DE_SpecialRadiation::multidrawer_star;
MultiDrawer DE_DamageDisplay::multidrawer;
MultiDrawer DE_PulseSpreaderBullet::multidrawer;

const float DE_PulseSpreaderBullet::FADE_TIME = 0.5f;

sf::IntRect DE_DamageDisplay::textureRect[10] =
{
    sf::IntRect(0, 0, 86, 76), sf::IntRect(89, 0, 30, 76), sf::IntRect(127, 0, 82, 76), sf::IntRect(216, 0, 79, 76), sf::IntRect(301, 0, 90, 76),
    sf::IntRect(396, 0, 81, 76), sf::IntRect(485, 0, 81, 76), sf::IntRect(568, 0, 84, 76), sf::IntRect(657, 0, 84, 76), sf::IntRect(749, 0, 82, 76)
};

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
    if(type == Bullet::LaserBullet || type == Bullet::DisintegratorBullet)
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
    if(type == Bullet::LaserBullet || type == Bullet::EnhancedLaserBullet || type == Bullet::DisintegratorBullet)
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

//DE_DamageDisplay
const float DE_DamageDisplay::SCALE = 0.2f;
const float DE_DamageDisplay::DURATION = 0.5f;

void DE_DamageDisplay::Object::Process(int& lastTenthMultiple, int& lastDamage, float& lastXRefPos)
{
    int digit = 0;

    if(lastDamage >= lastTenthMultiple)
    {
        lastTenthMultiple *= 10;
        Process(lastTenthMultiple, lastDamage, lastXRefPos);
    }

    lastTenthMultiple /= 10;
    digit = lastDamage / lastTenthMultiple;
    lastDamage -= digit * lastTenthMultiple;

    vertices.push_back(sf::Vertex(sf::Vector2f(lastXRefPos + position.x, position.y), sf::Color::White, sf::Vector2f(textureRect[digit].left, 0)));
    vertices.push_back(sf::Vertex(sf::Vector2f(lastXRefPos + position.x, position.y) + sf::Vector2f(textureRect[digit].width, 0) * SCALE, sf::Color::White, sf::Vector2f(textureRect[digit].left + textureRect[digit].width, 0)));
    vertices.push_back(sf::Vertex(sf::Vector2f(lastXRefPos + position.x, position.y) + sf::Vector2f(textureRect[digit].width, textureRect[digit].height) * SCALE, sf::Color::White, sf::Vector2f(textureRect[digit].left + textureRect[digit].width, textureRect[digit].height)));
    vertices.push_back(sf::Vertex(sf::Vector2f(lastXRefPos + position.x, position.y) + sf::Vector2f(0, textureRect[digit].height) * SCALE, sf::Color::White, sf::Vector2f(textureRect[digit].left, textureRect[digit].height)));

    lastXRefPos += textureRect[digit].width * SCALE;
    if(lastTenthMultiple >= 10)
        lastXRefPos += 10 * SCALE;
}

void DE_DamageDisplay::Object::PrepareVertices()
{
    int lastTenthMultiple = 10;
    int lastDamage = damage < 1 ? 1 : damage;
    float lastXRefPos = 0;

    vertices.clear();
    Process(lastTenthMultiple, lastDamage, lastXRefPos);

    lastXRefPos /= 2.f;
    for(sf::Vertex& vert : vertices)
        vert.position.x -= lastXRefPos;
}

void DE_DamageDisplay::Add(float damage_, sf::Vector2f position_)
{
    sf::Vector2f tempDist;
    bool createObj = true;

    for(auto& obj : objectList)
    {
        tempDist = obj.position - position_;
        if(MagnitudeSqureOfVector(tempDist) <= ADD_DIST_SQR)
        {
            obj.damage += damage_;

            if(obj.duration > 0.2f)
            {
                float tempFactor = obj.duration / DURATION;
                obj.position.y -= 30.f * tempFactor * tempFactor * tempFactor;

                obj.duration = 0.2f;
                tempFactor = obj.duration / DURATION;
                obj.position.y += 30.f * tempFactor * tempFactor * tempFactor;
            }

            obj.PrepareVertices();
            createObj = false;
            break;
        }
    }

    if(createObj)
        objectList.push_back(Object(damage_, position_));
}

void DE_DamageDisplay::Update(float dt)
{
    for(auto itr = objectList.begin(); itr != objectList.end();)
    {
        Object* obj = &*itr;
        obj->duration += dt;

        if(obj->duration > DURATION)
            itr = objectList.erase(itr);
        else
        {
            float factor = obj->duration / DURATION;

            float transFactor = 1.f;
            if(factor <= 0.2f)
            {
                transFactor = 1.f - factor / 0.2f;
                transFactor = 1.f - (transFactor * transFactor * transFactor);
            }
            else if(factor > 0.7f)
            {
                transFactor = (factor - 0.7f) / 0.3f;
                transFactor = 1.f - (transFactor * transFactor * transFactor);
            }

            factor = factor * factor * factor;
            for(int a = 0; a < obj->vertices.size(); a++)
            {
                obj->vertices[a].position.y = obj->position.y - factor * 30.f + (a % 4 > 1 ? textureRect[0].height : 0) * SCALE;
                obj->vertices[a].color.a = transFactor * 255;
            }

            multidrawer.AddVertex(obj->vertices);
            itr++;
        }
    }
}

DE_PulseSpreaderBullet::DE_PulseSpreaderBullet(const sf::Transform& transform_, const float& length_, const float& width_)
{
    vertices[0].position = transform_.transformPoint(sf::Vector2f(width_ / 2.f, 0));
    vertices[1].position = transform_.transformPoint(sf::Vector2f(-width_ / 2.f, 0));
    vertices[2].position = transform_.transformPoint(sf::Vector2f(-width_ / 2.f, -length_));
    vertices[3].position = transform_.transformPoint(sf::Vector2f(width_ / 2.f, -length_));
    vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = sf::Color(255, 255, 255, 0);
}
