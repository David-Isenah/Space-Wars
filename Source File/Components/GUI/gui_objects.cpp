#include "GUI/gui_objects.h"
#include <iostream>
#include <sstream>
#include <math.h>

float mouseScrollDelta = 0.f;
bool isCursorOnObject = false;

//Public Functions
void GUI::SetMouseScrollDalta(float scrollMagn)
{
    mouseScrollDelta = scrollMagn;
}

float GUI::GetMouseScrollDelta()
{
    return mouseScrollDelta;
}

bool GUI::IsCursorOnObject()
{
    return isCursorOnObject;
}

void GUI::SetCursorOnObjectState(bool state_)
{
    isCursorOnObject = state_;
}

//Style
GUI::Style::Style():
    fontSize(20),
    transmissionFactor(0),
    inputHoldDelay(0),
    inputType(OnRelease),
    textOriginFactor(sf::Vector2f(0.5f, 0.5f))
{
}

void GUI::Style::setPhase(PhaseStage stage_, float scale_, int textrIndex_, sf::Vector2f refPosition_, sf::Vector2f refObjectPosition_,
                          int transparancy_, sf::Vector2f boundarySize_, sf::Vector2f boundaryRefPos_)
{
    if(stage_ == DefaultPhase)
        defaultPhase.Set(scale_, textrIndex_, refPosition_, refObjectPosition_, transparancy_, boundarySize_, boundaryRefPos_);
    else if(stage_ == HighlighPhase)
        highlightPhase.Set(scale_, textrIndex_, refPosition_, refObjectPosition_, transparancy_, boundarySize_, boundaryRefPos_);
    else if(stage_ == PressedPhase)
        pressedPhase.Set(scale_, textrIndex_, refPosition_, refObjectPosition_, transparancy_, boundarySize_, boundaryRefPos_);
    else if(stage_ == StartPhase)
        startPhase.Set(scale_, textrIndex_, refPosition_, refObjectPosition_, transparancy_, boundarySize_, boundaryRefPos_);
    else if(stage_ == InactivePhase)
        inactivePhase.Set(scale_, textrIndex_, refPosition_, refObjectPosition_, transparancy_, boundarySize_, boundaryRefPos_);
    else if(stage_ == LockedPhase)
        lockedPhase.Set(scale_, textrIndex_, refPosition_, refObjectPosition_, transparancy_, boundarySize_, boundaryRefPos_);
}

void GUI::Style::setTextureRect(sf::IntRect rect_)
{
    textureRect = rect_;
}

void GUI::Style::setTextOriginFactor(sf::Vector2f factor_)
{
    textOriginFactor = factor_;

    if(textOriginFactor.x < 0)
        textOriginFactor.x = 0;
    else if(textOriginFactor.x > 1)
        textOriginFactor.x = 1;

    if(textOriginFactor.y < 0)
        textOriginFactor.y = 0;
    else if(textOriginFactor.y > 1)
        textOriginFactor.y = 1;
}

void GUI::Style::setTextRef(sf::Vector2f refPos_)
{
    textRef = refPos_;
}

void GUI::Style::setIconRef(sf::Vector2f refPos_)
{
    iconRef = refPos_;
}

void GUI::Style::setFontParameters(int size_, sf::Color color_)
{
    fontSize = size_;
    fontColor = color_;
}

void GUI::Style::setTransmissionFactor(float factor_)
{
    transmissionFactor = factor_;
}

void GUI::Style::setInputHoldDelay(float timeDelay_)
{
    inputHoldDelay = timeDelay_;
}

void GUI::Style::setInputType(InputType type_)
{
    inputType = type_;
}

void GUI::Style::copyFrom(const Style& style_)
{
    *this = style_;
}

float GUI::Style::getTransmissionFactor()
{
    return transmissionFactor;
}

sf::IntRect GUI::Style::getTextureRect()
{
    return textureRect;
}

int GUI::Style::getFontSize()
{
    return fontSize;
}

sf::Color GUI::Style::getFontColor()
{
    return fontColor;
}

int GUI::Style::getPhaseTransparency(PhaseStage stage_)
{
    return getPhase(stage_).transparency;
}

sf::Vector2f GUI::Style::getTextOriginFactor()
{
    return textOriginFactor;
}

sf::Vector2f GUI::Style::getTextRef(PhaseStage stage_)
{
    sf::Vector2f position;

    if(stage_ == DefaultPhase)
        position = (textRef + defaultPhase.refObjectPosition);
    else if(stage_ == HighlighPhase)
        position = (textRef + highlightPhase.refObjectPosition);
    else if(stage_ == PressedPhase)
        position = (textRef + pressedPhase.refObjectPosition);
    else if(stage_ == StartPhase)
        position = (textRef + startPhase.refObjectPosition);
    else if(stage_ == InactivePhase)
        position = (textRef + inactivePhase.refObjectPosition);
    else if(stage_ == LockedPhase)
        position = (textRef + lockedPhase.refObjectPosition);

    return position;
}

sf::Vector2f GUI::Style::getIconRef(PhaseStage stage_)
{
    sf::Vector2f position;

    if(stage_ == DefaultPhase)
        position = (iconRef + defaultPhase.refObjectPosition);
    else if(stage_ == HighlighPhase)
        position = (iconRef + highlightPhase.refObjectPosition);
    else if(stage_ == PressedPhase)
        position = (iconRef + pressedPhase.refObjectPosition);
    else if(stage_ == StartPhase)
        position = (iconRef + startPhase.refObjectPosition);
    else if(stage_ == InactivePhase)
        position = (iconRef + inactivePhase.refObjectPosition);
    else if(stage_ == LockedPhase)
        position = (iconRef + lockedPhase.refObjectPosition);

    return position;
}

sf::Vector2f GUI::Style::getPhasePosition(PhaseStage stage_)
{
    if(stage_ == DefaultPhase)
        return defaultPhase.refPosition;
    else if(stage_ == HighlighPhase)
        return highlightPhase.refPosition;
    else if(stage_ == PressedPhase)
        return pressedPhase.refPosition;
    else if(stage_ == StartPhase)
        return startPhase.refPosition;
    else if(stage_ == InactivePhase)
        return inactivePhase.refPosition;
    else if(stage_ == LockedPhase)
        return lockedPhase.refPosition;

    return sf::Vector2f(0.f, 0.f);
}

float GUI::Style::getPhaseScale(PhaseStage stage_)
{
    if(stage_ == DefaultPhase)
        return defaultPhase.scale;
    else if(stage_ == HighlighPhase)
        return highlightPhase.scale;
    else if(stage_ == PressedPhase)
        return pressedPhase.scale;
    else if(stage_ == StartPhase)
        return startPhase.scale;
    else if(stage_ == InactivePhase)
        return inactivePhase.scale;
    else if(stage_ == LockedPhase)
        return lockedPhase.scale;

    return 1.f;
}

GUI::Style::Phase GUI::Style::getPhase(PhaseStage stage_)
{
    if(stage_ == DefaultPhase)
        return defaultPhase;
    else if(stage_ == HighlighPhase)
        return highlightPhase;
    else if(stage_ == PressedPhase)
        return pressedPhase;
    else if(stage_ == StartPhase)
        return startPhase;
    else if(stage_ == InactivePhase)
        return inactivePhase;
    else if(stage_ == LockedPhase)
        return lockedPhase;

    return defaultPhase;
}

GUI::InputType GUI::Style::getInputType()
{
    return inputType;
}

float GUI::Style::getInputHoldDelay()
{
    return inputHoldDelay;
}

//Object
GUI::Object::~Object()
{
}

void GUI::Object::Draw(sf::RenderTarget& target_)
{
}

bool GUI::Object::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    return false;
}

std::string GUI::Object::GetMessage()
{
    return "";
}

void GUI::Object::Reset(std::string command)
{
}

void GUI::Object::SetInactive()
{
}

//Button
GUI::Button::Button() :
    currentPhase(StartPhase),
    scale(1),
    iconScale(1),
    style(nullptr),
    message(""),
    keyIndex(false),
    keyHoldTime(0),
    oneSecondDelay(false),
    lockState(false),
    onCursor(false),
    startDelay(0),
    endDelay(0),
    delayTime(0),
    onPush(false),
    currentTransparency(0)
{
}


