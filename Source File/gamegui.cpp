#include "gamegui.h"
#include "game.h"
#include "public_functions.h"
#include <iostream>
#include <string>
#include <assert.h>

GUI::ResourcePack resourcePack_shipProfileButton;
const sf::Vector2f& guiAllowance = Game::GetGuiScreenEdgeALlowance();
sf::Texture* texture_iconStates;
sf::Texture* texture_iconBoosters;
sf::Texture* texture_teams;
sf::Texture* texture_noTeam;
GUI::Style style_shipProfileButton;
GUI::Style style_baseInfoButton;
GUI::Style style_energyCostButton;
GUI::Style* style_iconShapeButton;

sf::Color mapTeamRangeColor[] = {sf::Color(220, 130, 130, 255 * 0.8f), sf::Color(185, 193, 119, 255 * 0.8f), sf::Color(255, 219, 93, 255 * 0.8f), sf::Color(129, 158, 161, 255 * 0.8f)};
const float FocusedShipDisplay::GunLength[Gun::NumOfGuns] = {60, 70, 84 , 84, 84, 84};

void InitialiseGameGui()
{
    resourcePack_shipProfileButton.font = Game::GetFont("Conthrax");
    texture_iconStates = &AssetManager::GetTexture("Icon_States");
    texture_iconBoosters = &AssetManager::GetTexture("Icon_Boosters");
    texture_teams = &AssetManager::GetTexture("Team_Logo");
    texture_noTeam = &AssetManager::GetTexture("Icon_NoTeam");
    style_iconShapeButton = Game::GetGuiStyle("Level_IconShapeButton");

    //GUI Styles
    style_shipProfileButton.setPhase(GUI::InactivePhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_shipProfileButton.setPhase(GUI::LockedPhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 70);
    style_shipProfileButton.setPhase(GUI::PressedPhase, 0.9f, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(100, 125), sf::Vector2f(0, -12));
    style_shipProfileButton.setTextRef(sf::Vector2f(-45, -74));
    style_shipProfileButton.setTextOriginFactor(sf::Vector2f(0, 0));
    style_shipProfileButton.setFontParameters(15);
    style_shipProfileButton.setTransmissionFactor();

    style_baseInfoButton.copyFrom(*style_iconShapeButton);
    style_baseInfoButton.setTextureRect(sf::IntRect(0, 0, 38, 40));
    style_baseInfoButton.setPhase(GUI::PressedPhase, 0.9f, 0, sf::Vector2f(), sf::Vector2f(), 255);

    style_energyCostButton.setPhase(GUI::StartPhase, 0.5f, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_energyCostButton.setPhase(GUI::InactivePhase, 0.5f, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_energyCostButton.setPhase(GUI::DefaultPhase, 0.5f, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_energyCostButton.setPhase(GUI::PressedPhase, 0.5f, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_energyCostButton.setPhase(GUI::HighlighPhase, 0.5f, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_energyCostButton.setTextOriginFactor(sf::Vector2f(0, 0));
    style_energyCostButton.setFontParameters(16 * 2);
    style_energyCostButton.setTextRef(sf::Vector2f(-136 * 2, -11 * 2));
    style_energyCostButton.setTransmissionFactor();
}

//ShipProfileButton
ShipProfileButton::ShipProfileButton(ShipIdentity* identity_, sf::Vector2f position_, std::string message_) :
    Button(&style_shipProfileButton, position_, 1, resourcePack_shipProfileButton, message_, identity_ != nullptr ? identity_->GetShipInfo()->name : "", identity_ != nullptr ? &identity_->GetShipInfo()->profileTexture : nullptr, 0.81967f, sf::IntRect(), 0.f, 0.f),
    identity(identity_),
    transparencyInfo(0.f),
    showInfo(true),
    profileTexture(nullptr),
    lastSpecialState(false),
    lastLevel(1)
{
    sprite.setTextureRect(sf::IntRect(0, 0, 100, 125));
    sprite.setOrigin(50, 75);

    profileVertex[0] = sf::Vertex(position + sf::Vector2f(-50, -50), sf::Color::White, sf::Vector2f(4, 90));
    profileVertex[1] = sf::Vertex(position + sf::Vector2f(50, -50), sf::Color::White, sf::Vector2f(90, 4));
    profileVertex[2] = sf::Vertex(position + sf::Vector2f(50, 50), sf::Color::White, sf::Vector2f(176, 90));
    profileVertex[3] = sf::Vertex(position + sf::Vector2f(-50, 50), sf::Color::White, sf::Vector2f(90, 176));
    if(identity != nullptr)
        profileTexture = &Game::GetSkin((Game::SkinType)identity->GetShipInfo()->skinType, identity->GetShipInfo()->skinIndex, identity->GetShipInfo()->team)->texture;

    stateIcon.setTexture(*texture_iconStates);
    stateIcon.setOrigin(30, 30);

    if(style != nullptr)
    {
        text.setCharacterSize(style->getFontSize() * 2.f);
        if(identity != nullptr)
            text.setString(identity_->GetShipInfo()->name);
        ShortenText(text, 140.f);
        text.setOrigin(text.getLocalBounds().width * style->getTextOriginFactor().x, text.getLocalBounds().height * style->getTextOriginFactor().y);

        textPrim.setFont(*resourcePack_shipProfileButton.font);
        textSec.setFont(*resourcePack_shipProfileButton.font);

        textPrim.setCharacterSize(25 * 2);
        textSec.setCharacterSize(14 * 2);

        textPrim.setScale(0.5f, 0.5f);
        textSec.setScale(0.5f, 0.5f);

        textSec.setString("sec");
        ShortenText(textSec, 60.f, true);
        textSec.setOrigin(0, textSec.getLocalBounds().height / 2.f);
    }

    textLevel.setFont(*resourcePack_shipProfileButton.font);
    if(style != nullptr)
        textLevel.setCharacterSize(style->getFontSize() * 2);
    if(identity_ != nullptr && identity_->GetShipEntity() != nullptr)
        textLevel.setString(ConvertIntToString(identity_->GetUnitLevel()));
    textLevel.setOrigin(textLevel.getGlobalBounds().width / 2.f, 0);
}

ShipProfileButton::~ShipProfileButton()
{
}

void ShipProfileButton::SetShipIdentity(ShipIdentity* identity_)
{
    identity = identity_;
    if(identity != nullptr)
        profileTexture = &Game::GetSkin((Game::SkinType)identity->GetShipInfo()->skinType, identity->GetShipInfo()->skinIndex, identity->GetShipInfo()->team)->texture;

    if(identity_ != nullptr && identity_->GetShipEntity() != nullptr)
    {
        textLevel.setString(ConvertIntToString(identity_->GetUnitLevel()));
        text.setString(identity_->GetShipInfo()->name);
        ShortenText(text, 140.f);

        lastSpecialState = identity_->GetShipEntity()->IsSpecialActive();
        profileVertex[0].texCoords = sf::Vector2f(4 + (lastSpecialState ? 180 : 0), 90);
        profileVertex[1].texCoords = sf::Vector2f(90 + (lastSpecialState ? 180 : 0), 4);
        profileVertex[2].texCoords = sf::Vector2f(176 + (lastSpecialState ? 180 : 0), 90);
        profileVertex[3].texCoords = sf::Vector2f(90 + (lastSpecialState ? 180 : 0), 176);
    }
    textLevel.setOrigin(textLevel.getLocalBounds().width / 2.f, 0);
    text.setOrigin(text.getLocalBounds().width * style->getTextOriginFactor().x, text.getLocalBounds().height * style->getTextOriginFactor().y);
}

ShipIdentity* ShipProfileButton::GetShipIdentity()
{
    return identity;
}

bool ShipProfileButton::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool prevCursorOnObject = GUI::IsCursorOnObject();
    bool return_ = GUI::Button::HandleInput(mousePos_, leftClick_, deltaTime, doInput);

    if(showInfo)
        TendTowards(transparencyInfo, 255, 0.1, 0.1, deltaTime);
    else
    {
        TendTowards(transparencyInfo, 0, 0.1, 0.1, deltaTime);
        GUI::SetCursorOnObjectState(prevCursorOnObject);
    }

    return return_;
}

void ShipProfileButton::Draw(sf::RenderTarget& target_)
{
    if(style != nullptr && identity != nullptr && sprite.getColor().a > 0)
    {
        sf::Vector2f scl = sprite.getScale();

        profileVertex[0].position = sprite.getPosition() + sf::Vector2f(-50 * scl.x, -50 * scl.y);
        profileVertex[1].position = sprite.getPosition() + sf::Vector2f(50 * scl.x, -50 * scl.y);
        profileVertex[2].position = sprite.getPosition() + sf::Vector2f(50 * scl.x, 50 * scl.y);
        profileVertex[3].position = sprite.getPosition() + sf::Vector2f(-50 * scl.x, 50 * scl.y);
        for(int b = 0; b < 4; b++)
            profileVertex[b].color = sprite.getColor();

        sf::Color mClr = Game::GetCurrentRankThemeColor();
        sf::VertexArray vertices;
        vertices.setPrimitiveType(sf::Quads);
        vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-50, -50) * scl.x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f)));
        vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(50, -50) * scl.x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f)));
        vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(50, 50) * scl.x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f)));
        vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-50, 50) * scl.x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f)));

        target_.draw(vertices);
        target_.draw(profileVertex, 4, sf::Quads, profileTexture);

        if(transparencyInfo > 0)
        {
            text.setScale(scl * 0.5f);
            text.setPosition(sprite.getPosition() + style->getTextRef(currentPhase) * sprite.getScale().x);
            text.setColor(sf::Color(255, 255, 255, (float)sprite.getColor().a * transparencyInfo / 255.f));

            textLevel.setScale(scl * 0.5f);
            textLevel.setPosition(sprite.getPosition() + sf::Vector2f(38, -74) * sprite.getScale().x);
            textLevel.setColor(sf::Color(255, 255, 255, (float)sprite.getColor().a * transparencyInfo / 255.f));

            bool isUnitAlive = identity->GetShipEntity() != nullptr && identity->GetShipEntity()->GetAlive();
            if(isUnitAlive == false)
            {
                textPrim.setString(ConvertIntToString((int)identity->GetSpawnTime()));
                ShortenText(textPrim, 100.f, true);
                textPrim.setOrigin(textPrim.getLocalBounds().width / 2.f, textPrim.getLocalBounds().height / 2.f);
                textPrim.setPosition(sprite.getPosition() + sf::Vector2f(0, -20 * scl.y));
                textPrim.setScale(scl * 0.5f);
                textPrim.setColor(sf::Color(255, 255, 255, (float)sprite.getColor().a * transparencyInfo / 255.f));

                textSec.setPosition(sprite.getPosition() + sf::Vector2f(-5 * scl.x, 1 * scl.y));
                textSec.setScale(scl * 0.5f);
                textSec.setColor(sf::Color(255, 255, 255, (float)sprite.getColor().a * transparencyInfo / 255.f));

                shadowVertex[0].position = sprite.getPosition() + sf::Vector2f(-50 * scl.x, -50 * scl.y);
                shadowVertex[1].position = sprite.getPosition() + sf::Vector2f(50 * scl.x, -50 * scl.y);
                shadowVertex[2].position = sprite.getPosition() + sf::Vector2f(50 * scl.x, 50 * scl.y);
                shadowVertex[3].position = sprite.getPosition() + sf::Vector2f(-50 * scl.x, 50 * scl.y);
                shadowVertex[0].color = shadowVertex[1].color = shadowVertex[2].color = shadowVertex[3].color = sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f * transparencyInfo / 255.f);
            }

            vertices.clear();
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-50, -75) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(30, -75) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(30, -55) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-50, -55) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f * transparencyInfo / 255.f)));

            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(30, -75) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(50, -75) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(50, -55) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(30, -55) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));

            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-50, 30) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(30, 30) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(30, 50) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-50, 50) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * 0.7f * transparencyInfo / 255.f)));

            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(30, 30) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(50, 30) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(50, 50) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(30, 50) * sprite.getScale().x, sf::Color(mClr.r, mClr.g, mClr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));

            ShipUnit* ship = identity->GetShipEntity();
            float healthFactor = isUnitAlive ? (ship->IsSpecialActive() ? ship->GetSpecialInFactors() : ship->GetHealthInFactors()) : identity->GetSpawnTimeFactor(true);
            sf::Color clr = isUnitAlive ? (healthFactor <= 0.25 && ship->IsSpecialActive() == false? sf::Color(203, 101, 101) : sf::Color::White) : ChangeColorHue(mClr, -20);

            if(isUnitAlive && ship->IsSpecialActive())
            {
                vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-45, 35) * sprite.getScale().x, sf::Color(255, 255, 255, 0.5f * (float)sprite.getColor().a * transparencyInfo / 255.f)));
                vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-45 + 70 * ship->GetHealthInFactors(), 35) * sprite.getScale().x, sf::Color(clr.r, clr.g, clr.b, 0.5f * (float)sprite.getColor().a * transparencyInfo / 255.f)));
                vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-45 + 70 * ship->GetHealthInFactors(), 45) * sprite.getScale().x, sf::Color(clr.r, clr.g, clr.b, 0.5f * (float)sprite.getColor().a * transparencyInfo / 255.f)));
                vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-45, 45) * sprite.getScale().x, sf::Color(255, 255, 255, 0.5f * (float)sprite.getColor().a * transparencyInfo / 255.f)));
            }

            if(isUnitAlive && ship->IsSpecialActive() != lastSpecialState)
            {
                lastSpecialState = !lastSpecialState;
                profileVertex[0].texCoords = sf::Vector2f(4 + (lastSpecialState ? 180 : 0), 90);
                profileVertex[1].texCoords = sf::Vector2f(90 + (lastSpecialState ? 180 : 0), 4);
                profileVertex[2].texCoords = sf::Vector2f(176 + (lastSpecialState ? 180 : 0), 90);
                profileVertex[3].texCoords = sf::Vector2f(90 + (lastSpecialState ? 180 : 0), 176);
            }

            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-45, 35) * sprite.getScale().x, sf::Color(clr.r, clr.g, clr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-45 + 70 * healthFactor, 35) * sprite.getScale().x, sf::Color(clr.r, clr.g, clr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-45 + 70 * healthFactor, 45) * sprite.getScale().x, sf::Color(clr.r, clr.g, clr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));
            vertices.append(sf::Vertex(sprite.getPosition() + sf::Vector2f(-45, 45) * sprite.getScale().x, sf::Color(clr.r, clr.g, clr.b, (float)sprite.getColor().a * transparencyInfo / 255.f)));


            if(ship != nullptr && ship != ShipUnit::GetActivePlayerUnit())
            {
                if(ship->GetState() == ShipUnit::Attacking)
                stateIcon.setTextureRect(sf::IntRect(60, 0, 60, 60));
                else if(ship->GetState() == ShipUnit::Defending)
                    stateIcon.setTextureRect(sf::IntRect(120, 0, 60, 60));
                else stateIcon.setTextureRect(sf::IntRect(540, 0, 60, 60));
            }
            else stateIcon.setTextureRect(sf::IntRect(540, 0, 60, 60));

            stateIcon.setPosition(sprite.getPosition() + sf::Vector2f(40, 40) * sprite.getScale().x);
            stateIcon.setScale(scl * 0.33333f);
            stateIcon.setColor(sf::Color(255, 255, 255, (float)sprite.getColor().a * transparencyInfo / 255.f));

            if(lastLevel != identity->GetUnitLevel())
            {
                textLevel.setString(ConvertIntToString(identity->GetUnitLevel()));
                textLevel.setOrigin(textLevel.getLocalBounds().width / 2.f, 0);
            }

            if(isUnitAlive == false)
                target_.draw(shadowVertex, 4, sf::Quads);
            target_.draw(vertices);
            target_.draw(text);
            target_.draw(textLevel);
            target_.draw(stateIcon);
            if(isUnitAlive == false)
            {
                target_.draw(textPrim);
                target_.draw(textSec);
            }
        }
    }
}

