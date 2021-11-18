#ifndef GUIOBJECTS_H_INCLUDED
#define GUIOBJECTS_H_INCLUDED

#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "GUI/gui_displayobjects.h"
#include <string>
#include <vector>

namespace GUI
{
    //Public Functions
    void SetMouseScrollDalta(float scrollMagn);
    float GetMouseScrollDelta();
    bool IsCursorOnObject();
    void SetCursorOnObjectState(bool state_);

    //Structures And Classes
    struct ResourcePack
    {
        sf::Texture* texture = nullptr;
        sf::Font* font = nullptr;
        sf::Sound* highlightSound = nullptr;
        sf::Sound* pressedSound = nullptr;
        sf::Sound* releasedSound = nullptr;

        ResourcePack(sf::Texture* texr_ = nullptr, sf::Font* font_ = nullptr, sf::Sound* hlSound_ = nullptr,
                     sf::Sound* prSound_ = nullptr, sf::Sound* rlSound_ = nullptr)
         {
             texture = texr_;
             font = font_;
             highlightSound = hlSound_;
             pressedSound = prSound_;
             releasedSound = rlSound_;
         }
    };

    class Style
    {
    public:
        struct Phase
        {
            float scale = 1.f;
            int transparency = 255;
            int textrIndex = 0;
            sf::Vector2f refPosition;
            sf::Vector2f refObjectPosition;
            sf::Vector2f boundarySize;
            sf::Vector2f boundaryRefPos;

            void Set(float scale_, int textrIndex_, sf::Vector2f refPosition_, sf::Vector2f refObjectPosition_, int transparency_ = 255,
                     sf::Vector2f boundarySize_ = sf::Vector2f(0, 0), sf::Vector2f boundaryRefPos_ = sf::Vector2f(0, 0))
            {
                scale = scale_;
                refPosition = refPosition_;
                refObjectPosition = refObjectPosition_;
                boundarySize = boundarySize_;
                boundaryRefPos = boundaryRefPos_;
                textrIndex = textrIndex_;
                transparency = transparency_;
            }
        };

    private:
        Phase startPhase;
        Phase defaultPhase;
        Phase highlightPhase;
        Phase pressedPhase;
        Phase inactivePhase;
        Phase lockedPhase;

        sf::IntRect textureRect;
        sf::Vector2f textRef;
        sf::Vector2f iconRef;
        sf::Vector2f textOriginFactor;

        int fontSize;
        sf::Color fontColor;
        InputType inputType;
        float inputHoldDelay;
        float transmissionFactor;

    public:
        Style();

        void setPhase(PhaseStage stage_, float scale_, int textrIndex_ = 0, sf::Vector2f refPosition_ = sf::Vector2f(0, 0), sf::Vector2f refObjectPosition_ = sf::Vector2f(0, 0),
                      int transparency_ = 255, sf::Vector2f boundarySize_ = sf::Vector2f(0, 0), sf::Vector2f boundaryRefPos_ = sf::Vector2f(0, 0));
        void setTextureRect(sf::IntRect rect_);
        void setTextRef(sf::Vector2f refPos_);
        void setTextOriginFactor(sf::Vector2f factor_);
        void setIconRef(sf::Vector2f refPos_);
        void setFontParameters(int size_, sf::Color color_ = sf::Color::White);
        void setTransmissionFactor(float factor_ = 0.1);
        void setInputHoldDelay(float timeDelay_);
        void setInputType(InputType type_);
        void copyFrom(const Style& style_);

        sf::IntRect getTextureRect();

        int getFontSize();
        float getTransmissionFactor();
        float getInputHoldDelay();
        InputType getInputType();
        sf::Color getFontColor();
        sf::Vector2f getTextOriginFactor();

        sf::Vector2f getTextRef(PhaseStage stage_);
        sf::Vector2f getIconRef(PhaseStage stage_);
        sf::Vector2f getPhasePosition(PhaseStage stage_);
        float getPhaseScale(PhaseStage stage_);
        int getPhaseTransparency(PhaseStage stage_);
        Style::Phase getPhase(PhaseStage stage_);

    };