GUI::Button::Button(Style* style_, sf::Vector2f position_, float scale_, const ResourcePack& rPack_, std::string message_,
               std::string text_, sf::Texture* iconTextr_, float iconScale_, sf::IntRect iconRect_, float strDelay_, float endDelay_) :
    currentPhase(StartPhase),
    position(position_),
    scale(scale_),
    iconScale(iconScale_),
    style(style_),
    message(message_),
    keyIndex(false),
    keyHoldTime(0),
    oneSecondDelay(false),
    lockState(false),
    resourcePack(rPack_),
    onCursor(false),
    startDelay(strDelay_),
    endDelay(endDelay_),
    delayTime(strDelay_),
    onPush(false),
    currentTransparency(0)
{
    texture = rPack_.texture;

    if(rPack_.font != nullptr)
        text.setFont(*rPack_.font);
    if(style != nullptr)
    {
        text.setCharacterSize(style_->getFontSize());
        text.setColor(style_->getFontColor());
        text.setString(text_);
        text.setOrigin(text.getLocalBounds().width * style->getTextOriginFactor().x, text.getLocalBounds().height * style->getTextOriginFactor().y);
    }

    if(texture != nullptr)
        sprite.setTexture(*texture, true);
    sprite.setTextureRect(style->getTextureRect());
    sprite.setOrigin(sprite.getLocalBounds().width / 2.f, sprite.getLocalBounds().height / 2.f);

    if(iconTextr_ != nullptr)
        icon.setTexture(*iconTextr_, true);
    if(iconRect_ != sf::IntRect())
        icon.setTextureRect(iconRect_);
    icon.setOrigin(icon.getLocalBounds().width / 2.f, icon.getLocalBounds().height / 2.f);

    Reset();
}

GUI::Button::~Button()
{
}

void GUI::Button::SetTexture(sf::Texture* texture_)
{
    texture = texture_;
    if(texture_ != nullptr)
        sprite.setTexture(*texture_);
}

void GUI::Button::SetIcon(sf::Texture* texture_, sf::IntRect rect_, float scale_)
{
    iconScale = scale_;
    if(texture_ != nullptr)
        icon.setTexture(*texture_, true);
    if(rect_ != sf::IntRect())
        icon.setTextureRect(rect_);
    icon.setOrigin(icon.getLocalBounds().width / 2.f, icon.getLocalBounds().height / 2.f);
}

void GUI::Button::SetText(std::string text_,sf::Font* font_)
{
    text.setFont(*font_);
    text.setCharacterSize(style->getFontSize());
    text.setColor(style->getFontColor());
    text.setString(text_);
    if(style != nullptr)
        text.setOrigin(text.getLocalBounds().width * style->getTextOriginFactor().x, text.getLocalBounds().height * style->getTextOriginFactor().y);
}

void GUI::Button::SetPosition(sf::Vector2f position_)
{
    position = position_;
}

sf::Vector2f GUI::Button::GetPosition()
{
    return position;
}

void GUI::Button::SetLock(bool state_)
{
    lockState = state_;

    if(state_ == true)
        currentPhase = LockedPhase;
    else currentPhase = DefaultPhase;
}

void GUI::Button::SetInactive()
{
    currentPhase = InactivePhase;
    delayTime = endDelay;
}

bool GUI::Button::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool returnState = false;

    if(style != nullptr && lockState == false && currentPhase != StartPhase && currentPhase != InactivePhase)
    {
        if(sprite.getGlobalBounds().contains((sf::Vector2f)mousePos_))
            isCursorOnObject = true;

        float xRef = style->getPhase(currentPhase).boundaryRefPos.x * scale, yRef = style->getPhase(currentPhase).boundaryRefPos.y * scale;
        float wSize = style->getPhase(currentPhase).boundarySize.x * scale, hSize = style->getPhase(currentPhase).boundarySize.y * scale;
        sf::FloatRect boundary(position.x - wSize / 2.f + xRef, position.y - hSize / 2.f + yRef, wSize, hSize);

        GUI::PhaseStage prvPhase = currentPhase;
        if(currentPhase == PressedPhase)
            prvPhase = HighlighPhase;
        else if(currentPhase == HighlighPhase)
            prvPhase = DefaultPhase;

        sf::FloatRect prvBoundary(0, 0, 0, 0);
        if(prvPhase != currentPhase)
        {
            xRef = style->getPhase(prvPhase).boundaryRefPos.x * scale, yRef = style->getPhase(prvPhase).boundaryRefPos.y * scale;
            wSize = style->getPhase(prvPhase).boundarySize.x * scale, hSize = style->getPhase(prvPhase).boundarySize.y * scale;
            prvBoundary = sf::FloatRect(position.x - wSize / 2.f + xRef, position.y - hSize / 2.f + yRef, wSize, hSize);
        }

        if(doInput && (sprite.getGlobalBounds().contains((sf::Vector2f)mousePos_) || ((currentPhase == PressedPhase || currentPhase == DefaultPhase) && boundary.contains((sf::Vector2f)mousePos_))))
        {
            if(onCursor == false)
                if(resourcePack.highlightSound != nullptr)
                    resourcePack.highlightSound->play();

            onCursor = true;
            currentPhase = HighlighPhase;

            auto inputType = style->getInputType();
            bool keyHold = sf::Mouse::isButtonPressed(sf::Mouse::Left);

            bool firstInstance = false;
            if(keyIndex == false && keyHold == true)
                firstInstance = true;

            if(leftClick_ == true)
                keyIndex = true;

            if(inputType == OnClick && keyIndex == true)
            {
                keyHoldTime += deltaTime;

                if(firstInstance)
                {
                    returnState = true;
                    if(resourcePack.pressedSound != nullptr)
                        resourcePack.pressedSound->play();
                }

                if(oneSecondDelay == false && keyHoldTime >= 0.5f)
                {
                    oneSecondDelay = true;
                    keyHoldTime = 0.f;
                }

                if(style->getInputHoldDelay() > 0 && keyHoldTime > style->getInputHoldDelay() && oneSecondDelay == true)
                {
                    keyHoldTime -= style->getInputHoldDelay();
                    returnState = true;

                    if(resourcePack.pressedSound != nullptr)
                        resourcePack.pressedSound->play();
                }

                if(keyHold == false)
                {
                    keyIndex = false;
                    keyHoldTime = 0;
                    oneSecondDelay = false;
                }
            }
            else if(inputType == OnRelease && keyIndex == true && keyHold == false)
            {
                returnState = true;
                keyIndex = false;

                if(resourcePack.releasedSound != nullptr)
                    resourcePack.releasedSound->play();
            }

            if(keyHold && keyIndex)
            {
                currentPhase = PressedPhase;

                if(onPush == false)
                {
                    if(resourcePack.pressedSound != nullptr)
                        resourcePack.pressedSound->play();
                }
                onPush = true;
            }
            else onPush = false;
        }
        else if(boundary.contains((sf::Vector2f)mousePos_) && doInput)
        {
            keyIndex = false;
            keyHoldTime = 0;
            oneSecondDelay = false;
            onPush = false;
        }
        else if(prvBoundary.contains((sf::Vector2f)mousePos_) && doInput)
        {
            currentPhase = prvPhase;
            keyIndex = false;
            keyHoldTime = 0;
            oneSecondDelay = false;
            onPush = false;
        }
        else
        {
            if(currentPhase != StartPhase && currentPhase!= InactivePhase && currentPhase != LockedPhase)
                currentPhase = DefaultPhase;
            keyIndex = false;
            keyHoldTime = 0;
            oneSecondDelay = false;
            onCursor = false;
            onPush = false;
        }

        if(returnState)
        {
            if(oneSecondDelay != true)
                currentPhase = prvPhase;
        }
    }

    if(style != nullptr && currentPhase != StartPhase && style->getTransmissionFactor() > 0.f && deltaTime > 0.f)
    {
        if(currentPhase == InactivePhase && delayTime > 0)
        {
            delayTime -= deltaTime;
            if(delayTime < 0)
                delayTime = 0;
        }
        else
        {
            sf::Vector2f realPos = (position + (style->getPhasePosition(currentPhase) * scale));
            if(sprite.getPosition() != realPos)
            {
                sf::Vector2f dist = realPos - sprite.getPosition();
                float magn = sqrt(dist.x * dist.x + dist.y * dist.y);

                if(magn > 0)
                {
                    sf::Vector2f unitVec = dist / magn;
                    float moveFactor = magn * deltaTime / style->getTransmissionFactor();
                    if(moveFactor < MIN_TRANSMISSION_VALUE)
                        moveFactor = MIN_TRANSMISSION_VALUE;
                    sprite.move(unitVec * moveFactor);

                    dist = realPos - sprite.getPosition();
                    if(dist.x * unitVec.x + dist.y * unitVec.y < 0)
                        sprite.setPosition(realPos);
                }
                else sprite.setPosition(position + (style->getPhasePosition(currentPhase) * scale));
            }

            float scale_ = style->getPhaseScale(currentPhase) * scale;
            if(sprite.getScale().x < scale_)
            {
                float factor = (scale_ - sprite.getScale().x) * deltaTime / style->getTransmissionFactor();
                if(factor < MIN_TRANSMISSION_SCALE)
                    factor = MIN_TRANSMISSION_SCALE;
                float newScale = sprite.getScale().x + factor;
                sprite.setScale(newScale, newScale);
                if(sprite.getScale().x > scale_)
                    sprite.setScale(scale_, scale_);
            }
            else if(sprite.getScale().x > scale_)
            {
                float factor = (scale_ - sprite.getScale().x) * deltaTime / style->getTransmissionFactor();
                if(factor > -MIN_TRANSMISSION_SCALE)
                    factor = -MIN_TRANSMISSION_SCALE;
                float newScale = sprite.getScale().x + factor;
                sprite.setScale(newScale, newScale);
                if(sprite.getScale().x < scale_)
                    sprite.setScale(scale_, scale_);
            }

            int transparency_ = style->getPhaseTransparency(currentPhase);
            if(currentTransparency < transparency_)
            {
                float factor = (transparency_ - currentTransparency) * deltaTime / style->getTransmissionFactor();
                if(factor < MIN_TRANSMISSION_VALUE)
                    factor = MIN_TRANSMISSION_VALUE;
                currentTransparency += factor;
                if(currentTransparency > transparency_)
                    currentTransparency = transparency_;
                sprite.setColor(sf::Color(255, 255, 255, currentTransparency));
            }

            if(currentTransparency > transparency_)
            {
                float factor = (transparency_ - currentTransparency) * deltaTime / style->getTransmissionFactor();
                if(factor > -MIN_TRANSMISSION_VALUE)
                    factor = -MIN_TRANSMISSION_VALUE;
                currentTransparency += factor;
                if(currentTransparency < transparency_)
                    currentTransparency = transparency_;
                sprite.setColor(sf::Color(255, 255, 255, currentTransparency));
            }
        }
    }
    else if(style != nullptr)
    {
        float scale_ = style->getPhaseScale(currentPhase) * scale;

        sprite.setPosition(position + (style->getPhasePosition(currentPhase) * scale));
        sprite.setColor(sf::Color(255, 255, 255, style->getPhaseTransparency(currentPhase)));
        currentTransparency = style->getPhaseTransparency(currentPhase);
        sprite.setScale(scale_, scale_);

        if(delayTime > 0)
        {
            delayTime -= deltaTime;
            if(delayTime < 0)
                delayTime = 0;
        }

        if(currentPhase == StartPhase && delayTime == 0)
        {
            if(lockState)
                currentPhase = LockedPhase;
            else currentPhase = DefaultPhase;
        }
    }

    return returnState;
}