void ShipProfileButton::SetShowInfo(bool status_)
{
    showInfo = status_;
}

//FadeDegree
FadeDegree::FadeDegree(GUI::Direction dir_, float length_, float width_, sf::Vector2f pos_, sf::Vector2f nodeSize_, float hideSize_, short hideSide_, sf::Color lineColr_, sf::Color nodeColr,
                       sf::Color extentColr_, StateType state_, float startDelay_, float endDelay_, float transmitFactor_, float* rateRef_) :
    GUI::Degree(dir_, length_, width_, pos_, nodeSize_, lineColr_, nodeColr, extentColr_, state_, startDelay_, endDelay_, transmitFactor_, rateRef_),
    lastActiveTime(0.f),
    fadeFactor(0.f),
    hideSide(hideSide_ > 1 ? 1 : (hideSide_ < -1 ? -1 : hideSide_)),
    hideSize(hideSize_),
    nodeSize(nodeSize_),
    isNodeRectangle(true),
    hideFactor(0),
    hideTime(0),
    hideDelay(0)
{
    if(dir_ == GUI::Vertical)
    {
        rect.width = width_;
        rect.height = length_;

        hideRect.height = length_;
        hideRect.width = width_ > nodeSize.x ? width_ : nodeSize.x;
    }
    else
    {
        rect.width = length_;
        rect.height = width_;

        hideRect.width = length_;
        hideRect.height = width_ > nodeSize.y ? width_ : nodeSize.y;
    }

    if(nodeSize_.x > rect.width)
        rect.width = nodeSize_.x;
    if(nodeSize_.y > rect.height)
        rect.height = nodeSize_.y;

    rect.left = pos_.x - rect.width / 2.f;
    rect.top = pos_.y - rect.height / 2.f;

    hideRect.left = pos_.x - hideRect.width / 2.f;
    hideRect.top = pos_.y - hideRect.height / 2.f;
}

FadeDegree::FadeDegree(GUI::Direction dir_, float length_, float width_, sf::Vector2f pos_, float nodeRadius_, float hideSize_, short hideSide_, sf::Color lineColr_, sf::Color nodeColr,
                       sf::Color extentColr_, StateType state_, float startDelay_, float endDelay_, float transmitFactor_, float* rateRef_) :
    GUI::Degree(dir_, length_, width_, pos_, nodeRadius_, lineColr_, nodeColr, extentColr_, state_, startDelay_, endDelay_, transmitFactor_, rateRef_),
    lastActiveTime(0.f),
    fadeFactor(0.f),
    hideSide(hideSide_ > 1 ? 1 : (hideSide_ < -1 ? -1 : hideSide_)),
    hideSize(hideSize_),
    nodeSize(nodeRadius_, 0),
    isNodeRectangle(false),
    hideFactor(0),
    hideTime(0),
    hideDelay(0)
{
    if(dir_ == GUI::Vertical)
    {
        rect.width = width_;
        rect.height = length_;

        hideRect.height = length_;
        hideRect.width = width_ > nodeRadius_ ? width_ : nodeRadius_;
    }
    else
    {
        rect.width = length_;
        rect.height = width_;

        hideRect.width = length_;
        hideRect.height = width_ > nodeRadius_ ? width_ : nodeRadius_;
    }

    if(nodeRadius_ > rect.width)
        rect.width = nodeRadius_;
    if(nodeRadius_ > rect.height)
        rect.height = nodeRadius_;

    rect.left = pos_.x - rect.width / 2.f;
    rect.top = pos_.y - rect.height / 2.f;

    hideRect.left = pos_.x - hideRect.width / 2.f;
    hideRect.top = pos_.y - hideRect.height / 2.f;
}

FadeDegree::FadeDegree(GUI::Direction dir_, sf::FloatRect rect_, float nodeSizeFactor_, float hideSize_, short hideSide_, sf::Color lineColr_, sf::Color nodeColr_,
                       sf::Color extentColr_, StateType state_, float startDelay_, float endDelay_, float transmitFactor_, float* rateRef_) :
    GUI::Degree(dir_, dir_ == GUI::Vertical ? rect_.height : rect_.width, dir_ == GUI::Vertical ? rect_.width : rect_.height, sf::Vector2f(rect_.left + rect_.width / 2.f, rect_.top + rect_.height / 2.f), dir_ == GUI::Vertical ? sf::Vector2f(rect_.width, rect_.height * nodeSizeFactor_) : sf::Vector2f(rect_.width * nodeSizeFactor_, rect_.height),
                lineColr_, nodeColr_, extentColr_, state_, startDelay_, endDelay_, transmitFactor_, rateRef_),
    lastActiveTime(0.f),
    fadeFactor(0.f),
    rect(rect_),
    nodeSize(rect_.width * (dir_ == GUI::Horizontal ? nodeSizeFactor_ : 1), rect_.height * (dir_ == GUI::Vertical ? nodeSizeFactor_ : 1)),
    hideSide(hideSide_ > 1 ? 1 : (hideSide_ < -1 ? -1 : hideSide_)),
    hideSize(hideSize_),
    isNodeRectangle(true),
    hideFactor(0),
    hideRect(rect),
    hideTime(0),
    hideDelay(0)
{
}

bool FadeDegree::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool prvCursoronObject = GUI::IsCursorOnObject();
    bool return_ = GUI::Degree::HandleInput(mousePos_, leftClick_, deltaTime, doInput);

    if(isInActive)
        GUI::SetCursorOnObjectState(prvCursoronObject);

    lastActiveTime -= deltaTime;
    if(rect.intersects(sf::FloatRect(mousePos_.x, mousePos_.y, 1, 1)) || return_)
        lastActiveTime = 5.f;

    float fade = 0.f;
    if(lastActiveTime > 0.f)
        fade = lastActiveTime <= 1.f ? lastActiveTime / 1.f : 1.f;
    TendTowards(fadeFactor, fade, transmissionFactor, 0.1f, deltaTime);

    if(hideRect.contains((sf::Vector2f)mousePos_) || active)
        hideDelay = 1.f;
    if(hideDelay > 0)
    {
        hideDelay -= deltaTime;
        hideTime += deltaTime;
        if(hideTime > 0.15f)
            hideTime = 0.15f;
    }
    else
    {
        hideTime -= deltaTime;
        if(hideTime < 0)
            hideTime = 0;
    }
    hideFactor = 1.f - (sin(22.f / 7.f * (1.5f + hideTime / 0.15f)) + 1) / 2.f;

    return return_;
}

void FadeDegree::Draw(sf::RenderTarget& target_)
{
    lineExtent.setFillColor(sf::Color(extentColor.r, extentColor.g, extentColor.b, fadeFactor * transparencyFactor * extentColor.a));
    line.setFillColor(sf::Color(lineColor.r, lineColor.g, lineColor.b, fadeFactor * transparencyFactor * lineColor.a));

    sf::Vector2f prvPos;
    sf::Vector2f prvSize;
    if(node != nullptr)
    {
        if(hideSize > 0)
        {
            prvPos = node->getPosition();

            if(isNodeRectangle)
            {
                sf::RectangleShape* tempNode = static_cast<sf::RectangleShape*>(node);
                prvSize = tempNode->getSize();
                sf::Vector2f tempDirSide = direction == GUI::Vertical ? sf::Vector2f(1, 0) : sf::Vector2f(0, 1);
                tempNode->setSize(nodeSize - tempDirSide * hideSize * hideFactor);
                tempNode->setPosition(prvPos + tempDirSide * hideSize * (hideSide + 1.f) * 0.5f * hideFactor);
            }
            else
            {
                sf::CircleShape* tempNode = static_cast<sf::CircleShape*>(node);
                prvSize.x = tempNode->getRadius();
                sf::Vector2f tempDirSide = direction == GUI::Vertical ? sf::Vector2f(1, 0) : sf::Vector2f(0, 1);
                tempNode->setRadius(nodeSize.x - hideSize * hideFactor);
                tempNode->setPosition(prvPos + tempDirSide * hideSize * (hideSide + 1.f) * 0.5f * hideFactor);
            }
        }
        node->setFillColor(sf::Color(nodeColor.r, nodeColor.g, nodeColor.b, fadeFactor * transparencyFactor * nodeColor.a));
    }

    target_.draw(line);
    target_.draw(lineExtent);
    if(node != nullptr)
    {
        target_.draw(*node);
        if(hideSize > 0)
        {
            node->setPosition(prvPos);
            if(isNodeRectangle)
                static_cast<sf::RectangleShape*>(node)->setSize(prvSize);
            else static_cast<sf::CircleShape*>(node)->setRadius(prvSize.x);
        }
    }
}

void FadeDegree::SetRatio(float ratio_)
{
    float prvRatio = GetRatio();
    GUI::Degree::SetRatio(ratio_);

    if(GetRatio() != prvRatio)
        lastActiveTime = 5.f;
}

