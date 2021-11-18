#ifndef GUI_ADDITIONALOBJECTS_H_INCLUDED
#define GUI_ADDITIONALOBJECTS_H_INCLUDED

#include "GUI/gui_objects.h"
#include "GUI/gui_displayobjects.h"

namespace GUI
{
    //ShapeButton
    class ShapeButton : public Button
    {
    protected:
        sf::Shape* shape;
        sf::Color defaultColor;
        sf::Color highlightColor;
        sf::Color pressedColor;
        bool colorTransmission;

        float rColor;
        float gColor;
        float bColor;
        float aColor;

    public:
        ShapeButton(Style* style_, sf::Vector2f rectangleSize_, sf::Color defaultColor_, sf::Color highlightColor_, sf::Color pressedColor_, bool colorTransmit_,
                    sf::Vector2f position_, ResourcePack rPack_ , std::string message_,
                    std::string text_ = "", sf::Texture* iconTextr_ = nullptr, float iconScale_ = 1.f, sf::IntRect iconRect_ = sf::IntRect(),
                    float strDelay_ = 0.f, float endDelay_ = 0.f);
        ShapeButton(Style* style_, float circleRadius_, sf::Color defaultColor_, sf::Color highlightColor_, sf::Color pressedColor_, bool colorTransmit_,
                    sf::Vector2f position_, ResourcePack rPack_ , std::string message_,
                    std::string text_ = "", sf::Texture* iconTextr_ = nullptr, float iconScale_ = 1.f, sf::IntRect iconRect_ = sf::IntRect(),
                    float strDelay_ = 0.f, float endDelay_ = 0.f);
        virtual ~ShapeButton();

        virtual bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
        virtual void Draw(sf::RenderTarget& target_);
    };

    //ShapeToggleButton
    class ShapeToggleButton : public ToggleButton
    {
    protected:
        sf::Shape* shape;
        bool colorTransmission;

        sf::Color defaultColor;
        sf::Color highlightColor;
        sf::Color pressedColor;

        sf::Color defaultOnColor;
        sf::Color highlightOnColor;
        sf::Color pressedOnColor;

        float rColor;
        float gColor;
        float bColor;
        float aColor;

    public:
        ShapeToggleButton(Style* styleOff_, Style* styleOn_, sf::Vector2f rectangleSize_,
                          sf::Color defaultColor_, sf::Color highlightColor_, sf::Color pressedColor_,
                          sf::Color defaultOnColor_, sf::Color highlightOnColor_, sf::Color pressedOnColor_, bool colorTransmit_,
                          sf::Vector2f position_, ResourcePack rPack_ , std::string message_, std::string messageOff_,
                          std::string text_ = "", sf::Texture* iconTextr_ = nullptr, float iconScale_ = 1.f, sf::IntRect iconRect_ = sf::IntRect(),
                          float strDelay_ = 0.f, float endDelay_ = 0.f, bool disableInputInOnState_ = false, std::string* stringRef_ = nullptr, bool canModifyRef_ = true);
        ShapeToggleButton(Style* styleOff_, Style* styleOn_, sf::Vector2f rectangleSize_, sf::Color defaultColor_, sf::Color highlightColor_, sf::Color pressedColor_, bool colorTransmit_,
                    sf::Vector2f position_, ResourcePack rPack_ , std::string message_, std::string messageOff_,
                    std::string text_ = "", sf::Texture* iconTextr_ = nullptr, float iconScale_ = 1.f, sf::IntRect iconRect_ = sf::IntRect(),
                    float strDelay_ = 0.f, float endDelay_ = 0.f, bool disableInputInOnState_ = false, std::string* stringRef_ = nullptr, bool canModifyRef_ = true);
        ShapeToggleButton(Style* styleOff_, Style* styleOn_, float circleRadius_, sf::Color defaultColor_, sf::Color highlightColor_, sf::Color pressedColor_, bool colorTransmit_,
                    sf::Vector2f position_, ResourcePack rPack_ , std::string message_, std::string messageOff_,
                    std::string text_ = "", sf::Texture* iconTextr_ = nullptr, float iconScale_ = 1.f, sf::IntRect iconRect_ = sf::IntRect(),
                    float strDelay_ = 0.f, float endDelay_ = 0.f, bool disableInputInOnState_ = false, std::string* stringRef_ = nullptr, bool canModifyRef_ = true);
        virtual ~ShapeToggleButton();

        virtual bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
        virtual void Draw(sf::RenderTarget& target_);
    };
}

#endif // GUI_ADDITIONALOBJECTS_H_INCLUDED