void GUI::Button::Draw(sf::RenderTarget& target_)
{
    if(style != nullptr)
    {
        float scale_ = style->getPhaseScale(currentPhase) * scale;
        sf::Color textColor = style->getFontColor();
        textColor.a = sprite.getColor().a;

        if(texture != nullptr)
        {
            sf::IntRect rect_ = style->getTextureRect();
            if(currentPhase == StartPhase)
                rect_.top += rect_.height * style->getPhase(DefaultPhase).textrIndex;
            else if(currentPhase == InactivePhase && lockState)
                rect_.top += rect_.height * style->getPhase(LockedPhase).textrIndex;
            else if(currentPhase == InactivePhase)
                rect_.top += rect_.height * style->getPhase(DefaultPhase).textrIndex;
            else
                rect_.top += rect_.height * style->getPhase(currentPhase).textrIndex;

            sprite.setTextureRect(rect_);
            sprite.setOrigin(sprite.getLocalBounds().width / 2.f, sprite.getLocalBounds().height / 2.f);

            target_.draw(sprite);
        }

        if(resourcePack.font != nullptr)
        {
            text.setScale(sprite.getScale());
            text.setPosition(sprite.getPosition() + (style->getTextRef(currentPhase) * scale * sprite.getScale().x));
            text.setColor(sf::Color(textColor));

            target_.draw(text);
        }

        icon.setScale(sprite.getScale() * iconScale);
        icon.setPosition(sprite.getPosition() + (style->getIconRef(currentPhase) * sprite.getScale().x));
        icon.setColor(sprite.getColor());

        target_.draw(icon);
    }
}

std::string GUI::Button::GetMessage()
{
    return message;
}

void GUI::Button::Reset(std::string command)
{
    if(style != nullptr)
    {
        if(command == "")
        {
            currentPhase = StartPhase;
            delayTime = startDelay;
            float scale_ = style->getPhaseScale(currentPhase) * scale;

            sprite.setPosition(position + (style->getPhasePosition(currentPhase) * scale));
            sprite.setScale(scale_, scale_);
            sprite.setColor(sf::Color(255, 255, 255, style->getPhaseTransparency(currentPhase)));
            currentTransparency = style->getPhaseTransparency(currentPhase);
        }
        else if(command == "default-phase-reset")
        {
            if(lockState)
                currentPhase = LockedPhase;
            else currentPhase = DefaultPhase;
        }
    }
}

//ToggleButton
GUI::ToggleButton::ToggleButton(Style* styleOff_, Style* styleOn_, sf::Vector2f position_, float scale_, const ResourcePack& rPack_, std::string message_, std::string offMessage_,
                                std::string text_, sf::Texture* iconTextr_, float iconScale_, sf::IntRect iconRect_, float strDelay_, float endDelay_, bool disableInputInOnState_, std::string* stringRef_, bool canModifyRef_) :
    styleOff(styleOff_),
    styleOn(styleOn_),
    status(false),
    offMessage(offMessage_),
    stringRef(stringRef_),
    disableInputInOnState(disableInputInOnState_),
    canModifyRef(canModifyRef_)
{
    resourcePack = rPack_;
    texture = rPack_.texture;
    currentPhase = StartPhase;
    position = position_;
    iconScale = iconScale_;
    scale = scale_;
    style = styleOff_;
    message = message_;
    startDelay = strDelay_;
    endDelay = endDelay_;

    if(rPack_.font != nullptr)
    {
        text.setFont(*rPack_.font);
        text.setCharacterSize(styleOff_->getFontSize());
        text.setColor(styleOff_->getFontColor());
        text.setString(text_);
        text.setOrigin(text.getGlobalBounds().width / 2, text.getGlobalBounds().height / 2);
    }

    if(rPack_.texture != nullptr)
    {
        float scaleReal = style->getPhaseScale(currentPhase) * scale;

        sprite.setTexture(*rPack_.texture);
        sprite.setOrigin(sprite.getLocalBounds().width / 2.f, sprite.getLocalBounds().height / 2.f);
        sprite.setScale(scaleReal, scaleReal);
        sprite.setPosition(position_);
    }
    if(iconTextr_ != nullptr)
    {
        icon.setTexture(*iconTextr_);
        if(iconRect_ != sf::IntRect())
            icon.setTextureRect(iconRect_);

        icon.setOrigin(icon.getLocalBounds().width / 2.f, icon.getLocalBounds().height / 2.f);
    }

    Reset();
}