//FocusedShipDisplay
FocusedShipDisplay::FocusedShipDisplay() :
    healthLength(0),
    healthFactor(0),
    damageFactor(0),
    reloadTransparency(0),
    reloadFadeTime(0),
    reloadFadeTimeTransit(0),
    reloadFadeTransparency(0),
    lastGun(-1),
    lastSpecialState(false),
    identity(nullptr),
    texture_guns(nullptr),
    texture_skin(nullptr)
{
    startPhase.position = defaultPhase.position = inactivePhase.position = sf::Vector2f(60, Game::SCREEN_HEIGHT - 60);
    startPhase.scale = defaultPhase.scale = inactivePhase.scale = sf::Vector2f(1.f, 1.f);
    startPhase.transparency  = inactivePhase.transparency = 0;
    defaultPhase.transparency = 255.f;
    transmissionFactor = 0.1f;

    text.setFont(*resourcePack_shipProfileButton.font);
    text.setCharacterSize(18 * 2);
    text.setScale(0.5f, 0.5f);
    text_bullet = text_bullet_ammo = text_level = text_surUnits = text;

    texture_guns = &AssetManager::GetTexture("Guns");
    texture_icon = &AssetManager::GetTexture("Icon_States");

    vertices.setPrimitiveType(sf::Quads);
    vertices_profile.setPrimitiveType(sf::Quads);
    vertices_gun.setPrimitiveType(sf::Quads);
    vertices_icon.setPrimitiveType(sf::Quads);

    //Profile
    vertices_profile.append(sf::Vertex(sf::Vector2f(guiAllowance.x, Game::SCREEN_HEIGHT - 100 - guiAllowance.y), sf::Color::White, sf::Vector2f(4, 90)));
    vertices_profile.append(sf::Vertex(sf::Vector2f(guiAllowance.x + 100, Game::SCREEN_HEIGHT - 100 - guiAllowance.y), sf::Color::White, sf::Vector2f(90, 4)));
    vertices_profile.append(sf::Vertex(sf::Vector2f(guiAllowance.x + 100, Game::SCREEN_HEIGHT - guiAllowance.y), sf::Color::White, sf::Vector2f(176, 90)));
    vertices_profile.append(sf::Vertex(sf::Vector2f(guiAllowance.x, Game::SCREEN_HEIGHT - guiAllowance.y), sf::Color::White, sf::Vector2f(90, 176)));

    //Gun
    vertices_gun.resize(4);

    //Icon
    sf::Vector2f refPos(guiAllowance.x + 105, Game::SCREEN_HEIGHT - 100 - guiAllowance.y);

    vertices_icon.append(sf::Vertex(refPos + sf::Vector2f(0, 35), sf::Color::White, sf::Vector2f(360, 0)));
    vertices_icon.append(sf::Vertex(refPos + sf::Vector2f(60, 35), sf::Color::White, sf::Vector2f(360 + 60, 0)));
    vertices_icon.append(sf::Vertex(refPos + sf::Vector2f(60, 65), sf::Color::White, sf::Vector2f(360 + 60, 60)));
    vertices_icon.append(sf::Vertex(refPos + sf::Vector2f(0, 65), sf::Color::White, sf::Vector2f(360, 60)));

    //Others
    float widthLimit = (Game::SCREEN_WIDTH / 2.f - 120.f) * 0.9f;
    healthLength = widthLimit - 14.f;

    vertices.resize(4 * 3);
    vertices.append(sf::Vertex(refPos + sf::Vector2f(0, 35)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(60, 35)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(60, 65)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(0, 65)));

    vertices.append(sf::Vertex(refPos + sf::Vector2f(60, 35)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(120, 35)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(120, 65)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(60, 65)));

    vertices.append(sf::Vertex(refPos + sf::Vector2f(0, 70)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(widthLimit, 70)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(widthLimit, 100)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(0, 100)));

    vertices.append(sf::Vertex(refPos + sf::Vector2f(7, 77)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(7, 77)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(7, 93)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(7, 93)));

    vertices.append(sf::Vertex(refPos + sf::Vector2f(7, 77)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(7, 77)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(7, 93)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(7, 93)));

    vertices.append(sf::Vertex(refPos + sf::Vector2f(-105, 0)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(-5, 0)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(-5, 100)));
    vertices.append(sf::Vertex(refPos + sf::Vector2f(-105, 100)));

    for(int c = 0; c < 3; c++)
    {
        vertices.append(sf::Vertex(refPos + sf::Vector2f(0, 35)));
        vertices.append(sf::Vertex(refPos + sf::Vector2f(60, 35)));
        vertices.append(sf::Vertex(refPos + sf::Vector2f(60, 65)));
        vertices.append(sf::Vertex(refPos + sf::Vector2f(0, 65)));
    }

    text_bullet.setPosition(refPos + sf::Vector2f(90, 38));
    text_bullet_ammo.setPosition(refPos + sf::Vector2f(90, 41));
    text_bullet_ammo.setScale(0.4f, 0.4f);
}

void FocusedShipDisplay::SetShipIdentity(ShipIdentity* identity_)
{
    identity = identity_;

    float nameBarLength = text.getGlobalBounds().width + 35;
    sf::Vector2f refPos(guiAllowance.x + 105, Game::SCREEN_HEIGHT - 100 - guiAllowance.y);

    if(identity_ != nullptr && identity_->GetShipEntity() != nullptr)
    {
        texture_skin = &Game::GetSkin((Game::SkinType)identity_->GetShipInfo()->skinType, identity_->GetShipInfo()->skinIndex, identity_->GetShipInfo()->team)->texture;

        text_level.setString(ConvertIntToString(identity_->GetShipEntity()->GetCurrentLevel()));
        text.setString(identity_->GetShipInfo()->name);

        nameBarLength = text.getGlobalBounds().width + 35;
        float widthLimit = (Game::SCREEN_WIDTH / 2.f - 120.f) * 0.9f;
        if(text.getGlobalBounds().width + 65 > widthLimit)
            ShortenText(text, widthLimit - 65, true);

        vertices[10].position = refPos + sf::Vector2f(nameBarLength + 32, 30);
        vertices[11].position = refPos + sf::Vector2f(nameBarLength + 30, 30);
        vertices[8].position = vertices[11].position + sf::Vector2f(0, -30 * identity_->GetUnitExpFactor());
        vertices[9].position = vertices[10].position + sf::Vector2f(0, -30 * identity_->GetUnitExpFactor());

        lastSpecialState = identity_->GetShipEntity()->IsSpecialActive();

        vertices_profile[0].texCoords = sf::Vector2f(4 + (lastSpecialState ? 180 : 0), 90);
        vertices_profile[1].texCoords = sf::Vector2f(90 + (lastSpecialState ? 180 : 0), 4);
        vertices_profile[2].texCoords = sf::Vector2f(176 + (lastSpecialState ? 180 : 0), 90);
        vertices_profile[3].texCoords = sf::Vector2f(90 + (lastSpecialState ? 180 : 0), 176);
    }
    text_level.setOrigin(text_level.getGlobalBounds().width / 2.f, 0);

    text.setPosition(refPos + sf::Vector2f(10, 3));
    text_level.setPosition(refPos + sf::Vector2f(nameBarLength + 15.f, 3));

    vertices[0].position = refPos + sf::Vector2f();
    vertices[1].position = refPos + sf::Vector2f(nameBarLength, 0);
    vertices[2].position = refPos + sf::Vector2f(nameBarLength, 30);
    vertices[3].position = refPos + sf::Vector2f(0, 30);

    vertices[4].position = refPos + sf::Vector2f(nameBarLength, 0);
    vertices[5].position = refPos + sf::Vector2f(nameBarLength + 30, 0);
    vertices[6].position = refPos + sf::Vector2f(nameBarLength + 30, 30);
    vertices[7].position = refPos + sf::Vector2f(nameBarLength, 30);
}

ShipIdentity* FocusedShipDisplay::GetShipIdentity()
{
    Reset();
    return identity;
}

void FocusedShipDisplay::Update(float deltaTime_)
{
    GUI::DisplayObject::Update(deltaTime_);

    Ship* ship = nullptr;
    ShipUnit* shipUnit = nullptr;
    if(identity != nullptr)
    {
        ship = identity->GetShipEntity();
        shipUnit = static_cast<ShipUnit*>(ship);
    }

    if(ship == nullptr)
    {
        identity = nullptr;
        SetInactive();
    }
    else if(state == GUI::InactivePhase)
    {
        Reset();
        healthFactor = 0.f;
        damageFactor = 0.f;
    }

    sf::Color clr = Game::GetCurrentRankThemeColor();

    if(ship != nullptr && ship->GetAlive())
    {
        text_level.setString(ConvertIntToString(ship->GetCurrentLevel()));

        if(ship->GetShipType() == Ship::UnitShip && static_cast<ShipUnit*>(ship)->IsSpecialActive() != lastSpecialState)
        {
            lastSpecialState = !lastSpecialState;
            if(lastSpecialState)
            {
                damageFactor = healthFactor;
                healthFactor = 0;
            }
            else
            {
                healthFactor = damageFactor;
                damageFactor = 0;
            }
            vertices_profile[0].texCoords = sf::Vector2f(4 + (lastSpecialState ? 180 : 0), 90);
            vertices_profile[1].texCoords = sf::Vector2f(90 + (lastSpecialState ? 180 : 0), 4);
            vertices_profile[2].texCoords = sf::Vector2f(176 + (lastSpecialState ? 180 : 0), 90);
            vertices_profile[3].texCoords = sf::Vector2f(90 + (lastSpecialState ? 180 : 0), 176);
        }
        float healthFactorReal = lastSpecialState ? ship->GetSpecialInFactors() : ship->GetHealthInFactors();
        TendTowards(healthFactor, healthFactorReal, 0.1f, 0.0001f, deltaTime_);

        if(lastSpecialState)
            TendTowards(damageFactor, ship->GetHealthInFactors(), 0.1f, 0.0001f, deltaTime_);
        else if(damageFactor > healthFactor)
            TendTowards(damageFactor, healthFactor, 0.4f, 0.0001f, deltaTime_);
        else if(healthFactor > damageFactor)
            damageFactor = healthFactor;

        sf::Vector2f refPos(guiAllowance.x + 105, Game::SCREEN_HEIGHT - 100 - guiAllowance.y);
        if(ship->GetCurrentGun() != lastGun)
        {
            lastGun = ship->GetCurrentGun();
            if(lastGun >= 0 && lastGun < Gun::NumOfGuns)
            {
                vertices[12].position = refPos + sf::Vector2f(0, 35);
                vertices[13].position = refPos + sf::Vector2f(GunLength[lastGun], 35);
                vertices[14].position = refPos + sf::Vector2f(GunLength[lastGun], 65);
                vertices[15].position = refPos + sf::Vector2f(0, 65);

                vertices[16].position = refPos + sf::Vector2f(GunLength[lastGun], 35);
                vertices[17].position = refPos + sf::Vector2f(GunLength[lastGun] + 105, 35);
                vertices[18].position = refPos + sf::Vector2f(GunLength[lastGun] + 105, 65);
                vertices[19].position = refPos + sf::Vector2f(GunLength[lastGun], 65);

                vertices[40].position = refPos + sf::Vector2f(GunLength[lastGun] + 110, 35);
                vertices[41].position = refPos + sf::Vector2f(GunLength[lastGun] + 150, 35);
                vertices[42].position = refPos + sf::Vector2f(GunLength[lastGun] + 150, 65);
                vertices[43].position = refPos + sf::Vector2f(GunLength[lastGun] + 110, 65);

                vertices[44].position = refPos + sf::Vector2f(GunLength[lastGun] + 150, 35);
                vertices[45].position = refPos + sf::Vector2f(GunLength[lastGun] + 205, 35);
                vertices[46].position = refPos + sf::Vector2f(GunLength[lastGun] + 205, 65);
                vertices[47].position = refPos + sf::Vector2f(GunLength[lastGun] + 150, 65);

                vertices_icon[0].position = refPos + sf::Vector2f(GunLength[lastGun] + 115, 35);
                vertices_icon[1].position = refPos + sf::Vector2f(GunLength[lastGun] + 145, 35);
                vertices_icon[2].position = refPos + sf::Vector2f(GunLength[lastGun] + 145, 65);
                vertices_icon[3].position = refPos + sf::Vector2f(GunLength[lastGun] + 115, 65);

                vertices[37].position.x = refPos.x + GunLength[lastGun];
                vertices[38].position.x = refPos.x + GunLength[lastGun];

                vertices_gun[0].texCoords = sf::Vector2f(0, 100 * lastGun);
                vertices_gun[1].texCoords = sf::Vector2f(280, 100 * lastGun);
                vertices_gun[2].texCoords = sf::Vector2f(280, 100 * lastGun + 100);
                vertices_gun[3].texCoords = sf::Vector2f(0, 100 * lastGun + 100);

                float xOffset = (int)((GunLength[lastGun] - 84.f) / 2.f);
                vertices_gun[0].position = refPos + sf::Vector2f(xOffset, 35);
                vertices_gun[1].position = refPos + sf::Vector2f(xOffset + 84, 35);
                vertices_gun[2].position = refPos + sf::Vector2f(xOffset + 84, 65);
                vertices_gun[3].position = refPos + sf::Vector2f(xOffset, 65);
            }
        }

        if(ship->IsReloading() && lastGun >= 0 && lastGun < Gun::NumOfGuns)
        {
            float factor = ship->GetReloadingFactor();
            float yFactor = factor / 0.9f;
            if(yFactor > 1)
                yFactor = 1.f;

            vertices[37].position.y = vertices[38].position.y - 30 * yFactor;
            vertices[36].position.y = vertices[39].position.y - 30 * yFactor;
            reloadFadeTransparency = reloadTransparency = 30 + 140 * (sin(22.f / 7.f * (1.5f + factor * factor * factor)) + 1) / 2.f;
            reloadFadeTimeTransit = reloadFadeTime = factor * 0.5f;
        }
        else if(reloadFadeTimeTransit > 0)
        {
            float factor = reloadFadeTimeTransit / reloadFadeTime;
            reloadTransparency = (sin(22.f / 7.f * (1.5f + factor)) + 1) / 2.f * reloadFadeTransparency;

            reloadFadeTimeTransit -= deltaTime_;
            if(reloadFadeTransparency <= 0)
                reloadFadeTransparency = 0;
        }

        vertices[8].position = vertices[11].position + sf::Vector2f(0, -30 * identity->GetUnitExpFactor());
        vertices[9].position = vertices[10].position + sf::Vector2f(0, -30 * identity->GetUnitExpFactor());

        text_bullet.setPosition(refPos + sf::Vector2f(GunLength[lastGun] + 105 / 2, 38));
        text_bullet.setString(ConvertIntToString(ship->GetGunMagazine()) + '/');
        text_surUnits.setPosition(refPos + sf::Vector2f(GunLength[lastGun] + 150 + 55 / 2.f, 38));
        text_surUnits.setString(ConvertIntToString(shipUnit->GetSurroundingTroops()));
        if(lastGun != Gun::LaserGun && ship->GetGunAmmo() != nullptr)
        {
            text_bullet_ammo.setString(ConvertIntToString(*ship->GetGunAmmo()));
            text_bullet_ammo.setPosition(refPos + sf::Vector2f(GunLength[lastGun] + 105 / 2.f, 41));
            text_bullet_ammo.setScale(0.4f, 0.4f);
        }
        else
        {
            text_bullet_ammo.setString(static_cast<wchar_t>(8734)); //infinity code
            text_bullet_ammo.setPosition(refPos + sf::Vector2f(GunLength[lastGun] + 105 / 2.f, 36));
            text_bullet_ammo.setScale(0.6f, 0.6f);
        }
    }
    else
    {
        TendTowards(healthFactor, 0, 0.1f, 0.0001f, deltaTime_);
        TendTowards(damageFactor, 0, 0.1f, 0.0001f, deltaTime_);

        if(reloadFadeTimeTransit > 0)
        {
            float factor = reloadFadeTimeTransit / reloadFadeTime;
            reloadTransparency = (sin(22.f / 7.f * (1.5f + factor)) + 1) / 2.f * reloadFadeTransparency;

            reloadFadeTimeTransit -= deltaTime_;
            if(reloadFadeTransparency <= 0)
                reloadFadeTransparency = 0;
        }
    }

    float barLength = healthLength * healthFactor;

    vertices[25].position.x = vertices[24].position.x + healthLength * damageFactor;
    vertices[26].position.x = vertices[25].position.x;

    vertices[29].position.x = vertices[28].position.x + barLength;
    vertices[30].position.x = vertices[29].position.x;

    for(int a = 0; a < 4; a++)
    {
        vertices[a].color = sf::Color(clr.r, clr.g, clr.b, transparency * 0.7f);
        vertices[a + 3 * 4].color = sf::Color(clr.r, clr.g, clr.b, transparency * 0.7f);
        vertices[a + 5 * 4].color = sf::Color(clr.r, clr.g, clr.b, transparency * 0.7f);
        vertices[a + 8 * 4].color = sf::Color(clr.r, clr.g, clr.b, transparency * 0.7f);
        vertices[a + 10 * 4].color = sf::Color(clr.r, clr.g, clr.b, transparency * 0.7f);
        vertices[a + 11 * 4].color = text_surUnits.getString() == "0" ? sf::Color(207, 117, 117, transparency) : sf::Color(clr.r, clr.g, clr.b, transparency);
        vertices[a + 1 * 4].color = sf::Color(clr.r, clr.g, clr.b, transparency);

        vertices[a + 4 * 4].color = ship != nullptr && ship->GetCurrentGun() != Gun::LaserGun && ship->GetGunMagazine() <= 0 && (ship->GetGunAmmo() != nullptr ? *ship->GetGunAmmo() < 1 : true) ? sf::Color(207, 117, 117, transparency) : sf::Color(clr.r, clr.g, clr.b, transparency);

        vertices[a + 2 * 4].color = sf::Color(255, 255, 255, transparency);
        vertices[a + 9 * 4].color = sf::Color(255, 255, 255, reloadTransparency * transparency / 255.f);

        if(ship != nullptr)
        {
            if(lastSpecialState)
            {
                vertices[a + 7 * 4].color = sf::Color(255, 255, 255, transparency);
                vertices[a + 6 * 4].color = sf::Color(255, 255, 255, 0.5f * transparency);
            }
            else if(ship->GetHealthInFactors() <= 0.25)
            {
                vertices[a + 7 * 4].color = sf::Color(207, 101, 101, transparency);
                vertices[a + 6 * 4].color = sf::Color(207, 101, 101, 0.5f * transparency);
            }
            else
            {
                vertices[a + 7 * 4].color = sf::Color(255, 255, 255, transparency);
                vertices[a + 6 * 4].color = sf::Color(clr.r, clr.g, clr.b, transparency);
            }
        }
        else
        {
            vertices[a + 7 * 4].color.a = transparency;
            vertices[a + 6 * 4].color.a = transparency * (lastSpecialState ? 0.5f : 1.f);
        }

        vertices_profile[a].color = sf::Color(255, 255, 255, transparency);
        vertices_gun[a].color = sf::Color(255, 255, 255, transparency);
        vertices_icon[a].color = sf::Color(255, 255, 255, transparency);
    }

    text.setColor(sf::Color(255, 255, 255, transparency));
    text_level.setColor(sf::Color(255, 255, 255, transparency));
    text_bullet.setColor(sf::Color(255, 255, 255, transparency));
    text_surUnits.setColor(sf::Color(255, 255, 255, transparency));
    text_bullet_ammo.setColor(sf::Color(255, 255, 255, 125 * transparency / 255.f));

    text_level.setOrigin(text_level.getLocalBounds().width / 2.f, 0);
    text_bullet.setOrigin(text_bullet.getLocalBounds().width, 0);
    text_surUnits.setOrigin(text_surUnits.getLocalBounds().width / 2.f, 0);
}

