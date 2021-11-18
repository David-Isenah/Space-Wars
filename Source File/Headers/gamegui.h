#ifndef GAMEGUI_H_INCLUDED
#define GAMEGUI_H_INCLUDED

#include "GUI/gui_objects.h"
#include "GUI/gui_displayobjects.h"
#include "GUI/gui_additionalobjects.h"
#include "shipidenty.h"
#include "game.h"

void InitialiseGameGui();

//Level GUI
class ShipProfileButton : public GUI::Button
{
public:
    ShipProfileButton(ShipIdentity* identity_, sf::Vector2f position_, std::string message_);
    ~ShipProfileButton();

    void SetShipIdentity(ShipIdentity* identity_);
    ShipIdentity* GetShipIdentity();
    void SetShowInfo(bool status_);

    bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
    void Draw(sf::RenderTarget& target_);

private:
    ShipIdentity* identity;
    sf::Text textLevel;
    sf::Text textPrim;
    sf::Text textSec;
    sf::Sprite stateIcon;
    sf::Vertex profileVertex[4];
    sf::Vertex shadowVertex[4];
    sf::Texture* profileTexture;
    float transparencyInfo;
    bool showInfo;
    bool lastSpecialState;
    short lastLevel;
};

class FadeDegree : public GUI::Degree
{
public:
    FadeDegree(GUI::Direction dir_, float length_, float width_, sf::Vector2f pos_, sf::Vector2f nodeSize_, float hideSize_, short hideSide_, sf::Color lineColr_, sf::Color nodeColr,
           sf::Color extentColr_ = sf::Color::Transparent, StateType state_ = ClickAnyPart, float startDelay_ = 0, float endDelay_ = 0, float transmitFactor_ = 0.1f, float* rateRef_ = nullptr);
    FadeDegree(GUI::Direction dir_, float length_, float width_, sf::Vector2f pos_, float nodeRadius_, float hideSize_, short hideSide_, sf::Color lineColr_, sf::Color nodeColr_,
           sf::Color extentColr_ = sf::Color::Transparent, StateType state_ = ClickAnyPart, float startDelay_ = 0, float endDelay_ = 0, float transmitFactor_ = 0.1f, float* rateRef_ = nullptr);
    FadeDegree(GUI::Direction dir_, sf::FloatRect rect_, float nodeSizeFactor_, float hideSize_, short hideSide_, sf::Color lineColr_, sf::Color nodeColr_,
           sf::Color extentColr_ = sf::Color::Transparent, StateType state_ = ClickAnyPart, float startDelay_ = 0, float endDelay_ = 0, float transmitFactor_ = 0.1f, float* rateRef_ = nullptr);

    void Draw(sf::RenderTarget& target_);
    bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
    void SetRatio(float ratio_);

private:
    float lastActiveTime;
    sf::FloatRect rect;
    float fadeFactor;
    float hideFactor;
    float hideSize;
    int hideSide;
    float hideTime;
    float hideDelay;
    sf::Vector2f nodeSize;
    sf::FloatRect hideRect;
    bool isNodeRectangle;
};

class FocusedShipDisplay : public GUI::DisplayObject
{
public:
    static const float GunLength[Gun::NumOfGuns];

    FocusedShipDisplay();

    void Draw(sf::RenderTarget& target_);
    void Update(float deltaTime_);

    void SetShipIdentity(ShipIdentity* identity_);
    ShipIdentity* GetShipIdentity();

private:
    ShipIdentity* identity;
    sf::VertexArray vertices;
    sf::VertexArray vertices_icon;
    sf::VertexArray vertices_profile;
    sf::VertexArray vertices_gun;
    sf::Text text;
    sf::Text text_level;
    sf::Text text_bullet;
    sf::Text text_bullet_ammo;
    sf::Text text_surUnits;
    sf::Texture* texture_guns;
    sf::Texture* texture_skin;
    sf::Texture* texture_icon;
    float healthLength;
    float healthFactor;
    short lastGun;
    float damageFactor;
    float reloadTransparency;
    float reloadFadeTime;
    float reloadFadeTimeTransit;
    float reloadFadeTransparency;
    bool lastSpecialState;
};

class FocusedShipButton : public GUI::ShapeButton
{
public:
    FocusedShipButton(sf::Color clr = Game::GetCurrentRankThemeColor());

    void Draw(sf::RenderTarget& target_);
    bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
    std::string GetMessage();
    void Reset(std::string command = "");
    void SetInactive();

    void UpdateInfo(sf::Texture* textureSkin_, bool isTextureSkin_, std::string name_, int level_, float expRatio_, float healthRatio_, float specialRatio_, short moral_, int surUnits_, int state_, std::string stateDesc_,
                    bool actionOptions_, bool focusOption_, bool thinkOption_, bool detailsOption_, float reviveSeconds_, std::string primText_, std::string secText_);
    void UpdateInfo();
    void SetThinkButtonStatus(bool status_);

private:
    sf::VertexArray va_bars;
    sf::VertexArray va_icons;
    sf::VertexArray va_profile;