bool GUI::ToggleButton::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool feedback = Button::HandleInput(mousePos_, leftClick_, deltaTime, status && disableInputInOnState ? false : doInput);

    if(feedback)
    {
        status = !status;

        if(stringRef != nullptr && canModifyRef)
        {
            if(status)
                *stringRef = message;
            else *stringRef = offMessage;
        }
    }

    if(stringRef != nullptr)
    {
        if(*stringRef == message)
            status = true;
        else status = false;
    }

    if(status)
        style = styleOn;
    else style = styleOff;


    return feedback;
}

 std::string GUI::ToggleButton::GetMessage()
 {
     std::string msg = "";

     if(status)
        msg = message;
     else msg = offMessage;

     return msg;
 }

 void GUI::ToggleButton::SetStatus(bool status_)
 {
     status = status_;
 }

 bool GUI::ToggleButton::GetStatus()
 {
     return status;
 }

 void GUI::ToggleButton::Reset(std::string command)
 {
    if(command == "default-phase-reset")
    {
        if(lockState)
            currentPhase = LockedPhase;
        else currentPhase = DefaultPhase;
    }
    else
    {
        currentPhase = StartPhase;
        delayTime = startDelay;
    }

    if(command == "on-style-reset")
        status = true;
    else status = false;

    if(command == "on-style-default-phase-reset")
    {
        status = true;
        currentPhase = DefaultPhase;
    }

    if(style != nullptr)
    {
        float scale_ = style->getPhaseScale(currentPhase) * scale;

        sprite.setPosition(position + (style->getPhasePosition(currentPhase) * scale));
        sprite.setScale(scale_, scale_);
        sprite.setColor(sf::Color(255, 255, 255, style->getPhaseTransparency(currentPhase)));
        currentTransparency = style->getPhaseTransparency(currentPhase);
    }

    if(stringRef != nullptr)
    {
        if(*stringRef == message)
            status = true;
        else status = false;
    }
 }

 //Group
 GUI::Group::Group(bool enableSoftReset_, bool drawDisplayObjectsFirst_) :
     message(""),
     enableSoftReset(enableSoftReset_),
     drawDisplayObjectsFirst(drawDisplayObjectsFirst_)
 {
 }

 GUI::Group::~Group()
 {
     for(int a = 0; a < objects.size(); a++){
        auto str = objects[a]->GetMessage();
        delete objects[a];
     }

    for(int a = 0; a < displayObjects.size(); a++)
        delete displayObjects[a];
 }

 void GUI::Group::AddObject(GUI::Object* object_)
 {
     if(object_ != nullptr)
        objects.push_back(object_);
 }

 void GUI::Group::AddObject(GUI::DisplayObject* object_)
 {
     if(object_ != nullptr)
        displayObjects.push_back(object_);
 }

 void GUI::Group::Draw(sf::RenderTarget& target_)
 {
     if(drawDisplayObjectsFirst)
     {
         for(int a = 0; a < displayObjects.size(); a++)
            displayObjects[a]->Draw(target_);
         for(int a = 0; a < objects.size(); a++)
            objects[a]->Draw(target_);
     }
     else
     {
         for(int a = 0; a < objects.size(); a++)
            objects[a]->Draw(target_);
        for(int a = 0; a < displayObjects.size(); a++)
            displayObjects[a]->Draw(target_);
     }
 }

bool GUI::Group::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool return_ = false;

    bool prvCursorOnObjectStatus = isCursorOnObject;
    isCursorOnObject = false;
    for(int a = objects.size() - 1; a >= 0; a--)
    {
        if(objects[a]->HandleInput(mousePos_,leftClick_, deltaTime, isCursorOnObject || return_ ? false : doInput))
        {
            return_ = true;

            if(enableSoftReset)
            {
                for(int b = 0; b < objects.size(); b++)
                    if(b != a)
                        objects[b]->Reset("default-phase-reset");
            }

            message = objects[a]->GetMessage();
        }
    }
    if(isCursorOnObject == false)
        isCursorOnObject = prvCursorOnObjectStatus;

    for(auto displayObj : displayObjects)
        displayObj->Update(deltaTime);

    return return_;
}

std::string GUI::Group::GetMessage()
{
    std::string msg = message;
    message = "";

    return msg;
}

void GUI::Group::Reset(std::string command)
{
    for(int a = 0; a < objects.size(); a++)
        objects[a]->Reset(command);

    if(command == "default-phase-reset")
        for(int a = 0; a < displayObjects.size(); a++)
            displayObjects[a]->SetPhase(DefaultPhase);
    else for(int a = 0; a < displayObjects.size(); a++)
            displayObjects[a]->Reset();
}

void GUI::Group::SetInactive()
{
    for(int a = 0; a < objects.size(); a++)
        objects[a]->SetInactive();

    for(int a = 0; a < displayObjects.size(); a++)
        displayObjects[a]->SetInactive();
}

// Selection
GUI::Selection::Selection(Style* style_, ResourcePack rPack_, sf::Vector2f position_, float itemScale_, sf::Vector2f displaySize_, sf::FloatRect itemRect_,
                  float seperation_, float timeFactor_, Direction dir_, std::vector<ItemInfo>* itemList_) :
    currentItem(-1),
    styleOff(style_),
    position(position_),
    displaySize(displaySize_),
    itemRect(itemRect_),
    direction(dir_),
    resourcePack(rPack_),
    timeFactor(timeFactor_),
    itemScale(itemScale_)
{
    sf::Vector2f startPoint;

    viewPosition = displaySize_ / 2.f;
    view.setSize(displaySize_);

    if(dir_ == Vertical)
        {
            startPoint = sf::Vector2f(displaySize_.x / 2.f, itemRect_.height / 2.f);
            view.setCenter(displaySize_.x / 2.f, 0);
            seperation.y = seperation_;
        }
    else if(dir_ == Horizontal)
        {
            startPoint = sf::Vector2f(itemRect_.width / 2.f, displaySize_.y / 2.f);
            view.setCenter(0, displaySize_.y / 2.f);
            seperation.x = seperation_;
        }

    if(style_ != nullptr && itemList_!= nullptr)
    {
        std::vector<ItemInfo>& list_ = *itemList_;
        for(int a = 0; a < list_.size(); a++)
        {
            sf::Vector2f itemPos = startPoint + seperation * (float)a;
            Button* obj = new GUI::Button(style_, itemPos, itemScale_, rPack_, list_[a].message, list_[a].text, list_[a].iconTextr, list_[a].iconScale, list_[a].iconRect, list_[a].startDelay, list_[a].endDelay);
            obj->SetLock(list_[a].lockState);
            objectList.push_back(obj);
        }
    }
}

GUI::Selection::Selection(Style* styleOff_, Style* styleOn_, ResourcePack rPack_, sf::Vector2f position_, float itemScale_, sf::Vector2f displaySize_, sf::FloatRect itemRect_,
                        float seperation_, float timeFactor_, Direction dir_, std::vector<ItemInfo>* itemList_) :
    currentItem(-1),
    styleOff(styleOff_),
    styleOn(styleOn_),
    position(position_),
    displaySize(displaySize_),
    itemRect(itemRect_),
    direction(dir_),
    resourcePack(rPack_),
    timeFactor(timeFactor_),
    itemScale(itemScale_)
{
    viewPosition = displaySize_ / 2.f;
    view.setSize(displaySize_);

    sf::Vector2f startPoint;

    if(dir_ == Vertical)
        {
            startPoint = sf::Vector2f(displaySize_.x / 2.f, itemRect_.height / 2.f);
            view.setCenter(displaySize_.x / 2.f, 0);
            seperation.y = seperation_;
        }
    else if(dir_ == Horizontal)
        {
            startPoint = sf::Vector2f(itemRect_.width / 2.f, displaySize_.y / 2.f);
            view.setCenter(0, displaySize_.y / 2.f);
            seperation.x = seperation_;
        }

    if(styleOff_ != nullptr && styleOn_ != nullptr && itemList_!= nullptr)
    {
        std::vector<ItemInfo>& list_ = *itemList_;
        for(int a = 0; a < list_.size(); a++)
        {
            sf::Vector2f itemPos = startPoint + seperation * (float)a;
            Button* obj = new GUI::ToggleButton(styleOff_, styleOn_, itemPos, itemScale_, rPack_, list_[a].message, list_[a].offMessage, list_[a].text, list_[a].iconTextr, list_[a].iconScale, list_[a].iconRect, list_[a].startDelay, list_[a].endDelay);
            obj->SetLock(list_[a].lockState);
            objectList.push_back(obj);
        }
    }
}

GUI::Selection::~Selection()
{
    if(objectList.size() > 0)
    {
        for(int a = 0; a < objectList.size(); a++)
            delete objectList[a];
    }
    objectList.clear();
}