void FocusedShipDisplay::Draw(sf::RenderTarget& target_)
{
    target_.draw(vertices);
    target_.draw(vertices_profile, texture_skin);
    target_.draw(vertices_gun, texture_guns);
    target_.draw(vertices_icon, texture_icon);
    target_.draw(text);
    target_.draw(text_level);
    target_.draw(text_bullet);
    target_.draw(text_bullet_ammo);
    target_.draw(text_surUnits);
}

//FocusedShipButton
FocusedShipButton::FocusedShipButton(sf::Color clr) :
    GUI::ShapeButton(style_iconShapeButton, sf::Vector2f(30, 30), clr, clr, sf::Color(clr.r + 30 < clr.r ? 255 : clr.r + 30, clr.g + 30 < clr.g ? 255 : clr.g + 30, clr.b + 30 < clr.b ? 255 : clr.b + 30), false, sf::Vector2f(guiAllowance.x + 306, guiAllowance.y + 40 + 155), resourcePack_shipProfileButton, "FocusShipButton_Close", "", &AssetManager::GetTexture("Icon_States"), 0.5f, sf::IntRect(480, 0, 60, 60)),
    button_defend(style_iconShapeButton, sf::Vector2f(30, 30), clr, clr, sf::Color(clr.r + 30 < clr.r ? 255 : clr.r + 30, clr.g + 30 < clr.g ? 255 : clr.g + 30, clr.b + 30 < clr.b ? 255 : clr.b + 30), false, sf::Vector2f(guiAllowance.x + 15, guiAllowance.y + 40 + 155), resourcePack_shipProfileButton, "FocusShipButton_Defend", "", &AssetManager::GetTexture("Icon_States"), 0.5f, sf::IntRect(120, 0, 60, 60)),
    button_attack(style_iconShapeButton, sf::Vector2f(30, 30), clr, clr, sf::Color(clr.r + 30 < clr.r ? 255 : clr.r + 30, clr.g + 30 < clr.g ? 255 : clr.g + 30, clr.b + 30 < clr.b ? 255 : clr.b + 30), false, sf::Vector2f(guiAllowance.x + 50, guiAllowance.y + 40 + 155), resourcePack_shipProfileButton, "FocusShipButton_Attack", "", &AssetManager::GetTexture("Icon_States"), 0.5f, sf::IntRect(60, 0, 60, 60)),
    button_cancel(style_iconShapeButton, sf::Vector2f(30, 30), clr, clr, sf::Color(clr.r + 30 < clr.r ? 255 : clr.r + 30, clr.g + 30 < clr.g ? 255 : clr.g + 30, clr.b + 30 < clr.b ? 255 : clr.b + 30), false, sf::Vector2f(guiAllowance.x + 85, guiAllowance.y + 40 + 155), resourcePack_shipProfileButton, "FocusShipButton_Cancel", "", &AssetManager::GetTexture("Icon_States"), 0.5f, sf::IntRect(540, 0, 60, 60)),
    button_focus(style_iconShapeButton, sf::Vector2f(30, 30), clr, clr, sf::Color(clr.r + 30 < clr.r ? 255 : clr.r + 30, clr.g + 30 < clr.g ? 255 : clr.g + 30, clr.b + 30 < clr.b ? 255 : clr.b + 30), false, sf::Vector2f(guiAllowance.x + 120, guiAllowance.y + 40 + 155), resourcePack_shipProfileButton, "FocusShipButton_Focus", "", &AssetManager::GetTexture("Icon_States"), 0.5f, sf::IntRect(600, 0, 60, 60)),
    button_think(style_iconShapeButton, style_iconShapeButton, sf::Vector2f(30, 30), clr, clr, sf::Color(clr.r + 30 < clr.r ? 255 : clr.r + 30, clr.g + 30 < clr.g ? 255 : clr.g + 30, clr.b + 30 < clr.b ? 255 : clr.b + 30), false, sf::Vector2f(guiAllowance.x + 155, guiAllowance.y + 40 + 155), resourcePack_shipProfileButton, "FocusShipButton_Think_On", "FocusShipButton_Think_Off", "", &AssetManager::GetTexture("Icon_States"), 0.5f, sf::IntRect(660, 0, 60, 60)),
    button_details(style_iconShapeButton, sf::Vector2f(30, 30), clr, clr, sf::Color(clr.r + 30 < clr.r ? 255 : clr.r + 30, clr.g + 30 < clr.g ? 255 : clr.g + 30, clr.b + 30 < clr.b ? 255 : clr.b + 30), false, sf::Vector2f(guiAllowance.x + 190, guiAllowance.y + 40 + 155), resourcePack_shipProfileButton, "FocusShipButton_Details", "", &AssetManager::GetTexture("Icon_States"),0.5f, sf::IntRect(720, 0, 60, 60)),
    isReviving(false),
    va_icons_size(0),
    specialFactor(0),
    specialFactorSmooth(0),
    specialIndex(0),
    healthFactor(0),
    healthFactorSmooth(0),
    healthIndex(-1),
    texture_icons(nullptr),
    texture_skin(nullptr)
{
    va_bars.setPrimitiveType(sf::Quads);
    va_icons.setPrimitiveType(sf::Quads);
    va_profile.setPrimitiveType(sf::Quads);

    va_profile.append(sf::Vertex(sf::Vector2f(guiAllowance.x, guiAllowance.y + 40)));
    va_profile.append(sf::Vertex(sf::Vector2f(guiAllowance.x + 100, guiAllowance.y + 40)));
    va_profile.append(sf::Vertex(sf::Vector2f(guiAllowance.x + 100, guiAllowance.y + 140)));
    va_profile.append(sf::Vertex(sf::Vector2f(guiAllowance.x, guiAllowance.y + 140)));

    texture_icons = &AssetManager::GetTexture("Icon_States");

    text_name.setFont(*resourcePack_shipProfileButton.font);
    text_name.setCharacterSize(18 * 2);
    text_name.setScale(0.5f, 0.5f);
    text_level = text_surUnits = text_stateDesc = text_prim = text_sec = text_name;
    text_prim.setCharacterSize(25 * 2);
    text_sec.setCharacterSize(14 * 2);

    UpdateInfo();
}

void FocusedShipButton::SetThinkButtonStatus(bool status_)
{
    button_think.SetStatus(status_);
    if(button_think.GetStatus())
        button_think.SetIcon(texture_icons, sf::IntRect(660, 0, 60, 60), 0.5f);
    else button_think.SetIcon(texture_icons, sf::IntRect(840, 0, 60, 60), 0.5f);
}