    class Object
    {
    public:
        virtual ~Object();
        virtual void Draw(sf::RenderTarget& target_);
        virtual bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
        virtual std::string GetMessage();
        virtual void Reset(std::string command = "");
        virtual void SetInactive();
    };

    class Button : public Object
    {
    protected:
        sf::Text text;
        sf::Sprite sprite;
        sf::Sprite icon;

        ResourcePack resourcePack;
        sf::Texture* texture;
        Style* style;
        sf::Vector2f position;
        float currentTransparency;
        float scale;
        float iconScale;
        float keyHoldTime;
        float delayTime;
        float startDelay;
        float endDelay;
        bool oneSecondDelay;
        bool keyIndex;
        bool lockState;
        bool onCursor;
        bool onPush;
        PhaseStage currentPhase;
        std::string message;

    public:
        Button(Style* style_, sf::Vector2f position_, float scale_, const ResourcePack& rPack_ , std::string message_,
               std::string text_ = "", sf::Texture* iconTextr_ = nullptr, float iconScale_ = 1.f, sf::IntRect iconRect_ = sf::IntRect(),
               float strDelay_ = 0.f, float endDelay_ = 0.f);
        Button();
        virtual ~Button();

        virtual void SetTexture(sf::Texture* texture_);
        virtual void SetIcon(sf::Texture* texture_, sf::IntRect rect_, float scale_ = 1.f);
        virtual void SetText(std::string text_, sf::Font* font_);
        virtual void SetPosition(sf::Vector2f position_);
        virtual void SetLock(bool state_);
        virtual void SetInactive();

        sf::Vector2f GetPosition();

        virtual bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
        virtual void Draw(sf::RenderTarget& target_);
        virtual std::string GetMessage();
        virtual void Reset(std::string command = "");
    };

    class ToggleButton : public Button
    {
    protected:
        GUI::Style* styleOff;
        GUI::Style* styleOn;
        bool status;
        std::string offMessage;
        std::string* stringRef;
        bool disableInputInOnState;
        bool canModifyRef;

    public:
        ToggleButton(Style* styleOff_, Style* styleOn_, sf::Vector2f position_, float scale_, const ResourcePack& rPack_, std::string message_, std::string offMessage_,
                     std::string text_ = "", sf::Texture* iconTextr_ = nullptr, float iconScale_ = 1.f, sf::IntRect iconRect_ = sf::IntRect(),
                     float strDelay_ = 0.f, float endDelay_ = 0.f, bool disableInputInOnState_ = false, std::string* stringRef_ = nullptr, bool canModifyRef_ = true);
        virtual ~ToggleButton()
        {
        }
        virtual bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
        virtual std::string GetMessage();
        virtual void Reset(std::string command = "");

        void SetStatus(bool status_);
        bool GetStatus();
    };

    class Group : virtual public Object
    {
    protected:
        std::vector<Object*> objects;
        std::vector<DisplayObject*> displayObjects;
        std::string message;
        bool enableSoftReset;
        bool drawDisplayObjectsFirst;

    public:
        Group(bool enableSoftReset_ = true, bool drawDisplayObjectsFirst_ = true);
        virtual ~Group();
        virtual void AddObject(Object* object_);
        virtual void AddObject(DisplayObject* object_);

        virtual void Draw(sf::RenderTarget& target_);
        virtual bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
        virtual std::string GetMessage();
        virtual void Reset(std::string command = "");
        virtual void SetInactive();
    };

    class AbstractSelection : virtual public Object
    {
    public:
        virtual ~AbstractSelection()
        {
        }

        virtual void Scroll(float amount_) = 0;
        virtual void MoveForeward() = 0;
        virtual void MoveBackward() = 0;
        virtual void SetViewPosition(float ratio_, bool doTransmit = false) = 0;
        virtual float GetViewPosition() = 0;
    };