void GUI::Selection::Scroll(float amount_)
{
    float varriable = 0;
    float fixedMin = 0;
    float fixedMax = 0;

    if(direction == Vertical)
    {
        varriable = viewPosition.y;
        fixedMin = displaySize.y / 2.f;
        float lenght = itemRect.height + seperation.y * (objectList.size() - 1);

        if(lenght <= displaySize.y)
            fixedMax = displaySize.y / 2.f;
        else
            fixedMax = lenght - displaySize.y / 2.f;
    }

    else if(direction == Horizontal)
    {
        varriable = viewPosition.x;
        fixedMin = displaySize.x / 2.f;
        float lenght = itemRect.width + seperation.x * (objectList.size() - 1);

        if(lenght <= displaySize.x)
            fixedMax = displaySize.x / 2.f;
        else
            fixedMax = lenght - displaySize.x / 2.f;
    }

    viewPosition += seperation * amount_;

    if(direction == Vertical)
    {
        if(viewPosition.y > fixedMax)
            viewPosition.y  = fixedMax;
        if(viewPosition.y < fixedMin)
            viewPosition.y = fixedMin;
    }
    else if(direction == Horizontal)
    {
        if(viewPosition.x > fixedMax)
            viewPosition.x  = fixedMax;
        if(viewPosition.x < fixedMin)
            viewPosition.x = fixedMin;
    }
}

void GUI::Selection::MoveForeward()
{
    Scroll(1.f);
}

void GUI::Selection::MoveBackward()
{
    Scroll(-1.f);
}

void GUI::Selection::Draw(sf::RenderTarget& target_)
{
    /*sf::View prvView = target_.getView();

    float screenX = target_.getDefaultView().getSize().x;
    float screenY = target_.getDefaultView().getSize().y;
    float prvX = prvView.getCenter().x - prvView.getSize().x / 2.f;
    float prvY = prvView.getCenter().y - prvView.getSize().y / 2.f;


    sf::FloatRect port;
    port.left = (position.x - view.getSize().x / 2.f - prvX) / screenX;
    port.top = (position.y - view.getSize().y / 2.f - prvY) / screenY;
    port.width = view.getSize().x / screenX;
    port.height = view.getSize().y / screenY;

    view.setViewport(port);

    target_.setView(view);
    for(int a = 0; a < objectList.size(); a++)
        objectList[a]->Draw(target_);

    target_.setView(prvView);*/

    const sf::View prvView = target_.getView();
    sf::Vector2f prvViewTopLeft(prvView.getCenter().x - prvView.getSize().x / 2.f, prvView.getCenter().y - prvView.getSize().y / 2.f);

    sf::FloatRect refRect(position.x - view.getSize().x / 2.f, position.y - view.getSize().y / 2.f, view.getSize().x, view.getSize().y);
    sf::FloatRect targetRect(prvViewTopLeft.x, prvViewTopLeft.y, prvView.getSize().x, prvView.getSize().y);
    sf::FloatRect intersectingRect;
    if(targetRect.intersects(refRect, intersectingRect))
    {
        sf::Vector2f diffTopLeft(intersectingRect.left - refRect.left, intersectingRect.top - refRect.top);
        sf::View refView(view.getCenter() - view.getSize() / 2.f + diffTopLeft + sf::Vector2f(intersectingRect.width / 2.f, intersectingRect.height / 2.f), sf::Vector2f(intersectingRect.width, intersectingRect.height));

        sf::FloatRect prvPort = prvView.getViewport();
        sf::FloatRect refPort(prvPort.left + prvPort.width * (intersectingRect.left - targetRect.left) / targetRect.width, prvPort.top + prvPort.height * (intersectingRect.top - targetRect.top) / targetRect.height, prvPort.width * intersectingRect.width / targetRect.width, prvPort.height * intersectingRect.height / targetRect.height);
        refView.setViewport(refPort);

        target_.setView(refView);
        for(int a = 0; a < objectList.size(); a++)
            objectList[a]->Draw(target_);

        target_.setView(prvView);
    }
}

bool GUI::Selection::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool return_ = false;
    sf::FloatRect rect(position.x - displaySize.x / 2.f, position.y - displaySize.y / 2.f, displaySize.x, displaySize.y);

    sf::Vector2i relativeMousePos = mousePos_  - (sf::Vector2i)(position - displaySize / 2.f)
    + (sf::Vector2i)(view.getCenter() - view.getSize() / 2.f);

    if(rect.intersects(sf::FloatRect(mousePos_.x , mousePos_.y, 1 , 1)))
    {
        isCursorOnObject = true;

        if(mouseScrollDelta != 0.f)
        {
            Scroll(mouseScrollDelta);
            mouseScrollDelta = 0.f;
        }

        for(int a = 0; a < objectList.size(); a++)
        {
            if(objectList[a]->HandleInput(relativeMousePos, leftClick_, deltaTime, doInput))
            {
                return_ = true;
                message = objectList[a]->GetMessage();

                if(currentItem >= 0 && currentItem != a)
                    objectList[currentItem]->Reset("default-phase-reset");

                if(currentItem == a)
                    currentItem = -1;
                else currentItem = a;
            }
        }
    }
    else
    {
        bool prvCursorOnObjectStatus = isCursorOnObject;
        for(int a = 0; a < objectList.size(); a++)
            objectList[a]->HandleInput(relativeMousePos, leftClick_, deltaTime, false);
        isCursorOnObject = prvCursorOnObjectStatus; //not changing it because cursor is outside view pan
    }


    if(view.getCenter() != viewPosition)
    {
        sf::Vector2f dist = viewPosition - view.getCenter();
        float magn = sqrt(dist.x * dist.x + dist.y * dist.y);
        if(magn > 0)
        {
            sf::Vector2f unitVec = dist / magn;

            if(timeFactor <= 0)
                view.move(unitVec * magn * deltaTime);
            else view.move(unitVec * magn * deltaTime / timeFactor);

            dist = viewPosition - view.getCenter();
            if(dist.x * unitVec.x + dist.y * unitVec.y < 0)
                view.setCenter(viewPosition);
        }
    }

    return return_;
}

void GUI::Selection::AddItem(ItemInfo item_)
{
    sf::Vector2f startPoint;

    if(direction == Vertical)
        startPoint = sf::Vector2f(displaySize.x / 2.f, itemRect.height / 2.f);

    else if(direction == Horizontal)
        startPoint = sf::Vector2f(itemRect.width / 2.f, displaySize.y / 2.f);


    if(styleOff != nullptr)
    {
        sf::Vector2f itemPos = startPoint + seperation * (float)objectList.size();

        if(styleOff != nullptr && styleOn != nullptr)
            objectList.push_back(new GUI::ToggleButton(styleOff, styleOn, itemPos, itemScale, resourcePack, item_.message, item_.offMessage, item_.text,
                                item_.iconTextr, item_.iconScale, item_.iconRect, item_.startDelay, item_.endDelay));
        else if(styleOff != nullptr)
            objectList.push_back(new GUI::Button(styleOff, itemPos, itemScale, resourcePack, item_.message, item_.text,
                                item_.iconTextr, item_.iconScale, item_.iconRect, item_.startDelay, item_.endDelay));
    }
}

void GUI::Selection::AddItem(std::vector<ItemInfo>* itemList_)
{
    if(itemList_ != nullptr)
    {
        std::vector<ItemInfo>& list_ = *itemList_;
        for(int a = 0; a < list_.size(); a++)
            AddItem(list_[a]);
    }
}

void GUI::Selection::SetViewPosition(float ratio_, bool doTransmit)
{
    if(direction == Horizontal)
    {
        float nodeMax = itemRect.width + seperation.x * (objectList.size() - 1) - view.getSize().x;
        if(nodeMax >= 0)
            viewPosition = sf::Vector2f(view.getSize().x / 2.f + nodeMax * ratio_,
                                        view.getCenter().y);
        if(doTransmit == false)
            view.setCenter(viewPosition);
    }
    else
    {
        float nodeMax = itemRect.height + seperation.y * (objectList.size() - 1) - view.getSize().y;
        if(nodeMax >= 0)
            viewPosition = sf::Vector2f(view.getCenter().x,
                                        view.getSize().y / 2.f + nodeMax * ratio_);

        if(doTransmit == false)
            view.setCenter(viewPosition);
    }
}

float GUI::Selection::GetViewPosition()
{
    float ratio_ = 0;

    if(direction == Horizontal)
    {
        float nodeMax = itemRect.width + seperation.x * (objectList.size() - 1) - view.getSize().x;
        ratio_ = (view.getCenter().x - view.getSize().x / 2.f) / nodeMax;
    }
    else
    {
        float nodeMax = itemRect.height + seperation.y * (objectList.size() - 1) - view.getSize().y;
        ratio_ = (view.getCenter().y - view.getSize().y / 2.f) / nodeMax;
    }
    return ratio_;
}

std::string GUI::Selection::GetMessage()
{
    std::string message_ = message;
    message = "";

    return message_;
}