    GUI::ShapeButton button_attack;
    GUI::ShapeButton button_defend;
    GUI::ShapeButton button_cancel;
    GUI::ShapeButton button_focus;
    GUI::ShapeToggleButton button_think;
    GUI::ShapeButton button_details;

    sf::Text text_name;
    sf::Text text_level;
    sf::Text text_surUnits;
    sf::Text text_stateDesc;
    sf::Text text_prim;
    sf::Text text_sec;

    sf::Texture* texture_icons;
    sf::Texture* texture_skin;
    std::string message__;
    bool isReviving;
    int va_icons_size;
    float specialFactor;
    float specialFactorSmooth;
    int specialIndex;
    float healthFactor;
    float healthFactorSmooth;
    int healthIndex;
    std::vector<sf::Color> barColorPerQuad;
};

class EnergyUsageButton : public GUI::ShapeButton
{
public:
    EnergyUsageButton(sf::Vector2f position_, sf::Color clr_ = Game::GetCurrentRankThemeColor());
    void Draw(sf::RenderTarget& target_);
    void UpdateInfo(std::string text_, std::string  message_, int cost_);
    float GetTransparenct();
    bool IsInactive();

private:
    sf::Text text_cost;
    sf::Sprite sprite_energy;
};

class BaseInfoButton : public GUI::ShapeButton
{
public:
    BaseInfoButton();
    virtual ~BaseInfoButton()
    {
    }

    void Draw(sf::RenderTarget& target_);
    bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
    std::string GetMessage();
    void Reset(std::string command = "");
    void SetInactive();

    void SetShowOptionStatus(bool status_);
    bool GetShowOptionStatus();
    void UpdateInfo(short team_, int numOfUnits_, int cost_, short baseId_);
    void UpdateInfo(int numOfUnits_);

private:
    sf::Sprite info_icon;
    sf::VertexArray info_va;
    sf::Text info_text;
    sf::Text info_text_units;
    bool showOption;
    bool showButton;
    bool showUnits;
    float optionTrans;
    float textTrans;
    std::string message__;
    EnergyUsageButton optionButton;
};

class BoosterButton : public GUI::ShapeToggleButton
{
public:
    BoosterButton(sf::Vector2f position_, sf::Color clr, std::string message_, std::string name_, std::string desc_, int cost_, int time_, float* timeRef_, std::string* stringRef_, int* versionRef_, int version_, sf::IntRect iconRect_);

    void Draw(sf::RenderTarget& target_);
    bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
    std::string GetMessage();
    void Reset(std::string command = "");
    void SetInactive();

protected:
    GUI::DisplayText display_name;
    GUI::DisplayText display_desc;
    GUI::DisplayText display_time;
    GUI::DisplayText display_cost;
    GUI::DisplaySprite display_sp_time;
    GUI::DisplaySprite display_sp_cost;
    GUI::DisplaySprite display_sp_icon;

    std::string string_time;
    std::string string_cost;
    std::string string_lastRef;
    float* timeRef;
    int* versionRef;
    float time;
    int cost;
    int version;
};

class MapButton : public GUI::Object
{
public:
    MapButton(sf::Color bgColor_);

    void SetDisplayBoarderStatus(bool status);
    const sf::Vector2f& GetButtonSize();
    const float& GetRealTimeScale();
    void SetTeamBaseCoverage(const short& team_, const int& coverage_);

    void Draw(sf::RenderTarget& target_);
    bool HandleInput(sf::Vector2i mousePos_, bool leftClick_, float deltaTime = 0.f, bool doInput = true);
    std::string GetMessage();
    void Reset(std::string command = "");
    void SetInactive();

private:
    GUI::DisplayVertexArray display_va;
    GUI::DisplayVertexArray display_va_boarder;
    GUI::DisplayVertexArray display_va_teamRange;

    bool displayBoarderStatus;
    bool zoomWhenScaling;
    float boarderTime;
    float mapScale;
    float scale;
    float scaleRealTime;
    float minScale;
    sf::Vector2f mapSizeScaleTransmit;
    sf::Vector2f mapSizeScaleTransmitRealTime;
    sf::Vector2f position;
    sf::Vector2f mapPosition;
    sf::Vector2f mapPositionNoClipping;
    sf::Vector2f mapSize;
    sf::Vector2f buttonSize;
    sf::FloatRect camRect;
    sf::Vector2f mapRefPos;
    sf::Vector2f clickMoveDir;
    bool clickIsDragging;
    sf::Vector2f teamRangeRefPos;
    int teamBaseCoverage[Game::NumOfTeams];
    float teamRangeFactor[Game::NumOfTeams];
    bool updateTeamRange;
    bool updateMapProperties;
    sf::Color bgColor;
    sf::Vector2f arrowPoints[3];
    float arrowTransparency;
};

#endif // GAMEGUI_H_INCLUDED