void FocusedShipButton::UpdateInfo(sf::Texture* textureSkin_, bool isTextureSkin_, std::string name_, int level_, float expRatio_, float healthRatio_, float specialRatio_, short moral_, int surUnits_, int state_, std::string stateDesc_,
                                   bool actionOptions_, bool focusOption_, bool thinkOption_, bool detailsOption_, float reviveSeconds_,  std::string primText_, std::string secText_)
{
    sf::Vector2f refPos(guiAllowance.x, guiAllowance.y + 40);
    sf::Color clr = Game::GetCurrentRankThemeColor();
    isReviving = reviveSeconds_ > 0.f;

    if(textureSkin_ != nullptr)
    {
        texture_skin = textureSkin_;

        if(isTextureSkin_)
        {
            float xOffset = specialRatio_ > 0 ? 180 : 0;
            va_profile[0].texCoords = sf::Vector2f(4 + xOffset, 90);
            va_profile[1].texCoords = sf::Vector2f(90 + xOffset, 4);
            va_profile[2].texCoords = sf::Vector2f(176 + xOffset, 90);
            va_profile[3].texCoords = sf::Vector2f(90 + xOffset, 176);
        }
        else
        {
            sf::Vector2u textrSize = textureSkin_->getSize();
            va_profile[0].texCoords = sf::Vector2f(0, 0);
            va_profile[1].texCoords = sf::Vector2f(textrSize.x, 0);
            va_profile[2].texCoords = sf::Vector2f(textrSize.x, textrSize.y);
            va_profile[3].texCoords = sf::Vector2f(0, textrSize.y);
        }
    }

    va_icons.clear();
    va_bars.clear();
    barColorPerQuad.clear();
    va_icons_size = 0;
    healthFactor = healthRatio_;
    specialFactor = specialRatio_;
    healthIndex = specialIndex = -1;

    AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x, refPos.y, 100, 100));
    barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.7f * 255));
    if(name_ != "")
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 105, refPos.y, 216, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.7f * 255));

        text_name.setString(name_);
        ShortenText(text_name, 165, true);
        text_name.setPosition(sf::Vector2f(115, 3) + refPos);
    }
    else
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 105, refPos.y, 216, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.2f * 255));

        text_name.setString("");
    }

    if(level_ >= 0)
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 291, refPos.y, 30, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 255));

        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 323, refPos.y + 30, -2, -30 * expRatio_));
        barColorPerQuad.push_back(sf::Color::White);

        text_level.setString(ConvertIntToString(level_));
        ShortenText(text_level, 165, true);
        text_level.setOrigin(text_level.getLocalBounds().width / 2.f, 0);
        text_level.setPosition(sf::Vector2f(306, 3) + refPos);
    }
    else text_level.setString("");

    if(healthRatio_ >= 0 && healthRatio_ <= 1.f)
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 105, refPos.y + 35, 216, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.7f * 255));

        if(isReviving)
            healthFactorSmooth = healthFactor;
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 112, refPos.y + 42, 202 * healthFactorSmooth, 16));
        if(specialRatio_ > 0)
            barColorPerQuad.push_back(sf::Color(255, 255, 255, 255.f * 0.5f));
        else if(isReviving)
            barColorPerQuad.push_back(clr);
        else if(healthRatio_ <= 0.25)
            barColorPerQuad.push_back(sf::Color(203, 101, 101));
        else barColorPerQuad.push_back(sf::Color::White);
        healthIndex = (barColorPerQuad.size() - 1) * 4;

        if(specialRatio_ >= 0 && specialRatio_ <= 1.f)
        {
            AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 112, refPos.y + 42, 202 * specialFactorSmooth, 16));
            barColorPerQuad.push_back(sf::Color::White);
            specialIndex = (barColorPerQuad.size() - 1) * 4;
        }
    }
    else
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 105, refPos.y + 35, 216, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.2f * 255));
    }

    AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 105, refPos.y + 70, 30, 30));
    if(moral_ >= 0)
    {
        barColorPerQuad.push_back(clr);
        AddRectVerticesToArray(va_icons, sf::FloatRect(refPos.x + 105, refPos.y + 70, 30, 30), sf::Color::White, 45 * (4 - (moral_ % 5)), sf::IntRect(780, 0, 60, 60));
        va_icons_size += 4;
    }
    else barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.2f * 255));

    if(surUnits_ >= 0)
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 140, refPos.y + 70, 64, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.7f * 255));

        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 204, refPos.y + 70, 30, 30));
        barColorPerQuad.push_back(clr);

        text_surUnits.setString(ConvertIntToString(surUnits_));
        text_surUnits.setOrigin(text_surUnits.getLocalBounds().width / 2.f, 0);
        text_surUnits.setPosition(sf::Vector2f(172, 73) + refPos);

        AddRectVerticesToArray(va_icons, sf::FloatRect(refPos.x + 204, refPos.y + 70, 30, 30), sf::Color::White, 0, sf::IntRect(360, 0, 60, 60));
        va_icons_size += 4;
    }
    else
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 140, refPos.y + 70, 94, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.2f * 255));

        text_surUnits.setString("");
    }

    if(state_ >= 0)
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x, refPos.y + 105, 291, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.7f * 255));

        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 291, refPos.y + 105, 30, 30));
        barColorPerQuad.push_back(clr);

        text_stateDesc.setString(stateDesc_);
        ShortenText(text_stateDesc, 265, true);
        text_stateDesc.setPosition(sf::Vector2f(15, 108) + refPos);

        if(state_ == ShipUnit::Attacking)
            AddRectVerticesToArray(va_icons, sf::FloatRect(refPos.x + 291, refPos.y + 105, 30, 30), sf::Color::White, 0, sf::IntRect(60, 0, 60, 60));
        else if(state_ == ShipUnit::Defending)
            AddRectVerticesToArray(va_icons, sf::FloatRect(refPos.x + 291, refPos.y + 105, 30, 30), sf::Color::White, 0, sf::IntRect(120, 0, 60, 60));
        else AddRectVerticesToArray(va_icons, sf::FloatRect(refPos.x + 291, refPos.y + 105, 30, 30), sf::Color::White, 0, sf::IntRect(540, 0, 60, 60));
        va_icons_size += 4;
    }
    else
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x, refPos.y + 105, 321, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.2f * 255));

        text_stateDesc.setString("");
    }

    if(primText_ != "")
    {
        text_prim.setString(primText_);
        ShortenText(text_prim, 100.f, true);
        text_prim.setOrigin(text_prim.getLocalBounds().width / 2.f, text_prim.getLocalBounds().height / 2.f);
        text_prim.setPosition(refPos + sf::Vector2f(50, 40));
    }
    else text_prim.setString("");

    if(secText_ != "")
    {
        text_sec.setString(secText_);
        ShortenText(text_sec, 60.f, true);
        text_sec.setOrigin(0, text_sec.getLocalBounds().height / 2.f);
        text_sec.setPosition(refPos + sf::Vector2f(45, 61));
    }
    else text_sec.setString("");

    //Buttons
    button_attack.SetLock(!actionOptions_);
    button_defend.SetLock(!actionOptions_);
    button_cancel.SetLock(!actionOptions_);
    if(actionOptions_ == false)
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x, refPos.y + 140, 30, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.2f * 255));

        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 35, refPos.y + 140, 30, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.2f * 255));

        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 70, refPos.y + 140, 30, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.2f * 255));
    }

    button_focus.SetLock(!focusOption_);
    button_think.SetLock(!thinkOption_);
    button_details.SetLock(!detailsOption_);
    if(focusOption_ == false)
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 105, refPos.y + 140, 30, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.2f * 255));
    }
    if(thinkOption_ == false)
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 140, refPos.y + 140, 30, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.2f * 255));
    }
    if(detailsOption_ == false)
    {
        AddRectVerticesToArray(va_bars, sf::FloatRect(refPos.x + 175, refPos.y + 140, 30, 30));
        barColorPerQuad.push_back(sf::Color(clr.r, clr.g, clr.b, 0.2f * 255));
    }
}

void FocusedShipButton::UpdateInfo()
{
    UpdateInfo(nullptr, false, "", -1, -1, -1, -1, -1, -1, -1, "", false, false, false, false, -1, "", "");
}

bool FocusedShipButton::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    message__ = "";
    if(GUI::ShapeButton::HandleInput(mousePos_, leftClick_, deltaTime, doInput))
        message__ = message;
    if(button_defend.HandleInput(mousePos_, leftClick_, deltaTime, doInput))
        message__ = button_defend.GetMessage();
    if(button_attack.HandleInput(mousePos_, leftClick_, deltaTime, doInput))
        message__ = button_attack.GetMessage();
    if(button_cancel.HandleInput(mousePos_, leftClick_, deltaTime, doInput))
        message__ = button_cancel.GetMessage();
    if(button_focus.HandleInput(mousePos_, leftClick_, deltaTime, doInput))
        message__ = button_focus.GetMessage();
    if(button_think.HandleInput(mousePos_, leftClick_, deltaTime, doInput))
    {
        message__ = button_think.GetMessage();

        if(button_think.GetStatus())
            button_think.SetIcon(texture_icons, sf::IntRect(660, 0, 60, 60), 0.5f);
        else button_think.SetIcon(texture_icons, sf::IntRect(840, 0, 60, 60), 0.5f);
    }
    if(button_details.HandleInput(mousePos_, leftClick_, deltaTime, doInput))
        message__ = button_details.GetMessage();

    TendTowards(healthFactorSmooth, healthFactor, 0.1f, 0.0001f, deltaTime);
    TendTowards(specialFactorSmooth, specialFactor, 0.1f, 0.0001f, deltaTime);
    if(healthIndex >= 0)
    {
        if(healthFactorSmooth < 0)
            healthFactorSmooth = 0;
        else if(healthFactorSmooth > 1)
            healthFactorSmooth = 1;

        va_bars[healthIndex + 1].position.x = va_bars[healthIndex].position.x + 202 * healthFactorSmooth;
        va_bars[healthIndex + 2].position.x = va_bars[healthIndex].position.x + 202 * healthFactorSmooth;
    }

    if(specialIndex >= 0)
    {
        if(specialFactorSmooth < 0)
            specialFactorSmooth = 0;
        else if(specialFactorSmooth > 1)
            specialFactorSmooth = 1;

        va_bars[specialIndex + 1].position.x = va_bars[specialIndex].position.x + 202 * specialFactorSmooth;
        va_bars[specialIndex + 2].position.x = va_bars[specialIndex].position.x + 202 * specialFactorSmooth;
    }

    return message__ != "";
}

void FocusedShipButton::Draw(sf::RenderTarget& target_)
{
    int transparency = sprite.getColor().a;
    if(transparency > 0)
    {
        for(int a = 0; a < barColorPerQuad.size() * 4; a++)
            va_bars[a].color = sf::Color(barColorPerQuad[a / 4].r, barColorPerQuad[a / 4].g, barColorPerQuad[a / 4].b, barColorPerQuad[a / 4].a * ((float)transparency / 255.f));
        for(int a = 0; a < va_icons_size; a++)
            va_icons[a].color = sf::Color(255, 255, 255, transparency);
        for(int a = 0; a < 4; a++)
            va_profile[a].color = sf::Color(255, 255, 255, transparency);

        text_level.setFillColor(sf::Color(255, 255, 255, transparency));
        text_name.setFillColor(sf::Color(255, 255, 255, transparency));
        text_stateDesc.setFillColor(sf::Color(255, 255, 255, transparency));
        text_surUnits.setFillColor(sf::Color(255, 255, 255, transparency));
        text_prim.setFillColor(sf::Color(255, 255, 255, transparency));
        text_sec.setFillColor(sf::Color(255, 255, 255, transparency));


        bool drawProfileFirst = isReviving || text_prim.getString() != "" || text_sec.getString() != "";
        if(drawProfileFirst && texture_skin != nullptr)
            target_.draw(va_profile, texture_skin);
        target_.draw(va_bars);
        if(drawProfileFirst == false && texture_skin != nullptr)
            target_.draw(va_profile, texture_skin);
        target_.draw(va_icons, texture_icons);
        target_.draw(text_name);
        target_.draw(text_level);
        target_.draw(text_surUnits);
        target_.draw(text_stateDesc);
        target_.draw(text_prim);
        target_.draw(text_sec);
        button_defend.Draw(target_);
        button_attack.Draw(target_);
        button_cancel.Draw(target_);
        button_focus.Draw(target_);
        button_think.Draw(target_);
        button_details.Draw(target_);
        GUI::ShapeButton::Draw(target_);
    }
}

std::string FocusedShipButton::GetMessage()
{
    return message__;
}

void FocusedShipButton::Reset(std::string command)
{
    GUI::ShapeButton::Reset(command);
    button_defend.Reset(command);
    button_attack.Reset(command);
    button_cancel.Reset(command);
    button_focus.Reset(command);
    button_think.Reset(command);
    button_details.Reset(command);
    healthFactor = 0.f;
}

void FocusedShipButton::SetInactive()
{
    GUI::ShapeButton::SetInactive();
    button_defend.SetInactive();
    button_attack.SetInactive();
    button_cancel.SetInactive();
    button_focus.SetInactive();
    button_think.SetInactive();
    button_details.SetInactive();
    healthFactor = specialFactor = 0.f;
}

//BaseInfoButton
BaseInfoButton::BaseInfoButton() :
    GUI::ShapeButton(&style_baseInfoButton, sf::Vector2f(38, 40), sf::Color::Transparent, sf::Color::Transparent, sf::Color::Transparent, false,
                     sf::Vector2f(Game::SCREEN_WIDTH / 2.f, guiAllowance.y + 57), resourcePack_shipProfileButton, "BaseInfoOptions"),
    optionButton(sf::Vector2f(Game::SCREEN_WIDTH / 2.f, guiAllowance.y + 134)),
    showOption(false),
    showButton(false),
    showUnits(false),
    optionTrans(0),
    textTrans(0)
{
    info_icon.setTexture(*texture_iconStates);
    info_icon.setTextureRect(sf::IntRect(0, 60, 60, 60));
    info_icon.setOrigin(info_icon.getLocalBounds().width / 2.f, info_icon.getLocalBounds().height / 2.f);
    info_icon.setScale(0.5f, 0.5f);
    info_icon.setColor(sf::Color::Transparent);

    info_text.setFont(*resourcePack_shipProfileButton.font);
    info_text.setCharacterSize(25);
    info_text.setString("No available option");
    info_text.setOrigin(info_text.getLocalBounds().width / 2.f, info_text.getLocalBounds().height / 2.f);
    info_text.setScale(0.5f, 0.5f);
    info_text.setPosition(Game::SCREEN_WIDTH / 2.f, guiAllowance.y + 107);
    info_text.setFillColor(sf::Color::Transparent);

    info_text_units = info_text;
    info_text_units.setCharacterSize(18 * 2);
    info_text_units.setOrigin(info_text_units.getLocalBounds().width / 2.f, 0);
    info_text_units.setString("");
    info_text_units.setPosition(Game::SCREEN_WIDTH / 2.f + 15, guiAllowance.y + 3);

    sf::Color clr = Game::GetCurrentRankThemeColor();
    info_va.setPrimitiveType(sf::Quads);
    AddRectVerticesToArray(info_va, sf::FloatRect(Game::SCREEN_WIDTH / 2.f - 47, guiAllowance.y, 94, 30), sf::Color(clr.r, clr.g, clr.b, 0));
    AddRectVerticesToArray(info_va, sf::FloatRect(Game::SCREEN_WIDTH / 2.f - 47, guiAllowance.y, 30, 30), sf::Color(clr.r, clr.g, clr.b, 0));
    AddRectVerticesToArray(info_va, sf::FloatRect(Game::SCREEN_WIDTH / 2.f - 47, guiAllowance.y + 30, 94, 2), sf::Color(255, 255, 255, 0));
    sf::Vector2f arrowRefPos(Game::SCREEN_WIDTH / 2.f - 10, guiAllowance.y + 84);
    info_va.append(sf::Vertex(arrowRefPos + sf::Vector2f(0, 0), sf::Color(255, 255, 255, 0)));
    info_va.append(sf::Vertex(arrowRefPos + sf::Vector2f(5, 0), sf::Color(255, 255, 255, 0)));
    info_va.append(sf::Vertex(arrowRefPos + sf::Vector2f(10, 5), sf::Color(255, 255, 255, 0)));
    info_va.append(sf::Vertex(arrowRefPos + sf::Vector2f(10, 10), sf::Color(255, 255, 255, 0)));
    info_va.append(sf::Vertex(arrowRefPos + sf::Vector2f(10, 10), sf::Color(255, 255, 255, 0)));
    info_va.append(sf::Vertex(arrowRefPos + sf::Vector2f(10, 5), sf::Color(255, 255, 255, 0)));
    info_va.append(sf::Vertex(arrowRefPos + sf::Vector2f(15, 0), sf::Color(255, 255, 255, 0)));
    info_va.append(sf::Vertex(arrowRefPos + sf::Vector2f(20, 0), sf::Color(255, 255, 255, 0)));
}