void GUI::Selection::Reset(std::string command)
{
    for(int a = 0; a < objectList.size(); a++)
        objectList[a]->Reset(command);
    SetViewPosition(0);
    currentItem = -1;
}

void GUI::Selection::SetInactive()
{
    for(int a = 0; a < objectList.size(); a++)
        objectList[a]->SetInactive();
}

//ObjectSelection
GUI::ObjectSelection::ObjectSelection(sf::Vector2f position_, sf::Vector2f displaySize_, float scrollLength_, float moveFactor_, float transmitFactor_, Direction dir_, bool enableSoftReset_, bool shouldResetView_, bool canSetCursorOnObject_, bool respondToScroll_) :
    GUI::Group::Group(enableSoftReset_),
    transmitFactor(transmitFactor_),
    scrollLength(scrollLength_),
    scrollPositionFactor(0.f),
    originPosition(sf::Vector2f(displaySize_.x / 2.f, displaySize_.y / 2.f)),
    moveFactor(moveFactor_),
    position(position_),
    shouldResetView(shouldResetView_),
    canSetCursorOnObject(canSetCursorOnObject_),
    respondToScroll(respondToScroll_)
{
    if(dir_ == Horizontal)
        direction = sf::Vector2f(1, 0);
    else direction = sf::Vector2f(0, 1);

    view.setSize(displaySize_);
    view.setCenter(originPosition);
}

bool GUI::ObjectSelection::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool return_ = false;;

    sf::FloatRect rect(position.x - view.getSize().x / 2.f, position.y - view.getSize().y / 2.f, view.getSize().x, view.getSize().y);
    sf::Vector2i relativeMousePos = mousePos_  - (sf::Vector2i)(position - view.getSize() / 2.f) + (sf::Vector2i)(view.getCenter() - view.getSize() / 2.f);

    if(rect.intersects(sf::FloatRect(mousePos_.x , mousePos_.y, 1 , 1)))
    {
        return_ = GUI::Group::HandleInput(relativeMousePos, leftClick_, deltaTime, doInput);

        if(canSetCursorOnObject)
            isCursorOnObject = true;
        if(respondToScroll && mouseScrollDelta != 0.f)
        {
            Scroll(mouseScrollDelta);
            mouseScrollDelta = 0.f;
        }
    }
    else
    {
        bool prvCursorOnObjectStatus = isCursorOnObject;
        return_ = GUI::Group::HandleInput(relativeMousePos, leftClick_, deltaTime, false);
        isCursorOnObject = prvCursorOnObjectStatus; //not changing it because cursor is outside view pan
    }

    sf::Vector2f viewPosition = originPosition + direction * scrollLength * scrollPositionFactor;
    viewPosition.x = (int)viewPosition.x;
    viewPosition.y = (int)viewPosition.y;
    if(view.getCenter() != viewPosition)
    {
        sf::Vector2f dist = viewPosition - view.getCenter();
        float magn = sqrt(dist.x * dist.x + dist.y * dist.y);
        if(magn > 0)
        {
            sf::Vector2f unitVec = dist / magn;

            if(transmitFactor <= 0)
                view.move(unitVec * magn);
            else
            {
                float moveMagn = magn * deltaTime / transmitFactor;
                if(moveMagn < MIN_TRANSMISSION_VALUE)
                    moveMagn = MIN_TRANSMISSION_VALUE;

                view.move(unitVec * moveMagn);
            }

            dist = viewPosition - view.getCenter();
            if(dist.x * unitVec.x + dist.y * unitVec.y < 0)
                view.setCenter(viewPosition);
        }
    }

    return return_;
}

void GUI::ObjectSelection::Draw(sf::RenderTarget& target_)
{
    /*sf::View prvView = target_.getView();

    float screenX = target_.getDefaultView().getSize().x;
    float screenY = target_.getDefaultView().getSize().y;
    float prvX = prvView.getCenter().x - prvView.getSize().x / 2.f;
    float prvY = prvView.getCenter().y - prvView.getSize().y / 2.f;


    sf::FloatRect port;
    port.left = (position.x - view.getSize().x / 2.f - prvX) / screenX;
    port.top = (position.y - view.getSize().y / 2.f - prvY) / screenY;
    port.width = view.getSize().x / screenX;
    port.height = view.getSize().y / screenY;

    view.setViewport(port);

    target_.setView(view);
    GUI::Group::Draw(target_);
    target_.setView(prvView);*/

    const sf::View prvView = target_.getView();
    sf::Vector2f prvViewTopLeft(prvView.getCenter().x - prvView.getSize().x / 2.f, prvView.getCenter().y - prvView.getSize().y / 2.f);

    sf::FloatRect refRect(position.x - view.getSize().x / 2.f, position.y - view.getSize().y / 2.f, view.getSize().x, view.getSize().y);
    sf::FloatRect targetRect(prvViewTopLeft.x, prvViewTopLeft.y, prvView.getSize().x, prvView.getSize().y);
    sf::FloatRect intersectingRect;
    if(targetRect.intersects(refRect, intersectingRect))
    {
        sf::Vector2f diffTopLeft(intersectingRect.left - refRect.left, intersectingRect.top - refRect.top);

        sf::FloatRect prvPort = prvView.getViewport();
        sf::FloatRect refPort(prvPort.left + prvPort.width * (intersectingRect.left - targetRect.left) / targetRect.width, prvPort.top + prvPort.height * (intersectingRect.top - targetRect.top) / targetRect.height, prvPort.width * intersectingRect.width / targetRect.width, prvPort.height * intersectingRect.height / targetRect.height);

        sf::View refView(view.getCenter() - view.getSize() / 2.f + diffTopLeft + sf::Vector2f(intersectingRect.width / 2.f, intersectingRect.height / 2.f), sf::Vector2f(intersectingRect.width, intersectingRect.height));
        refView.setViewport(refPort);

        target_.setView(refView);
        GUI::Group::Draw(target_);
        target_.setView(prvView);
    }
}

void GUI::ObjectSelection::SetScrollLength(float scrollLength_)
{
    scrollLength = scrollLength_;
}

void GUI::ObjectSelection::SetPosition(const sf::Vector2f& position_)
{
    position = position_;
}

const sf::Vector2f& GUI::ObjectSelection::GetPosition()
{
    return position;
}

void GUI::ObjectSelection::Scroll(float amount_)
{
    scrollPositionFactor += amount_ * moveFactor;

    if(scrollPositionFactor > 1)
        scrollPositionFactor = 1.f;
    else if(scrollPositionFactor < 0.f)
        scrollPositionFactor = 0.f;
}

void GUI::ObjectSelection::MoveForeward()
{
    Scroll(1.f);
}

void GUI::ObjectSelection::MoveBackward()
{
    Scroll(-1.f);
}

void GUI::ObjectSelection::SetViewPosition(float ratio_, bool doTransmit)
{
    scrollPositionFactor = ratio_;

    if(scrollPositionFactor > 1)
        scrollPositionFactor = 1.f;
    else if(scrollPositionFactor < 0.f)
        scrollPositionFactor = 0.f;

    if(doTransmit == false)
        view.setCenter(originPosition + direction * scrollLength * scrollPositionFactor);
}

void GUI::ObjectSelection::SetDisplaySize(const sf::Vector2f& displaySize_, const sf::Vector2f& originPos_, const float& scrollLength_, bool fixedToScrollPos_)
{
    view.setSize(displaySize_);
    originPosition = originPos_;
    scrollLength = scrollLength_;

    if(fixedToScrollPos_)
        view.setCenter(originPosition + direction * scrollLength * scrollPositionFactor);
}

const sf::Vector2f& GUI::ObjectSelection::GetDisplaySize()
{
    return view.getSize();
}

float GUI::ObjectSelection::GetViewPosition()
{
    if(scrollLength > 0)
    {
        sf::Vector2f diff = view.getCenter() - originPosition;
        float diffMagn = sqrt(diff.x * diff.x + diff.y * diff.y);
        return diffMagn / scrollLength;
    }
    return 0.f;
}

void GUI::ObjectSelection::SetViewCenter(const sf::Vector2f& center_, bool alsoSetOriginToThisCenter_)
{
    view.setCenter(center_);
    if(alsoSetOriginToThisCenter_)
        originPosition = center_;
}

const sf::Vector2f& GUI::ObjectSelection::GetViewCenter()
{
    return view.getCenter();
}

const sf::Vector2f& GUI::ObjectSelection::GetViewPositionOrigin()
{
    return originPosition;
}

