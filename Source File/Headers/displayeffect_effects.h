#ifndef DISPLAYEFFECT_EFFECTS_H_INCLUDED
#define DISPLAYEFFECT_EFFECTS_H_INCLUDED

#include "displayeffect.h"
#include "AssetManager.h"
#include "bullet.h"
#include "public_functions.h"
#include "entitymanager.h"
#include "multidrawer.h"
#include <iostream>

//Shoot Effect
class DE_Shoot : public DisplayEffect
{
protected:
    float elaspedTime;
    sf::Sprite sprite;
    Entity* entity;
    sf::Vector2f refPos;
    MultiDrawer* multiDrawer;
    sf::Transformable transformation;
    sf::Vertex vertices[4];

    virtual void InitialiseMultiDrawer(short& type);

public:
    static void Draw(sf::RenderTarget& target_);

    DE_Shoot() :
        elaspedTime(0.f),
        entity(nullptr),
        multiDrawer(nullptr)
    {
    }

    DE_Shoot(sf::Vector2f position_, float angle_, float scale_, short type_, Entity* entity_) :
        elaspedTime(0.f),
        entity(entity_),
        multiDrawer(nullptr)
    {
        InitialiseMultiDrawer(type_);

        if(multiDrawer != nullptr && multiDrawer->GetTexture() != nullptr)
        {
            sf::Vector2f sizeTextr = (sf::Vector2f)multiDrawer->GetTexture()->getSize();

            vertices[0].texCoords = sf::Vector2f(0, 0);
            vertices[1].texCoords = sf::Vector2f(sizeTextr.x, 0);
            vertices[2].texCoords = sf::Vector2f(sizeTextr.x, sizeTextr.y);
            vertices[3].texCoords = sf::Vector2f(0, sizeTextr.y);

            transformation.setOrigin(sizeTextr.x / 2.f, sizeTextr.y);
            transformation.setScale(scale_, scale_);
            transformation.setPosition(position_);
            transformation.setRotation(angle_);

            if(entity_ != nullptr)
                refPos = position_ - entity_->GetPosition();
        }
    }

    void Update(float dt)
    {
        if(isActive)
        {
            elaspedTime += dt;
            if(elaspedTime >= REACTION_TIME)
                isActive = false;

            if(entity != nullptr)
                transformation.setPosition(entity->GetPosition() + refPos);

            if(multiDrawer != nullptr && multiDrawer->GetTexture() != nullptr)
            {
                sf::Vector2f sizeTextr = (sf::Vector2f)multiDrawer->GetTexture()->getSize();
                vertices[0].position = transformation.getTransform().transformPoint(sf::Vector2f(0, 0));
                vertices[1].position = transformation.getTransform().transformPoint(sf::Vector2f(sizeTextr.x, 0));
                vertices[2].position = transformation.getTransform().transformPoint(sf::Vector2f(sizeTextr.x, sizeTextr.y));
                vertices[3].position = transformation.getTransform().transformPoint(sf::Vector2f(0, sizeTextr.y));

                multiDrawer->AddVertex(vertices, 4);
            }
        }
    }

    void RefreshEntities()
    {
        if(entity != nullptr && entity->GetAlive() == false)
            entity = nullptr;
    }
};

//Hit Effect
class DE_Hit : public DE_Shoot
{
protected:
    void InitialiseMultiDrawer(short& type);

public:
    static void Draw(sf::RenderTarget& target_);

    DE_Hit(sf::Vector2f position_, float scale_, short type_, Entity* entity_)
    {
        entity = entity_;

        InitialiseMultiDrawer(type_);

        if(multiDrawer != nullptr && multiDrawer->GetTexture() != nullptr)
        {
            sf::Vector2f sizeTextr = (sf::Vector2f)multiDrawer->GetTexture()->getSize();

            vertices[0].texCoords = sf::Vector2f(0, 0);
            vertices[1].texCoords = sf::Vector2f(sizeTextr.x, 0);
            vertices[2].texCoords = sf::Vector2f(sizeTextr.x, sizeTextr.y);
            vertices[3].texCoords = sf::Vector2f(0, sizeTextr.y);

            transformation.setOrigin(sizeTextr / 2.f);
            transformation.setScale(scale_, scale_);
            transformation.setPosition(position_);
            transformation.setRotation(GenerateRandomNumber(360));

            if(entity_ != nullptr)
                refPos = position_ - entity_->GetPosition();
        }
    }
};

//Bullet Speed Effect
class DE_BulletSpeed : public DisplayEffect
{
protected:
    const float FADE_REACTION = 0.15f;

