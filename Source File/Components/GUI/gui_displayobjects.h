#ifndef DISPLAYOBJECTS_H_INCLUDED
#define DISPLAYOBJECTS_H_INCLUDED
#include "GUI/gui_global.h"
#include <SFML/Graphics.hpp>

namespace GUI
{
    // Displayable objects with transmission properties
    class DisplayObject
    {
    public:
        struct DisplayPhase
        {
            sf::Vector2f position = sf::Vector2f(0, 0);
            sf::Vector2f scale = sf::Vector2f(0, 0);
            float transparency = 255;

            DisplayPhase(sf::Vector2f position_ = sf::Vector2f(0, 0), sf::Vector2f scale_ = sf::Vector2f(1, 1),
                         float transparency_ = 255)
            {
                position = position_;
                scale = scale_;
                transparency = transparency_;
            }
        };

        DisplayObject();
        virtual ~DisplayObject();

        virtual void Draw(sf::RenderTarget& target_) = 0;
        virtual void Update(float deltaTime_);
        void SetPhase(PhaseStage stage_, bool quickTransmit = false);
        void SetSinglePhasePosition(sf::Vector2f position_);
        void EditPhase(DisplayPhase phase_, PhaseStage stage_);
        void SetDrawStatus(bool status_);
        void SetInactive();
        void Reset();

    protected:
        PhaseStage state;
        float startTime;
        float endTime;
        float waitStartTime;
        float waitEndTime;
        float transmissionFactor;

        DisplayPhase startPhase;
        DisplayPhase defaultPhase;
        DisplayPhase inactivePhase;

        sf::Vector2f position;
        sf::Vector2f scale;
        float transparency;
        bool drawStatus;
    };

    //Display objects from base class
    class DisplayText : public DisplayObject
    {
    public:
        DisplayText(std::wstring text_, sf::Font* font_, sf::Color color_, float size_, sf::Vector2f originFactor_,
                    DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                    float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);
        DisplayText(std::string text_, sf::Font* font_, sf::Color color_, float size_, sf::Vector2f originFactor_,
                    DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                    float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);
        DisplayText(std::string* stringRef_, sf::Font* font_, sf::Color color_, float size_, sf::Vector2f originFactor_,
                    DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                    float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);
        DisplayText(std::string text_, sf::Font* font_, sf::Color color_, float size_,
                    DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                    float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);
        DisplayText(sf::Text text_,
                    DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                    float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);

        void Draw(sf::RenderTarget& traget_);
        void SetText(std::string text_);
        void SetText(std::wstring text_);
        void SetText(std::string* stringRef_);

    private:
        sf::Text text;
        sf::Vector2f originFactor;
        std::string* stringRef;
    };

    class DisplayShape : public DisplayObject
    {
    public:
        DisplayShape(sf::Vector2f rectangleSize_, sf::Color color_, sf::Vector2f originFactor_,
                     DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                     float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);
        DisplayShape(sf::Vector2f rectangleSize_, sf::Color color_,
                     DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                     float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);
        DisplayShape(float circleRadius, sf::Color color_,
                     DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                     float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);
        DisplayShape(sf::ConvexShape shape_,
                     DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                     float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);

        ~DisplayShape();

        void Draw(sf::RenderTarget& target_);

    private:
        sf::Shape* shape;
        sf::Color color;
    };

    class DisplaySprite : public DisplayObject
    {
    public:
        DisplaySprite(sf::Texture* texture_,
                      DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                      float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);
        DisplaySprite(sf::Texture* texture_, sf::IntRect textureRect_,
                      DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                      float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);
        DisplaySprite(sf::Texture* texture_, sf::IntRect textureRect_, sf::Vector2f originFactor_,
                      DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                      float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);
        DisplaySprite(sf::Sprite& sprite_,
                      DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                      float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);

        void Draw(sf::RenderTarget& target_);
        void setTexture(sf::Texture* texture_);
        void setTexture(sf::Texture* texture_, sf::IntRect rect_);
        void setTextureRect(sf::IntRect rect_);

    private:
        sf::Sprite sprite;
        sf::Vector2f originFactor;
    };

    class DisplayVertexArray : public DisplayObject
    {
    public:
        DisplayVertexArray(sf::PrimitiveType primitiveType_, sf::Texture* texture_ = nullptr,
                           DisplayPhase startPhase_ = DisplayPhase(), DisplayPhase defaultPhase_ = DisplayPhase(), DisplayPhase InactivePhase_ = DisplayPhase(),
                           float waitStartTime_ = 0.f, float waitEndTime_ = 0.f, float transmissionFactor_ = 0.1f);

        void Draw(sf::RenderTarget& target_);
        void AddVertex(const sf::Vertex& vertex_);
        void AddRectVertices(sf::FloatRect rect_, sf::Color color_ = sf::Color::White, float rotation_ = 0, sf::IntRect textureRect_ = sf::IntRect());
        void EditVertext(const int& index_, const sf::Vector2f& position_, const sf::Color& color_, const sf::Vector2f& textrCoords_);
        const sf::Vertex* GetVertex(const int& index_);
        void Resize(const int& size_);
        void Clear();

    private:
        std::vector<int> vertexTransparency;
        sf::VertexArray vertexArray;
        sf::Texture* texture;
        sf::Vector2f lastPosition;
        float lastTransparency;
    };
}

#endif // DISPLAYOBJECTS_H_INCLUDED