void GUI::ObjectSelection::SetViewPositionOrigin(const sf::Vector2f& positionOrigin_)
{
    originPosition = positionOrigin_;
}

void GUI::ObjectSelection::Reset(std::string command)
{
    GUI::Group::Reset(command);
    if(shouldResetView)
        SetViewPosition(0);
}

//Degree
GUI::Degree::Degree(Direction dir_, float length_, float width_, sf::Vector2f pos_, sf::Vector2f nodeSize_, sf::Color lineColr_, sf::Color nodeColr_,
                    sf::Color extentColr_, StateType state_, float startDelay_, float endDelay_, float transmitFactor_, float* rateRef_) :
    direction(dir_),
    state(state_),
    active(false),
    rateRef(rateRef_),
    isClicked(false),
    keyIndex(false),
    startDelay(startDelay_),
    endDelay(endDelay_),
    wait(0),
    transparencyFactor(0),
    lineColor(lineColr_),
    nodeColor(nodeColr_),
    extentColor(extentColr_),
    isInActive(false),
    transmissionFactor(transmitFactor_)
{
    if(dir_ == Horizontal)
        line.setSize(sf::Vector2f(length_, width_));
    else
        line.setSize(sf::Vector2f(width_, length_));

    lineExtent.setPosition(line.getGlobalBounds().left, line.getGlobalBounds().top);

    sf::RectangleShape* rectangle = new sf::RectangleShape(nodeSize_);

    line.setFillColor(lineColr_);
    rectangle->setFillColor(nodeColr_);
    lineExtent.setFillColor(extentColr_);

    line.setOrigin(line.getSize().x / 2.f, line.getSize().y / 2.f);
    rectangle->setOrigin(nodeSize_.x / 2.f, nodeSize_.y / 2.f);

    line.setPosition(pos_);
    lineExtent.setPosition(line.getGlobalBounds().left, line.getGlobalBounds().top);

    node = rectangle;
}

GUI::Degree::Degree(Direction dir_, float length_, float width_, sf::Vector2f pos_, float nodeRadius_, sf::Color lineColr_, sf::Color nodeColr_,
                    sf::Color extentColr_, StateType state_, float startDelay_, float endDelay_, float transmitFactor_, float* rateRef_) :
    direction(dir_),
    state(state_),
    active(false),
    rateRef(rateRef_),
    isClicked(false),
    keyIndex(false),
    startDelay(startDelay_),
    endDelay(endDelay_),
    wait(0),
    transparencyFactor(0),
    lineColor(lineColr_),
    nodeColor(nodeColr_),
    extentColor(extentColr_),
    isInActive(false),
    transmissionFactor(transmitFactor_)
{
    if(dir_ == Horizontal)
        line.setSize(sf::Vector2f(length_, width_));
    else
        line.setSize(sf::Vector2f(width_, length_));

    lineExtent.setPosition(line.getGlobalBounds().left, line.getGlobalBounds().top);

    sf::CircleShape* circle = new sf::CircleShape(nodeRadius_);

    line.setFillColor(lineColr_);
    circle->setFillColor(nodeColr_);
    lineExtent.setFillColor(extentColr_);

    line.setOrigin(line.getSize().x / 2.f, line.getSize().y / 2.f);
    circle->setOrigin(nodeRadius_, nodeRadius_);

    line.setPosition(pos_);
    lineExtent.setPosition(line.getGlobalBounds().left, line.getGlobalBounds().top);

    node = circle;
}

GUI::Degree::~Degree()
{
    if(node != nullptr)
        delete node;
}

void GUI::Degree::SetInactive()
{
    wait = endDelay;
    isInActive = true;
}

void GUI::Degree::Reset(std::string command)
{
    isInActive = false;
    wait = startDelay;
    transparencyFactor = 0;
}

void GUI::Degree::Draw(sf::RenderTarget& target_)
{
    lineExtent.setFillColor(sf::Color(extentColor.r, extentColor.g, extentColor.b, transparencyFactor * extentColor.a));
    line.setFillColor(sf::Color(lineColor.r, lineColor.g, lineColor.b, transparencyFactor * lineColor.a));
    if(node != nullptr)
        node->setFillColor(sf::Color(nodeColor.r, nodeColor.g, nodeColor.b, transparencyFactor * nodeColor.a));

    target_.draw(line);
    target_.draw(lineExtent);
    if(node != nullptr)
        target_.draw(*node);
}

bool GUI::Degree::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    sf::FloatRect mouseRect(mousePos_.x, mousePos_.y, 1, 1);
    if(line.getGlobalBounds().intersects(mouseRect))
        isCursorOnObject = true;
    else if(node != nullptr && node->getGlobalBounds().intersects(mouseRect))
        isCursorOnObject = true;

    if(rateRef != nullptr)
        SetRatio(*rateRef);

    if(wait > 0)
    {
        wait -= deltaTime;
        if(wait < 0)
            wait = 0;
    }
    else
    {
        sf::FloatRect mouseRect(mousePos_.x, mousePos_.y, 1, 1);
        bool keyHold = sf::Mouse::isButtonPressed(sf::Mouse::Left);

        if(keyIndex == false && keyHold)
            if(node->getGlobalBounds().intersects(mouseRect) || line.getGlobalBounds().intersects(mouseRect))
                isClicked = true;

        if(keyHold)
            keyIndex = true;

        if(doInput)
        {
            if(isClicked == true && keyHold)
            {
                if(active == false)
                {
                    if(node->getGlobalBounds().intersects(mouseRect))
                    {
                        active = true;

                        sf::Vector2f refPos = sf::Vector2f(mousePos_) - sf::Vector2f(node->getGlobalBounds().left, node->getGlobalBounds().top);

                        if(direction == Horizontal)
                            node->setOrigin(refPos.x, node->getLocalBounds().height / 2.f);
                        else node->setOrigin(node->getLocalBounds().width / 2.f, refPos.y);
                    }
                    else if(state == ClickAnyPart && line.getGlobalBounds().intersects(mouseRect))
                    {
                        active = true;
                        node->setOrigin(node->getLocalBounds().width / 2.f, node->getLocalBounds().height / 2.f);
                    }
                }
            }
            else active = false;

            if(keyHold == false)
                keyIndex = isClicked = false;

            if(active)
            {
                if(direction == Horizontal)
                    node->setPosition(mousePos_.x, line.getPosition().y);
                else node->setPosition(line.getPosition().x, mousePos_.y);
            }

            if(direction == Horizontal)
            {
                float nodeMax = line.getGlobalBounds().left + line.getGlobalBounds().width - (node->getGlobalBounds().width - node->getOrigin().x);
                float nodeMin = line.getGlobalBounds().left + node->getOrigin().x;

                if(node->getPosition().x >  nodeMax)
                    node->setPosition(nodeMax, line.getPosition().y);
                if(node->getPosition().x < nodeMin)
                    node->setPosition(nodeMin, line.getPosition().y);

                lineExtent.setSize(sf::Vector2f(node->getPosition().x - line.getGlobalBounds().left, line.getSize().y));
            }
            else
            {
                float nodeMax = line.getGlobalBounds().top + line.getGlobalBounds().height - (node->getGlobalBounds().height - node->getOrigin().y);
                float nodeMin = line.getGlobalBounds().top + node->getOrigin().y;

                if(node->getPosition().y > nodeMax)
                    node->setPosition(line.getPosition().x,nodeMax);
                if(node->getPosition().y < nodeMin)
                    node->setPosition(line.getPosition().x, nodeMin);

                lineExtent.setSize(sf::Vector2f(line.getSize().x, node->getPosition().y - line.getGlobalBounds().top));
            }

            float transValue = 0;
            if(isInActive == false)
                transValue = 1;

            if(transmissionFactor > 0)
            {
                if(transparencyFactor != transValue)
                {
                    float difference = transValue - transparencyFactor;
                    if(difference != 0)
                    {
                        float transmitFactor = deltaTime / transmissionFactor;
                        if(transmitFactor < MIN_TRANSMISSION_SCALE)
                            transmitFactor = MIN_TRANSMISSION_SCALE;

                        float add = difference * transmitFactor;
                        transparencyFactor += add;
                    }
                }
            }
            else transparencyFactor = transValue;

            if(rateRef != nullptr)
                *rateRef = GetRatio();
        }
    }

    return active;
}

