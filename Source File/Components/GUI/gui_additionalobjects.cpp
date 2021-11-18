#include "GUI/gui_additionalobjects.h"
#include "public_functions.h"
#include <math.h>
#include <iostream>

//ShapeButton
GUI::ShapeButton::ShapeButton(Style* style_, sf::Vector2f rectangleSize_, sf::Color defaultColor_, sf::Color highlightColor_, sf::Color pressedColor_, bool colorTransmit_,
                              sf::Vector2f position_, ResourcePack rPack_ , std::string message_,
                              std::string text_, sf::Texture* iconTextr_, float iconScale_, sf::IntRect iconRect_,
                              float strDelay_, float endDelay_) :
    Button(style_, position_, 1, rPack_, message_, text_, iconTextr_, iconScale_, iconRect_, strDelay_, endDelay_),
    defaultColor(defaultColor_),
    highlightColor(highlightColor_),
    pressedColor(pressedColor_),
    colorTransmission(colorTransmit_),
    rColor(defaultColor_.r),
    gColor(defaultColor_.g),
    bColor(defaultColor_.b),
    aColor(defaultColor_.a)
{
    sf::RectangleShape* rectangle = new sf::RectangleShape;
    rectangle->setFillColor(defaultColor_);
    rectangle->setSize(rectangleSize_);
    rectangle->setOrigin(rectangleSize_ / 2.f);

    shape = rectangle;
    sprite.setTextureRect(sf::IntRect(0, 0, rectangleSize_.x, rectangleSize_.y));
    sprite.setOrigin(rectangleSize_ / 2.f);
}

GUI::ShapeButton::ShapeButton(Style* style_, float circleRadius_, sf::Color defaultColor_, sf::Color highlightColor_, sf::Color pressedColor_, bool colorTransmit_,
                              sf::Vector2f position_, ResourcePack rPack_ , std::string message_,
                              std::string text_, sf::Texture* iconTextr_, float iconScale_, sf::IntRect iconRect_,
                              float strDelay_, float endDelay_) :
    Button(style_, position_, 1, rPack_, message_, text_, iconTextr_, iconScale_, iconRect_, strDelay_, endDelay_),
    defaultColor(defaultColor_),
    highlightColor(highlightColor_),
    pressedColor(pressedColor_),
    colorTransmission(colorTransmit_),
    rColor(defaultColor_.r),
    gColor(defaultColor_.g),
    bColor(defaultColor_.b),
    aColor(defaultColor_.a)
{
    sf::CircleShape* circle = new sf::CircleShape;
    circle->setFillColor(defaultColor_);
    circle->setRadius(circleRadius_);
    circle->setOrigin(circle->getLocalBounds().width / 2.f, circle->getLocalBounds().height / 2.f);

    shape = circle;
    sprite.setTextureRect(sf::IntRect(0, 0, circle->getLocalBounds().width, circle->getLocalBounds().height));
    sprite.setOrigin(circle->getLocalBounds().width / 2.f, circle->getLocalBounds().height / 2.f);
}

GUI::ShapeButton::~ShapeButton()
{
    if(shape != nullptr)
        delete shape;
}

bool GUI::ShapeButton::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool return_ = Button::HandleInput(mousePos_, leftClick_, deltaTime, doInput);

    sf::Color transColor;
    if(currentPhase == HighlighPhase)
        transColor = highlightColor;
    else if(currentPhase == PressedPhase)
        transColor = pressedColor;
    else transColor = defaultColor;

    if(colorTransmission)
    {
        TendTowards(rColor, transColor.r, 0.1f, 0.0001f, deltaTime);
        TendTowards(bColor, transColor.b, 0.1f, 0.0001f, deltaTime);
        TendTowards(gColor, transColor.g, 0.1f, 0.0001f, deltaTime);
        TendTowards(aColor, transColor.a, 0.1f, 0.0001f, deltaTime);
    }
    else
    {
        rColor = transColor.r;
        gColor = transColor.g;
        bColor = transColor.b;
        aColor = transColor.a;
    }

    return return_;
}