    class Selection : public AbstractSelection
    {
    public:
        struct ItemInfo
        {
            std::string text = "";
            std::string message = "";
            std::string offMessage = "";
            sf::Texture* iconTextr = nullptr;
            sf::IntRect iconRect = sf::IntRect();
            float iconScale = 1.f;
            float startDelay = 0.f;
            float endDelay = 0.f;
            bool lockState = false;

            ItemInfo(std::string message_ = "", std::string offMessage_ = "", std::string text_ = "", bool lockState_ = false, sf::Texture* iconTextr_ = nullptr, float iconScale_ = 1.f, sf::IntRect iconRect_ = sf::IntRect(),
                     float startDelay_ = 0.f, float endDelay_ = 0.f)
            {
                text = text_;
                message = message_;
                offMessage = offMessage_;
                iconTextr = iconTextr_;
                iconRect = iconRect_;
                iconScale = iconScale_;
                lockState = lockState_;
                startDelay = startDelay_;
                endDelay = endDelay_;
            }
        };

    private:
        std::vector<Object*> objectList;
        std::string message;
        int currentItem;
        float timeFactor;
        sf::Vector2f position;
        sf::Vector2f viewPosition;
        sf::Vector2f displaySize;
        sf::View view;
        sf::FloatRect itemRect;
        sf::Vector2f seperation;
        Direction direction;

        ResourcePack resourcePack;
        Style* styleOff;
        Style* styleOn;
        float itemScale;

    public:
        Selection(Style* style_, ResourcePack rPack_, sf::Vector2f position_, float itemScale_, sf::Vector2f displaySize_, sf::FloatRect itemRect_,
                  float seperation_, float timeFactor_, Direction dir_, std::vector<ItemInfo>* itemList_ = nullptr);
        Selection(Style* styleOff_, Style* styleOn_, ResourcePack rPack_, sf::Vector2f position_, float itemScale_, sf::Vector2f displaySize_, sf::FloatRect itemRect_,
                  float seperation_, float timeFactor_, Direction dir_, std::vector<ItemInfo>* itemList_ = nullptr);
        ~Selection();

        void AddItem(ItemInfo item_);
        void AddItem(std::vector<ItemInfo>* itemList_);

        void Scroll(float amount_);
        void MoveForeward();
        void MoveBackward();
        void SetViewPosition(float ratio_, bool doTransmit = false);

        float GetViewPosition();

        void Draw(sf::RenderTarget& target_);
        bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
        std::string GetMessage();
        void Reset(std::string command = "");
        void SetInactive();
    };

    class ObjectSelection : public AbstractSelection, public Group
    {
    public:
        ObjectSelection(sf::Vector2f position_, sf::Vector2f displaySize_, float scrollLength_, float moveFactor_ = 1.f, float transmitFactor_ = 0.1f, Direction dir_ = Vertical, bool enableSoftReset_ = true, bool shouldResetView_ = true, bool canSetCursorOnObject_ = true, bool respondToScroll_ = true);
        virtual ~ObjectSelection()
        {
        }

        bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
        void Draw(sf::RenderTarget& target_);
        void Reset(std::string command = "");

        void Scroll(float amount_);
        void MoveForeward();
        void MoveBackward();
        void SetViewPosition(float ratio_, bool doTransmit = false);
        float GetViewPosition();
        void SetViewPositionOrigin(const sf::Vector2f& positionOrigin_);
        const sf::Vector2f& GetViewPositionOrigin();
        void SetViewCenter(const sf::Vector2f& center_, bool alsoSetOriginToThisCenter_ = false);
        const sf::Vector2f& GetViewCenter();
        void SetScrollLength(float scrollLength_);
        void SetPosition(const sf::Vector2f& position_);
        const sf::Vector2f& GetPosition();
        void SetDisplaySize(const sf::Vector2f& displaySize_, const sf::Vector2f& originPos_, const float& scrollLength_, bool fixedToScrollPos_ = false);
        const sf::Vector2f& GetDisplaySize();