float GUI::Degree::GetRatio()
{
    float ratio_ = 0.f;
    if(direction == Horizontal)
    {
        float nodeMax = line.getGlobalBounds().width - node->getGlobalBounds().width;
        float ratioPos = node->getGlobalBounds().left - line.getGlobalBounds().left;
        ratio_ = ratioPos / nodeMax;
    }
    else
    {
        float nodeMax = line.getGlobalBounds().height - node->getGlobalBounds().height;
        float ratioPos = node->getGlobalBounds().top - line.getGlobalBounds().top;
        ratio_ = ratioPos / nodeMax;
    }
    return ratio_;
}

void GUI::Degree::SetRatio(float ratio_)
{
    node->setOrigin(node->getLocalBounds().width / 2.f, node->getLocalBounds().height / 2.f);

    if(direction == Horizontal)
    {
        float nodeMax = line.getGlobalBounds().width - node->getGlobalBounds().width;
        node->setPosition(line.getGlobalBounds().left + node->getGlobalBounds().width / 2.f + nodeMax * ratio_,
                          line.getPosition().y);
    }
    else
    {
        float nodeMax = line.getGlobalBounds().height - node->getGlobalBounds().height;
        node->setPosition(line.getPosition().x,
                          line.getGlobalBounds().top + node->getGlobalBounds().height / 2.f + nodeMax * ratio_);
    }
}

//Navigator
GUI::Navigator::Navigator(Button* upButton_, Button* downButton_, Degree* slidder_, AbstractSelection* selection_) :
    upButton(upButton_),
    downButton(downButton_),
    slidder(slidder_),
    selection(selection_)
{
}

bool GUI::Navigator::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool return_= false;

    if(selection != nullptr)
        if(selection->HandleInput(mousePos_, leftClick_, deltaTime, doInput))
        {
            message = selection->GetMessage();
            return_ = true;
        }

    if(upButton != nullptr)
        if(upButton->HandleInput(mousePos_, leftClick_, deltaTime, doInput) && selection != nullptr)
        {
            selection->MoveBackward();
            return_ = true;
        }

    if(downButton != nullptr)
        if(downButton->HandleInput(mousePos_, leftClick_, deltaTime, doInput) && selection != nullptr)
        {
            selection->MoveForeward();
            return_ = true;
        }

    if(slidder != nullptr)
    {
        if(slidder->HandleInput(mousePos_, leftClick_, deltaTime, doInput) && selection != nullptr)
        {
            selection->SetViewPosition(slidder->GetRatio());
            return_ = true;
        }
        else if(selection != nullptr)
            slidder->SetRatio(selection->GetViewPosition());
    }

    return return_;
}

GUI::Navigator::~Navigator()
{
    if(upButton != nullptr)
        delete upButton;
    if(downButton != nullptr)
        delete downButton;
    if(slidder != nullptr)
        delete slidder;
    if(selection != nullptr)
        delete selection;
}

void GUI::Navigator::Draw(sf::RenderTarget& target_)
{
    if(selection != nullptr)
        selection->Draw(target_);
    if(slidder != nullptr)
        slidder->Draw(target_);
    if(upButton != nullptr)
        upButton->Draw(target_);
    if(downButton != nullptr)
        downButton->Draw(target_);
}

void GUI::Navigator::Reset(std::string command)
{
    if(upButton != nullptr)
        upButton->Reset(command);
    if(downButton != nullptr)
        downButton->Reset(command);
    if(slidder != nullptr)
        slidder->Reset(command);
    if(selection != nullptr)
        selection->Reset(command);
}

void GUI::Navigator::SetInactive()
{
    if(upButton != nullptr)
        upButton->SetInactive();
    if(downButton != nullptr)
        downButton->SetInactive();
    if(slidder != nullptr)
        slidder->SetInactive();
    if(selection != nullptr)
        selection->SetInactive();
}

std::string GUI::Navigator::GetMessage()
{
    std::string msg = message;
    message = "";
    return msg;
}

// Layer
GUI::Layer::Layer() :
    GUI::Group(false)
{
}

GUI::Layer::~Layer()
{
}

// Session
GUI::Session::Session() :
    waitTime(0.f),
    moveBack(0.f),
    isEmpty(false)
{
}

GUI::Session::~Session()
{
    Clear();

    for(auto layer_ = layers.begin(); layer_ != layers.end(); ++layer_)
    {
        delete layer_->second;
    }
    layers.clear();
}

void GUI::Session::Create(std::string name_)
{
    auto findLayer_ = layers.find(name_);
    if(findLayer_ == layers.end())
    {
        Layer* layer_ = new GUI::Layer();
        layers.insert(std::make_pair(name_, layer_));
    }
}

void GUI::Session::AddObject(Object* object, std::string name_)
{
    if(object != nullptr)
    {
        auto layer_ = layers.find(name_);
        if(layer_ != layers.end())
            layer_->second->AddObject(object);
        else delete object;
    }
}

void GUI::Session::AddObject(DisplayObject* object, std::string name_)
{
    auto layer_ = layers.find(name_);
    if(layer_ != layers.end())
        layer_->second->AddObject(object);
    else delete object;
}

void GUI::Session::TransmitTo(std::string name_, float time_)
{
    if(time_ > 0.f)
    {
        for(int a = 0; a < layerList.size(); a++)
            layerList[a]->SetInactive();

        if(waitLayer != "" && waitLayer != name_)
            TransmitTo(waitLayer);
        waitLayer = name_;
        waitTime = time_;
    }
    else
    {
        waitTime = 0;
        waitLayer = "";
        layerList.clear();
        currentList.clear();

        auto layer_ = layers.find(name_);
        if(layer_ != layers.end())
        {
            layer_->second->Reset();
            layerList.push_back(layer_->second);
            currentList.push_back(layer_->first);
        }
    }
}

void GUI::Session::AddInFront(std::string name_)
{
    auto layer_ = layers.find(name_);
    if(layer_ != layers.end())
    {
        layer_->second->Reset();
        layerList.push_back(layer_->second);
        currentList.push_back(layer_->first);
    }
}

void GUI::Session::MoveBack(float wait_)
{
    if(wait_ > 0)
    {
        SetInactive(currentList[currentList.size() - 1]);
        waitTime = wait_;
        moveBack = true;
    }
    else if(layerList.size() > 0)
    {
        layerList[layerList.size() - 1]->Reset();
        layerList.erase(layerList.begin() + layerList.size() - 1);
        currentList.erase(currentList.begin() + currentList.size() - 1);
    }
}

std::string GUI::Session::GetCurrentLayer()
{
    std::string curLayer = "";

    if(currentList.size() > 0)
        curLayer = currentList[currentList.size() - 1];

    return curLayer;

}

void GUI::Session::Clear()
{
    layerList.clear();
    currentList.clear();
}

void GUI::Session::Draw(sf::RenderTarget& target_)
{
    for(int a = 0; a < layerList.size(); a++)
    {
        layerList[a]->Draw(target_);
    }
}

bool GUI::Session::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool return_ = false;

    if(waitTime > 0)
    {
        waitTime -= deltaTime;

        if(waitTime <= 0)
        {
            if(moveBack)
            {
                MoveBack();
                moveBack = false;
            }
            else
            {
                TransmitTo(waitLayer);
                waitLayer = "";
            }

            waitTime = 0.f;
        }
    }
    else if(layerList.size() == 0 && isEmpty == false)
    {
        return_ = true;
        isEmpty = true;
    }

    for(int a = 0; a < layerList.size(); a++)
    {
        isEmpty = false;
        return_ = layerList[a]->HandleInput(mousePos_, leftClick_, deltaTime, a == layerList.size() - 1 ? doInput : false);
    }

    return return_;
}

std::string GUI::Session::GetMessage()
{
    std::string message_ = "";

    if(layerList.size() > 0)
        message_ = layerList[layerList.size() - 1]->GetMessage();
    else message_ = emptyListMessage;

    return message_;
}

void GUI::Session::Reset(std::string command)
{
    for(auto layerObj : layers)
        layerObj.second->Reset(command);
}

void GUI::Session::ResetActiveLayers(std::string command)
{
    for(auto layerObj : layerList)
        layerObj->Reset(command);
}

void GUI::Session::SetInactive()
{
    for(auto itr = layers.begin(); itr != layers.end(); itr++)
        itr->second->SetInactive();
}

void GUI::Session::SetInactive(std::string name_)
{
    auto layer_ = layers.find(name_);
    if(layer_ != layers.end())
        layer_->second->SetInactive();
}

void GUI::Session::SetEmptyMessage(std::string message_)
{
    emptyListMessage = message_;
}