void GUI::ShapeButton::Draw(sf::RenderTarget& target_)
{
    if(style != nullptr)
    {
        sf::Color textColor = style->getFontColor();
        textColor.a = sprite.getColor().a;

        shape->setScale(sprite.getScale());
        shape->setPosition(sprite.getPosition());
        shape->setFillColor(sf::Color(rColor, gColor, bColor, aColor / 255.f * (float)sprite.getColor().a));

        icon.setScale(sprite.getScale() * iconScale);
        icon.setPosition(sprite.getPosition() + (style->getIconRef(currentPhase) * sprite.getScale().x));
        icon.setColor(sprite.getColor());

        text.setScale(sprite.getScale());
        text.setPosition(sprite.getPosition() + (style->getTextRef(currentPhase) * scale * sprite.getScale().x));
        text.setColor(sf::Color(textColor));

        target_.draw(*shape);
        target_.draw(icon);
        target_.draw(text);
    }
}

//ShapeToggleButton
GUI::ShapeToggleButton::ShapeToggleButton(Style* styleOff_, Style* styleOn_, sf::Vector2f rectangleSize_,
                                          sf::Color defaultColor_, sf::Color highlightColor_, sf::Color pressedColor_,
                                          sf::Color defaultOnColor_, sf::Color highlightOnColor_, sf::Color pressedOnColor_, bool colorTransmit_,
                                          sf::Vector2f position_, ResourcePack rPack_ , std::string message_, std::string messageOff_,
                                          std::string text_, sf::Texture* iconTextr_, float iconScale_, sf::IntRect iconRect_,
                                          float strDelay_, float endDelay_, bool disableInputInOnState_, std::string* stringRef_, bool canModifyRef_) :
    ToggleButton(styleOff_, styleOn_, position_, 1, rPack_, message_, messageOff_, text_, iconTextr_, iconScale_, iconRect_, strDelay_, endDelay_, disableInputInOnState_, stringRef_, canModifyRef_),
    defaultColor(defaultColor_),
    highlightColor(highlightColor_),
    pressedColor(pressedColor_),
    defaultOnColor(defaultOnColor_),
    highlightOnColor(highlightOnColor_),
    pressedOnColor(pressedOnColor_),
    colorTransmission(colorTransmit_),
    rColor(defaultColor_.r),
    gColor(defaultColor_.g),
    bColor(defaultColor_.b),
    aColor(defaultColor_.a)
{
    sf::RectangleShape* rectangle = new sf::RectangleShape;
    rectangle->setFillColor(defaultColor_);
    rectangle->setSize(rectangleSize_);
    rectangle->setOrigin(rectangleSize_ / 2.f);

    shape = rectangle;
    sprite.setTextureRect(sf::IntRect(0, 0, rectangleSize_.x, rectangleSize_.y));
    sprite.setOrigin(rectangleSize_ / 2.f);
}

GUI::ShapeToggleButton::ShapeToggleButton(Style* styleOff_, Style* styleOn_, sf::Vector2f rectangleSize_, sf::Color defaultColor_, sf::Color highlightColor_, sf::Color pressedColor_, bool colorTransmit_,
                              sf::Vector2f position_, ResourcePack rPack_ , std::string message_, std::string messageOff_,
                              std::string text_, sf::Texture* iconTextr_, float iconScale_, sf::IntRect iconRect_,
                              float strDelay_, float endDelay_, bool disableInputInOnState_, std::string* stringRef_, bool canModifyRef_) :
    ToggleButton(styleOff_, styleOn_, position_, 1, rPack_, message_, messageOff_, text_, iconTextr_, iconScale_, iconRect_, strDelay_, endDelay_, disableInputInOnState_, stringRef_, canModifyRef_),
    defaultColor(defaultColor_),
    highlightColor(highlightColor_),
    pressedColor(pressedColor_),
    defaultOnColor(defaultColor_),
    highlightOnColor(highlightColor_),
    pressedOnColor(pressedColor_),
    colorTransmission(colorTransmit_),
    rColor(defaultColor_.r),
    gColor(defaultColor_.g),
    bColor(defaultColor_.b),
    aColor(defaultColor_.a)
{
    sf::RectangleShape* rectangle = new sf::RectangleShape;
    rectangle->setFillColor(defaultColor_);
    rectangle->setSize(rectangleSize_);
    rectangle->setOrigin(rectangleSize_ / 2.f);

    shape = rectangle;
    sprite.setTextureRect(sf::IntRect(0, 0, rectangleSize_.x, rectangleSize_.y));
    sprite.setOrigin(rectangleSize_ / 2.f);
}