void BaseInfoButton::SetShowOptionStatus(bool status_)
{
    showOption = status_;
}

bool BaseInfoButton::GetShowOptionStatus()
{
    return showOption;
}

void BaseInfoButton::UpdateInfo(short team_, int numOfUnits_, int cost_, short baseId_)
{
    if(team_ >= 0 && team_ < Game::NumOfTeams)
    {
        SetIcon(texture_teams, sf::IntRect(0, 200 * team_, 188, 200), 40.f / 200.f);
    }
    else
    {
        SetIcon(texture_noTeam, sf::IntRect(94, 0, 94, 100), 40.f / 100.f);
    }

    if(baseId_ >= 0 && showOption)
    {
        Base* base = Game::GetBase(baseId_);
        int numOfInactiveDefences = base->GetNumOfInactiveDefences();

        if(optionButton.IsInactive())
            optionButton.Reset("default-phase-reset");

        if(base->IsActive() == false)
            optionButton.UpdateInfo(base->GetTeam() == Game::GetPlayerTeam() ? "Activate Base" : "Claim Base", "Activate_Base", cost_);
        else if(base->GetTeam() == Game::GetPlayerTeam() && numOfInactiveDefences > 0)
            optionButton.UpdateInfo("Rebuild Defences", "Rebuild_Defnces", cost_ * numOfInactiveDefences);
        else optionButton.SetInactive();
    }
    else optionButton.SetInactive();

    UpdateInfo(numOfUnits_);
}

void BaseInfoButton::UpdateInfo(int numOfUnits_)
{
    if(currentPhase == GUI::InactivePhase)
        currentPhase = GUI::DefaultPhase;

    if(numOfUnits_ >= 0)
    {
        info_icon.setTextureRect(sf::IntRect(360, 0, 60, 60));
        info_icon.setPosition(Game::SCREEN_WIDTH / 2.f - 32, guiAllowance.y + 15);

        info_text_units.setString(ConvertIntToString(numOfUnits_));
        info_text_units.setOrigin(info_text_units.getLocalBounds().width / 2.f, 0);
    }
    else
    {
        info_icon.setTextureRect(sf::IntRect(0, 60, 60, 60));
        info_icon.setPosition(Game::SCREEN_WIDTH / 2.f, guiAllowance.y + 15);

        info_text_units.setString("");
    }
    showUnits = numOfUnits_ >= 0;
}

bool BaseInfoButton::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
   message__ = "";

   if(GUI::ShapeButton::HandleInput(mousePos_, leftClick_, deltaTime, doInput))
        message__ = message;
   if(optionButton.HandleInput(mousePos_, leftClick_, deltaTime, doInput))
        message__ = optionButton.GetMessage();

   TendTowards(optionTrans, showOption ? 255 : 0, 0.1f, 0.00001f, deltaTime);
   TendTowards(textTrans, optionButton.IsInactive() && showOption ? 255.f : 0, 0.1f, 0.00001f, deltaTime);
   float trans = sprite.getColor().a;
   for(int a = 0; a < 4; a++)
   {
       info_va[a].color.a = trans * 0.7f;
       info_va[4 + a].color.a = showUnits ? trans : 0;
       info_va[8 + a].color.a = trans;
       info_va[12 + a].color.a = trans / 255.f * optionTrans;
       info_va[16 + a].color.a = trans / 255.f * optionTrans;
   }

    sf::Color clr = sprite.getColor();
    info_icon.setColor(clr);
    info_text_units.setFillColor(clr);
    info_text.setFillColor(sf::Color(clr.r, clr.g, clr.b, (float)clr.a / 255.f * textTrans));

    return message__ != "";
}

std::string BaseInfoButton::GetMessage()
{
    return message__;
}

void BaseInfoButton::Reset(std::string command)
{
    GUI::ShapeButton::Reset(command);
}

void BaseInfoButton::SetInactive()
{
    GUI::ShapeButton::SetInactive();
    optionButton.SetInactive();
    showOption = false;
}

void BaseInfoButton::Draw(sf::RenderTarget& target_)
{
    target_.draw(info_va);
    target_.draw(info_icon);
    target_.draw(info_text);
    target_.draw(info_text_units);
    GUI::ShapeButton::Draw(target_);
    optionButton.Draw(target_);
}

//EnergyUsageButton
EnergyUsageButton::EnergyUsageButton(sf::Vector2f position_, sf::Color clr) :
    ShapeButton(&style_energyCostButton, sf::Vector2f(590, 60), clr, clr, sf::Color(clr.r + 30 < clr.r ? 255 : clr.r + 30, clr.g + 30 < clr.g ? 255 : clr.g + 30, clr.b + 30 < clr.b ? 255 : clr.b + 30), false,
                position_, resourcePack_shipProfileButton, "")
{
    text_cost.setFont(*resourcePack_shipProfileButton.font);
    text_cost.setCharacterSize(16 * 2);
    text_cost.setPosition(position_ + sf::Vector2f(295 / 2.f - 30.f, -11));
    text_cost.setScale(0.5f, 0.5f);
    text_cost.setFillColor(sf::Color::White);

    sprite_energy.setTexture(*texture_iconStates);
    sprite_energy.setTextureRect(sf::IntRect(300, 60, 60, 60));
    sprite_energy.setOrigin(30, 30);
    sprite_energy.setScale(0.5f, 0.5f);
    sprite_energy.setPosition(position_ + sf::Vector2f(295 / 2.f - 15.f, 0));
}

void EnergyUsageButton::Draw(sf::RenderTarget& target_)
{
    text_cost.setFillColor(sprite.getColor());
    sprite_energy.setColor(sprite.getColor());

    ShapeButton::Draw(target_);
    target_.draw(text_cost);
    target_.draw(sprite_energy);
}

void EnergyUsageButton::UpdateInfo(std::string text_, std::string  message_, int cost_)
{
    text_cost.setString(ConvertIntToString(cost_));
    text_cost.setOrigin(text_cost.getLocalBounds().width, 0);

    text.setString(text_);
    text.setOrigin(0, 0);

    message = message_;
}

float EnergyUsageButton::GetTransparenct()
{
    return sprite.getColor().a;
}

bool EnergyUsageButton::IsInactive()
{
    return currentPhase == GUI::InactivePhase;
}

//BoosterButton
BoosterButton::BoosterButton(sf::Vector2f position_, sf::Color clr, std::string message_, std::string name_, std::string desc_, int cost_, int time_, float* timeRef_, std::string* stringRef_, int* versionRef_, int version_, sf::IntRect iconRect_) :
    string_time(ConvertTimeToString(time_)),
    string_cost(ConvertIntToString(cost_)),
    cost(cost_),
    time(time_),
    timeRef(timeRef_),
    version(version_),
    versionRef(versionRef_),
    display_name(name_, resourcePack_shipProfileButton.font, sf::Color::White, 24, sf::Vector2f(),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(14, 8), sf::Vector2f(0.5f, 0.5f), 0),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(14, 8), sf::Vector2f(0.5f, 0.5f), 255),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(14, 8), sf::Vector2f(0.5f, 0.5f), 0)),
    display_desc(desc_, resourcePack_shipProfileButton.font, sf::Color::White, 24, sf::Vector2f(),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(14, 27), sf::Vector2f(0.5f, 0.5f), 0),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(14, 27), sf::Vector2f(0.5f, 0.5f), 255),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(14, 27), sf::Vector2f(0.5f, 0.5f), 0)),
    display_time(ConvertTimeToString(time_), resourcePack_shipProfileButton.font, sf::Color::White, 24, sf::Vector2f(),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(34, 60), sf::Vector2f(0.5f, 0.5f), 0),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(34, 60), sf::Vector2f(0.5f, 0.5f), 255),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(34, 60), sf::Vector2f(0.5f, 0.5f), 0)),
    display_cost(ConvertIntToString(cost_), resourcePack_shipProfileButton.font, sf::Color::White, 24, sf::Vector2f(),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(134, 60), sf::Vector2f(0.5f, 0.5f), 0),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(134, 60), sf::Vector2f(0.5f, 0.5f), 255),
                                  GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(134, 60), sf::Vector2f(0.5f, 0.5f), 0)),
    display_sp_time(texture_iconStates, sf::IntRect(360, 60, 60, 60), sf::Vector2f(),
                                       GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(10, 56), sf::Vector2f(0.38f, 0.38f), 0),
                                       GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(10, 56), sf::Vector2f(0.38f, 0.38f), 255),
                                       GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(10, 56), sf::Vector2f(0.38f, 0.38f), 0)),
    display_sp_cost(texture_iconStates, sf::IntRect(300, 60, 60, 60), sf::Vector2f(),
                                       GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(112, 56), sf::Vector2f(0.38f, 0.38f), 0),
                                       GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(112, 56), sf::Vector2f(0.38f, 0.38f), 255),
                                       GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(112, 56), sf::Vector2f(0.38f, 0.38f), 0)),
    display_sp_icon(texture_iconBoosters, iconRect_, sf::Vector2f(),
                                       GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(250, 11), sf::Vector2f(1, 1), 0),
                                       GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(250, 11), sf::Vector2f(1, 1), 255),
                                       GUI::DisplayObject::DisplayPhase(position_ + sf::Vector2f(250, 11), sf::Vector2f(1, 1), 0)),
    ShapeToggleButton(style_iconShapeButton, style_iconShapeButton, sf::Vector2f(321, 83),
                      sf::Color(clr.r, clr.g, clr.b, 177), sf::Color(clr.r, clr.g, clr.b, 177), ChangeColorHue(sf::Color(clr.r, clr.g, clr.b, 177), 30),
                      clr, clr, ChangeColorHue(clr, 30), false, position_ + sf::Vector2f(321 * 0.5f, 83 * 0.5f), resourcePack_shipProfileButton,
                      message_, "", "", nullptr, 1, sf::IntRect(), 0, 0, true, stringRef_, false)
{
}

bool BoosterButton::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool feedback = GUI::ShapeToggleButton::HandleInput(mousePos_, leftClick_, deltaTime, stringRef != nullptr && versionRef != nullptr && *stringRef != "" && *versionRef > version ? false : doInput);

    if(feedback)
    {
        if(cost > Game::GetLevelEnergy(Game::GetPlayerTeam()))
            ;//error message
        else
        {
            if(stringRef != nullptr)
                *stringRef = message;
            Game::GetLevelEnergy(Game::GetPlayerTeam()) -= cost;
        }
    }

    display_name.Update(deltaTime);
    display_time.Update(deltaTime);
    display_desc.Update(deltaTime);
    display_cost.Update(deltaTime);
    display_sp_cost.Update(deltaTime);
    display_sp_time.Update(deltaTime);
    display_sp_icon.Update(deltaTime);

    if(feedback)
    {
        if(versionRef != nullptr) *versionRef = version;
        if(timeRef != nullptr) *timeRef = time;
    }

    if(timeRef != nullptr && status)
        display_time.SetText(ConvertTimeToString(*timeRef));

    if(stringRef != nullptr && string_lastRef != *stringRef)
    {
        string_lastRef = *stringRef;

        if(*stringRef == message)
        {
            display_sp_cost.SetDrawStatus(true);
            display_cost.SetDrawStatus(true);
            display_sp_cost.setTextureRect(sf::IntRect(420, 60, 60, 60));
            display_cost.SetText("Active");

            display_sp_time.setTextureRect(sf::IntRect(360, 60, 60, 60));
        }
        else if(*stringRef != "" && versionRef != nullptr && *versionRef > version)
        {
            display_sp_cost.SetDrawStatus(false);
            display_cost.SetDrawStatus(false);

            display_sp_time.setTextureRect(sf::IntRect(0, 60, 60, 60));
            display_time.SetText("Higher version is active");
        }
        else
        {
            display_sp_cost.SetDrawStatus(true);
            display_cost.SetDrawStatus(true);
            display_sp_cost.setTextureRect(sf::IntRect(300, 60, 60, 60));
            display_cost.SetText(string_cost);

            display_sp_time.setTextureRect(sf::IntRect(360, 60, 60, 60));
            display_time.SetText(string_time);
        }
    }

    return feedback;
}

std::string BoosterButton::GetMessage()
{
    return "Update_Booster_Status";
}

void BoosterButton::Draw(sf::RenderTarget& target_)
{
    GUI::ShapeToggleButton::Draw(target_);

    display_name.Draw(target_);
    display_cost.Draw(target_);
    display_desc.Draw(target_);
    display_time.Draw(target_);
    display_sp_time.Draw(target_);
    display_sp_cost.Draw(target_);
    display_sp_icon.Draw(target_);
}

void BoosterButton::Reset(std::string command)
{
    GUI::ShapeToggleButton::Reset(command);

    display_name.Reset();
    display_cost.Reset();
    display_desc.Reset();
    display_time.Reset();
    display_sp_time.Reset();
    display_sp_cost.Reset();
}

void BoosterButton::SetInactive()
{
    GUI::ShapeToggleButton::SetInactive();

    display_name.SetInactive();
    display_cost.SetInactive();
    display_desc.SetInactive();
    display_time.SetInactive();
    display_sp_time.SetInactive();
    display_sp_cost.SetInactive();
    display_sp_icon.SetInactive();
}

