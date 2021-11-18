#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "string"
#include "SFML/Graphics.hpp"
#include "AssetManager.h"
#include "entitymanager.h"
#include "maptilemanager.h"
#include "GUI/gui.h"
#include "base.h"
#include "shipidenty.h"
#include "skin.h"
#include <sstream>

class Game
{
    public:
        static int SCREEN_WIDTH;
        static int SCREEN_HEIGHT;
        const std::string WINDOW_TITLE = "Space Wars";
        static const float MIN_FRAMERATE;
        static const int RANK_UNLOCK_LEVEL_EDITOR = 3;

        enum GameState{Prototype, MapEdit};
        enum GameMode {NoGameMode = -1, BattleMode, DefendMode, BossMode};
        enum GameTeams {NoTeam = -1, Alpha, Delta, Vortex, Omega, NumOfTeams};
        enum SkinType {NoSkinType = -1, SkinMainShip, SkinSubShip, SkinMiniShip, SkinMicroShip, SkinMainDefence, SkinMiniDefence, SkinBoss, NumOfSkinTypes};
        enum NotificationStyle {InsertNotif = 0, SmartInsertNotif, ReplaceNotif};
        enum TextInputState {NoInputState = -1, WordInputState, SentenceInputState};
        enum GameBoosters {Bst_AttackI = 0, Bst_AttackII, Bst_AttackIII, Bst_DefenceI, Bst_DefenceII, Bst_DefenceIII, Bst_SpeedI, Bst_SpeedII, Bst_UnitBuildBoostI, Bst_UnitBuildBoostII, Bst_HealthRecovery, NumOfGameBoosters};
        enum GameRanks {RecruitRank = 0, NumOfRanks};
        enum InputState{Nothing = -1, Pressed, Released, Hold};

        struct DifficultyInfo
        {
            int subShipMin = 0;
            int subShipMax = 0;
            int miniShip = 0;
            int microShip = 0;
            int mainDefence = 0;
            int miniDefence = 0;
            int baseCore = 0;
        };

        Game();
        ~Game();

        void SetGameState(GameState state_);
        GameState GetGameState();
        static sf::Color GetCurrentRankThemeColor();
        static const sf::Vector2f& GetScreenStretchFactor();
        static const DifficultyInfo& GetDifficultyInfo(int difficultyLevel);
        static const DifficultyInfo& GetDifficultyInfo();
        static double GetGameExecutionTimeInSec();

        static GUI::Style* GetGuiStyle(std::string id);
        static sf::Font* GetFont(std::string id);
        static sf::Color GetColor(std::string id);
        static const sf::Vector2f& GetGuiScreenEdgeALlowance();

        static void SetTeamUpType(short team_, short type_);
        static short GetTeamUpType(const short& team_);
        static void SetPlayerTeam(const short team_);
        static short GetPlayerTeam();

        static Skin* GetSkin(SkinType skinType, short index, short team);
        static int GetSkinSize(SkinType skinType, short team);
        static void InitializeSkin();

        static ShipInfo* GetCharacterInfo(unsigned short characterType, unsigned short index, unsigned short team);
        static int GetCharacterTypeInfoSize(unsigned short characterType, unsigned short team);
        static void UpdateCharacterTexture(unsigned short characterType, unsigned short index, unsigned short team, sf::Color color);
        static void UpdateCharacterTexture(ShipInfo* shipInfo, sf::Color color);
        static void UpdateCharacterTexture(sf::Color color);
        static ShipInfo* AddCharacterInfo(ShipInfo* shipInfo, unsigned short characterType);
        static ShipIdentity* AddShipIdentity(ShipInfo* shipInfo, sf::Vector2i grid_, bool isMainShip_ = false, bool isPlayable_ = false, short rotation_ = 0);
        static ShipIdentity* GetShipIdentity(int index);
        static ShipIdentity* GetShipIdentity(Ship* ship_);
        static ShipIdentity* GetShipIdentity(sf::Vector2i startGridPos);
        static void RemoveShipIdentity(ShipIdentity* identity);
        static unsigned int GetShipIdentitySize();
        static void ClearShipIdentities();
        static FormUnit* AddFormUnit(FormUnit* formUnit);
        static void RemoveFormUnit(FormUnit* formUnit);
        static FormUnit* GetFormUnit(int index);
        static FormUnit* GetFormUnit(sf::Vector2i grid);
        static bool isOnFormUnit(sf::IntRect rect);
        static unsigned int GetFormUnitSize();
        static void InitializeCharacterInfo();