    private:
        sf::View view;
        sf::Vector2f direction;
        sf::Vector2f originPosition;
        sf::Vector2f position;
        float scrollLength;
        float scrollPositionFactor;
        float moveFactor;
        float transmitFactor;
        bool shouldResetView;
        bool canSetCursorOnObject;
        bool respondToScroll;
    };

    class Degree : public Object
    {
    public:
        enum StateType{ClickAnyPart = 0, ClickNode, ClickNoPart};

    protected:
        sf::RectangleShape line;
        sf::RectangleShape lineExtent;
        sf::Shape* node;
        Direction direction;
        StateType state;
        bool active;
        bool isClicked;
        bool keyIndex;
        bool isInActive;
        float nodeMax;
        float nodeMin;
        float* rateRef;
        float startDelay;
        float endDelay;
        float wait;
        float transmissionFactor;
        float transparencyFactor;
        sf::Color lineColor;
        sf::Color nodeColor;
        sf::Color extentColor;

    public:
        Degree(Direction dir_, float length_, float width_, sf::Vector2f pos_, sf::Vector2f nodeSize_, sf::Color lineColr_, sf::Color nodeColr,
               sf::Color extentColr_ = sf::Color::Transparent, StateType state_ = ClickAnyPart, float startDelay_ = 0, float endDelay_ = 0, float transmitFactor_ = 0.1f, float* rateRef_ = nullptr);
        Degree(Direction dir_, float length_, float width_, sf::Vector2f pos_, float nodeRadius_, sf::Color lineColr_, sf::Color nodeColr_,
               sf::Color extentColr_ = sf::Color::Transparent, StateType state_ = ClickAnyPart, float startDelay_ = 0, float endDelay_ = 0, float transmitFactor_ = 0.1f, float* rateRef_ = nullptr);
        virtual ~Degree();

        virtual void Draw(sf::RenderTarget& target_);
        virtual bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
        virtual void Reset(std::string command = "");
        virtual void SetInactive();

        float GetRatio();
        virtual void SetRatio(float ratio_);
    };

    class Navigator : public Object
    {
    private:
        Button* upButton;
        Button* downButton;
        Degree* slidder;
        AbstractSelection* selection;
        std::string message;

    public:
        Navigator(Button* upButton_, Button* downButton_, Degree* slidder_ = nullptr, AbstractSelection* selection_ = nullptr);
        ~Navigator();

        void Draw(sf::RenderTarget& target_);
        bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
        std::string GetMessage();
        void Reset(std::string command = "");
        void SetInactive();
    };

    class Layer : public Group //Group duplicate with a more descriptive name Layer for GUI::Session
    {
    public:
        Layer();
        virtual ~Layer();
    };

    class Session : public Object
    {
    private:
        std::vector<Layer*> layerList;
        std::vector<std::string> currentList;
        std::map<std::string, Layer*> layers;
        std::string waitLayer;
        std::string emptyListMessage;
        float waitTime;
        bool moveBack;
        bool isEmpty;

    public:
        Session();
        ~Session();

        void Create(std::string name_);
        void AddObject(Object* object, std::string name_);
        void AddObject(DisplayObject* object, std::string name_);
        void TransmitTo(std::string name_, float wait_ = 0.f);
        void AddInFront(std::string name_);
        void MoveBack(float wait_ = 0.f);
        std::string GetCurrentLayer();
        void Clear();

        void Draw(sf::RenderTarget& target_);
        bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
        std::string GetMessage();
        void Reset(std::string command = "");
        void ResetActiveLayers(std::string command = "");
        void SetInactive();
        void SetInactive(std::string name_);
        void SetEmptyMessage(std::string message_);
    };
}

//

#endif // GUIOBJECTS_H_INCLUDED