//Map Button
MapButton::MapButton(sf::Color bgColor_) :
    display_va(sf::Quads, nullptr,
               GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
               GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
               GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0)),
    display_va_boarder(sf::Quads, nullptr,
                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0)),
    display_va_teamRange(sf::Quads, nullptr,
                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0)),
    displayBoarderStatus(true),
    boarderTime(0.f),
    mapScale(0.f),
    scale(1.f),
    scaleRealTime(1.f),
    minScale(1.f),
    updateTeamRange(true),
    updateMapProperties(true),
    bgColor(bgColor_),
    zoomWhenScaling(false),
    arrowTransparency(0.f),
    clickIsDragging(false),
    mapSizeScaleTransmit(1, 1),
    mapSizeScaleTransmitRealTime(1, 1)
{
    sf::Vector2i levelSize = Game::GetLevelSize();

    mapScale = 0.5f;//((float)Game::GetLevelSize().x * 2) / (Game::GetMapTexture().getSize().x - 30);
    mapSize = (sf::Vector2f)Game::GetMapTexture().getSize() * mapScale;
    zoomWhenScaling = (MapTileManager::MAX_MAPTILE / (levelSize.x > levelSize.y ? levelSize.x : levelSize.y)) == 1;

    if(mapSize.x > mapSize.y)
    {
        //viewRect = sf::FloatRect(0, 0, mapTextureSize.x, mapTextureSize.y + 20);
        position = sf::Vector2f(Game::SCREEN_WIDTH - Game::GetMapTexture().getSize().x - guiAllowance.x, guiAllowance.y);
        mapRefPos = sf::Vector2f(0, 0) + position;
        teamRangeRefPos = sf::Vector2f(0, 10 + mapSize.y) + position;
        buttonSize = mapSize + sf::Vector2f(0, 20);

        if(mapSize.x * 0.75f >= 100)
        {
            if(mapSize.x * 0.5f >= 100)
                minScale = 0.5f;
            else minScale = 0.75f;
        }
    }
    else
    {
        //viewRect = sf::FloatRect(-20 , 0, mapTextureSize.x + 20, mapTextureSize.y);
        position = sf::Vector2f(Game::SCREEN_WIDTH - Game::GetMapTexture().getSize().x - 20 - guiAllowance.x, guiAllowance.y);
        mapRefPos = sf::Vector2f(20, 0) + position;
        teamRangeRefPos = sf::Vector2f(0, 0) + position;
        buttonSize = mapSize + sf::Vector2f(20, 0);

        if(mapSize.y * 0.75f >= 100)
        {
            if(mapSize.y * 0.5f >= 100)
                minScale = 0.5f;
            else minScale = 0.75f;
        }
    }

    display_va.AddRectVertices(sf::FloatRect(mapRefPos.x, mapRefPos.y, mapSize.x, mapSize.y), bgColor_);
    display_va_teamRange.Resize(4 * Game::NumOfTeams);

    for(int a = 0; a < Game::NumOfTeams; a++)
    {
        teamBaseCoverage[a] = 0;
        teamRangeFactor[a] = 0;
    }

    //Preparing arrow points
    arrowPoints[0] = sf::Vector2f(-10, 0);
    arrowPoints[1] = sf::Vector2f(0, -10);
    arrowPoints[2] = sf::Vector2f(10, 0);
}

void MapButton::SetDisplayBoarderStatus(bool status)
{
    displayBoarderStatus = status;
}

void MapButton::SetTeamBaseCoverage(const short& team_, const int& coverage_)
{
    if(team_ >= 0 && team_ < Game::NumOfTeams && teamBaseCoverage[team_] != coverage_)
    {
        teamBaseCoverage[team_] = coverage_;
        updateTeamRange = true;
    }
}

const sf::Vector2f& MapButton::GetButtonSize()
{
    //return sf::Vector2f(viewRect.width, viewRect.height);
    return buttonSize;
}

bool MapButton::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    sf::FloatRect cameraRect = EntityManager::GetCameraRect();
    sf::Vector2u mapTextrSize = Game::GetMapTexture().getSize();
    sf::Vector2i levelWorldSize = Game::GetLevelSize() * MapTileManager::TILE_SIZE;
    clickMoveDir = sf::Vector2f();
    bool isMouseWithinMap = false;

    if(boarderTime > 0)
        boarderTime -= deltaTime;

    //Input
    {
        sf::Vector2f refMousePos = (sf::Vector2f)mousePos_ - position;

        if(sf::FloatRect(mapRefPos.x - position.x, mapRefPos.y - position.y, mapSize.x, mapSize.y).contains(refMousePos))
        {
            GUI::SetCursorOnObjectState(true);
            isMouseWithinMap = true;

            if(doInput)
            {
                if(leftClick_ && clickIsDragging == false)
                    clickIsDragging = true;

                if(GUI::GetMouseScrollDelta() != 0 && clickIsDragging == false)
                {
                    scale += 0.25f * GUI::GetMouseScrollDelta();
                    GUI::SetMouseScrollDalta(0.f);

                    if(scale > 1.f)
                        scale = 1.f;
                    else if(scale < minScale)
                        scale = minScale;
                }
            }

            TendTowards(arrowTransparency, zoomWhenScaling && scale < 1.f ? 255.f : 0.f, 0.1, 0.0001f, deltaTime);
        }
        else if(clickIsDragging == false)
            TendTowards(arrowTransparency, 0.f, 0.1, 0.0001f, deltaTime);

        //Map Click
        sf::Vector2f mouseMapFactor = sf::Vector2f((mousePos_.x - mapRefPos.x) / mapSize.x, (mousePos_.y - mapRefPos.y) / mapSize.y);
        if(cameraRect.width > levelWorldSize.x)
            mouseMapFactor.x = 0.5f;
        if(cameraRect.height > levelWorldSize.y)
            mouseMapFactor.y = 0.5f;

        if(clickIsDragging && (Game::GetInput(sf::Mouse::Left, Game::Hold) || Game::GetInput(sf::Mouse::Left, Game::Pressed)))
        {
            if(zoomWhenScaling)
            {
                /*if(mouseMapFactor.x < 0.15f)
                    clickMoveDir.x = -1;
                else if(mouseMapFactor.x > 0.85f)
                    clickMoveDir.x = 1;

                if(mouseMapFactor.y < 0.15f)
                    clickMoveDir.y = -1;
                else if(mouseMapFactor.y > 0.85f)
                    clickMoveDir.y = 1;*/


                sf::Vector2f mappingFactor = (mouseMapFactor - sf::Vector2f(0.5f, 0.5f)) * 2.f;
                if(MagnitudeSqureOfVector(mappingFactor) > 0.6f * 0.6f)
                {
                    sf::Vector2f mapFactorUnitVec = NormalizeVector(mappingFactor);
                    sf::Vector2f limitVec(mapFactorUnitVec.x * mapSize.x / 2.f * 0.6f, mapFactorUnitVec.y * mapSize.y / 2.f * 0.6f);
                    sf::Vector2f diff(mousePos_.x - (mapRefPos.x + mapSize.x / 2.f) - limitVec.x, mousePos_.y - (mapRefPos.y + mapSize.y / 2.f) - limitVec.y);
                    float diffMagn = MagnitudeOfVector(diff);

                    float smoothing = MagnitudeOfVector(sf::Vector2f(mapFactorUnitVec.x * mapSize.x / 2.f * 0.4f, mapFactorUnitVec.y * mapSize.y / 2.f * 0.4f));
                    if(smoothing > 0.f)
                    {
                        smoothing = diffMagn / smoothing;
                        if(smoothing != 0.f)
                            clickMoveDir = diff / diffMagn * smoothing * 1.5f;
                    }
                }

                mapPositionNoClipping += clickMoveDir * (cameraRect.width < cameraRect.height ? cameraRect.width : cameraRect.height) * 1.7f / (float)MapTileManager::TILE_SIZE * 4.f * deltaTime;

                if(mapPositionNoClipping.x < 0)
                    mapPositionNoClipping.x = 0;
                else if(mapPositionNoClipping.x > mapTextrSize.x - mapSize.x * 2)
                    mapPositionNoClipping.x = mapTextrSize.x - mapSize.x * 2;

                if(mapPositionNoClipping.y  < 0)
                    mapPositionNoClipping.y = 0;
                else if(mapPositionNoClipping.y > mapTextrSize.y - mapSize.y * 2)
                    mapPositionNoClipping.y = mapTextrSize.y - mapSize.y * 2;
            }
            else mapPositionNoClipping = sf::Vector2f();

            sf::Vector2f mouseMapFactorClipped = mouseMapFactor;
            if(mouseMapFactor.x < 0)
                mouseMapFactorClipped.x = 0;
            else if(mouseMapFactor.x > 1)
                mouseMapFactorClipped.x = 1;

            if(mouseMapFactor.y < 0)
                mouseMapFactorClipped.y = 0;
            else if(mouseMapFactor.y > 1)
                mouseMapFactorClipped.y = 1;

            mapPosition = (sf::Vector2f)((sf::Vector2i)mapPositionNoClipping / 2 * 2);

            sf::Vector2i levelSize = Game::GetLevelSize();
            float levelScaling = MapTileManager::MAX_MAPTILE / (levelSize.x > levelSize.y ? levelSize.x : levelSize.y);
            sf::Vector2f newCameraCenter = (mapPositionNoClipping + sf::Vector2f(mouseMapFactorClipped.x * mapSize.x, mouseMapFactorClipped.y * mapSize.y) / mapScale / 0.5f - sf::Vector2f(30, 30)) / (levelScaling * 4.f) * (float)MapTileManager::TILE_SIZE;
            EntityManager::QuickPeakWithCamera(newCameraCenter);
            cameraRect = EntityManager::GetCameraRect();
        }
        else clickIsDragging = false;
    }

    float lastScale = scaleRealTime;
    if(clickIsDragging == false)
        TendTowards(scaleRealTime, scale, 0.1f, 0.00001f, deltaTime);
    if(lastScale != scale)
        updateMapProperties = true;

    if(zoomWhenScaling && clickIsDragging == false && mapSizeScaleTransmit != mapSizeScaleTransmitRealTime)
    {
        updateMapProperties = true;
        TendTowards(mapSizeScaleTransmitRealTime.x, mapSizeScaleTransmit.x, 0.1f, 0.00001f, deltaTime);
        TendTowards(mapSizeScaleTransmitRealTime.y, mapSizeScaleTransmit.y, 0.1f, 0.00001f, deltaTime);
    }

    if(updateMapProperties)
    {
        mapScale = zoomWhenScaling ? 1.f : scaleRealTime;
        if(zoomWhenScaling)
        {
            mapSizeScaleTransmit.x = camRect.width + 10 > mapTextrSize.x * 0.5f * scale ? 1 : scale;
            mapSizeScaleTransmit.y = camRect.height + 10 > mapTextrSize.y * 0.5f * scale ? 1 : scale;

            mapSize.x = mapTextrSize.x * 0.5f * mapSizeScaleTransmitRealTime.x;
            mapSize.y = mapTextrSize.y * 0.5f * mapSizeScaleTransmitRealTime.y;
        }
        else mapSize = (sf::Vector2f)mapTextrSize * 0.5f * scaleRealTime;
        updateTeamRange = true;

        if(mapSize.x > mapSize.y)
        {
            buttonSize = mapSize + sf::Vector2f(0, 20);
            position = sf::Vector2f(Game::SCREEN_WIDTH - buttonSize.x - guiAllowance.x, guiAllowance.y);
            mapRefPos = position;
            teamRangeRefPos = sf::Vector2f(0, 10 + mapSize.y) + position;
        }
        else
        {
            buttonSize = mapSize + sf::Vector2f(20, 0);
            position = sf::Vector2f(Game::SCREEN_WIDTH - buttonSize.x - guiAllowance.x, guiAllowance.y);
            mapRefPos = sf::Vector2f(20 + position.x, position.y);
            teamRangeRefPos = sf::Vector2f(0, 0) + position;
        }

        if(zoomWhenScaling && mapSizeScaleTransmit != mapSizeScaleTransmitRealTime)
        {
            sf::Vector2f centerToMap = EntityManager::GetCamera().getCenter() / (float)MapTileManager::TILE_SIZE * 4.f + sf::Vector2f(30, 30);
            mapPositionNoClipping = centerToMap - mapSize;

            if(mapPositionNoClipping.x < 0)
                mapPositionNoClipping.x = 0;
            else if(mapPositionNoClipping.x > mapTextrSize.x - mapSize.x * 2)
                mapPositionNoClipping.x = mapTextrSize.x - mapSize.x * 2;

            if(mapPositionNoClipping.y  < 0)
                mapPositionNoClipping.y = 0;
            else if(mapPositionNoClipping.y > mapTextrSize.y - mapSize.y * 2)
                mapPositionNoClipping.y = mapTextrSize.y - mapSize.y * 2;
        }

        display_va.EditVertext(0, mapRefPos, bgColor, sf::Vector2f());
        display_va.EditVertext(1, mapRefPos + sf::Vector2f(mapSize.x, 0), bgColor, sf::Vector2f());
        display_va.EditVertext(2, mapRefPos + sf::Vector2f(mapSize.x, mapSize.y), bgColor, sf::Vector2f());
        display_va.EditVertext(3, mapRefPos + sf::Vector2f(0, mapSize.y), bgColor, sf::Vector2f());

        mapRefPos = (sf::Vector2f)(sf::Vector2i)mapRefPos;
        Game::SetMapIconScale(zoomWhenScaling ? 1.f : 1.f + (1.f - scaleRealTime) / 0.5f);
    }

    display_va.Update(deltaTime);
    display_va_boarder.Update(deltaTime);
    display_va_teamRange.Update(deltaTime);

    //Team Base Range
    for(int a = 0; a < Game::NumOfTeams; a++)
        SetTeamBaseCoverage(a, Base::GetTeamActiveBaseCoverage(a));

    int totalBaseCoverage = 0;
    for(int a = 0; a < Game::NumOfTeams; a++)
        totalBaseCoverage += teamBaseCoverage[a];
    if(totalBaseCoverage > 0 && updateTeamRange)
    {
        updateTeamRange = false;

        if(mapSize.x > mapSize.y)
        {
            float nextPos = 0;
            for(int a = 0; a < Game::NumOfTeams; a++)
            {
                float factor = (float)teamBaseCoverage[a] / totalBaseCoverage;
                TendTowards(teamRangeFactor[a], factor, 0.2f, 0.00001f, deltaTime);
                if(teamRangeFactor[a] != factor)
                    updateTeamRange = true;

                float length = teamRangeFactor[a] * mapSize.x;
                display_va_teamRange.EditVertext(4 * a, teamRangeRefPos + sf::Vector2f(nextPos, 0), mapTeamRangeColor[a], sf::Vector2f());
                display_va_teamRange.EditVertext(4 * a + 1, teamRangeRefPos + sf::Vector2f(nextPos + length, 0), mapTeamRangeColor[a], sf::Vector2f());
                display_va_teamRange.EditVertext(4 * a + 2, teamRangeRefPos + sf::Vector2f(nextPos + length, 10), mapTeamRangeColor[a], sf::Vector2f());
                display_va_teamRange.EditVertext(4 * a + 3, teamRangeRefPos + sf::Vector2f(nextPos, 10), mapTeamRangeColor[a], sf::Vector2f());

                nextPos += length;
            }
        }
        else
        {
            float nextPos = 0;
            for(int a = 0; a < Game::NumOfTeams; a++)
            {
                float factor = (float)teamBaseCoverage[a] / totalBaseCoverage;
                TendTowards(teamRangeFactor[a], factor, 0.2f, 0.00001f, deltaTime);
                if(teamRangeFactor[a] != factor)
                    updateTeamRange = true;

                float length = teamRangeFactor[a] * mapSize.y;
                display_va_teamRange.EditVertext(4 * a, teamRangeRefPos + sf::Vector2f(0, nextPos), mapTeamRangeColor[a], sf::Vector2f());
                display_va_teamRange.EditVertext(4 * a + 1, teamRangeRefPos + sf::Vector2f(10, nextPos), mapTeamRangeColor[a], sf::Vector2f());
                display_va_teamRange.EditVertext(4 * a + 2, teamRangeRefPos + sf::Vector2f(10, nextPos + length), mapTeamRangeColor[a], sf::Vector2f());
                display_va_teamRange.EditVertext(4 * a + 3, teamRangeRefPos + sf::Vector2f(0, nextPos + length), mapTeamRangeColor[a], sf::Vector2f());

                nextPos += length;
            }
        }
    }

    //Map and Camera
    {
        sf::Vector2f cameraCenter = zoomWhenScaling && EntityManager::GetCameraFocus() != nullptr ? EntityManager::GetCameraFocus()->GetPosition() : EntityManager::GetCamera().getCenter();

        if(clickIsDragging)
            ;
        else if(zoomWhenScaling)
        {
            sf::Vector2f tendPosition = cameraCenter / (float)MapTileManager::TILE_SIZE * 4.f + sf::Vector2f(30, 30) - mapSize;
            sf::Vector2i levelWorldSize = Game::GetLevelSize() * MapTileManager::TILE_SIZE;

            if(tendPosition.x < 0)
                tendPosition.x = 0;
            else if(tendPosition.x > mapTextrSize.x - mapSize.x * 2)
                tendPosition.x = mapTextrSize.x - mapSize.x * 2;

            if(tendPosition.y < 0)
                tendPosition.y = 0;
            else if(tendPosition.y > mapTextrSize.y - mapSize.y * 2)
                tendPosition.y = mapTextrSize.y - mapSize.y * 2;

            if(mapPositionNoClipping != tendPosition)
            {
                sf::Vector2f diff = mapPositionNoClipping - tendPosition;
                float diffMagn = MagnitudeOfVector(diff);
                sf::Vector2f unitVec = diff / diffMagn;
                TendTowards(diffMagn, 0.f, 0.15f, 0.01f, deltaTime);
                mapPositionNoClipping = tendPosition + unitVec * diffMagn;
            }

            mapPosition = (sf::Vector2f)((sf::Vector2i)mapPositionNoClipping / 2 * 2); //Fixes top-left to grid causing no artifacts(one pixel movement)
        }
        else mapPositionNoClipping = mapPosition =  sf::Vector2f();

        //Camera Boarder
        sf::Vector2i levelSize = Game::GetLevelSize();
        float levelScaling = MapTileManager::MAX_MAPTILE / (levelSize.x > levelSize.y ? levelSize.x : levelSize.y);

        sf::Vector2f cameraTopLeft = sf::Vector2f(cameraRect.left, cameraRect.top) / (float)MapTileManager::TILE_SIZE * 4.f * levelScaling + sf::Vector2f(30, 30);
        sf::Vector2f camInMapTopLeft = cameraTopLeft - mapPosition;

        sf::FloatRect curCamRect;
        curCamRect.left = camInMapTopLeft.x * 0.5f * mapScale + mapRefPos.x;
        curCamRect.top = camInMapTopLeft.y * 0.5f * mapScale + mapRefPos.y;
        curCamRect.width = cameraRect.width / (float)MapTileManager::TILE_SIZE * 4 * 0.5f * levelScaling * mapScale;
        curCamRect.height = cameraRect.height / (float)MapTileManager::TILE_SIZE * 4 * 0.5f * levelScaling * mapScale;

        if(displayBoarderStatus == false || EntityManager::GetCameraFocus())
            boarderTime = 0.f;
        else if((sf::IntRect)curCamRect != (sf::IntRect)camRect)
        {
            boarderTime = 2.f;
            if(zoomWhenScaling)
                updateMapProperties = true;
        }

        if((isMouseWithinMap || clickIsDragging) && boarderTime <= 0.f)
            boarderTime = 0.01f;

        camRect = curCamRect;

        if(boarderTime > 0)
        {
            display_va_boarder.SetPhase(GUI::DefaultPhase, false);
            sf::FloatRect intersect;
            sf::FloatRect(mapRefPos.x, mapRefPos.y, mapSize.x, mapSize.y).intersects(curCamRect, intersect);

            sf::Color color_ = ChangeColorHue(Game::GetCurrentRankThemeColor(), 90);//(180, 180, 180);
            float otl = 1.f;
            display_va_boarder.Clear();
            if(camRect.top >= mapRefPos.y)
                display_va_boarder.AddRectVertices(sf::FloatRect(intersect.left - otl, intersect.top - otl, intersect.width + otl * 2, otl), color_);
            if(camRect.top + camRect.height < mapRefPos.y + mapSize.y)
                display_va_boarder.AddRectVertices(sf::FloatRect(intersect.left - otl, intersect.top + intersect.height, intersect.width + otl * 2, otl), color_);
            if(camRect.left >= mapRefPos.x)
                display_va_boarder.AddRectVertices(sf::FloatRect(intersect.left - otl, intersect.top, otl, intersect.height), color_);
            if(camRect.left + camRect.width < mapRefPos.x + mapSize.x)
                display_va_boarder.AddRectVertices(sf::FloatRect(intersect.left + intersect.width, intersect.top, otl, intersect.height), color_);
        }
        else display_va_boarder.SetPhase(GUI::InactivePhase, false);

        /*sf::Vector2f camPos = EntityManager::GetCamera().getCenter();
        sf::Vector2i camPosInt = (sf::Vector2i)((camPos / (float)MapTileManager::TILE_SIZE) * 4.f + sf::Vector2f(60, 60));
        camPos = sf::Vector2f(camPosInt.x % 2 > 0 ? camPosInt.x - 1 : camPosInt.x, camPosInt.y % 2 > 0 ? camPosInt.y - 1 : camPosInt.y);
        if(mapPosition != camPos)
        {
            sf::Vector2f diff = mapPosition - camPos;
            float tendMagn = MagnitudeOfVector(diff);
            sf::Vector2f unitVec = diff / tendMagn;
            TendTowards(tendMagn, 0, 0.1f, 0.1f, deltaTime);
            mapPosition = camPos + unitVec * tendMagn;
        }*/
    }

    return false;
}