        static sf::Vector2f GetMousePosition();
        static void DrawCursorCentered(const float& dt);
        static void DrawCursor();
        static void SetMousePosition(sf::Vector2f position);
        static void SetCursorTextrue(sf::Texture* texture_);
        static void SetCursorAttachment(sf::Texture* texture_ = nullptr);

        static void Initialise();
        static void LoadResources();
        static void Execute();

        static void Play(std::string file);
        static void MapEditior(std::string file = "");
        static void InitializeLevel();

        static void AddNotification(NotificationStyle style_, std::string text_, sf::Color boarderColor_ = GetColor("Default"), sf::Color textColor_ = sf::Color::White, float duration_ = 4.f);
        static void AddNotification(std::string text_, sf::Color boarderColor_ = GetColor("Default"), sf::Color textColor_ = sf::Color::White, float duration_ = 4.f);
        static void RemoveNotification(std::string text_);
        static void UpdateNotification(float dt);
        static void UpdateWorldTiles();
        static void UpdateWorldTiles(sf::IntRect range);
        static void UpdateFpsDisplay(float deltaTime, float cpuTime); //Remove later

        static void DrawFpsDisplay(); //Remove later
        static void DrawNotification();
        static void DrawFloors();
        static void DrawWalls_Lower();
        static void DrawWalls_Upper();
        static void DrawSmallWalls_Lower();
        static void DrawSmallWalls_Upper();
        static void DrawSpawnPoints();
        static void DrawEverything();

        static short CreateBase(short _team, sf::Vector2i _spGrid, sf::IntRect _range);
        static Base* GetBase(int id);
        static Base* GetMainBase(int team);
        static int GetNumberOfBases();
        static void DeletBase(int id);
        static void DeletAllBases();
        static void DrawAllBaseHighlight();
        static void DrawAllBaseOutline();

        static bool AddSpawnPointToControlUnit(sf::Vector2i spGrid_, short team_);
        static void RemoveSpawnPointFromControlUnit(sf::Vector2i spGrid_);
        static void ResetAllControlUnits();

        static bool IsControlUnitSpawnPointHere(sf::Vector2i grid_);
        static bool IsBaseSpawnPointHere(sf::Vector2i grid_);
        static bool IsAnyBaseCoreHere(sf::Vector2i grid_);

        static void HandleInputs();
        static void SetTextInputState(TextInputState state);
        static std::string& GetTextInput();
        static bool isMouseScrolled();
        static float GetMouseScrollDelta();
        static InputState GetInput(const sf::Keyboard::Key& key_);
        static InputState GetInput(const sf::Mouse::Button& button_);
        static bool GetInput(const sf::Keyboard::Key& key_, const InputState& state_);
        static bool GetInput(const sf::Mouse::Button& button_, const InputState& state_);

        static void SetLevelSize(const sf::Vector2i& size_);
        static void SetLevelMode(GameMode mode_);
        static sf::Vector2i GetLevelSize();
        static GameMode GetLevelMode();
        static unsigned int& GetLevelEnergy(short team_);

        static void UpdateMapLevelTexture(bool updateBaseTexture);
        static void UpdateMapTexture(float dt = 0);
        static void SetMapIconScale(float scale_);
        static const sf::Texture& GetMapTexture();

    private:
        sf::RenderWindow window;
        GameState gameState;

        static Game* gameInstance;
};

struct GridInfo
{
    std::vector<Ship*> ships;
    std::vector<Ship*> positionShips;
    short int tileId = -1;
    short int baseId = -1;
    Defence* defence = nullptr;
    bool baseArea = false;

    GridInfo(short tileId_ = -1, short baseId_ = -1, Defence* defence_ = nullptr)
    {
        tileId = tileId_;
        baseId = baseId_;
        defence = defence_;
    }
};

GridInfo& GetGridInfo(const int& y, const int& x);
GridInfo& GetGridInfo(const sf::Vector2i& grid_);
MapTileManager::Tile GetGridInfoTile(const int& y, const int& x);
void ClearEntireShipGridInfo(int yRange = MapTileManager::MAX_MAPTILE, int xRange = MapTileManager::MAX_MAPTILE);
void ClearEntireTileGridInfo(int yRange = MapTileManager::MAX_MAPTILE, int xRange = MapTileManager::MAX_MAPTILE);
void ClearEntireBaseIdGridInfo(int yRange = MapTileManager::MAX_MAPTILE, int xRange = MapTileManager::MAX_MAPTILE);
void ClearEntireGridInfo(int yRange = MapTileManager::MAX_MAPTILE, int xRange = MapTileManager::MAX_MAPTILE);

#endif // GAME_H_INCLUDED