GUI::ShapeToggleButton::ShapeToggleButton(Style* styleOff_, Style* styleOn_, float circleRadius_, sf::Color defaultColor_, sf::Color highlightColor_, sf::Color pressedColor_, bool colorTransmit_,
                              sf::Vector2f position_, ResourcePack rPack_ , std::string message_, std::string messageOff_,
                              std::string text_, sf::Texture* iconTextr_, float iconScale_, sf::IntRect iconRect_,
                              float strDelay_, float endDelay_, bool disableInputInOnState_, std::string* stringRef_, bool canModifyRef_) :
    ToggleButton(styleOff_, styleOn_, position_, 1, rPack_, message_, messageOff_, text_, iconTextr_, iconScale_, iconRect_, strDelay_, endDelay_, disableInputInOnState_, stringRef_, canModifyRef_),
    defaultColor(defaultColor_),
    highlightColor(highlightColor_),
    pressedColor(pressedColor_),
    defaultOnColor(defaultColor_),
    highlightOnColor(highlightColor_),
    pressedOnColor(pressedColor_),
    colorTransmission(colorTransmit_),
    rColor(defaultColor_.r),
    gColor(defaultColor_.g),
    bColor(defaultColor_.b),
    aColor(defaultColor_.a)
{
    sf::CircleShape* circle = new sf::CircleShape;
    circle->setFillColor(defaultColor_);
    circle->setRadius(circleRadius_);
    circle->setOrigin(circle->getLocalBounds().width / 2.f, circle->getLocalBounds().height / 2.f);

    shape = circle;
    sprite.setTextureRect(sf::IntRect(0, 0, circle->getLocalBounds().width, circle->getLocalBounds().height));
    sprite.setOrigin(circle->getLocalBounds().width / 2.f, circle->getLocalBounds().height / 2.f);
}

GUI::ShapeToggleButton::~ShapeToggleButton()
{
    if(shape != nullptr)
        delete shape;
}

bool GUI::ShapeToggleButton::HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime, bool doInput)
{
    bool return_ = ToggleButton::HandleInput(mousePos_, leftClick_, deltaTime, doInput);

    sf::Color transColor;
    if(currentPhase == HighlighPhase)
        transColor = (status ? highlightOnColor : highlightColor);
    else if(currentPhase == PressedPhase)
        transColor = (status ? pressedOnColor : pressedColor);
    else transColor = (status ? defaultOnColor : defaultColor);

    if(colorTransmission)
    {
        TendTowards(rColor, transColor.r, 0.1f, 0.0001f, deltaTime);
        TendTowards(bColor, transColor.b, 0.1f, 0.0001f, deltaTime);
        TendTowards(gColor, transColor.g, 0.1f, 0.0001f, deltaTime);
        TendTowards(aColor, transColor.a, 0.1f, 0.0001f, deltaTime);
    }
    else
    {
        rColor = transColor.r;
        gColor = transColor.g;
        bColor = transColor.b;
        aColor = transColor.a;
    }

    shape->setFillColor(sf::Color(rColor, gColor, bColor));
    return return_;
}

void GUI::ShapeToggleButton::Draw(sf::RenderTarget& target_)
{
    if(style != nullptr)
    {
        float trans = sprite.getColor().a;
        sf::Color textColor = style->getFontColor();
        textColor.a = sprite.getColor().a;

        shape->setScale(sprite.getScale());
        shape->setPosition(sprite.getPosition());
        shape->setFillColor(sf::Color(rColor, gColor, bColor, aColor / 255.f * (float)sprite.getColor().a));

        icon.setScale(sprite.getScale() * iconScale);
        icon.setPosition(sprite.getPosition() + (style->getIconRef(currentPhase) * sprite.getScale().x));
        icon.setColor(sprite.getColor());

        text.setScale(sprite.getScale());
        text.setPosition(sprite.getPosition() + (style->getTextRef(currentPhase) * scale * sprite.getScale().x));
        text.setColor(sf::Color(textColor));

        target_.draw(*shape);
        target_.draw(icon);
        target_.draw(text);
    }
}