    sf::Vector2f refPos[8];
    sf::Vertex vertices[8];
    Bullet* bullet;
    float fadeTime;
    float fadeLength;
    float fadeEndColor;
    float velocityMagn;
    sf::Transform transformation;
    sf::FloatRect bulletRect;
    unsigned short colorTransparency;
    float bulletScale;
    bool skipFirst;
    MultiDrawer* multiDrawer;

    void InitialiseMultiDrawer();

public:
    static void Draw(sf::RenderTarget& target_);

    DE_BulletSpeed(Bullet* bullet_, sf::Color fadeColor_, float fadeLenght_) :
        bullet(bullet_),
        fadeTime(FADE_REACTION),
        fadeEndColor(0),
        fadeLength(fadeLenght_),
        velocityMagn(0),
        skipFirst(false),
        bulletScale(0),
        multiDrawer(nullptr)
    {
        InitialiseMultiDrawer();

        if(bullet_ != nullptr)
        {
            bulletRect = bullet_->GetLocalBoundingRect();
            bulletScale = bullet_->GetScale();
            colorTransparency = fadeColor_.a;
            transformation = bullet->GetTransform();

            for(int a = 0; a < 8; a++)
                vertices[a].color = fadeColor_;

            refPos[0] = sf::Vector2f(0, bulletRect.height);
            refPos[1] = sf::Vector2f(bulletRect.width, bulletRect.height);
            refPos[2] = sf::Vector2f(bulletRect.width, bulletRect.height);
            refPos[3] = sf::Vector2f(0, bulletRect.height);
            refPos[4] = sf::Vector2f(0, 0);
            refPos[5] = sf::Vector2f(bulletRect.width, 0);
            refPos[6] = sf::Vector2f(bulletRect.width, bulletRect.height);
            refPos[7] = sf::Vector2f(0, bulletRect.height);

            velocityMagn = bullet_->GetVelocityMagn();
        }
        else isActive = false;
    }

    void Update(float dt)
    {
        if(isActive && skipFirst)
        {
            if(bullet != nullptr)
            {
                transformation = bullet->GetTransform();

                float lengthY = refPos[2].y + velocityMagn / bulletScale * dt;
                if(lengthY > fadeLength / bulletScale + bulletRect.height)
                    lengthY = fadeLength / bulletScale + bulletRect.height;

                refPos[2].y = lengthY;
                refPos[3].y = lengthY;

                for(int a = 0; a < 8; a++)
                    vertices[a].position = transformation.transformPoint(refPos[a].x, refPos[a].y);
                vertices[2].color.a = vertices[3].color.a = colorTransparency * (1 - (lengthY / (fadeLength / bulletScale + bulletRect.height)));
                fadeEndColor = vertices[2].color.a;

                if(bullet->GetAlive() == false)
                    bullet = nullptr;
            }
            else
            {
                if(fadeTime > 0)
                {
                    fadeTime -= dt;
                    if(fadeTime < 0)
                        fadeTime = 0;

                    float lengthY = refPos[2].y - velocityMagn / bulletScale * dt;
                    if(lengthY < bulletRect.height)
                        lengthY = bulletRect.height;

                    refPos[2].y = lengthY;
                    refPos[3].y = lengthY;

                    vertices[0].color.a = vertices[1].color.a = colorTransparency * (fadeTime / FADE_REACTION);
                    vertices[2].color.a = vertices[3].color.a = fadeEndColor * (fadeTime / FADE_REACTION);
                    vertices[4].color.a = vertices[5].color.a = vertices[6].color.a = vertices[7].color.a = colorTransparency * (fadeTime / FADE_REACTION);

                    for(int a = 0; a < 8; a++)
                        vertices[a].position = transformation.transformPoint(refPos[a].x, refPos[a].y);
                }
                else isActive = false;
            }

            if(multiDrawer != nullptr)
                multiDrawer->AddVertex(vertices, 8);
        }

        if(skipFirst == false)
            skipFirst = true;
    }

    void RefreshEntities()
    {
        if(bullet != nullptr && bullet->GetAlive() == false)
            bullet = nullptr;
    }
};

//Spacial Radiation Effect
class DE_SpecialRadiation : public DisplayEffect
{
protected:
    static MultiDrawer multidrawer;
    static MultiDrawer multidrawer_star;

    float elaspedTime;
    float scale;
    sf::Vector2f position;
    ShipUnit* unit;
    sf::Vertex vertices[4];
    MultiDrawer* multidrawer_ptr;

public:

    DE_SpecialRadiation(ShipUnit* unit_, bool isStarShape_ = false) :
        elaspedTime(1.5f),
        scale(0),
        unit(unit_),
        multidrawer_ptr(nullptr)
    {
        if(isStarShape_)
        {
            multidrawer_ptr = &multidrawer_star;
            scale = 0.8f;

            if(multidrawer_star.GetTexture() == nullptr)
                multidrawer_star.SetTexture(&AssetManager::GetTexture("Shape_Star"));
        }
        else
        {
            multidrawer_ptr = &multidrawer;
            scale = 0.3f;

            if(multidrawer.GetTexture() == nullptr)
                multidrawer.SetTexture(&AssetManager::GetTexture("Shape_Circle"));
        }

        float sizeX = multidrawer_ptr->GetTexture()->getSize().x;
        vertices[0] = sf::Vertex(sf::Vector2f(), sf::Color::White, sf::Vector2f(0, 0));
        vertices[1] = sf::Vertex(sf::Vector2f(), sf::Color::White, sf::Vector2f(sizeX, 0));
        vertices[2] = sf::Vertex(sf::Vector2f(), sf::Color::White, sf::Vector2f(sizeX, sizeX));
        vertices[3] = sf::Vertex(sf::Vector2f(), sf::Color::White, sf::Vector2f(0, sizeX));
    }

    void Update(float dt)
    {
        if(isActive)
        {
            elaspedTime -= dt;

            if(elaspedTime > 0)
            {
                if(unit != nullptr)
                    position = unit->GetPosition();

                float factor = elaspedTime / 1.5f;
                float clrFactor = 1.f - (factor * factor);
                factor = 1.f - (factor * factor * factor * factor);
                float sizeX = multidrawer_ptr->GetTexture()->getSize().x * scale * 0.5f * factor;

                vertices[0].position = position + sf::Vector2f(-sizeX, -sizeX);
                vertices[1].position = position + sf::Vector2f(sizeX, -sizeX);
                vertices[2].position = position + sf::Vector2f(sizeX, sizeX);
                vertices[3].position = position + sf::Vector2f(-sizeX, sizeX);

                vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = sf::Color(255, 255, 255, 255.f * (1.f - clrFactor));

                multidrawer_ptr->AddVertex(vertices, 4);
            }
            else isActive = false;
        }
    }

    void RefreshEntities()
    {
        if(unit != nullptr && unit->GetAlive() == false)
            unit = nullptr;
    }

    static void Draw(sf::RenderTarget& target_)
    {
        multidrawer.Draw(target_);
        multidrawer_star.Draw(target_);
    }
};

//Damage Display Effect
class DE_DamageDisplay : public DisplayEffect
{
protected:
    static sf::IntRect textureRect[10];
    static MultiDrawer multidrawer;
    static const int ADD_DIST_SQR = 50 * 50;
    static const float SCALE;
    static const float DURATION;

    struct Object
    {
        float damage;
        float duration;
        sf::Vector2f position;
        std::vector<sf::Vertex> vertices;

        void PrepareVertices();
        void Process(int&, int&, float&);

        Object(const float& damage_, const sf::Vector2f& position_) :
            damage(damage_),
            duration(0),
            position(position_)
        {
            PrepareVertices();
        }
    };

    std::list<Object> objectList;

public:
    DE_DamageDisplay()
    {
        if(multidrawer.GetTexture() == nullptr)
            multidrawer.SetTexture(&AssetManager::GetTexture("DE_DamageDisplay_Numbers"));
    }

    static void Draw(sf::RenderTarget& target_)
    {
        multidrawer.Draw(target_);
    }

    void Add(float damage_, sf::Vector2f position_);
    void Update(float dt);
};

//DE_PulseSpreaderBullet
class DE_PulseSpreaderBullet : public DisplayEffect
{
public:
    static const float FADE_TIME;

protected:
    static MultiDrawer multidrawer;
    sf::Vertex vertices[4];
    float time = FADE_TIME;

public:
    DE_PulseSpreaderBullet(const sf::Transform& transform_, const float& length_, const float& width_);

    void Update(float dt)
    {
        if(time > 0)
        {
            time -= dt;
            if(time < 0)
                time = 0;
            vertices[0].color.a = vertices[1].color.a = vertices[2].color.a = vertices[3].color.a = time / FADE_TIME * 255.f;
            multidrawer.AddVertex(vertices, 4);
        }
        else isActive = false;
    }

    static void Draw(sf::RenderTarget& target_)
    {
        multidrawer.Draw(target_);
    }
};

#endif // DISPLAYEFFECT_EFFECTS_H_INCLUDED