void MapButton::Draw(sf::RenderTarget& target_)
{
    display_va.Draw(target_);

    sf::Vertex vertices[4];

    float allignment = 30 * 0.5f * mapScale;
    allignment = allignment - (int)allignment;
    sf::Vector2f allignmentVec(allignment, allignment);

    sf::Vector2f textrSize = (sf::Vector2f)Game::GetMapTexture().getSize();
    sf::Vector2f vertexTextrSize = zoomWhenScaling ? mapSize * 2.f : textrSize;
    vertices[0] = sf::Vertex(mapRefPos - allignmentVec, sf::Color::White, mapPosition);
    vertices[1] = sf::Vertex(mapRefPos - allignmentVec + sf::Vector2f(mapSize.x, 0), sf::Color::White, mapPosition + sf::Vector2f(vertexTextrSize.x, 0));
    vertices[2] = sf::Vertex(mapRefPos - allignmentVec + mapSize, sf::Color::White, mapPosition + vertexTextrSize);
    vertices[3] = sf::Vertex(mapRefPos - allignmentVec + sf::Vector2f(0, mapSize.y), sf::Color::White, mapPosition + sf::Vector2f(0, vertexTextrSize.y));

    target_.draw(vertices, 4, sf::Quads, &Game::GetMapTexture());
    display_va_boarder.Draw(target_);
    display_va_teamRange.Draw(target_);

    if(zoomWhenScaling && arrowTransparency > 0)
    {
        sf::Transformable transformation;
        sf::Vector2u mapTextrSize = Game::GetMapTexture().getSize();

        //Top
        if(mapPositionNoClipping.y > 5.f && mapPositionNoClipping.y > 35.f + camRect.height - mapSize.y)
        {
            transformation.setPosition(mapRefPos + sf::Vector2f(mapSize.x / 2.f, 5.f));
            transformation.setScale(sf::Vector2f(1, 1) * (clickMoveDir.y < -0.2 ? 1.5f : 1.f));
            transformation.setRotation(0);
            for(int a = 0; a < 3; a++)
            {
                vertices[a].position = transformation.getTransform().transformPoint(arrowPoints[a]);
                vertices[a].color = sf::Color(255, 255, 255, arrowTransparency);
            }
            target_.draw(vertices, 3, sf::Triangles);
        }

        //Right
        if(mapPositionNoClipping.x < mapTextrSize.x - mapTextrSize.x * 2 - 5.f && mapPositionNoClipping.x < mapTextrSize.x - 35.f - camRect.width - mapSize.x)
        {
            transformation.setPosition(mapRefPos + sf::Vector2f(mapSize.x - 5.f, mapSize.y / 2.f));
            transformation.setScale(sf::Vector2f(1, 1) * (clickMoveDir.x > 0.2 ? 1.5f : 1.f));
            transformation.setRotation(90);
            for(int a = 0; a < 3; a++)
            {
                vertices[a].position = transformation.getTransform().transformPoint(arrowPoints[a]);
                vertices[a].color = sf::Color(255, 255, 255, arrowTransparency);
            }
            target_.draw(vertices, 3, sf::Triangles);
        }

        //Bottom
        if(mapPositionNoClipping.y < mapTextrSize.y - mapSize.y * 2 - 5.f && mapPositionNoClipping.y < mapTextrSize.y - 35.f - camRect.height - mapSize.y)
        {
            transformation.setPosition(mapRefPos + sf::Vector2f(mapSize.x / 2.f, mapSize.y - 5.f));
            transformation.setScale(sf::Vector2f(1, 1) * (clickMoveDir.y > 0.2 ? 1.5f : 1.f));
            transformation.setRotation(180);
            for(int a = 0; a < 3; a++)
            {
                vertices[a].position = transformation.getTransform().transformPoint(arrowPoints[a]);
                vertices[a].color = sf::Color(255, 255, 255, arrowTransparency);
            }
            target_.draw(vertices, 3, sf::Triangles);
        }

        //Left
        if(mapPositionNoClipping.x > 5.f && mapPositionNoClipping.x > 35.f + camRect.width - mapSize.x)
        {
            transformation.setPosition(mapRefPos + sf::Vector2f(5.f, mapSize.y / 2.f));
            transformation.setScale(sf::Vector2f(1, 1) * (clickMoveDir.x < -0.2 ? 1.5f : 1.f));
            transformation.setRotation(270);
            for(int a = 0; a < 3; a++)
            {
                vertices[a].position = transformation.getTransform().transformPoint(arrowPoints[a]);
                vertices[a].color = sf::Color(255, 255, 255, arrowTransparency);
            }
            target_.draw(vertices, 3, sf::Triangles);
        }
    }
}

std::string MapButton::GetMessage()
{
    return "";
}

const float& MapButton::GetRealTimeScale()
{
    return scaleRealTime;
}

void MapButton::Reset(std::string command)
{
    display_va.Reset();
    display_va_boarder.Reset();
}

void MapButton::SetInactive()
{
    display_va.SetInactive();
    display_va_boarder.SetInactive();
}
