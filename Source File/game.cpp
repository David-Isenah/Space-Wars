#include "game.h"
#include "shipunit.h"
#include "shipdefence.h"
#include "bullet.h"
#include "base.h"
#include "iostream"
#include "assert.h"
#include "public_functions.h"
#include "teamcontrolunit.h"
#include "displayeffect_effects.h"
#include "astarpathfinding.h"
#include "gamegui.h"

std::map<std::string, GUI::Style*> gameGuiStyles;
std::map<std::string, sf::Font*> gameFonts;
std::map<std::string, sf::Color> gameColors;
std::vector<Base*> gameBases;
float gameBoostersTime[Game::NumOfGameBoosters];
int gameBoostersCost[Game::NumOfGameBoosters];
int gameBoostersRank[Game::NumOfGameBoosters];
bool gameBoostersStatus[Game::NumOfTeams][Game::NumOfGameBoosters];
sf::Clock gameExecutionTime;
Game::InputState gameInputs[sf::Keyboard::KeyCount + sf::Mouse::ButtonCount];
sf::Vector2f mousePosition;
sf::Sprite mouseCursor;
float scrollWheelDelta = 0;
float mouseAimSensitivity = 1.5f;
bool isScrolled = false;
bool isMousePlayerCentered = true;
bool isMouseOutsideWindow = true;
std::string textInput;
Game::TextInputState textInputState = Game::NoInputState;
GridInfo gamegridInfo[MapTileManager::MAX_MAPTILE][MapTileManager::MAX_MAPTILE];
TeamControlUnit teamcontrolunits[Game::NumOfTeams];
short teamUpType[4] = {-1, -1, -1, -1};
short gamePlayerTeam = -1;
short currentRank = 0;
short currentRankTheme = 0;
sf::Texture rankCursorTextures[Game::NumOfRanks];
sf::Color rankColors[Game::NumOfRanks];
Game::DifficultyInfo gameDifficultyInfo[10];
short currentDifficulty = 0;
Game::GameMode levelMode = Game::NoGameMode;
unsigned int levelTeamEnergy[Game::NumOfTeams] = {};

sf::Vector2i levelSize;
float mapRenderTime = 0;
float mapIconScale = 1.f;
sf::FloatRect mapShowScreenBoarder;
sf::RenderTexture mapLevelRenderTexture;
sf::RenderTexture mapBaseRenderTexture;
sf::RenderTexture mapRenderTexture;
sf::Vertex mapIconVertices_higherUnits[16];
sf::Vertex mapIconVertices_basecore[12];
sf::VertexArray map_va_smallUnits[Game::NumOfTeams];
sf::VertexArray map_va_defenceColor[Game::NumOfTeams];
sf::VertexArray map_va_defenceTrans[Game::NumOfTeams];
sf::VertexArray map_va_basecore;
sf::VertexArray map_va_higherUnits;

int Game::SCREEN_WIDTH = 1280;//sf::VideoMode::getDesktopMode().width;
int Game::SCREEN_HEIGHT = 720;//sf::VideoMode::getDesktopMode().height;
const float Game::MIN_FRAMERATE = 1 / 30.f;
sf::Vector2f screenStretchFactor = (Game::SCREEN_WIDTH < Game::SCREEN_HEIGHT ? sf::Vector2f(1, (float)Game::SCREEN_HEIGHT / Game::SCREEN_WIDTH) : sf::Vector2f((float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 1));
sf::Vector2f gameGuiScreenEdgeAllowance = sf::Vector2f(15, 15);//(sf::Vector2f)(sf::Vector2i)(sf::Vector2f(Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT) * 0.5f * 15.f * 2.f / 720.f);
bool gameIsFullscreen = false;

sf::VertexArray va_floor;
sf::VertexArray va_smallwall_lower;
sf::VertexArray va_smallwall_upper;
sf::VertexArray va_wall_lower;
sf::VertexArray va_wall_upper;
sf::VertexArray va_spawnpoint;
sf::VertexArray va_baseHighlight;

sf::Texture* tx_floor;
sf::Texture* tx_smallWall;
sf::Texture* tx_wall;
sf::Texture* tx_spawnpoint;

//Remove later
sf::Text hud_fps;
std::vector<float> hud_fps_frameTime;
std::vector<float> hud_fps_cpuTime;
float hud_fps_time = 0.f;

Game* Game::gameInstance = nullptr;
Game game;
AssetManager assetManagerInstance;

//GridInfo Functions
GridInfo& GetGridInfo(const int& y, const int& x)
{
    if(y < 0 || y >= MapTileManager::MAX_MAPTILE || x < 0 || x >= MapTileManager::MAX_MAPTILE)
    {
        return gamegridInfo[0][0];

        //Remove later
        std::cout << "...improper use of GetGridInfo : (" << x << ", " << y << ")\n";
    }
    return gamegridInfo[y][x];
}

GridInfo& GetGridInfo(const sf::Vector2i& grid_)
{
    return GetGridInfo(grid_.y, grid_.x);
}

MapTileManager::Tile GetGridInfoTile(const int& y, const int& x)
{
    if(y < 0 || y >= MapTileManager::MAX_MAPTILE || x < 0 || x >= MapTileManager::MAX_MAPTILE)
    {
        return MapTileManager::GetTile(gamegridInfo[0][0].tileId);

        //Remove later
        std::cout << "...improper use of GetGridInfoTile : (" << x << ", " << y << ")\n";
    }
    return MapTileManager::GetTile(gamegridInfo[y][x].tileId);
}

void ClearEntireShipGridInfo(int yRange, int xRange)
{
    for(int y = 0; y < yRange; y++)
        for(int x = 0; x < xRange; x++)
        {
            gamegridInfo[y][x].ships.clear();
            gamegridInfo[y][x].positionShips.clear();
        }
}

void ClearEntireTileGridInfo(int yRange, int xRange)
{
    for(int y = 0; y < yRange; y++)
        for(int x = 0; x < xRange; x++)
            gamegridInfo[y][x].tileId = -1;
}

void ClearEntireBaseIdGridInfo(int yRange, int xRange)
{
    for(int y = 0; y < yRange; y++)
        for(int x = 0; x < xRange; x++)
        {
            gamegridInfo[y][x].baseId = -1;
            gamegridInfo[y][x].baseArea = false;
        }
}

void ClearEntireGridInfo(int yRange, int xRange)
{
    for(int y = 0; y < yRange; y++)
        for(int x = 0; x < xRange; x++)
        {
            gamegridInfo[y][x].ships.clear();
            gamegridInfo[y][x].positionShips.clear();
            gamegridInfo[y][x].tileId = -1;
            gamegridInfo[y][x].baseId = -1;
            gamegridInfo[y][x].baseArea = false;
            gamegridInfo[y][x].defence = nullptr;
        }
}

//Game
Game::Game()
{
    assert(gameInstance == nullptr);
    gameInstance = this;

    window.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), WINDOW_TITLE, sf::Style::Close);
    //window.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), WINDOW_TITLE, sf::Style::Fullscreen);
    window.setMouseCursorVisible(true);
    window.setVerticalSyncEnabled(false);
    gameState = MapEdit;
    SetRandomSeed(time(NULL));
}

Game::~Game()
{
    for(auto itr = gameGuiStyles.begin(); itr != gameGuiStyles.end(); itr++)
        if(itr->second != nullptr)
            delete itr->second;

    for(auto itr = gameFonts.begin(); itr != gameFonts.end(); itr++)
        if(itr->second != nullptr)
            delete itr->second;

    std::cout << "BasesEnemy: " << Base::GetFrontLineBasesEnemy(0).size() << "\n";

    Bullet::DeleteBullets();
}

double Game::GetGameExecutionTimeInSec()
{
    return gameExecutionTime.getElapsedTime().asSeconds();
}

const sf::Vector2f& Game::GetScreenStretchFactor()
{
    return screenStretchFactor;
}

float Game::GetMouseScrollDelta()
{
    return scrollWheelDelta;
}

bool Game::isMouseScrolled()
{
    return isScrolled;
}

void Game::SetTextInputState(Game::TextInputState state)
{
    textInput.clear();
    textInputState = state;
}

std::string& Game::GetTextInput()
{
    return textInput;
}

void Game::HandleInputs()
{
    //handling inputs
    for(int i = 0; i < sf::Keyboard::KeyCount; i++)
        gameInputs[i] = gameInstance->window.hasFocus() && sf::Keyboard::isKeyPressed((sf::Keyboard::Key)i) ? Hold : Nothing;
    for(int i = 0; i < sf::Mouse::ButtonCount; i++)
        gameInputs[sf::Keyboard::KeyCount + i] = gameInstance->window.hasFocus() && sf::Mouse::isButtonPressed((sf::Mouse::Button)i) ? Hold : Nothing;

    //handle events
    sf::Event event;
    bool lastMouseOutsideState = isMouseOutsideWindow;
    scrollWheelDelta = 0;
    isScrolled = false;
    while(gameInstance->window.pollEvent(event))
    {
        if(event.type == sf::Event::Closed)
            gameInstance->window.close();
        if(event.type == sf::Event::KeyPressed)
            gameInputs[event.key.code] = Pressed;
        if(event.type == sf::Event::KeyReleased)
            gameInputs[event.key.code] = Released;
        if(event.type == sf::Event::MouseButtonPressed)
            gameInputs[sf::Keyboard::KeyCount + event.mouseButton.button] = Pressed;
        if(event.type == sf::Event::MouseButtonReleased)
            gameInputs[sf::Keyboard::KeyCount + event.mouseButton.button] = Released;
        if(event.type == sf::Event::MouseWheelScrolled && event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
        {
            scrollWheelDelta = event.mouseWheelScroll.delta;
            isScrolled = true;
        }
        if(event.type == sf::Event::MouseMoved && gameInstance->window.hasFocus())
        {
            if(!lastMouseOutsideState)
            {
                if(!gameIsFullscreen && (mousePosition.x < 0 || mousePosition.x > SCREEN_WIDTH || mousePosition.y < 0 || mousePosition.y > SCREEN_HEIGHT) && ShipUnit::IsActivePlayerManualAiming() == false)
                {
                    sf::Mouse::setPosition((sf::Vector2i)mousePosition, gameInstance->window);
                    gameInstance->window.setMouseCursorVisible(true);
                    isMouseOutsideWindow = true;
                }
                else
                {
                    mousePosition += sf::Vector2f(event.mouseMove.x - SCREEN_WIDTH / 2.f, event.mouseMove.y - SCREEN_HEIGHT / 2.f) * (ShipUnit::IsActivePlayerManualAiming() ? mouseAimSensitivity : 1.f);
                    sf::Mouse::setPosition(sf::Vector2i(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f), gameInstance->window);
                    if(gameIsFullscreen && ShipUnit::IsActivePlayerManualAiming() == false)
                    {
                        if(mousePosition.x < 0) mousePosition.x = 0;
                        else if(mousePosition.x > SCREEN_WIDTH) mousePosition.x = SCREEN_WIDTH;
                        if(mousePosition.y < 0) mousePosition.y = 0;
                        else if(mousePosition.y > SCREEN_HEIGHT) mousePosition.y = SCREEN_HEIGHT;
                    }
                }
            }
            else if(event.mouseMove.x >= 0 && event.mouseMove.y >= 0 && event.mouseMove.y <= SCREEN_HEIGHT && event.mouseMove.x <= SCREEN_WIDTH)
            {
                isMouseOutsideWindow = false;
                gameInstance->window.setMouseCursorVisible(false);
                mousePosition = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
            }
        }
        if(textInputState != NoInputState && event.type == sf::Event::TextEntered)
        {
            if(textInputState == WordInputState)
            {
                if(textInput.length() < 10)
                {
                    if(event.text.unicode >= 48 && event.text.unicode <= 57) //Numbers
                        textInput.push_back(event.text.unicode);
                    else if(event.text.unicode >= 65 && event.text.unicode <= 122) //Letters
                        textInput.push_back(event.text.unicode);
                }
                if(event.text.unicode == '\b' && textInput.length() > 0)
                    textInput.pop_back();
            }
            else if(textInputState == SentenceInputState)
            {
                //All Characters
                if(event.text.unicode >= 32 && event.text.unicode <= 126)
                    textInput.push_back(event.text.unicode);
                else if(event.text.unicode = '\r')
                    textInput.push_back('\n');
                else if(event.text.unicode = '\b' && textInput.length() > 0)
                    textInput.pop_back();
            }
        }
    }

    if(!isMouseOutsideWindow)
    {
        if(lastMouseOutsideState)
            sf::Mouse::setPosition(sf::Vector2i(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f), gameInstance->window);
        else
        {
            sf::Vector2i realMousePosition = sf::Mouse::getPosition(gameInstance->window);
            if(realMousePosition.x < 0 || realMousePosition.x > SCREEN_WIDTH || realMousePosition.y < 0 || realMousePosition.y > SCREEN_HEIGHT)
            {
                isMouseOutsideWindow = true;
                gameInstance->window.setMouseCursorVisible(true);
            }
        }
    }
    GUI::SetMouseScrollDalta(-scrollWheelDelta);
}

Game::InputState Game::GetInput(const sf::Keyboard::Key& key_)
{
    if(key_ >= 0 && key_ < sf::Keyboard::KeyCount + sf::Mouse::ButtonCount)
        return gameInputs[key_];
    return Nothing;
}

Game::InputState Game::GetInput(const sf::Mouse::Button& button_)
{
    if(button_ >= 0 && button_ < sf::Mouse::ButtonCount)
        return gameInputs[button_ + sf::Keyboard::KeyCount];
    return Nothing;
}

bool Game::GetInput(const sf::Keyboard::Key& key_, const InputState& state_)
{
    if(key_ >= 0 && key_ < sf::Keyboard::KeyCount + sf::Mouse::ButtonCount)
        if(gameInputs[key_] == state_ || (state_ == Hold && gameInputs[key_] == Pressed))
            return true;
    return false;
}

bool Game::GetInput(const sf::Mouse::Button& button_, const InputState& state_)
{
    if(button_ >= 0 && button_ < sf::Mouse::ButtonCount)
        if(gameInputs[button_ + sf::Keyboard::KeyCount] == state_ || (state_ == Hold && gameInputs[button_ + sf::Keyboard::KeyCount] == Pressed))
            return true;
    return false;
}

sf::Vector2f Game::GetMousePosition()
{
    return mousePosition;
}

void Game::SetMousePosition(sf::Vector2f position)
{
    //sf::Mouse::setPosition((sf::Vector2i)position, gameInstance->window);
    mousePosition = position;
    mouseCursor.setPosition(position);
}

void Game::DrawCursor()
{
    mouseCursor.setPosition((sf::Vector2f)mousePosition);
    gameInstance->window.setView(gameInstance->window.getDefaultView());
    gameInstance->window.draw(mouseCursor);
}

void Game::DrawCursorCentered(const float& dt)
{
    float tendAngle = 0;
    float cursorAngle = mouseCursor.getRotation();
    if(isMousePlayerCentered && ShipUnit::GetActivePlayerUnit() != nullptr)
    {
        sf::FloatRect camRect = EntityManager::GetCameraRect();
        sf::Vector2f diff = (sf::Vector2f)mousePosition - (ShipUnit::GetActivePlayerUnit()->GetPosition() - sf::Vector2f(camRect.left, camRect.top)) * (SCREEN_HEIGHT / camRect.height);
        tendAngle = 90 + (atan2f(diff.y, diff.x) * 180 / 3.142f) + 45;
    }

    cursorAngle += (int)(-cursorAngle / 360.f) * 360;
    if(cursorAngle < 0)
        cursorAngle += 360;

    tendAngle += (int)(-tendAngle / 360.f) * 360;
    if(tendAngle < 0)
        tendAngle += 360;

    float refAngle[3];
    refAngle[0] = tendAngle;
    refAngle[1] = refAngle[0] + 360;
    refAngle[2] = refAngle[0] - 360;

    float tendToAngle = 0.f;
    float minDiff = 0.f;
    for(int a = 0; a < 3; a++)
    {
        float diff = refAngle[a] - cursorAngle;
        if(diff < 0)
            diff = -diff;

        if(a == 0 || diff < minDiff)
        {
            tendToAngle = refAngle[a];
            minDiff = diff;
        }
    }
    TendTowards(cursorAngle, tendToAngle, 0.05f, 0.5f, dt);
    mouseCursor.setRotation(cursorAngle);

    float scale = mouseCursor.getScale().x;
    if(ShipUnit::IsActivePlayerJustShooting() && ShipUnit::IsActivePlayerManualAiming() && ShipUnit::IsActivePlayerAiming())
        scale = 2.f;
    else TendTowards(scale, 1.f, 0.15f, 0, dt);

    mouseCursor.setScale(scale, scale);
    mouseCursor.setOrigin(sf::Vector2f(mouseCursor.getLocalBounds().width, mouseCursor.getLocalBounds().height) * (scale / 2.f - 0.5f) / scale);

    sf::Vector2f newPos = mousePosition;
    if(ShipUnit::IsActivePlayerManualAiming())
    {
        newPos = mouseCursor.getPosition();
        TendTowards(newPos.x, mousePosition.x, 0.2, 0, dt);
        TendTowards(newPos.y, mousePosition.y, 0.2, 0, dt);
    }
    mouseCursor.setPosition(newPos);

    gameInstance->window.setView(gameInstance->window.getDefaultView());
    gameInstance->window.draw(mouseCursor);
}

void Game::SetCursorTextrue(sf::Texture* texture_)
{
    if(texture_ != nullptr)
        mouseCursor.setTexture(*texture_);
}

void Game::SetCursorAttachment(sf::Texture* texture_)
{

}

void Game::UpdateFpsDisplay(float deltaTime, float cpuTime)
{
    hud_fps_frameTime.push_back(deltaTime);
    hud_fps_cpuTime.push_back(cpuTime);

    hud_fps_time += deltaTime;
    if(hud_fps_time >= 0.5)
    {
        hud_fps_time -= 0.5;

        float average_dt = 0;
        for(int a = 0; a < hud_fps_frameTime.size(); a++)
            average_dt += hud_fps_frameTime[a];
        average_dt /= hud_fps_frameTime.size();
        hud_fps_frameTime.clear();

        int frameRate = 1.f / average_dt;
        //if(frameRate > 60)
            //frameRate = 60;
        std::stringstream ss;
        ss << "f: " << frameRate;

        for(int a = 0; a < hud_fps_cpuTime.size(); a++)
            average_dt += hud_fps_cpuTime[a];
        average_dt /= hud_fps_cpuTime.size();
        hud_fps_cpuTime.clear();
        ss << " - c:" << (int)(average_dt / (1.f / 60.f) * 100);

        hud_fps.setString(ss.str());
        hud_fps.setOrigin(sf::Vector2f(0, 0));
        hud_fps.setPosition(SCREEN_WIDTH - hud_fps.getGlobalBounds().width - 8, SCREEN_HEIGHT - hud_fps.getGlobalBounds().height - 8);
    }
}

void Game::DrawFpsDisplay()
{
    gameInstance->window.draw(hud_fps);
}

void Game::SetLevelSize(const sf::Vector2i& size_)
{
    levelSize = size_;
}

sf::Vector2i Game::GetLevelSize()
{
    return levelSize;
}

void Game::SetLevelMode(Game::GameMode mode_)
{
    levelMode = mode_;
}

Game::GameMode Game::GetLevelMode()
{
    return levelMode;
}

unsigned int& Game::GetLevelEnergy(short team_)
{
    if(team_ >= 0 && team_ < Game::NumOfTeams)
        return levelTeamEnergy[team_];
    return levelTeamEnergy[0];
}

void Game::Initialise()
{
    //Resources
    LoadResources();

    //Some game components
    mousePosition = sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    for(int a = 0; a < sf::Keyboard::KeyCount + sf::Mouse::ButtonCount; a++)
        gameInputs[a] = Nothing;

    for(int a = 0; a < NumOfGameBoosters; a++)
    {
        gameBoostersTime[a] = 100;
        gameBoostersCost[a] = 240;
        if(a > 5)
            gameBoostersRank[a] = 1;
        else gameBoostersRank[a] = 0;
    }

    va_floor.setPrimitiveType(sf::Quads);
    va_wall_lower.setPrimitiveType(sf::Quads);
    va_wall_upper.setPrimitiveType(sf::Quads);
    va_smallwall_lower.setPrimitiveType(sf::Quads);
    va_smallwall_upper.setPrimitiveType(sf::Quads);
    va_spawnpoint.setPrimitiveType(sf::Quads);
    va_baseHighlight.setPrimitiveType(sf::Quads);

    for(int a = 0; a < NumOfTeams; a++)
    {
        map_va_smallUnits[a].setPrimitiveType(sf::Quads);
        map_va_defenceColor[a].setPrimitiveType(sf::Quads);
        map_va_defenceTrans[a].setPrimitiveType(sf::Quads);
    }
    map_va_higherUnits.setPrimitiveType(sf::Quads);
    map_va_basecore.setPrimitiveType(sf::Quads);

    tx_floor = &AssetManager::GetTexture("TileSheet_Floor");
    tx_smallWall = &AssetManager::GetTexture("TileSheet_SmallWall");
    tx_wall = &AssetManager::GetTexture("TileSheet_Wall");
    tx_spawnpoint = &AssetManager::GetTexture("TileSheet_SpawnPoint");
    Base::outlineTexture = &AssetManager::GetTexture("Editor_Outline");

    //Remove later
    hud_fps.setCharacterSize(15);
    hud_fps.setFont(*GetFont("Default"));
    hud_fps.setFillColor(sf::Color::White);

    //Skins
    InitializeSkin();

    //Bullets
    Bullet::InitializeBullets();

    //Characters
    InitializeCharacterInfo();

    //Display Effect
    DisplayEffect::Initialise();

    //Initialize MultiDrawers
    Defence::InitialiseMultiDrawer();
    ShipDefence::InitialiseMultiDrawer();
    ShipUnit::InitialiseMultiDrawer();

    //Game GUI
    InitialiseGameGui();

    //Difficulty Info
    /*gameDifficultyInfo[0].subShipMax = 15;
    gameDifficultyInfo[0].subShipMin = 10;
    gameDifficultyInfo[0].microShip = 4;
    gameDifficultyInfo[0].miniShip = 6;
    gameDifficultyInfo[0].mainDefence = 10;
    gameDifficultyInfo[0].miniDefence = 8;
    gameDifgameDifficultyInfo[0].baseCore = 1000*/

    gameDifficultyInfo[0].subShipMax = 200;
    gameDifficultyInfo[0].subShipMin = 150;
    gameDifficultyInfo[0].microShip = 100;
    gameDifficultyInfo[0].miniShip = 100;
    gameDifficultyInfo[0].mainDefence = 400;
    gameDifficultyInfo[0].miniDefence = 200;
    gameDifficultyInfo[0].baseCore = 10000;

    //Initialize Map Variables
    //__Higher Units
    mapIconVertices_higherUnits[0] = sf::Vertex(sf::Vector2f(-67, 21), sf::Color::Transparent);
    mapIconVertices_higherUnits[1] = sf::Vertex(sf::Vector2f(0, -47), sf::Color::Transparent);
    mapIconVertices_higherUnits[2] = sf::Vertex(sf::Vector2f(67, 21), sf::Color::Transparent);
    mapIconVertices_higherUnits[3] = sf::Vertex(sf::Vector2f(0, 21), sf::Color::Transparent);

    mapIconVertices_higherUnits[4] = sf::Vertex(sf::Vector2f(-34, 13), sf::Color::Transparent);
    mapIconVertices_higherUnits[5] = sf::Vertex(sf::Vector2f(0, -21), sf::Color::Transparent);
    mapIconVertices_higherUnits[6] = sf::Vertex(sf::Vector2f(34, 13), sf::Color::Transparent);
    mapIconVertices_higherUnits[7] = sf::Vertex(sf::Vector2f(0, 47), sf::Color::Transparent);

    mapIconVertices_higherUnits[8] = sf::Vertex(sf::Vector2f(-48, 13), sf::Color::White);
    mapIconVertices_higherUnits[9] = sf::Vertex(sf::Vector2f(0, -35), sf::Color::White);
    mapIconVertices_higherUnits[10] = sf::Vertex(sf::Vector2f(48, 13), sf::Color::White);
    mapIconVertices_higherUnits[11] = sf::Vertex(sf::Vector2f(0, 13), sf::Color::White);

    mapIconVertices_higherUnits[12] = sf::Vertex(sf::Vector2f(-23, 13), sf::Color::Black);
    mapIconVertices_higherUnits[13] = sf::Vertex(sf::Vector2f(0, -10), sf::Color::Black);
    mapIconVertices_higherUnits[14] = sf::Vertex(sf::Vector2f(23, 13), sf::Color::Black);
    mapIconVertices_higherUnits[15] = sf::Vertex(sf::Vector2f(0, 35), sf::Color::Black);

    //__Base Core
    mapIconVertices_basecore[0].position = sf::Vector2f(-13, -13);
    mapIconVertices_basecore[1].position = sf::Vector2f(13, -13);
    mapIconVertices_basecore[2].position = sf::Vector2f(13, 13);
    mapIconVertices_basecore[3].position = sf::Vector2f(-13, 13);
    for(int a = 0; a < 4; a++)
        mapIconVertices_basecore[a].color = sf::Color::Transparent;

    mapIconVertices_basecore[4].position = sf::Vector2f(-10, -10);
    mapIconVertices_basecore[5].position = sf::Vector2f(10, -10);
    mapIconVertices_basecore[6].position = sf::Vector2f(10, 10);
    mapIconVertices_basecore[7].position = sf::Vector2f(-10, 10);

    mapIconVertices_basecore[8].position = sf::Vector2f(-10, -10);
    mapIconVertices_basecore[9].position = sf::Vector2f(0, -10);
    mapIconVertices_basecore[10].position = sf::Vector2f(0, 0);
    mapIconVertices_basecore[11].position = sf::Vector2f(-10, 0);
    for(int a = 0; a < 8; a++)
        mapIconVertices_basecore[4 + a].color = sf::Color::White;
}

void Game::LoadResources()
{
    //Textures
    AssetManager::LoadTexture("TileSheet_Floor", "Resources/Images/Tile Sheets/tilesheet_floor.png");
    AssetManager::LoadTexture("TileSheet_SmallWall", "Resources/Images/Tile Sheets/tilesheet_smallwall.png");
    AssetManager::LoadTexture("TileSheet_Wall", "Resources/Images/Tile Sheets/tilesheet_wall.png");
    AssetManager::LoadTexture("TileSheet_SpawnPoint", "Resources/Images/Tile Sheets/tilesheet_spawnpoint.png");
    AssetManager::LoadTexture("TileGrid", "Resources/Images/Others/tilegrid.png");

    AssetManager::LoadTexture("Editor_Default", "Resources/Images/GUI/button_editor_default.png");
    AssetManager::LoadTexture("Editor_Save", "Resources/Images/GUI/button_editor_save.png");
    AssetManager::LoadTexture("Editor_Exit", "Resources/Images/GUI/button_editor_exit.png");
    AssetManager::LoadTexture("Editor_Arrow", "Resources/Images/GUI/button_editor_arrow.png");
    AssetManager::LoadTexture("Editor_SmallArrow", "Resources/Images/GUI/button_editor_smallarrow.png");
    AssetManager::LoadTexture("Editor_Mini", "Resources/Images/GUI/button_editor_mini.png");
    AssetManager::LoadTexture("Editor_Selection", "Resources/Images/GUI/button_editor_selection.png");
    AssetManager::LoadTexture("Editor_TileModes", "Resources/Images/GUI/button_editor_tilemodes.png");
    AssetManager::LoadTexture("Editor_NewMenu", "Resources/Images/GUI/button_editor_newmenu.png");
    AssetManager::LoadTexture("Editor_SelectMode", "Resources/Images/GUI/button_editor_selectmode.png");
    AssetManager::LoadTexture("Editor_TeamStatus", "Resources/Images/GUI/button_editor_teamstatus.png");
    AssetManager::LoadTexture("Editor_TeamOption", "Resources/Images/GUI/button_editor_teamoption.png");
    AssetManager::LoadTexture("Editor_ShipType", "Resources/Images/GUI/button_editor_shiptype.png");
    AssetManager::LoadTexture("Editor_Outline", "Resources/Images/GUI/outline.png");
    AssetManager::LoadTexture("Editor_BS_SS_Help", "Resources/Images/GUI/bs_ss_help.png");
    AssetManager::LoadTexture("Editor_StateDisplay", "Resources/Images/GUI/statedisplay.png");
    AssetManager::LoadTexture("Editor_Highlight_Position", "Resources/Images/GUI/highlight_position.png");
    AssetManager::LoadTexture("Editor_Plain", "Resources/Images/GUI/button_editor_plain.png");

    AssetManager::LoadTexture("Level_HideSubShips", "Resources/Images/GUI/button_level_hidesubships.png");

    AssetManager::LoadTexture("BaseCore_Alpha", "Resources/Images/basecorealpha.png");
    AssetManager::LoadTexture("BaseCore_Delta", "Resources/Images/basecoredelta.png");
    AssetManager::LoadTexture("BaseCore_Vortex", "Resources/Images/basecorevortex.png");
    AssetManager::LoadTexture("BaseCore_Omega", "Resources/Images/basecoreomega.png");

    AssetManager::LoadTexture("Icon_ShipVersus", "Resources/Images/GUI/icon_shipversus.png");
    AssetManager::LoadTexture("Icon_EditTile", "Resources/Images/GUI/icon_edittile.png");
    AssetManager::LoadTexture("Icon_SetupBase", "Resources/Images/GUI/icon_setupbase.png");
    AssetManager::LoadTexture("Icon_SetupShip", "Resources/Images/GUI/icon_setupship.png");
    AssetManager::LoadTexture("Icon_Mini_White", "Resources/Images/GUI/icon_mini_white.png");
    AssetManager::LoadTexture("Icon_SelectMode", "Resources/Images/GUI/icon_selectmode.png");
    AssetManager::LoadTexture("Icon_TeamStatus", "Resources/Images/GUI/icon_teamstatus.png");
    AssetManager::LoadTexture("Icon_ShipType", "Resources/Images/GUI/icon_shiptype.png");
    AssetManager::LoadTexture("Icon_States", "Resources/Images/GUI/icon_states.png");
    AssetManager::LoadTexture("Icon_Boosters", "Resources/Images/GUI/icon_boosters.png");
    AssetManager::LoadTexture("Icon_NoTeam", "Resources/Images/GUI/icon_noteam.png");
    AssetManager::LoadTexture("Team_Logo", "Resources/Images/teamlogo.png");
    AssetManager::LoadTexture("ShipInfo_Data", "Resources/Images/GUI/shipinfodata.png");
    AssetManager::LoadTexture("Defence_Spot", "Resources/Images/Others/defencespot.png");

    AssetManager::LoadTexture("DE_Shoot_Laser_1", "Resources/Images/Others/shoot_laser1.png");
    AssetManager::LoadTexture("DE_Shoot_Laser_2", "Resources/Images/Others/shoot_laser2.png");
    AssetManager::LoadTexture("DE_Hit_Laser", "Resources/Images/Others/hit_laser.png");
    AssetManager::LoadTexture("DE_DamageDisplay_Numbers", "Resources/Images/Others/damagedisplay_numbers.png");

    AssetManager::LoadTexture("Cursor_MapEditor", "Resources/Images/Cursors/cursor_mapeditor.png");

    AssetManager::LoadTexture("Popup_Head", "Resources/Images/GUI/popup_head.png");
    AssetManager::LoadTexture("FocusedShipButton_Others", "Resources/Images/GUI/focusedshipbutton_others.png");
    AssetManager::LoadTexture("Guns", "Resources/Images/guns.png");
    AssetManager::LoadTexture("Shape_Star", "Resources/Images/Others/shape_star.png");

    //Preparing some textures
    {
        sf::RenderTexture rTexture;
        rTexture.setSmooth(true);

        //Circle
        {
            rTexture.create(600, 600);
            rTexture.clear(sf::Color::Transparent);

            sf::CircleShape circle(300 - 1, 1000);
            circle.setPosition(1, 1);
            circle.setFillColor(sf::Color::White);

            rTexture.draw(circle);
            rTexture.display();

            AssetManager::LoadTexture("Shape_Circle", sf::Texture(rTexture.getTexture()));
        }
    }

    //Fonts
    sf::Font* font_default = new sf::Font();
    font_default->loadFromFile("Resources/Fonts/square721bt.ttf");
    gameFonts.insert(std::make_pair("Default", font_default));

    sf::Font* font_conthrax = new sf::Font();
    font_conthrax->loadFromFile("Resources/Fonts/conthrax-sb.ttf");
    gameFonts.insert(std::make_pair("Conthrax", font_conthrax));

    sf::Font* font_praetorian = new sf::Font();
    font_praetorian->loadFromFile("Resources/Fonts/praetorian.ttf");
    gameFonts.insert(std::make_pair("Praetorian", font_praetorian));

    //Colors
    gameColors["Default"] = sf::Color(123, 161, 179);
    gameColors["Default_Light"] = sf::Color(163, 190, 203);
    gameColors["Default_Lighter"] = sf::Color(200, 216, 224);
    gameColors["Alpha"] = sf::Color(207, 117, 117);
    gameColors["Delta"] = sf::Color(182, 189, 116);
    gameColors["Vortex"] = sf::Color(255, 210, 31);
    gameColors["Omega"] = sf::Color(126, 151, 154);
    gameColors["Alpha_Light"] = sf::Color(212, 130, 130);
    gameColors["Delta_Light"] = sf::Color(185, 193, 119);
    gameColors["Vortex_Light"] = sf::Color(255, 219, 93);
    gameColors["Omega_Light"] = sf::Color(129, 158, 161);

    //Rank Data
    rankCursorTextures[RecruitRank].loadFromFile("Resources/Images/Cursors/cursor_gameplay#01.png");
    rankColors[RecruitRank] = sf::Color(130, 135, 150);

    //Styles
    GUI::Style* style_editor_default = new GUI::Style();
    style_editor_default->setFontParameters(32, GetColor("Default_Light"));
    style_editor_default->setIconRef(sf::Vector2f(0, -32));
    style_editor_default->setTextRef(sf::Vector2f(0, 60));
    style_editor_default->setTextureRect(sf::IntRect(0, 0, 262, 206));
    style_editor_default->setTransmissionFactor();
    style_editor_default->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(-300, 0), sf::Vector2f(), 255);
    style_editor_default->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(-300, 0), sf::Vector2f(), 255);
    style_editor_default->setPhase(GUI::DefaultPhase, 1, 0);
    style_editor_default->setPhase(GUI::HighlighPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(262, 206));
    style_editor_default->setPhase(GUI::PressedPhase, 1, 2, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(262, 206));
    style_editor_default->setPhase(GUI::LockedPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 50);
    gameGuiStyles.insert(std::make_pair("Editor_Default", style_editor_default));

    GUI::Style* style_editor_hider = new GUI::Style();
    style_editor_hider->setTextureRect(sf::IntRect(0, 0, 10, SCREEN_HEIGHT / 2));
    style_editor_hider->setTransmissionFactor();
    style_editor_hider->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(-310, 0), sf::Vector2f(), 100);
    style_editor_hider->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(-310, 0), sf::Vector2f(), 100);
    style_editor_hider->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 100, sf::Vector2f(10, SCREEN_HEIGHT));
    style_editor_hider->setPhase(GUI::HighlighPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 200, sf::Vector2f(10, SCREEN_HEIGHT));
    style_editor_hider->setPhase(GUI::PressedPhase, 1, 2, sf::Vector2f(), sf::Vector2f(), 200, sf::Vector2f(10, SCREEN_HEIGHT));
    gameGuiStyles.insert(std::make_pair("Editor_Hider", style_editor_hider));

    GUI::Style* style_editor_exit = new GUI::Style();
    style_editor_exit->setTextureRect(sf::IntRect(0, 0, 61, 61));
    style_editor_exit->setTransmissionFactor();
    style_editor_exit->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(-300, 0), sf::Vector2f(), 255);
    style_editor_exit->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(-300, 0), sf::Vector2f(), 255);
    style_editor_exit->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_editor_exit->setPhase(GUI::HighlighPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(61, 61));
    style_editor_exit->setPhase(GUI::PressedPhase, 1, 2, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(61, 61));
    gameGuiStyles.insert(std::make_pair("Editor_Exit", style_editor_exit));

    GUI::Style* style_editor_save = new GUI::Style();
    style_editor_save->setTextureRect(sf::IntRect(0, 0, 181, 61));
    style_editor_save->setTransmissionFactor();
    style_editor_save->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(-300, 0), sf::Vector2f(), 255);
    style_editor_save->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(-300, 0), sf::Vector2f(), 255);
    style_editor_save->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_editor_save->setPhase(GUI::HighlighPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(181, 61));
    style_editor_save->setPhase(GUI::PressedPhase, 1, 2, sf::Vector2f(), sf::Vector2f(), 255), sf::Vector2f(181, 61);
    gameGuiStyles.insert(std::make_pair("Editor_Save", style_editor_save));

    GUI::Style* style_editor_tilemodes_floor = new GUI::Style();
    style_editor_tilemodes_floor->setTextureRect(sf::IntRect(0, 0, 60, 60));
    style_editor_tilemodes_floor->setTransmissionFactor();
    style_editor_tilemodes_floor->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(-60, 0));
    style_editor_tilemodes_floor->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(-60, 0));
    style_editor_tilemodes_floor->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_editor_tilemodes_floor->setPhase(GUI::HighlighPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_editor_tilemodes_floor->setPhase(GUI::PressedPhase, 0.9, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    gameGuiStyles.insert(std::make_pair("Editor_TileModes_Floor", style_editor_tilemodes_floor));

    GUI::Style* style_editor_tilemodes_floor_on = new GUI::Style();
    style_editor_tilemodes_floor_on->setTextureRect(sf::IntRect(0, 0, 60, 60));
    style_editor_tilemodes_floor_on->setTransmissionFactor();
    style_editor_tilemodes_floor_on->setPhase(GUI::StartPhase, 1, 1, sf::Vector2f(-60, 0));
    style_editor_tilemodes_floor_on->setPhase(GUI::InactivePhase, 1, 1, sf::Vector2f(-60, 0));
    style_editor_tilemodes_floor_on->setPhase(GUI::DefaultPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255);
    style_editor_tilemodes_floor_on->setPhase(GUI::HighlighPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_editor_tilemodes_floor_on->setPhase(GUI::PressedPhase, 0.9, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    gameGuiStyles.insert(std::make_pair("Editor_TileModes_Floor_On", style_editor_tilemodes_floor_on));

    GUI::Style* style_editor_tilemodes_smallWall = new GUI::Style();
    style_editor_tilemodes_smallWall->copyFrom(*style_editor_tilemodes_floor);
    style_editor_tilemodes_smallWall->setTextureRect(sf::IntRect(60, 0, 60, 60));
    gameGuiStyles.insert(std::make_pair("Editor_TileModes_SmallWall", style_editor_tilemodes_smallWall));

    GUI::Style* style_editor_tilemodes_smallWall_on = new GUI::Style();
    style_editor_tilemodes_smallWall_on->copyFrom(*style_editor_tilemodes_floor_on);
    style_editor_tilemodes_smallWall_on->setTextureRect(sf::IntRect(60, 0, 60, 60));
    gameGuiStyles.insert(std::make_pair("Editor_TileModes_SmallWall_On", style_editor_tilemodes_smallWall_on));

    GUI::Style* style_editor_tilemodes_empty = new GUI::Style();
    style_editor_tilemodes_empty->copyFrom(*style_editor_tilemodes_floor);
    style_editor_tilemodes_empty->setTextureRect(sf::IntRect(180, 0, 60, 60));
    gameGuiStyles.insert(std::make_pair("Editor_TileModes_Empty", style_editor_tilemodes_empty));

    GUI::Style* style_editor_tilemodes_empty_on = new GUI::Style();
    style_editor_tilemodes_empty_on->copyFrom(*style_editor_tilemodes_floor_on);
    style_editor_tilemodes_empty_on->setTextureRect(sf::IntRect(180, 0, 60, 60));
    gameGuiStyles.insert(std::make_pair("Editor_TileModes_Empty_On", style_editor_tilemodes_empty_on));

    GUI::Style* style_editor_tilemodes_wall = new GUI::Style();
    style_editor_tilemodes_wall->setTextureRect(sf::IntRect(120, 0, 60, 77));
    style_editor_tilemodes_wall->setTransmissionFactor();
    style_editor_tilemodes_wall->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(-60, 0));
    style_editor_tilemodes_wall->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(-60, 0));
    style_editor_tilemodes_wall->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_editor_tilemodes_wall->setPhase(GUI::HighlighPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 77));
    style_editor_tilemodes_wall->setPhase(GUI::PressedPhase, 0.9, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 77));
    gameGuiStyles.insert(std::make_pair("Editor_TileModes_Wall", style_editor_tilemodes_wall));

    GUI::Style* style_editor_tilemodes_wall_on = new GUI::Style();
    style_editor_tilemodes_wall_on->setTextureRect(sf::IntRect(120, 0, 60, 77));
    style_editor_tilemodes_wall_on->setTransmissionFactor();
    style_editor_tilemodes_wall_on->setPhase(GUI::StartPhase, 1, 1, sf::Vector2f(-60, 0));
    style_editor_tilemodes_wall_on->setPhase(GUI::InactivePhase, 1, 1, sf::Vector2f(-60, 0));
    style_editor_tilemodes_wall_on->setPhase(GUI::DefaultPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255);
    style_editor_tilemodes_wall_on->setPhase(GUI::HighlighPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 77));
    style_editor_tilemodes_wall_on->setPhase(GUI::PressedPhase, 0.9, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 77));
    gameGuiStyles.insert(std::make_pair("Editor_TileModes_Wall_On", style_editor_tilemodes_wall_on));

    GUI::Style* style_editor_mini = new GUI::Style();
    style_editor_mini->copyFrom(*style_editor_tilemodes_floor);
    style_editor_mini->setPhase(GUI::PressedPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_editor_mini->setPhase(GUI::LockedPhase, 1, 1, sf::Vector2f(-60, 0), sf::Vector2f(), 0);
    gameGuiStyles.insert(std::make_pair("Editor_Mini", style_editor_mini));

    GUI::Style* style_editor_mini_off = new GUI::Style();
    style_editor_mini_off->copyFrom(*style_editor_mini);
    style_editor_mini_off->setTextureRect(sf::IntRect(180, 0, 60, 60));
    style_editor_mini_off->setPhase(GUI::PressedPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_editor_mini_off->setPhase(GUI::LockedPhase, 1, 0, sf::Vector2f(-60, 0), sf::Vector2f(), 0);
    gameGuiStyles.insert(std::make_pair("Editor_Mini_Off", style_editor_mini_off));

    GUI::Style* style_editor_mini_on = new GUI::Style();
    style_editor_mini_on->copyFrom(*style_editor_mini);
    style_editor_mini_on->setTextureRect(sf::IntRect(180, 0, 60, 60));
    style_editor_mini_on->setPhase(GUI::StartPhase, 1, 1, sf::Vector2f(-60, 0), sf::Vector2f(), 255);
    style_editor_mini_on->setPhase(GUI::InactivePhase, 1, 1, sf::Vector2f(-60, 0), sf::Vector2f(), 255);
    style_editor_mini_on->setPhase(GUI::DefaultPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_editor_mini_on->setPhase(GUI::HighlighPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_editor_mini_on->setPhase(GUI::PressedPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_editor_mini_on->setPhase(GUI::LockedPhase, 1, 1, sf::Vector2f(-60, 0), sf::Vector2f(), 0);
    gameGuiStyles.insert(std::make_pair("Editor_Mini_On", style_editor_mini_on));

    GUI::Style* style_editor_arrow_up = new GUI::Style();
    style_editor_arrow_up->setTextureRect(sf::IntRect(0, 0, 235, 51));
    style_editor_arrow_up->setTransmissionFactor();
    style_editor_arrow_up->setInputHoldDelay(0.1f);
    style_editor_arrow_up->setInputType(GUI::OnClick);
    style_editor_arrow_up->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(0, -51));
    style_editor_arrow_up->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(0, -51));
    style_editor_arrow_up->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_editor_arrow_up->setPhase(GUI::HighlighPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(235, 51));
    style_editor_arrow_up->setPhase(GUI::PressedPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(235, 51));
    gameGuiStyles.insert(std::make_pair("Editor_Arrow_Up", style_editor_arrow_up));

    GUI::Style* style_editor_arrow_down = new GUI::Style();
    style_editor_arrow_down->copyFrom(*style_editor_arrow_up);
    style_editor_arrow_down->setTextureRect(sf::IntRect(235, 0, 235, 51));
    style_editor_arrow_down->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(0, 51));
    style_editor_arrow_down->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(0, 51));
    gameGuiStyles.insert(std::make_pair("Editor_Arrow_Down", style_editor_arrow_down));

    GUI::Style* style_editor_selection = new GUI::Style();
    style_editor_selection->setTextureRect(sf::IntRect(0, 0, 200, 200));
    style_editor_selection->setTransmissionFactor();
    style_editor_selection->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_selection->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_selection->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_editor_selection->setPhase(GUI::HighlighPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(200, 200));
    style_editor_selection->setPhase(GUI::PressedPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(200, 200));
    gameGuiStyles.insert(std::make_pair("Editor_Selection", style_editor_selection));

    GUI::Style* style_editor_selection_On = new GUI::Style();
    style_editor_selection_On->setTextureRect(sf::IntRect(0, 0, 200, 200));
    style_editor_selection_On->setTransmissionFactor();
    style_editor_selection_On->setPhase(GUI::StartPhase, 1, 2, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_selection_On->setPhase(GUI::InactivePhase, 1, 2, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_selection_On->setPhase(GUI::DefaultPhase, 1, 2, sf::Vector2f(), sf::Vector2f(), 255);
    style_editor_selection_On->setPhase(GUI::HighlighPhase, 1, 2, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(200, 200));
    style_editor_selection_On->setPhase(GUI::PressedPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(200, 200));
    gameGuiStyles.insert(std::make_pair("Editor_Selection_On", style_editor_selection_On));

    GUI::Style* style_backarrow = new GUI::Style();
    style_backarrow->setTextureRect(sf::IntRect(120, 0, 60, 60));
    style_backarrow->setTransmissionFactor();
    style_backarrow->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(0, 0), sf::Vector2f(), 0);
    style_backarrow->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(0, 0), sf::Vector2f(), 0);
    style_backarrow->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_backarrow->setPhase(GUI::HighlighPhase, 1.1f, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_backarrow->setPhase(GUI::PressedPhase, 0.9f, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    gameGuiStyles.insert(std::make_pair("BackArrow", style_backarrow));

    GUI::Style* style_editor_close = new GUI::Style();
    style_editor_close->copyFrom(*style_backarrow);
    style_editor_close->setTextureRect(sf::IntRect(120, 60, 60, 60));
    gameGuiStyles.insert(std::make_pair("Editor_Close", style_editor_close));

    GUI::Style* style_editor_switch = new GUI::Style();
    style_editor_switch->copyFrom(*style_backarrow);
    style_editor_switch->setTextureRect(sf::IntRect(240, 0, 60, 60));
    gameGuiStyles.insert(std::make_pair("Editor_Switch", style_editor_switch));

    GUI::Style* style_editor_teamoption = new GUI::Style();
    style_editor_teamoption->copyFrom(*style_editor_close);
    style_editor_teamoption->setTextureRect(sf::IntRect(180, 0, 60, 60));
    gameGuiStyles.insert(std::make_pair("Editor_TeamOption", style_editor_teamoption));

    GUI::Style* style_editor_newmenu = new GUI::Style();
    style_editor_newmenu->setTextureRect(sf::IntRect(0, 0, 262, 71));
    style_editor_newmenu->setTransmissionFactor();
    style_editor_newmenu->setIconRef(sf::Vector2f(-95, 0));
    style_editor_newmenu->setTextRef(sf::Vector2f(31, -4));
    style_editor_newmenu->setFontParameters(24, GetColor("Default_Light"));
    style_editor_newmenu->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(-300, 0), sf::Vector2f(), 0);
    style_editor_newmenu->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(-300, 0), sf::Vector2f(), 0);
    style_editor_newmenu->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(0, 0), sf::Vector2f(), 255, sf::Vector2f(262, 71));
    style_editor_newmenu->setPhase(GUI::HighlighPhase, 1, 1, sf::Vector2f(0, 0), sf::Vector2f(), 255, sf::Vector2f(262, 71));
    style_editor_newmenu->setPhase(GUI::PressedPhase, 1, 2, sf::Vector2f(0, 0), sf::Vector2f(), 255, sf::Vector2f(262, 71));
    gameGuiStyles.insert(std::make_pair("Editor_NewMenu", style_editor_newmenu));

    GUI::Style* style_editor_selectmode_on = new GUI::Style();
    style_editor_selectmode_on->setTextureRect(sf::IntRect(0, 0, 760, 145));
    style_editor_selectmode_on->setTransmissionFactor();
    style_editor_selectmode_on->setIconRef(sf::Vector2f(-309, 0));
    style_editor_selectmode_on->setTextRef(sf::Vector2f(69, -4));
    style_editor_selectmode_on->setFontParameters(23, GetColor("Default"));
    style_editor_selectmode_on->setPhase(GUI::StartPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_selectmode_on->setPhase(GUI::InactivePhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_selectmode_on->setPhase(GUI::DefaultPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(760, 145));
    style_editor_selectmode_on->setPhase(GUI::HighlighPhase, 1.02, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(775, 148));
    style_editor_selectmode_on->setPhase(GUI::PressedPhase, 0.98, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(745, 142));
    gameGuiStyles.insert(std::make_pair("Editor_SelectMode_On", style_editor_selectmode_on));

    GUI::Style* style_editor_selectmode_off = new GUI::Style();
    style_editor_selectmode_off->copyFrom(*style_editor_selectmode_on);
    style_editor_selectmode_off->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_selectmode_off->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_selectmode_off->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(760, 145));
    style_editor_selectmode_off->setPhase(GUI::HighlighPhase, 1.02, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(775, 148));
    style_editor_selectmode_off->setPhase(GUI::PressedPhase, 0.98, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(745, 142));
    gameGuiStyles.insert(std::make_pair("Editor_SelectMode_Off", style_editor_selectmode_off));

    GUI::Style* style_editor_teamstatus = new GUI::Style();
    style_editor_teamstatus->setTextureRect(sf::IntRect(0, 0, 50, 50));
    style_editor_teamstatus->setTransmissionFactor();
    style_editor_teamstatus->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_teamstatus->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_teamstatus->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_editor_teamstatus->setPhase(GUI::HighlighPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_editor_teamstatus->setPhase(GUI::PressedPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    gameGuiStyles.insert(std::make_pair("Editor_TeamStatus", style_editor_teamstatus));

    GUI::Style* style_editor_teamtype_off = new GUI::Style();
    style_editor_teamtype_off->setTextureRect(sf::IntRect(0, 92, 91, 91));
    style_editor_teamtype_off->setTransmissionFactor();
    style_editor_teamtype_off->setFontParameters(34, GetColor("Default"));
    style_editor_teamtype_off->setTextRef(sf::Vector2f(0, -10));
    style_editor_teamtype_off->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_teamtype_off->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_teamtype_off->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(91, 91), sf::Vector2f());
    style_editor_teamtype_off->setPhase(GUI::HighlighPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(91, 91), sf::Vector2f());
    style_editor_teamtype_off->setPhase(GUI::PressedPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(91, 91), sf::Vector2f());
    gameGuiStyles.insert(std::make_pair("Editor_TeamType_Off", style_editor_teamtype_off));

    GUI::Style* style_editor_teamtype_on = new GUI::Style();
    style_editor_teamtype_on->copyFrom(*style_editor_teamtype_off);
    style_editor_teamtype_on->setTextureRect(sf::IntRect(0, 0, 91, 91));
    gameGuiStyles.insert(std::make_pair("Editor_TeamType_On", style_editor_teamtype_on));

    GUI::Style* style_editor_plain = new GUI::Style();
    style_editor_plain->setTextureRect(sf::IntRect(0, 0, 193, 45));
    style_editor_plain->setTransmissionFactor();
    style_editor_plain->setFontParameters(18, GetColor("Default_Light"));
    style_editor_plain->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_plain->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_plain->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(193, 45));
    style_editor_plain->setPhase(GUI::HighlighPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(193, 45));
    style_editor_plain->setPhase(GUI::PressedPhase, 1, 2, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(193, 45));
    style_editor_plain->setPhase(GUI::LockedPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 100, sf::Vector2f(193, 45));
    gameGuiStyles.insert(std::make_pair("Editor_Plain", style_editor_plain));

    GUI::Style* style_editor_smallarrow_left = new GUI::Style();
    style_editor_smallarrow_left->setTextureRect(sf::IntRect(0, 0, 60, 60));
    style_editor_smallarrow_left->setTransmissionFactor();
    style_editor_smallarrow_left->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_smallarrow_left->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_smallarrow_left->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_editor_smallarrow_left->setPhase(GUI::HighlighPhase, 1.5f, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(90, 90));
    style_editor_smallarrow_left->setPhase(GUI::PressedPhase, 0.9f, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(60, 60));
    style_editor_smallarrow_left->setInputHoldDelay(0.05f);
    style_editor_smallarrow_left->setInputType(GUI::OnClick);
    gameGuiStyles.insert(std::make_pair("Editor_SmallArrow_Left", style_editor_smallarrow_left));

    GUI::Style* style_editor_smallarrow_right = new GUI::Style();
    style_editor_smallarrow_right->copyFrom(*style_editor_smallarrow_left);
    style_editor_smallarrow_right->setTextureRect(sf::IntRect(60, 0, 60, 60));
    gameGuiStyles.insert(std::make_pair("Editor_SmallArrow_Right", style_editor_smallarrow_right));

    GUI::Style* style_editor_shiptype = new GUI::Style();
    style_editor_shiptype->setPhase(GUI::StartPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_shiptype->setPhase(GUI::InactivePhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_shiptype->setPhase(GUI::DefaultPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(205, 125), sf::Vector2f());
    style_editor_shiptype->setPhase(GUI::HighlighPhase, 1, 1, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(205, 125), sf::Vector2f());
    style_editor_shiptype->setPhase(GUI::PressedPhase, 1, 2, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(205, 125), sf::Vector2f());
    style_editor_shiptype->setTextureRect(sf::IntRect(0, 0, 205, 125));
    style_editor_shiptype->setFontParameters(25, GetColor("Default_Light"));
    style_editor_shiptype->setTextRef(sf::Vector2f(0, 38));
    style_editor_shiptype->setIconRef(sf::Vector2f(0, -20));
    style_editor_shiptype->setTransmissionFactor();
    gameGuiStyles.insert(std::make_pair("Editor_ShipType", style_editor_shiptype));

    GUI::Style* style_editor_skinSelect = new GUI::Style();
    style_editor_skinSelect->setPhase(GUI::StartPhase, 0.9, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_skinSelect->setPhase(GUI::InactivePhase, 0.9, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_skinSelect->setPhase(GUI::DefaultPhase, 0.9, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(174.6, 174.6));
    style_editor_skinSelect->setPhase(GUI::HighlighPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(194, 194));
    style_editor_skinSelect->setPhase(GUI::PressedPhase, 0.8, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(155.2, 155.2));
    style_editor_skinSelect->setTransmissionFactor();
    style_editor_skinSelect->setTextureRect(sf::IntRect(0, 0, 194, 194));
    gameGuiStyles.insert(std::make_pair("Editor_SkinSelect", style_editor_skinSelect));

    GUI::Style* style_editor_characterSelect = new GUI::Style();
    style_editor_characterSelect->setPhase(GUI::StartPhase, 0.9f, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_characterSelect->setPhase(GUI::InactivePhase, 0.9f, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_editor_characterSelect->setPhase(GUI::DefaultPhase, 0.9f, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(143, 234));
    style_editor_characterSelect->setPhase(GUI::HighlighPhase, 1, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(160, 260));
    style_editor_characterSelect->setPhase(GUI::PressedPhase, 0.8f, 0, sf::Vector2f(), sf::Vector2f(), 255, sf::Vector2f(127, 208));
    style_editor_characterSelect->setTransmissionFactor();
    style_editor_characterSelect->setTextureRect(sf::IntRect(0, 0, 160, 260));
    gameGuiStyles.insert(std::make_pair("Editor_CharacterSelect", style_editor_characterSelect));

    GUI::Style* style_level_hideSubShips_on = new GUI::Style();
    {
        float diffX = gameGuiScreenEdgeAllowance.x > 15.f ? gameGuiScreenEdgeAllowance.x : 15.f;

        style_level_hideSubShips_on->setTextureRect(sf::IntRect(30, 0, 30, 60));
        style_level_hideSubShips_on->setPhase(GUI::StartPhase, 0.8f, 0, sf::Vector2f(15, 0), sf::Vector2f(), 255, sf::Vector2f(diffX + 30, 150), sf::Vector2f(-(diffX + 30) / 2.f + 15.f, 0));
        style_level_hideSubShips_on->setPhase(GUI::InactivePhase, 0.8f, 0, sf::Vector2f(15, 0), sf::Vector2f(), 0, sf::Vector2f(diffX + 30, 150), sf::Vector2f(-(diffX + 30) / 2.f + 15.f, 0));
        style_level_hideSubShips_on->setPhase(GUI::DefaultPhase, 0.8f, 0, sf::Vector2f(15, 0), sf::Vector2f(), 255, sf::Vector2f(diffX + 30, 150), sf::Vector2f(-(diffX + 30) / 2.f + 15.f, 0));
        style_level_hideSubShips_on->setPhase(GUI::HighlighPhase, 0.8f, 0, sf::Vector2f(-diffX, 0), sf::Vector2f(), 255, sf::Vector2f(diffX + 30, 150), sf::Vector2f(-(diffX + 30) / 2.f + 15.f, 0));
        style_level_hideSubShips_on->setPhase(GUI::PressedPhase, 0.6f, 0, sf::Vector2f(-diffX, 0), sf::Vector2f(), 255, sf::Vector2f(diffX + 30, 150), sf::Vector2f(-(diffX + 30) / 2.f + 15.f, 0));
        style_level_hideSubShips_on->setTransmissionFactor();
    }
    gameGuiStyles.insert(std::make_pair("Level_HideSubShips_On", style_level_hideSubShips_on));

    GUI::Style* style_level_hideSubShips_off = new GUI::Style();
    style_level_hideSubShips_off->copyFrom(*style_level_hideSubShips_on);
    style_level_hideSubShips_off->setTextureRect(sf::IntRect(0, 0, 30, 60));
    gameGuiStyles.insert(std::make_pair("Level_HideSubShips_Off", style_level_hideSubShips_off));

    GUI::Style* style_level_iconShapeButton = new GUI::Style();
    style_level_iconShapeButton->setTextureRect(sf::IntRect(0, 0, 30, 30));
    style_level_iconShapeButton->setPhase(GUI::StartPhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_level_iconShapeButton->setPhase(GUI::InactivePhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_level_iconShapeButton->setPhase(GUI::DefaultPhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_level_iconShapeButton->setPhase(GUI::HighlighPhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_level_iconShapeButton->setPhase(GUI::PressedPhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_level_iconShapeButton->setPhase(GUI::LockedPhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_level_iconShapeButton->setTransmissionFactor();
    gameGuiStyles.insert(std::make_pair("Level_IconShapeButton", style_level_iconShapeButton));

    GUI::Style* style_generalShapeButton = new GUI::Style();
    style_generalShapeButton->setPhase(GUI::StartPhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_generalShapeButton->setPhase(GUI::InactivePhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 0);
    style_generalShapeButton->setPhase(GUI::DefaultPhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_generalShapeButton->setPhase(GUI::HighlighPhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_generalShapeButton->setPhase(GUI::PressedPhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 255);
    style_generalShapeButton->setPhase(GUI::LockedPhase, 1.f, 0, sf::Vector2f(), sf::Vector2f(), 100);
    style_generalShapeButton->setTransmissionFactor();
    gameGuiStyles.insert(std::make_pair("GeneralShapeButton", style_generalShapeButton));

    GUI::Style* style_generalShapeButtonNoTransmit = new GUI::Style();
    style_generalShapeButtonNoTransmit->copyFrom(*style_generalShapeButton);
    style_generalShapeButtonNoTransmit->setTransmissionFactor(0);
    gameGuiStyles.insert(std::make_pair("GeneralShapeButtonNoTransmit", style_generalShapeButtonNoTransmit));
}

GUI::Style* Game::GetGuiStyle(std::string id)
{
    auto found = gameGuiStyles.find(id);
    if(found != gameGuiStyles.end());
        return found->second;

    return nullptr;
}

sf::Font* Game::GetFont(std::string id)
{
    auto found = gameFonts.find(id);
    if(found != gameFonts.end());
        return found->second;

    return nullptr;
}

sf::Color Game::GetColor(std::string id)
{
    auto found = gameColors.find(id);
    if(found != gameColors.end());
        return found->second;

    return sf::Color::White;
}

sf::Color Game::GetCurrentRankThemeColor()
{
    return rankColors[currentRankTheme];
}

const sf::Vector2f& Game::GetGuiScreenEdgeALlowance()
{
    return gameGuiScreenEdgeAllowance;
}

void Game::Execute()
{
    Initialise();

    while(gameInstance->window.isOpen())
    {
        HandleInputs();

        if(gameInstance->gameState == Prototype)
        {
            //AddNotification("This level is a prototype for testing the game.");
            AddNotification("Demo Showcase for an incomplete project.");
            Play("Map Data/prototype.map");
        }
        else if(gameInstance->gameState == MapEdit)
            MapEditior("");
    }
}

void Game::Play(std::string file)
{
    sf::Clock clock;
    bool quit = false;

    bool highlighting = false;
    bool updateInfo_reset = false;
    ShipIdentity* updateInfo_identity = nullptr;
    sf::Vector2f highlightStart;
    sf::RectangleShape highlightRectangle;
    short highlightState = -1;
    sf::Vector2f highlightDestination;
    std::list<Ship*> focusedShips;
    sf::VertexArray va_highlighedShips;
    float baseHighlightTransparency = 0;
    float baseHighlightSelectTime = 0;
    short baseHighlightSelectId = -1;
    short baseFocusedId = -1;
    float baseLeftClickDuration = 0;
    bool baseLeftClick = 0;
    bool updateBoosterStatus = false;
    std::vector<short> boostersInUse[Game::NumOfTeams];
    float text_commandTime = 0.f;
    sf::Text text_command("", *Game::GetFont("Conthrax"), 20 * 2);
    float lastMapButtonScale = 1.f;
    short gun_info_count = 0;
    float gun_info_time = 0;
    float time_energyIncrease = 5.f;

    highlightRectangle.setFillColor(sf::Color(255, 255, 255, 40));
    highlightRectangle.setOutlineColor(sf::Color(255, 255, 255, 100));
    highlightRectangle.setOutlineThickness(-4);

    text_command.setFillColor(sf::Color::White);
    text_command.setScale(0.5f, 0.5f);

    va_highlighedShips.setPrimitiveType(sf::Quads);
    float deltaTime = 0.f;
    float dtFPS = 0.f;

    InitializeLevel();

    //Preparing game play GUI
    std::string message = "";
    sf::Vector2f selection_position;
    float selection_subShips_width = 0;
    bool selection_hide = false;

    sf::Texture* texture_iconStates = &AssetManager::GetTexture("Icon_States");
    sf::Texture* texture_boosters = &AssetManager::GetTexture("Icon_Boosters");
    sf::Texture* texture_focusedShipProfile_others = &AssetManager::GetTexture("FocusedShipButton_Others");
    sf::Texture* texture_guns = &AssetManager::GetTexture("Guns");
    sf::Font* font_conthrax = GetFont("Conthrax");
    const sf::Color rankClr = GetCurrentRankThemeColor();
    const sf::Color rankTransClr(rankClr.r, rankClr.g, rankClr.b, 255 * 0.7f);
    sf::Color teamTransColor[] = {sf::Color(220, 130, 130, 255 * 0.8f), sf::Color(185, 193, 119, 255 * 0.8f), sf::Color(255, 219, 93, 255 * 0.8f), sf::Color(129, 158, 161, 255 * 0.8f)};
    GUI::Style* style_iconShapeButton = Game::GetGuiStyle("Level_IconShapeButton");
    GUI::Style* style_generalShapeButton = Game::GetGuiStyle("GeneralShapeButton");
    GUI::Style* style_generalShapeButtonNoTransmit = Game::GetGuiStyle("GeneralShapeButtonNoTransmit");
    GUI::ResourcePack resourcePack_general(nullptr, font_conthrax);

    GUI::Session guiSession;
    GUI::ObjectSelection* selection_subShips = nullptr;
    GUI::Degree* degree_subShips = nullptr;
    FocusedShipDisplay* focusedProfile = nullptr;
    FocusedShipButton* focusedButton = nullptr;
    BaseInfoButton* baseInfoButton = nullptr;

    guiSession.Create("gameplay ui");

    int numOfPlayerTeamSubShips = 0;
    for(int a = 0; a < Game::GetShipIdentitySize(); a++)
        if(GetShipIdentity(a)->GetShipInfo()->team == gamePlayerTeam)
            numOfPlayerTeamSubShips++;
    if(numOfPlayerTeamSubShips > 0)
    {
        float selection_subShips_limit = SCREEN_WIDTH / 2.f - 20.f;
        selection_subShips_width = (int)(selection_subShips_limit / 100.f) * 105;
        while(true)
        {
            selection_subShips_width -= 5.f;
            if(selection_subShips_width > selection_subShips_limit)
                selection_subShips_width -= 100.f;
            else break;
        }
        if(selection_subShips_width > numOfPlayerTeamSubShips * 105.f - 5.f)
            selection_subShips_width = numOfPlayerTeamSubShips * 105.f - 5.f;
        if(selection_subShips_width < 100.f)
            selection_subShips_width = 100.f;

        sf::Vector2f selection_subShips_size(selection_subShips_width, 125.f);
        float selection_subShips_length = numOfPlayerTeamSubShips * 105.f - 5.f - selection_subShips_width;
        if(selection_subShips_length < 0)
            selection_subShips_length = 0.f;

        selection_position = sf::Vector2f(SCREEN_WIDTH - selection_subShips_width / 2.f - gameGuiScreenEdgeAllowance.x, SCREEN_HEIGHT - 63 - gameGuiScreenEdgeAllowance.y);
        selection_subShips = new GUI::ObjectSelection(selection_position, selection_subShips_size, selection_subShips_length, 105.f / selection_subShips_length, 0.1f, GUI::Horizontal);
        int index = 0;
        for(int a = 0; a < GetShipIdentitySize(); a++)
        {
            ShipIdentity* identity = GetShipIdentity(a);
            if(identity->GetShipInfo()->team == gamePlayerTeam)
            {
                selection_subShips->AddObject(new ShipProfileButton(identity, sf::Vector2f(50 + 105 * index, 75), "Identity " + ConvertIntToString(a)));
                index++;
            }
        }

        if(selection_subShips_length > 0)
            degree_subShips = new FadeDegree(GUI::Horizontal, sf::FloatRect(SCREEN_WIDTH - selection_subShips_width - gameGuiScreenEdgeAllowance.x, SCREEN_HEIGHT - gameGuiScreenEdgeAllowance.y, selection_subShips_width, 10), selection_subShips_width / (numOfPlayerTeamSubShips * 105.f - 5.f), 0, 0, sf::Color::Transparent, sf::Color::White, sf::Color::Transparent, GUI::Degree::ClickNode);

        guiSession.AddObject(new GUI::Navigator(nullptr, nullptr, degree_subShips, selection_subShips), "gameplay ui");
        guiSession.AddObject(new GUI::ToggleButton(GetGuiStyle("Level_HideSubShips_Off"), GetGuiStyle("Level_HideSubShips_On"), sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT - 64 - gameGuiScreenEdgeAllowance.y), 1.f,
                                                   GUI::ResourcePack(&AssetManager::GetTexture("Level_HideSubShips")), "HideSubShips", "ShowSubShips", ""), "gameplay ui");
    }

    guiSession.AddObject(new GUI::DisplayShape(sf::Vector2f(321, 30), sf::Color(rankClr.r, rankClr.g, rankClr.b, 255.f * 0.7f), sf::Vector2f(),
                                               GUI::DisplayObject::DisplayPhase(gameGuiScreenEdgeAllowance, sf::Vector2f(1, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(gameGuiScreenEdgeAllowance, sf::Vector2f(1, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(gameGuiScreenEdgeAllowance, sf::Vector2f(1, 1), 0)), "gameplay ui");

    {
        GUI::DisplayVertexArray* temp = new GUI::DisplayVertexArray(sf::Quads, texture_iconStates,
                                                                   GUI::DisplayObject::DisplayPhase(gameGuiScreenEdgeAllowance, sf::Vector2f(1, 1), 0),
                                                                   GUI::DisplayObject::DisplayPhase(gameGuiScreenEdgeAllowance, sf::Vector2f(1, 1), 255),
                                                                   GUI::DisplayObject::DisplayPhase(gameGuiScreenEdgeAllowance, sf::Vector2f(1, 1), 0));
        temp->AddRectVertices(sf::FloatRect(gameGuiScreenEdgeAllowance.x, gameGuiScreenEdgeAllowance.y, 30, 30), sf::Color::White, 0, sf::IntRect(300, 60, 60, 60));
        temp->AddRectVertices(sf::FloatRect(gameGuiScreenEdgeAllowance.x + 100, gameGuiScreenEdgeAllowance.y, 30, 30), sf::Color::White, 0, sf::IntRect(360, 0, 60, 60));

        guiSession.AddObject(temp, "gameplay ui");
    }

    std::string ref_energy = ConvertIntToString(levelTeamEnergy[gamePlayerTeam]);
    std::string ref_unitsLeft = "0";
    GUI::DisplayText* displayText_energy = new GUI::DisplayText(&ref_energy, font_conthrax, sf::Color::White, 18 * 2, sf::Vector2f(0, 0.5f),
                                                                GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 30, gameGuiScreenEdgeAllowance.y + 10), sf::Vector2f(0.5f, 0.5f), 0),
                                                                GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 30, gameGuiScreenEdgeAllowance.y + 10), sf::Vector2f(0.5f, 0.5f), 255),
                                                                GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 30, gameGuiScreenEdgeAllowance.y + 10), sf::Vector2f(0.5f, 0.5f), 0));
    GUI::DisplayText* displayText_teamUnits = new GUI::DisplayText(&ref_unitsLeft, font_conthrax, sf::Color::White, 18 * 2, sf::Vector2f(),
                                                                GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 134, gameGuiScreenEdgeAllowance.y + 3), sf::Vector2f(0.5f, 0.5f), 0),
                                                                GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 134, gameGuiScreenEdgeAllowance.y + 3), sf::Vector2f(0.5f, 0.5f), 255),
                                                                GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 134, gameGuiScreenEdgeAllowance.y + 3), sf::Vector2f(0.5f, 0.5f), 0));
    guiSession.AddObject(displayText_energy, "gameplay ui");
    guiSession.AddObject(displayText_teamUnits, "gameplay ui");

    std::string ref_showMoreOptions = "ShowMoreOption_Close";
    GUI::ShapeToggleButton* button_showMoreOptions = new GUI::ShapeToggleButton(style_iconShapeButton, style_iconShapeButton, sf::Vector2f(30, 30),
                                                                                rankClr, rankClr, ChangeColorHue(rankClr, 30), false,
                                                                                gameGuiScreenEdgeAllowance + sf::Vector2f(306, 15), resourcePack_general, "ShowMoreOption", "ShowMoreOption_Close", "", texture_iconStates, 0.5f, sf::IntRect(240, 60, 60, 60),
                                                                                0, 0, false, &ref_showMoreOptions, true);
    guiSession.AddObject(button_showMoreOptions, "gameplay ui");

    GUI::Session* session_topLeft = new GUI::Session();
    session_topLeft->Create("focused units");
    session_topLeft->Create("options");

    std::string ref_topLeftOptionTab = "BehaviorTab";
    session_topLeft->AddObject(new GUI::ShapeToggleButton(style_generalShapeButton, style_generalShapeButton, sf::Vector2f(321 / 2.f, 30),
                                                             rankTransClr, rankTransClr, ChangeColorHue(rankTransClr, 30),
                                                             rankClr, rankClr, ChangeColorHue(rankClr, 30), false,
                                                             sf::Vector2f(gameGuiScreenEdgeAllowance.x + 321 * 0.25, gameGuiScreenEdgeAllowance.y + 55), resourcePack_general, "BehaviorTab", "", "", nullptr, 1.f, sf::IntRect(), 0, 0,
                                                             true, &ref_topLeftOptionTab, true), "options");
    session_topLeft->AddObject(new GUI::ShapeToggleButton(style_generalShapeButton, style_generalShapeButton, sf::Vector2f(321 / 2.f, 30),
                                                             rankTransClr, rankTransClr, ChangeColorHue(rankTransClr, 30),
                                                             rankClr, rankClr, ChangeColorHue(rankClr, 30), false,
                                                             sf::Vector2f(gameGuiScreenEdgeAllowance.x + 321 * 0.75, gameGuiScreenEdgeAllowance.y + 55), resourcePack_general, "BoosterTab", "", "", nullptr, 1.f, sf::IntRect(), 0, 0,
                                                             true, &ref_topLeftOptionTab, true), "options");

    GUI::Session* session_topLeft_tab = new GUI::Session();
    session_topLeft->AddObject(session_topLeft_tab, "options");
    session_topLeft_tab->Create("behavior");
    session_topLeft_tab->Create("boosters");
    std::string ref_behavior = "Behavior_Free";
    {
        GUI::Layer* temp = new GUI::Layer();
        temp->AddObject(new GUI::DisplayText("Behavior", font_conthrax, sf::Color::White, 18 * 2,
                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 80, gameGuiScreenEdgeAllowance.y + 50), sf::Vector2f(0.5f, 0.5f), 0),
                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 80, gameGuiScreenEdgeAllowance.y + 50), sf::Vector2f(0.5f, 0.5f), 255),
                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 80, gameGuiScreenEdgeAllowance.y + 50), sf::Vector2f(0.5f, 0.5f), 0)));
        temp->AddObject(new GUI::DisplayText("Boosters", font_conthrax, sf::Color::White, 18 * 2,
                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 241, gameGuiScreenEdgeAllowance.y + 50), sf::Vector2f(0.5f, 0.5f), 0),
                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 241, gameGuiScreenEdgeAllowance.y + 50), sf::Vector2f(0.5f, 0.5f), 255),
                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 241, gameGuiScreenEdgeAllowance.y + 50), sf::Vector2f(0.5f, 0.5f), 0)));
        temp->AddObject(new GUI::DisplayShape(sf::Vector2f(321, 2), sf::Color::White, sf::Vector2f(0, 0),
                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x, gameGuiScreenEdgeAllowance.y + 68), sf::Vector2f(1, 1), 0),
                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x, gameGuiScreenEdgeAllowance.y + 68), sf::Vector2f(1, 1), 255),
                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x, gameGuiScreenEdgeAllowance.y + 68), sf::Vector2f(1, 1), 0)));
        session_topLeft->AddObject(temp, "options");

        session_topLeft_tab->AddObject(new GUI::ShapeToggleButton(style_generalShapeButton, style_generalShapeButton, sf::Vector2f(321, 87),
                                                             rankTransClr, rankTransClr, ChangeColorHue(rankTransClr, 30),
                                                             rankClr, rankClr, ChangeColorHue(rankClr, 30), false,
                                                             sf::Vector2f(gameGuiScreenEdgeAllowance.x + 321 * 0.5f, gameGuiScreenEdgeAllowance.y + 113), resourcePack_general, "Behavior_Free", "", "", nullptr, 1.f, sf::IntRect(), 0, 0,
                                                             true, &ref_behavior, true), "behavior");
        session_topLeft_tab->AddObject(new GUI::ShapeToggleButton(style_generalShapeButton, style_generalShapeButton, sf::Vector2f(321, 87),
                                                             rankTransClr, rankTransClr, ChangeColorHue(rankTransClr, 30),
                                                             rankClr, rankClr, ChangeColorHue(rankClr, 30), false,
                                                             sf::Vector2f(gameGuiScreenEdgeAllowance.x + 321 * 0.5f, gameGuiScreenEdgeAllowance.y + 200), resourcePack_general, "Behavior_Attack", "", "", nullptr, 1.f, sf::IntRect(), 0, 0,
                                                             true, &ref_behavior, true), "behavior");
        session_topLeft_tab->AddObject(new GUI::ShapeToggleButton(style_generalShapeButton, style_generalShapeButton, sf::Vector2f(321, 87),
                                                             rankTransClr, rankTransClr, ChangeColorHue(rankTransClr, 30),
                                                             rankClr, rankClr, ChangeColorHue(rankClr, 30), false,
                                                             sf::Vector2f(gameGuiScreenEdgeAllowance.x + 321 * 0.5f, gameGuiScreenEdgeAllowance.y + 287), resourcePack_general, "Behavior_Defend", "", "", nullptr, 1.f, sf::IntRect(), 0, 0,
                                                             true, &ref_behavior, true), "behavior");

        GUI::Layer* temp_behaviorLayer = new GUI::Layer();
        temp_behaviorLayer->AddObject(new GUI::DisplayText("Free", font_conthrax, sf::Color::White, 24, sf::Vector2f(),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 81), sf::Vector2f(1, 1), 0),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 81), sf::Vector2f(1, 1), 255),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 81), sf::Vector2f(1, 1), 0)));
        temp_behaviorLayer->AddObject(new GUI::DisplayText("Units are free to make their\nown decisions.", font_conthrax, sf::Color::White, 12, sf::Vector2f(),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 111), sf::Vector2f(1, 1), 0),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 111), sf::Vector2f(1, 1), 255),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 111), sf::Vector2f(1, 1), 0)));
        temp_behaviorLayer->AddObject(new GUI::DisplayText("Attack", font_conthrax, sf::Color::White, 24, sf::Vector2f(),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 168), sf::Vector2f(1, 1), 0),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 168), sf::Vector2f(1, 1), 255),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 168), sf::Vector2f(1, 1), 0)));
        temp_behaviorLayer->AddObject(new GUI::DisplayText("Units attack regardless of\ntheir current state.", font_conthrax, sf::Color::White, 12, sf::Vector2f(),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 198), sf::Vector2f(1, 1), 0),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 198), sf::Vector2f(1, 1), 255),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 198), sf::Vector2f(1, 1), 0)));
        temp_behaviorLayer->AddObject(new GUI::DisplayText("Defend", font_conthrax, sf::Color::White, 24, sf::Vector2f(),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 255), sf::Vector2f(1, 1), 0),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 255), sf::Vector2f(1, 1), 255),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 255), sf::Vector2f(1, 1), 0)));
        temp_behaviorLayer->AddObject(new GUI::DisplayText("Units never attack, they\nonly defend.", font_conthrax, sf::Color::White, 12, sf::Vector2f(),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 285), sf::Vector2f(1, 1), 0),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 285), sf::Vector2f(1, 1), 255),
                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 13, gameGuiScreenEdgeAllowance.y + 285), sf::Vector2f(1, 1), 0)));

        GUI::DisplayVertexArray* temp_va = new GUI::DisplayVertexArray(sf::Quads, texture_iconStates,
                                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0));
        temp_va->AddRectVertices(sf::FloatRect(gameGuiScreenEdgeAllowance.x + 249, gameGuiScreenEdgeAllowance.y + 84, 60, 60), sf::Color::White, 0, sf::IntRect(660, 0, 60, 60));
        temp_va->AddRectVertices(sf::FloatRect(gameGuiScreenEdgeAllowance.x + 249, gameGuiScreenEdgeAllowance.y + 170, 60, 60), sf::Color::White, 0, sf::IntRect(60, 0, 60, 60));
        temp_va->AddRectVertices(sf::FloatRect(gameGuiScreenEdgeAllowance.x + 249, gameGuiScreenEdgeAllowance.y + 257, 60, 60), sf::Color::White, 0, sf::IntRect(120, 0, 60, 60));
        temp_behaviorLayer->AddObject(temp_va);

        session_topLeft_tab->AddObject(temp_behaviorLayer, "behavior");
    }

    GUI::DisplayText* displayText_boostersLeft = new GUI::DisplayText("No boosters are active.", font_conthrax, sf::Color::White, 24, sf::Vector2f(),
                                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 14, gameGuiScreenEdgeAllowance.y + 78), sf::Vector2f(0.5f, 0.5f), 0),
                                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 14, gameGuiScreenEdgeAllowance.y + 78), sf::Vector2f(0.5f, 0.5f), 255),
                                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 14, gameGuiScreenEdgeAllowance.y + 78), sf::Vector2f(0.5f, 0.5f), 0));

    std::string ref_bst_attack[Game::NumOfTeams];
    std::string ref_bst_defence[Game::NumOfTeams];
    std::string ref_bst_speed[Game::NumOfTeams];
    std::string ref_bst_unitBuildBoost[Game::NumOfTeams];
    std::string ref_bst_helthRecovery[Game::NumOfTeams];
    float ref_bst_attack_time[Game::NumOfTeams] = {0.f};
    float ref_bst_defence_time[Game::NumOfTeams] = {0.f};
    float ref_bst_speed_time[Game::NumOfTeams] = {0.f};
    float ref_bst_unitBuildBoost_time[Game::NumOfTeams] = {0.f};
    float ref_bst_helthRecovery_time[Game::NumOfTeams] = {0.f};
    int ref_bst_attack_ver = 0;
    int ref_bst_defence_ver = 0;
    bool boosterShouldCloseGunSelection = false;
    float* boostersRefTime[Game::NumOfTeams][Game::NumOfGameBoosters] = {};
    {
        int numOfAvailBoostersOnView = 0;
        int sizeHeight = 0;

        for(int a = 0; a < NumOfGameBoosters; a++)
        {
            sizeHeight += 88;
            if(sizeHeight > SCREEN_HEIGHT - 345 - gameGuiScreenEdgeAllowance.y * 2)
            {
                if(sizeHeight - 5.f <= SCREEN_HEIGHT - 345 - gameGuiScreenEdgeAllowance.y * 2)
                {
                    sizeHeight -= 5.f;
                    numOfAvailBoostersOnView++;
                }
                else sizeHeight -= 88.f + 5.f;
                break;
            }
            else numOfAvailBoostersOnView++;
        }

        if(sizeHeight < 87 * 3 - 40)
            sizeHeight = 87 * 3 - 40;
        if(sizeHeight > SCREEN_HEIGHT - 370 - gameGuiScreenEdgeAllowance.y *2)
            boosterShouldCloseGunSelection = true;

        float selection_scrollLength = 88.f * NumOfGameBoosters - sizeHeight - 5.f;
        if(selection_scrollLength < 0.f)
            selection_scrollLength = 0.f;
        GUI::ObjectSelection* temp_selection = new GUI::ObjectSelection(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 321 / 2.f, gameGuiScreenEdgeAllowance.y + 105 + sizeHeight / 2.f), sf::Vector2f(322, sizeHeight), selection_scrollLength, 88.f / selection_scrollLength, 0.1f, GUI::Vertical, false);

        FadeDegree* temp_degree = nullptr;
        if(selection_scrollLength > 0.f)
            temp_degree = new FadeDegree(GUI::Vertical, sf::FloatRect(gameGuiScreenEdgeAllowance.x + 326, gameGuiScreenEdgeAllowance.y + 105, 12, sizeHeight), sizeHeight / (88.f * Game::NumOfGameBoosters - 5.f), 6, -1, sf::Color::Transparent, sf::Color::White, sf::Color::Transparent, GUI::Degree::ClickNode);

        GUI::Navigator* temp_navigator = new GUI::Navigator(nullptr, nullptr, temp_degree, temp_selection);

        GUI::DisplayVertexArray* temp_va = new GUI::DisplayVertexArray(sf::Quads, nullptr,
                                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0));
        temp_va->AddRectVertices(sf::FloatRect(gameGuiScreenEdgeAllowance.x, gameGuiScreenEdgeAllowance.y + 70, 321, 30), rankTransClr);

        session_topLeft_tab->AddObject(temp_navigator, "boosters");
        session_topLeft_tab->AddObject(temp_va, "boosters");
        session_topLeft_tab->AddObject(displayText_boostersLeft, "boosters");

        int numOfAvailBoosters = 0;
        float nextYPos = 0;
        //Attack
        if(currentRank >= gameBoostersRank[Bst_AttackI])
        {
            int index = Bst_AttackI;
            temp_selection->AddObject(new BoosterButton(sf::Vector2f(0, nextYPos), rankClr, "Bst_AttackI", "Attack I", "Increases attack of all units\nby 25%.",
                                                        gameBoostersCost[index], gameBoostersTime[index], &ref_bst_attack_time[gamePlayerTeam], &ref_bst_attack[gamePlayerTeam], &ref_bst_attack_ver, 1, sf::IntRect(0, 0, 60, 60)));
            nextYPos += 88.f;
            numOfAvailBoosters++;
        }
        if(currentRank >= gameBoostersRank[Bst_AttackII])
        {
            int index = Bst_AttackII;
            temp_selection->AddObject(new BoosterButton(sf::Vector2f(0, nextYPos), rankClr, "Bst_AttackII", "Attack II", "Increases attack of all units\nby 50%.",
                                                        gameBoostersCost[index], gameBoostersTime[index], &ref_bst_attack_time[gamePlayerTeam], &ref_bst_attack[gamePlayerTeam], &ref_bst_attack_ver, 2, sf::IntRect(60, 0, 60, 60)));
            nextYPos += 88.f;
            numOfAvailBoosters++;
        }
        if(currentRank >= gameBoostersRank[Bst_AttackIII])
        {
            int index = Bst_AttackIII;
            temp_selection->AddObject(new BoosterButton(sf::Vector2f(0, nextYPos), rankClr, "Bst_AttackIII", "Attack III", "Increases attack of all units\nby 75%.",
                                                        gameBoostersCost[index], gameBoostersTime[index], &ref_bst_attack_time[gamePlayerTeam], &ref_bst_attack[gamePlayerTeam], &ref_bst_attack_ver, 3, sf::IntRect(120, 0, 60, 60)));
            nextYPos += 88.f;
            numOfAvailBoosters++;
        }

        //Defense
        if(currentRank >= gameBoostersRank[Bst_DefenceI])
        {
            int index = Bst_DefenceI;
            temp_selection->AddObject(new BoosterButton(sf::Vector2f(0, nextYPos), rankClr, "Bst_DefenseI", "Defense I", "Increases defense of all\nunits by 25%.",
                                                        gameBoostersCost[index], gameBoostersTime[index], &ref_bst_defence_time[gamePlayerTeam], &ref_bst_defence[gamePlayerTeam], &ref_bst_defence_ver, 1, sf::IntRect(180, 0, 60, 60)));
            nextYPos += 88.f;
            numOfAvailBoosters++;
        }
        if(currentRank >= gameBoostersRank[Bst_DefenceII])
        {
            int index = Bst_DefenceII;
            temp_selection->AddObject(new BoosterButton(sf::Vector2f(0, nextYPos), rankClr, "Bst_DefenseII", "Defense II", "Increases defense of all\nunits by 50%.",
                                                        gameBoostersCost[index], gameBoostersTime[index], &ref_bst_defence_time[gamePlayerTeam], &ref_bst_defence[gamePlayerTeam], &ref_bst_defence_ver, 2, sf::IntRect(240, 0, 60, 60)));
            nextYPos += 88.f;
            numOfAvailBoosters++;
        }
        if(currentRank >= gameBoostersRank[Bst_DefenceIII])
        {
            int index = Bst_DefenceIII;
            temp_selection->AddObject(new BoosterButton(sf::Vector2f(0, nextYPos), rankClr, "Bst_DefenseIII", "Defense III", "Increases defense of all\nunits by 75%.",
                                                        gameBoostersCost[index], gameBoostersTime[index], &ref_bst_defence_time[gamePlayerTeam], &ref_bst_defence[gamePlayerTeam], &ref_bst_defence_ver, 3, sf::IntRect(300, 0, 60, 60)));
            nextYPos += 88.f;
            numOfAvailBoosters++;
        }

        //Unavailable Boosters
        GUI::DisplayVertexArray* temp_va_locks = new GUI::DisplayVertexArray(sf::Quads, texture_iconStates,
                                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(0, 0), sf::Vector2f(1, 1), 0),
                                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(0, 0), sf::Vector2f(1, 1), 255),
                                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(0, 0), sf::Vector2f(1, 1), 0));
        GUI::DisplayVertexArray* temp_va_locks_bar = new GUI::DisplayVertexArray(sf::Quads, nullptr,
                                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(0, 0), sf::Vector2f(1, 1), 0),
                                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(0, 0), sf::Vector2f(1, 1), 255),
                                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(0, 0), sf::Vector2f(1, 1), 0));
        GUI::Layer* temp_layer_locks = new GUI::Layer();
        temp_layer_locks->AddObject(temp_va_locks_bar);
        temp_layer_locks->AddObject(temp_va_locks);
        temp_selection->AddObject(temp_layer_locks);

        for(int a = 0; a < NumOfGameBoosters - numOfAvailBoosters; a++)
        {
            temp_va_locks_bar->AddRectVertices(sf::FloatRect(0, nextYPos, 321, 83), rankTransClr);
            temp_va_locks->AddRectVertices(sf::FloatRect(139, nextYPos + 19, 44, 44), sf::Color::White, 0, sf::IntRect(120, 60, 60, 60));
            nextYPos += 88.f;
        }

        //BoosterRefTime
        for(int a = 0; a < NumOfTeams; a++)
        {
            boostersRefTime[a][Bst_AttackI] = &ref_bst_attack_time[a];
            boostersRefTime[a][Bst_AttackII] = &ref_bst_attack_time[a];
            boostersRefTime[a][Bst_AttackIII] = &ref_bst_attack_time[a];
            boostersRefTime[a][Bst_DefenceI] = &ref_bst_defence_time[a];
            boostersRefTime[a][Bst_DefenceII] = &ref_bst_defence_time[a];
            boostersRefTime[a][Bst_DefenceIII] = &ref_bst_defence_time[a];
            boostersRefTime[a][Bst_HealthRecovery] = &ref_bst_helthRecovery_time[a];
            boostersRefTime[a][Bst_SpeedI] = &ref_bst_speed_time[a];
            boostersRefTime[a][Bst_SpeedII] = &ref_bst_speed_time[a];
            boostersRefTime[a][Bst_UnitBuildBoostI] = &ref_bst_unitBuildBoost_time[a];
            boostersRefTime[a][Bst_UnitBuildBoostII] = &ref_bst_unitBuildBoost_time[a];
        }
    }

    //Booster HUD
    float boosterHUD_transparency[Game::NumOfTeams];
    float boosterHUD_timeRatio[Game::NumOfTeams];
    sf::Vector2f boosterHUD_position[Game::NumOfTeams];
    for(int a = 0; a < Game::NumOfTeams; a++)
    {
        boosterHUD_transparency[a] = 0.f;
        boosterHUD_timeRatio[a] = 0.f;
        boosterHUD_position[a] = sf::Vector2f(SCREEN_WIDTH - GetMapTexture().getSize().x / 2.f - 40 - gameGuiScreenEdgeAllowance.x, gameGuiScreenEdgeAllowance.y);
    }
    GUI::DisplayVertexArray* displayVA_boosters_boarder = new GUI::DisplayVertexArray(sf::Quads, nullptr,
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0));
    GUI::DisplayVertexArray* displayVA_boosters_icon = new GUI::DisplayVertexArray(sf::Quads, texture_boosters,
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0));
    displayVA_boosters_boarder->Resize(4 * Game::NumOfTeams * 2);
    displayVA_boosters_icon->Resize(4 * Game::NumOfTeams);

    //Map
    MapButton* button_map = nullptr;
    GUI::ObjectSelection* selection_map = nullptr;
    {
        sf::Color clr = Game::GetCurrentRankThemeColor();
        clr.a = 255 * 0.8f;
        button_map = new MapButton(clr);

        sf::Vector2f size_ = button_map->GetButtonSize();
        selection_map = new GUI::ObjectSelection(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f), sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT), -size_.x, 1.f, 0.2f, GUI::Horizontal, false, false, false, false);

        selection_map->AddObject(button_map);
        selection_map->AddObject(displayVA_boosters_boarder);
        selection_map->AddObject(displayVA_boosters_icon);
        guiSession.AddObject(selection_map, "gameplay ui");
    }

    //Gun HUD
    GUI::Session* session_gun_sellection = new GUI::Session();
    GUI::Session* session_gun_info = new GUI::Session();
    GUI::ObjectSelection* selection_gun_info = new GUI::ObjectSelection(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 108, SCREEN_HEIGHT - 170 - gameGuiScreenEdgeAllowance.y), sf::Vector2f(216, 110), 0, 0, 0.1f, GUI::Horizontal, false, false, false, false);
    std::string ref_gun_hud[Gun::NumOfGuns];
    std::string ref_gun_hud_ammo[Gun::NumOfGuns];
    std::map<std::string, GUI::DisplayVertexArray*> gun_info_displayVA;
    bool isGunHUDOpen = false;
    GUI::DisplayVertexArray* lastGunInfoVA = nullptr;
    {
        ShipIdentity* temp_identity = nullptr;

        for(int a = 0; a < GetShipIdentitySize(); a++)
        {
            temp_identity = GetShipIdentity(a);
            if(temp_identity->GetShipInfo()->team == gamePlayerTeam)// && temp_identity->GetPlayableStatus())
            {
                std::stringstream ss;
                ss << temp_identity;

                std::string id = ss.str();
                ShipInfo* shipInfo = temp_identity->GetShipInfo();
                short level = ShipInfo::CalculateClassLevel(shipInfo->shipClass, shipInfo->experiencePoint);
                session_gun_sellection->Create(id);
                session_gun_info->Create(id);

                //Gun Selection
                GUI::DisplayVertexArray* displayVA_guns = new GUI::DisplayVertexArray(sf::Quads, texture_guns,
                                                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                                                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                                                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0));
                GUI::DisplayVertexArray* displayVA_ammo = new GUI::DisplayVertexArray(sf::Quads, texture_iconStates,
                                                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                                                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                                                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0));
                GUI::Group* tempGroup = new GUI::Group(true, false);
                session_gun_sellection->AddObject(tempGroup, id);
                sf::Vector2f refPos(gameGuiScreenEdgeAllowance.x, SCREEN_HEIGHT - 150 - gameGuiScreenEdgeAllowance.y);

                for(int b = 0; b < Gun::NumOfGuns; b++)
                    if(shipInfo->unlock_gun[b] <= level)
                    {
                        tempGroup->AddObject(new GUI::ShapeToggleButton(style_generalShapeButton, style_generalShapeButton, sf::Vector2f(175, 30),
                                                                                     rankTransClr, rankTransClr, ChangeColorHue(rankTransClr, 30),
                                                                                     sf::Color(207, 117, 117, 255.f * 0.7f), sf::Color(207, 117, 117, 255.f * 0.7f), ChangeColorHue(sf::Color(207, 117, 117, 255.f * 0.7f), 30), false,
                                                                                     refPos + sf::Vector2f(175.f / 2.f, 15), resourcePack_general, "Switch_Gun " + ConvertIntToString(b), "Switch_Gun " + ConvertIntToString(b), "", nullptr, 1.f, sf::IntRect(), 0, 0,
                                                                                     false, &ref_gun_hud[b], false));
                        if(b == Gun::LaserGun)
                        {
                            GUI::DisplayText* tempText = new GUI::DisplayText("", font_conthrax, sf::Color::White, 18 * 2, sf::Vector2f(1, 0),
                                                                       GUI::DisplayObject::DisplayPhase(refPos + sf::Vector2f(143, -2), sf::Vector2f(0.7f, 0.7f), 0),
                                                                       GUI::DisplayObject::DisplayPhase(refPos + sf::Vector2f(143, -2), sf::Vector2f(0.7f, 0.7f), 255),
                                                                       GUI::DisplayObject::DisplayPhase(refPos + sf::Vector2f(143, -2), sf::Vector2f(0.7f, 0.7f), 0));
                            tempText->SetText(std::wstring() + static_cast<wchar_t>(8734));
                            tempGroup->AddObject(tempText);
                        }
                        else tempGroup->AddObject(new GUI::DisplayText(&ref_gun_hud_ammo[b], font_conthrax, sf::Color::White, 18 * 2, sf::Vector2f(1, 0),
                                                                       GUI::DisplayObject::DisplayPhase(refPos + sf::Vector2f(143, 3), sf::Vector2f(0.5f, 0.5f), 0),
                                                                       GUI::DisplayObject::DisplayPhase(refPos + sf::Vector2f(143, 3), sf::Vector2f(0.5f, 0.5f), 255),
                                                                       GUI::DisplayObject::DisplayPhase(refPos + sf::Vector2f(143, 3), sf::Vector2f(0.5f, 0.5f), 0)));

                        displayVA_guns->AddRectVertices(sf::FloatRect(refPos.x, refPos.y, 84, 30), sf::Color::White, 0, sf::IntRect(0, 100 * b, 280, 100));
                        displayVA_ammo->AddRectVertices(sf::FloatRect(refPos.x + 145, refPos.y, 30, 30), sf::Color::White, 0, sf::IntRect(480, 60, 60, 60));

                        refPos.y -= 35.f;
                    }
                tempGroup->AddObject(displayVA_ammo);
                tempGroup->AddObject(displayVA_guns);
                tempGroup->AddObject(new GUI::DisplayShape(sf::Vector2f(175, 5), sf::Color::White, sf::Vector2f(0, 0),
                                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x, SCREEN_HEIGHT - 120 - gameGuiScreenEdgeAllowance.y), sf::Vector2f(1, 1), 0),
                                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x, SCREEN_HEIGHT - 120 - gameGuiScreenEdgeAllowance.y), sf::Vector2f(1, 1), 255),
                                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x, SCREEN_HEIGHT - 120 - gameGuiScreenEdgeAllowance.y), sf::Vector2f(1, 1), 0)));

                //Gun Info
                selection_gun_info->AddObject(session_gun_info);

                GUI::DisplayVertexArray* tempShapesDisplayVA = new GUI::DisplayVertexArray(sf::Quads, nullptr,
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0));
                GUI::DisplayVertexArray* tempGunsDisplayVA = new GUI::DisplayVertexArray(sf::Quads, texture_guns,
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0));
                gun_info_displayVA.insert(std::make_pair(id, tempShapesDisplayVA));
                session_gun_info->AddObject(tempShapesDisplayVA, id);
                session_gun_info->AddObject(tempGunsDisplayVA, id);
                sf::Vector2f refPos_info(-216 - 10, 0);

                for(int b = 0; b < 2; b++)
                {
                    tempShapesDisplayVA->AddRectVertices(sf::FloatRect(refPos_info.x, refPos_info.y, 216, 105), rankTransClr);
                    tempShapesDisplayVA->AddRectVertices(sf::FloatRect(refPos_info.x, refPos_info.y + 105, 216, 5), sf::Color::White);
                    tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159, 0), sf::Color::White));
                    tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159 + 57, 0), sf::Color::White));
                    tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159 + 57, 22), sf::Color::White));
                    tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159 + 7, 22), sf::Color::White));
                    if(b != 0)
                    {
                        tempGunsDisplayVA->AddRectVertices(sf::FloatRect(refPos_info.x + (216 - 210) / 2, refPos_info.y + (105 - 75) / 2, 210, 75), sf::Color::White, 0, sf::IntRect(0, 0, 280, 100));
                        session_gun_info->AddObject(new GUI::DisplayText(std::wstring() + static_cast<wchar_t>(8734), font_conthrax, rankClr, 18 * 2, sf::Vector2f(0.5f, 0.5f),
                                                                         GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(188, 1), sf::Vector2f(0.7f, 0.7f), 0),
                                                                         GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(188, 1), sf::Vector2f(0.7f, 0.7f), 255),
                                                                         GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(188, 1), sf::Vector2f(0.7f, 0.7f), 0)), id);
                    }
                    refPos_info.x += 226;
                }

                short lastGun = 0;
                for(int b = 1; b < Gun::NumOfGuns; b++)
                    if(shipInfo->unlock_gun[b] <= level)
                    {
                        tempShapesDisplayVA->AddRectVertices(sf::FloatRect(refPos_info.x, refPos_info.y, 216, 105), rankTransClr);
                        tempShapesDisplayVA->AddRectVertices(sf::FloatRect(refPos_info.x, refPos_info.y + 105, 216, 5), sf::Color::White);
                        tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159, 0), sf::Color::White));
                        tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159 + 57, 0), sf::Color::White));
                        tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159 + 57, 22), sf::Color::White));
                        tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159 + 7, 22), sf::Color::White));
                        tempGunsDisplayVA->AddRectVertices(sf::FloatRect(refPos_info.x + (216 - 210) / 2, refPos_info.y + (105 - 75) / 2, 210, 75), sf::Color::White, 0, sf::IntRect(0, 100 * b, 280, 100));
                        session_gun_info->AddObject(new GUI::DisplayText(&ref_gun_hud_ammo[b], font_conthrax, rankClr, 16 * 2, sf::Vector2f(0.5f, 0.5f),
                                                                         GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(190, 6), sf::Vector2f(0.5f, 0.5f), 0),
                                                                         GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(190, 6), sf::Vector2f(0.5f, 0.5f), 255),
                                                                         GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(190, 6), sf::Vector2f(0.5f, 0.5f), 0)), id);
                        refPos_info.x += 226;
                        lastGun = b;
                    }

                tempShapesDisplayVA->AddRectVertices(sf::FloatRect(refPos_info.x, refPos_info.y, 216, 105), rankTransClr);
                tempShapesDisplayVA->AddRectVertices(sf::FloatRect(refPos_info.x, refPos_info.y + 105, 216, 5), sf::Color::White);
                tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159, 0), sf::Color::White));
                tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159 + 57, 0), sf::Color::White));
                tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159 + 57, 22), sf::Color::White));
                tempShapesDisplayVA->AddVertex(sf::Vertex(refPos_info + sf::Vector2f(159 + 7, 22), sf::Color::White));
                tempGunsDisplayVA->AddRectVertices(sf::FloatRect(refPos_info.x + (216 - 210) / 2, refPos_info.y + (105 - 75) / 2, 210, 75), sf::Color::White, 0, sf::IntRect(0, 100 * 0, 280, 100));
                session_gun_info->AddObject(new GUI::DisplayText(std::wstring() + static_cast<wchar_t>(8734), font_conthrax, rankClr, 18 * 2, sf::Vector2f(0.5f, 0.5f),
                                                                 GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(188, 1), sf::Vector2f(0.7f, 0.7f), 0),
                                                                 GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(188, 1), sf::Vector2f(0.7f, 0.7f), 255),
                                                                 GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(188, 1), sf::Vector2f(0.7f, 0.7f), 0)), id);
                refPos_info.x = -226;
                tempGunsDisplayVA->AddRectVertices(sf::FloatRect(refPos_info.x + (216 - 210) / 2, refPos_info.y + (105 - 75) / 2, 210, 75), sf::Color::White, 0, sf::IntRect(0, 100 * lastGun, 280, 100));
                session_gun_info->AddObject(new GUI::DisplayText(&ref_gun_hud_ammo[lastGun], font_conthrax, rankClr, 16 * 2, sf::Vector2f(0.5f, 0.5f),
                                                                 GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(190, 6), sf::Vector2f(0.5f, 0.5f), 0),
                                                                 GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(190, 6), sf::Vector2f(0.5f, 0.5f), 255),
                                                                 GUI::DisplayObject::DisplayPhase(refPos_info + sf::Vector2f(190, 6), sf::Vector2f(0.5f, 0.5f), 0)), id);
            }
        }
    }

    focusedButton = new FocusedShipButton();
    focusedProfile = new FocusedShipDisplay();
    baseInfoButton = new BaseInfoButton();

    guiSession.AddObject(focusedProfile, "gameplay ui");
    guiSession.AddObject(baseInfoButton, "gameplay ui");
    guiSession.AddObject(session_gun_sellection, "gameplay ui");
    guiSession.AddObject(selection_gun_info, "gameplay ui");
    guiSession.AddObject(session_topLeft, "gameplay ui");

    session_topLeft->AddObject(focusedButton, "focused units");
    session_topLeft->TransmitTo("focused units");
    session_topLeft_tab->TransmitTo("behavior");
    guiSession.TransmitTo("gameplay ui");
    focusedButton->SetInactive();
    focusedProfile->SetInactive();
    baseInfoButton->SetInactive();

    if(currentRank >= 0 && currentRank < NumOfRanks)
        SetCursorTextrue(&rankCursorTextures[currentRankTheme]);
    clock.restart();

    auto updateBoosterInfo = [&](std::string& stringRef_, float& timeRef_, const float& deltaTime_)
    {
        if(timeRef_ > 0.f)
            timeRef_ -= deltaTime_;
        if(stringRef_ != "" && timeRef_ <= 0)
        {
            timeRef_ = 0.f;
            stringRef_ = "";
            updateBoosterStatus = true;
        }
    };

    /*
    std::vector<sf::Vector2i> path;
    if(AStar::FindFastPath(sf::Vector2i(0, 0), sf::Vector2i(9, 5), path, AStar::NoSearchType, -1, true, true))
    {
        std::cerr << "Found path...\n";
        for(auto grid : path)
            std::cerr << "...[" << grid.x << ", " << grid.y << "]\n";
    }
    path.clear();
    AStar::ContinueLastTargetFastSearch(sf::Vector2i(0, 2));
    if(AStar::FindFastPath(sf::Vector2i(), sf::Vector2i(), path))
    {
        std::cerr << "Found path...\n";
        for(auto grid : path)
            std::cerr << "...[" << grid.x << ", " << grid.y << "]\n";
    }
    path.clear();
    AStar::ContinueLastTargetFastSearch(sf::Vector2i(0, 4));
    if(AStar::FindFastPath(sf::Vector2i(), sf::Vector2i(), path))
    {
        std::cerr << "Found path...\n";
        for(auto grid : path)
            std::cerr << "...[" << grid.x << ", " << grid.y << "]\n";
    }
    path.clear();
    AStar::ContinueLastTargetFastSearch(sf::Vector2i(4, 2));
    if(AStar::FindFastPath(sf::Vector2i(), sf::Vector2i(), path))
    {
        std::cerr << "Found path...\n";
        for(auto grid : path)
            std::cerr << "...[" << grid.x << ", " << grid.y << "]\n";
    }*/

    //Play loop
    while(gameInstance->window.isOpen() && quit == false)
    {
        dtFPS = clock.getElapsedTime().asSeconds();
        deltaTime = clock.restart().asSeconds();

        if(deltaTime > MIN_FRAMERATE)
            deltaTime = MIN_FRAMERATE;

        UpdateMapTexture(deltaTime);
        HandleInputs();

        //Handle Boosters
        for(int a = 0; a < Game::NumOfTeams; a++)
        {
            updateBoosterInfo(ref_bst_attack[a], ref_bst_attack_time[a], deltaTime);
            updateBoosterInfo(ref_bst_defence[a], ref_bst_defence_time[a], deltaTime);
            updateBoosterInfo(ref_bst_speed[a], ref_bst_speed_time[a], deltaTime);
            updateBoosterInfo(ref_bst_unitBuildBoost[a], ref_bst_unitBuildBoost_time[a], deltaTime);
            updateBoosterInfo(ref_bst_helthRecovery[a], ref_bst_helthRecovery_time[a], deltaTime);
        }

        if(updateBoosterStatus)
        {
            for(int a = 0; a < Game::NumOfTeams; a++)
            {
                gameBoostersStatus[a][Bst_AttackI] = ref_bst_attack[a] == "Bst_AttackI";
                gameBoostersStatus[a][Bst_AttackII] = ref_bst_attack[a] == "Bst_AttackII";
                gameBoostersStatus[a][Bst_AttackIII] = ref_bst_attack[a] == "Bst_AttackIII";
                gameBoostersStatus[a][Bst_DefenceI] = ref_bst_defence[a] == "Bst_DefenseI";
                gameBoostersStatus[a][Bst_DefenceII] = ref_bst_defence[a] == "Bst_DefenseII";
                gameBoostersStatus[a][Bst_DefenceIII] = ref_bst_defence[a] == "Bst_DefenseIII";
                gameBoostersStatus[a][Bst_SpeedI] = ref_bst_speed[a] == "Bst_SpeedI";
                gameBoostersStatus[a][Bst_SpeedII] = ref_bst_speed[a] == "Bst_SpeedII";
                gameBoostersStatus[a][Bst_UnitBuildBoostI] = ref_bst_unitBuildBoost[a] == "Bst_UnitBuildBoostI";
                gameBoostersStatus[a][Bst_UnitBuildBoostII] = ref_bst_unitBuildBoost[a] == "Bst_UnitBuildBoostII";
                gameBoostersStatus[a][Bst_HealthRecovery] = ref_bst_helthRecovery[a] == "Bst_HealthRecovery";

                boostersInUse[a].clear();
                for(int b = 0; b < NumOfGameBoosters; b++)
                    if(gameBoostersStatus[a][b])
                        boostersInUse[a].push_back(b);
            }

            int numOfActiveBoosters = boostersInUse[gamePlayerTeam].size();
            if(numOfActiveBoosters > 0)
                displayText_boostersLeft->SetText(ConvertIntToString(numOfActiveBoosters) + (numOfActiveBoosters == 1 ? " booster is currently active." : " boosters are currently active."));
            else displayText_boostersLeft->SetText("No boosters are active.");
        }

        {
            bool isAnyTeamBoosterActive = false;
            for(int a = 0; a < NumOfTeams; a++)
                if(boostersInUse[a].size() > 0)
                {
                    isAnyTeamBoosterActive = true;
                    break;
                }

            if(isAnyTeamBoosterActive)
            {
                displayVA_boosters_boarder->SetPhase(GUI::DefaultPhase);
                displayVA_boosters_icon->SetPhase(GUI::DefaultPhase);

                if(lastMapButtonScale != button_map->GetRealTimeScale())
                {
                    float displacement = (lastMapButtonScale - button_map->GetRealTimeScale()) * GetMapTexture().getSize().x / 2.f;

                    for(int a = 0; a < NumOfTeams; a++)
                        boosterHUD_position[a].x += displacement;
                    lastMapButtonScale = button_map->GetRealTimeScale();
                }

                float nextPos = SCREEN_WIDTH - button_map->GetButtonSize().x - 55;
                for(int a = Game::NumOfTeams - 1; a >= 0; a--)
                {
                    if(boostersInUse[a].size() > 0)
                    {
                        TendTowards(boosterHUD_position[a].x, nextPos, 0.1f, 0.1f, deltaTime);
                        TendTowards(boosterHUD_transparency[a], 255, 0.1f, 0.1f, deltaTime);
                        nextPos -= 35;
                    }
                    else
                    {
                        TendTowards(boosterHUD_position[a].x, 105, 0.1f, 0.1f, deltaTime);
                        TendTowards(boosterHUD_transparency[a], 0, 0.1f, 0.1f, deltaTime);
                    }

                    float textrPosX = displayVA_boosters_boarder->GetVertex(4 * Game::NumOfTeams + 4 * a)->texCoords.x;
                    if(boostersInUse[a].size() > 0)
                    {
                        float timeRatio = 0;
                        short currentBooster = boostersInUse[a][(int)Game::GetGameExecutionTimeInSec() % (boostersInUse[a].size() * 2) / 2];
                        textrPosX = 60 * currentBooster;
                        timeRatio = boostersRefTime[a][currentBooster] != nullptr ? *boostersRefTime[a][currentBooster] / gameBoostersTime[currentBooster] : 0.f;
                        if(timeRatio > 1.f) timeRatio = 1.f;
                        if(timeRatio < 0.f) timeRatio = 0.f;

                        TendTowards(boosterHUD_timeRatio[a], timeRatio, 0.1f, 0.00001f, deltaTime);
                    }

                    sf::Color clr_boarder(255, 255, 255, boosterHUD_transparency[a]);
                    sf::Color clr = teamTransColor[a];
                    clr.a = (float)clr.a * (boosterHUD_transparency[a] / 255.f);

                    displayVA_boosters_boarder->EditVertext(4 * a, boosterHUD_position[a] + sf::Vector2f(0, 0), clr, sf::Vector2f());
                    displayVA_boosters_boarder->EditVertext(4 * a + 1, boosterHUD_position[a] + sf::Vector2f(30, 0), clr, sf::Vector2f());
                    displayVA_boosters_boarder->EditVertext(4 * a + 2, boosterHUD_position[a] + sf::Vector2f(30, 30), clr, sf::Vector2f());
                    displayVA_boosters_boarder->EditVertext(4 * a + 3, boosterHUD_position[a] + sf::Vector2f(0, 30), clr, sf::Vector2f());

                    displayVA_boosters_boarder->EditVertext(4 * Game::NumOfTeams + 4 * a, boosterHUD_position[a] + sf::Vector2f(30, 30 * (1.f - boosterHUD_timeRatio[a])), clr_boarder, sf::Vector2f());
                    displayVA_boosters_boarder->EditVertext(4 * Game::NumOfTeams + 4 * a + 1, boosterHUD_position[a] + sf::Vector2f(32, 30 * (1.f - boosterHUD_timeRatio[a])), clr_boarder, sf::Vector2f());
                    displayVA_boosters_boarder->EditVertext(4 * Game::NumOfTeams + 4 * a + 2, boosterHUD_position[a] + sf::Vector2f(32, 30), clr_boarder, sf::Vector2f());
                    displayVA_boosters_boarder->EditVertext(4 * Game::NumOfTeams + 4 * a + 3, boosterHUD_position[a] + sf::Vector2f(30, 30), clr_boarder, sf::Vector2f());

                    displayVA_boosters_icon->EditVertext(4 * a, boosterHUD_position[a] + sf::Vector2f(0, 0), clr_boarder, sf::Vector2f(textrPosX, 0));
                    displayVA_boosters_icon->EditVertext(4 * a + 1, boosterHUD_position[a] + sf::Vector2f(30, 0), clr_boarder, sf::Vector2f(textrPosX + 60, 0));
                    displayVA_boosters_icon->EditVertext(4 * a + 2, boosterHUD_position[a] + sf::Vector2f(30, 30), clr_boarder, sf::Vector2f(textrPosX + 60, 60));
                    displayVA_boosters_icon->EditVertext(4 * a + 3, boosterHUD_position[a] + sf::Vector2f(0, 30), clr_boarder, sf::Vector2f(textrPosX, 60));
                }
            }
            else
            {
                displayVA_boosters_boarder->SetPhase(GUI::InactivePhase);
                displayVA_boosters_icon->SetPhase(GUI::InactivePhase);
            }
        }

        //Updating GUI
        {
            GUI::SetCursorOnObjectState(false);
            if(guiSession.HandleInput((sf::Vector2i)mousePosition, GetInput(sf::Mouse::Left, Pressed), deltaTime))
                message = guiSession.GetMessage();

            //Handle Map
            selection_map->SetScrollLength(button_map->GetButtonSize().x);

            //Highlighting
            sf::FloatRect cameraRect = EntityManager::GetCameraRect();
            sf::Vector2f mouseWorldPosition = sf::Vector2f(cameraRect.left, cameraRect.top) + mousePosition * (cameraRect.height / SCREEN_HEIGHT);
            if(highlighting == false && EntityManager::GetCameraFocus() == nullptr && GUI::IsCursorOnObject() == false)
            {
                if(GetInput(sf::Mouse::Left, Nothing) == false && GetInput(sf::Keyboard::LShift, Nothing) == false)
                {
                    highlighting = true;
                    highlightStart = mouseWorldPosition;
                }
                else if(GetInput(sf::Mouse::Left, Pressed))
                {
                    focusedShips.clear();
                    updateInfo_identity = nullptr;
                    sf::Vector2i mouseGrid = GetGridPosition(mouseWorldPosition);
                    sf::FloatRect mouseRect(mouseWorldPosition.x, mouseWorldPosition.y, 1, 1);
                    if(mouseGrid.x >= 0 && mouseGrid.y >= 0 && mouseGrid.x < MapTileManager::MAX_MAPTILE && mouseGrid.y < MapTileManager::MAX_MAPTILE)
                        for(Ship*& unit : GetGridInfo(mouseGrid).ships)
                            if(mouseRect.intersects(unit->GetBoundingRect()))
                            {
                                focusedShips.push_back(unit);
                                updateInfo_identity = GetShipIdentity(unit);
                                break;
                            }
                    if(focusedShips.size() == 0)
                        focusedButton->SetInactive();
                    else focusedButton->Reset();
                }
            }

            if(highlighting)
            {
                if(GetInput(sf::Mouse::Left, Nothing) || GetInput(sf::Keyboard::LShift, Nothing))
                    highlighting = false;


                highlightRectangle.setPosition((highlightStart - sf::Vector2f(cameraRect.left, cameraRect.top)) * (SCREEN_HEIGHT / cameraRect.height));
                highlightRectangle.setSize((mouseWorldPosition - highlightStart) * (SCREEN_HEIGHT / cameraRect.height));

                if(highlighting == false)
                {
                    focusedShips.clear();

                    int xMin = highlightStart.x / MapTileManager::TILE_SIZE;
                    int xMax = mouseWorldPosition.x / MapTileManager::TILE_SIZE;
                    if(xMax < xMin)
                    {
                        int temp = xMax;
                        xMax = xMin;
                        xMin = temp;
                    }

                    int yMin = highlightStart.y / MapTileManager::TILE_SIZE;
                    int yMax = mouseWorldPosition.y / MapTileManager::TILE_SIZE;
                    if(yMax < yMin)
                    {
                        int temp = yMax;
                        yMax = yMin;
                        yMin = temp;
                    }

                    sf::FloatRect rect(highlightStart.x, highlightStart.y, mouseWorldPosition.x - highlightStart.x, mouseWorldPosition.y - highlightStart.y);
                    for(int y = yMin; y <= yMax; y++)
                        for(int x = xMin; x <= xMax; x++)
                            if(y >= 0 && x >= 0 && y < MapTileManager::MAX_MAPTILE && x < MapTileManager::MAX_MAPTILE)
                                for(Ship*& unit : GetGridInfo(y, x).positionShips)
                                {
                                    sf::Vector2f unitPos = unit->GetPosition();
                                    if(rect.intersects(sf::FloatRect(unitPos.x, unitPos.y, 1, 1)))
                                        focusedShips.push_back(unit);
                                }
                    updateInfo_identity = nullptr;
                    if(focusedShips.size() == 1)
                        updateInfo_identity = GetShipIdentity(*focusedShips.begin());

                    if(focusedShips.size() == 0)
                        focusedButton->SetInactive();
                    else focusedButton->Reset();
                }
            }

            if(focusedShips.size() > 0)
            {
                //VertexArrays
                for(Ship*& unit : focusedShips)
                {
                    sf::FloatRect camRect = EntityManager::GetCameraRect();
                    sf::Vector2f camTopLeft(cameraRect.left, cameraRect.top);

                    float radius = (unit->GetSizeRadius() + 10) * ((float)SCREEN_HEIGHT / cameraRect.height);
                    if(radius < 30)
                        radius = 30;
                    sf::Vector2f unitPosition = (unit->GetPosition() - camTopLeft) * ((float)SCREEN_HEIGHT / cameraRect.height);
                    sf::Vector2f refPos = unitPosition - sf::Vector2f(radius, radius);

                    if(sf::FloatRect(refPos.x, refPos.y, radius * 2, radius * 2).intersects(sf::FloatRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)))
                    {
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(20, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(20, 4), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 4), sf::Color::White));

                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(4, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(4, 20), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 20), sf::Color::White));

                        refPos = unitPosition + sf::Vector2f(radius, -radius);
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(-20, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(-20, 4), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 4), sf::Color::White));

                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(-4, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(-4, 20), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 20), sf::Color::White));

                        refPos = unitPosition + sf::Vector2f(radius, radius);
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(-20, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(-20, -4), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, -4), sf::Color::White));

                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(-4, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(-4, -20), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, -20), sf::Color::White));

                        refPos = unitPosition + sf::Vector2f(-radius, radius);
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(20, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(20, -4), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, -4), sf::Color::White));

                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(4, 0), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(4, -20), sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(refPos + sf::Vector2f(0, -20), sf::Color::White));
                    }
                    else
                    {
                        sf::Vector2f diff = unitPosition - sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);
                        sf::Vector2f areaMagn(SCREEN_WIDTH / 2.f  - 20.f, SCREEN_HEIGHT / 2.f - 20.f);

                        float scaleX = (diff.x < 0 ? -diff.x : diff.x) / areaMagn.x;
                        float scaleY = (diff.y < 0 ? -diff.y : diff.y) / areaMagn.y;

                        if(scaleX > scaleY)
                            refPos = diff / scaleX;
                        else refPos = diff / scaleY;
                        refPos = sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f) + refPos;

                        float angle = 90 + (atan2f(diff.y, diff.x) * 180 / 3.142f);
                        sf::Transform transform_;
                        transform_.rotate(angle);

                        va_highlighedShips.append(sf::Vertex(transform_.transformPoint(sf::Vector2f(0, 0)) + refPos, sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(transform_.transformPoint(sf::Vector2f(10, 20)) + refPos, sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(transform_.transformPoint(sf::Vector2f(0, 20)) + refPos, sf::Color::White));
                        va_highlighedShips.append(sf::Vertex(transform_.transformPoint(sf::Vector2f(-10, 20)) + refPos, sf::Color::White));
                    }
                }

                //Mouse Commands
                if(GetInput(sf::Mouse::Right, Released) && GUI::IsCursorOnObject() == false)
                {
                    sf::Vector2i mouseGrid = GetGridPosition(mouseWorldPosition);
                    short mouseBaseId = -1;
                    if(mouseGrid.x >= 0 && mouseGrid.y >= 0 && mouseGrid.x < MapTileManager::MAX_MAPTILE && mouseGrid.y < MapTileManager::MAX_MAPTILE)
                        mouseBaseId = GetGridInfo(mouseGrid).baseId;
                    Base* base = GetBase(mouseBaseId);

                    if(mouseBaseId >= 0 && base->IsActive())
                    {
                        baseHighlightSelectTime = 0.5f;
                        baseHighlightSelectId = mouseBaseId;
                        highlightDestination = mouseWorldPosition;
                        highlightState = -1;

                        if(GetTeamUpType(base->GetTeam()) == GetTeamUpType(GetPlayerTeam()))
                        {
                            text_command.setString("Defend Base");
                            highlightState = ShipUnit::Defending;
                        }
                        else
                        {
                            text_command.setString("Attack Base");
                            highlightState = ShipUnit::Attacking;
                        }
                    }
                    else
                    {
                        text_command.setString("Travel Here");
                        highlightState = ShipUnit::Traveling;
                    }

                    if(highlightState >= 0)
                    {
                        std::list<ShipUnit*> groupLeaders;
                        for(Ship*& ship : focusedShips)
                            if(ship->GetShipType() == Ship::UnitShip)
                            {
                                if(ship->GetGroup() != nullptr)
                                {
                                    if(ship->GetGroup()->leader == ship)
                                        groupLeaders.push_back((ShipUnit*)ship);
                                    else
                                    {
                                        bool leave = true;
                                        for(Ship*& check : focusedShips)
                                            if(ship->GetGroup()->leader == check)
                                            {
                                                leave = false;
                                                break;
                                            }
                                        if(leave)
                                            ship->LeaveGroup();
                                    }
                                }
                                else ship->LeaveGroup();
                            }

                        for(auto& ship : groupLeaders)
                            EntityManager::RecruitGroupMembers(ship);

                        for(Ship*& ship : focusedShips)
                            if(ship->GetShipType() == Ship::UnitShip)
                                if(ship->GetGroup() == nullptr)
                                {
                                    groupLeaders.push_back((ShipUnit*)ship);
                                    ship->JoinGroup(Ship::UnitGroup::CreatUnitGroups(), true);
                                    EntityManager::RecruitGroupMembers(ship);
                                }

                        if(groupLeaders.size() > 0)
                        {
                            for(ShipUnit*& ship : groupLeaders)
                            {
                                if(highlightState == ShipUnit::Attacking)
                                    ship->Attack(mouseBaseId);
                                else if(highlightState == ShipUnit::Defending)
                                    ship->Defend(mouseBaseId, false, 2.f);
                                else if(highlightState == ShipUnit::Traveling)
                                    ship->Travel(mouseWorldPosition, true, 10 * focusedShips.size());
                            }

                            EntityManager::MoveMultipleUnits(groupLeaders, (sf::Vector2f)(mouseGrid * MapTileManager::TILE_SIZE), AStar::NoSearchType, GetTeamUpType(gamePlayerTeam));
                        }
                    }

                    text_commandTime = 0.5f;
                    text_command.setOrigin(text_command.getLocalBounds().width / 2.f, text_command.getLocalBounds().height / 2.f);
                    highlightDestination = mouseWorldPosition;
                }

                //Move point
                if(highlightState >= 0)
                    for(Ship*& ship : focusedShips)
                        if(ship->GetShipType() == Ship::UnitShip && ((ShipUnit*)ship)->GetState() == highlightState)
                        {
                            if(highlightState == ShipUnit::Traveling && ((ShipUnit*)ship)->GetTravellingGrid() == GetGridPosition(highlightDestination))
                            {
                                sf::Vector2f refPos = (highlightDestination - sf::Vector2f(cameraRect.left, cameraRect.top)) * ((float)SCREEN_HEIGHT / cameraRect.height);
                                AddRectVerticesToArray(va_highlighedShips, sf::FloatRect(refPos.x - 10, refPos.y - 2, 20, 4), sf::Color::White, 45);
                                AddRectVerticesToArray(va_highlighedShips, sf::FloatRect(refPos.x - 10, refPos.y - 2, 20, 4), sf::Color::White, -45);
                            }
                            break;
                        }
            }

            if(text_commandTime > 0.f)
            {
                text_commandTime -= deltaTime;

                float factor = text_commandTime / 0.5f;
                factor = 1 - (1 - factor) * (1 - factor);
                text_command.setPosition((highlightDestination - sf::Vector2f(cameraRect.left, cameraRect.top)) * ((float)SCREEN_HEIGHT / cameraRect.height) + sf::Vector2f(0, -15 - 20 * factor));
                text_command.setFillColor(sf::Color(255, 255, 255, 255 * factor));
            }


            //Update FocusedButton Info
            if(focusedShips.size() > 0 || updateInfo_identity != nullptr)
            {
                sf::Texture* texture_ = nullptr;
                std::string name_ = "";
                int level_ = -1;
                float expRatio_ = -1;
                float healthRatio_ = -1;
                float specialRatio_ = -1;
                short moral_ = -1;
                int surUnits_ = -1;
                int state_ = -1;
                std::string stateDesc_ = "";
                std::string prim_ = "";
                std::string sec_ = "";
                bool isTextureSkin_ = false;
                bool actionOptions_ = false;
                bool focusOption_ = false;
                bool detailsOption_ = false;
                bool thinkOption_ = false;
                float reviveSeconds_ = -1;
                updateInfo_reset = true;

                if(updateInfo_identity != nullptr)
                {
                    if(focusedShips.size() == 0 && updateInfo_identity->GetShipEntity() != nullptr)
                        focusedShips.push_back(updateInfo_identity->GetShipEntity());

                    isTextureSkin_ = true;
                    texture_ = &Game::GetSkin((SkinType)updateInfo_identity->GetShipInfo()->skinType,
                                              updateInfo_identity->GetShipInfo()->skinIndex,
                                              updateInfo_identity->GetShipInfo()->team)->texture;

                    name_ = updateInfo_identity->GetShipInfo()->name;
                    level_ = updateInfo_identity->GetUnitLevel();
                    expRatio_ = updateInfo_identity->GetUnitExpFactor();
                    moral_ = 3;
                    if(updateInfo_identity->GetShipEntity() != nullptr)
                    {
                        healthRatio_ = updateInfo_identity->GetShipEntity()->GetHealthInFactors();
                        specialRatio_ = updateInfo_identity->GetShipEntity()->IsSpecialActive() ? updateInfo_identity->GetShipEntity()->GetSpecialInFactors() : -1;
                        surUnits_ = updateInfo_identity->GetShipEntity()->GetSurroundingTroops();
                        state_ = updateInfo_identity->GetShipEntity()->GetState();
                        if(state_ == ShipUnit::Attacking)
                            stateDesc_ = "Attacking Base";
                        else if(state_ == ShipUnit::Defending)
                            stateDesc_ = "Defending Base";
                        else if(state_ == ShipUnit::Idle)
                            stateDesc_ = "Idle";
                        else if(state_ == ShipUnit::FollowingGroup)
                            stateDesc_ = "Following Group";
                        else if(state_ == ShipUnit::Traveling)
                            stateDesc_ = "Traveling";
                        else stateDesc_ = "...";

                        actionOptions_ = focusOption_ = detailsOption_ = thinkOption_ = true;
                    }
                    else
                    {
                        healthRatio_ = updateInfo_identity->GetSpawnTimeFactor(true);
                        prim_ = ConvertIntToString(updateInfo_identity->GetSpawnTime());
                        sec_ = "sec";
                    }
                }
                else if(focusedShips.size() == 1)
                {
                    Ship* ship = *focusedShips.begin();
                    if(ship->GetShipType() == Ship::UnitShip)
                    {
                        ShipUnit* unitShip = (ShipUnit*)ship;
                        if(unitShip->GetUnitType() == ShipUnit::PlayerShip ||
                           unitShip->GetUnitType() == ShipUnit::MainShip ||
                           unitShip->GetUnitType() == ShipUnit::SubShip)
                        {
                            updateInfo_identity = GetShipIdentity(ship);

                            if(updateInfo_identity == nullptr)
                                focusedShips.clear();
                        }
                        else
                        {
                            texture_ = texture_focusedShipProfile_others;
                            name_ = "Unit";
                            level_ = ship->GetCurrentLevel();
                            healthRatio_ = ship->GetHealthInFactors();
                            moral_ = 3;
                            state_ = unitShip->GetState();

                            int baseLvlExp = level_ == 1 ? 0 : Ship::BaseLevelExpShared[level_ - 2];
                            int nextBaselvlExp = level_ == 5 ? 0 : Ship::BaseLevelExpShared[level_ - 1];

                            if(nextBaselvlExp > 0)
                                expRatio_ = (float)(*Ship::GetSharedExp(ship->GetTeam()) - baseLvlExp) / (nextBaselvlExp - baseLvlExp);
                            else expRatio_ = 0.f;

                            if(state_ == ShipUnit::Attacking)
                                stateDesc_ = "Attacking Base";
                            else if(state_ == ShipUnit::Defending)
                                stateDesc_ = "Defending Base";
                            else if(state_ == ShipUnit::Idle)
                                stateDesc_ = "Idle";
                            else if(state_ == ShipUnit::FollowingGroup)
                                stateDesc_ = "Following Group";
                            else if(state_ == ShipUnit::Traveling)
                                stateDesc_ = "Traveling";
                            else stateDesc_ = "...";
                            actionOptions_ = focusOption_ = thinkOption_ = true;
                        }
                    }
                    else if(ship->GetShipType() == Ship::DefenceShip)
                    {
                        texture_ = texture_focusedShipProfile_others;
                        name_ = "Defence Unit";
                        level_ = ship->GetCurrentLevel();
                        healthRatio_ = ship->GetHealthInFactors();
                        moral_ = 3;

                        int baseLvlExp = level_ == 1 ? 0 : Ship::BaseLevelExpShared[level_ - 2];
                        int nextBaselvlExp = level_ == 5 ? 0 : Ship::BaseLevelExpShared[level_ - 1];

                        if(nextBaselvlExp > 0)
                            expRatio_ = (float)(*Ship::GetSharedExp(ship->GetTeam()) - baseLvlExp) / (nextBaselvlExp - baseLvlExp);
                        else expRatio_ = 0.f;
                    }
                }
                else
                {
                    texture_ = texture_focusedShipProfile_others;
                    name_ = "Units";
                    prim_ = 'x';
                    prim_ += ConvertIntToString(focusedShips.size());
                    sec_ = "units";

                    bool isThereIdentityShip = false;
                    for(Ship*& unit : focusedShips)
                        if(unit->GetShipType() == Ship::UnitShip)
                        {
                            ShipUnit* unitShip = (ShipUnit*)unit;
                            if(unitShip->GetUnitType() == ShipUnit::PlayerShip ||
                               unitShip->GetUnitType() == ShipUnit::MainShip ||
                               unitShip->GetUnitType() == ShipUnit::SubShip)
                           {
                               isThereIdentityShip = true;
                               break;
                           }
                        }

                    if(isThereIdentityShip == false)
                    {
                        moral_ = 3;
                        actionOptions_ = true;

                        level_ = 1;
                        for(int a = 3; a >= 0; a--)
                            if(*Ship::GetSharedExp(gamePlayerTeam) >= Ship::BaseLevelExpShared[a])
                            {
                                level_ = a + 2;
                                break;
                            }

                        int baseLvlExp = level_ <= 1 ? 0 : Ship::BaseLevelExpShared[level_ - 2];
                        int nextBaselvlExp = level_ >= 5 ? 0 : Ship::BaseLevelExpShared[level_ - 1];

                        if(nextBaselvlExp > 0)
                            expRatio_ = (float)(*Ship::GetSharedExp(gamePlayerTeam) - baseLvlExp) / (nextBaselvlExp - baseLvlExp);
                        else expRatio_ = 0.f;
                    }
                }

                focusedButton->UpdateInfo(texture_, isTextureSkin_, name_, level_, expRatio_, healthRatio_, specialRatio_, moral_, surUnits_, state_, stateDesc_, actionOptions_, focusOption_, thinkOption_, detailsOption_, reviveSeconds_, prim_, sec_);
            }
            else if(updateInfo_reset)
            {
                updateInfo_reset = false;
                focusedButton->SetInactive();
            }

            if(GetInput(sf::Keyboard::LControl, Released))
            {
                if(ShipUnit::GetActivePlayerUnit() != nullptr || EntityManager::GetCameraFocus() != nullptr)
                {
                    if(ShipUnit::GetActivePlayerUnit() != nullptr)
                    {
                        focusedShips.push_back(ShipUnit::GetActivePlayerUnit());
                        updateInfo_identity = GetShipIdentity(ShipUnit::GetActivePlayerUnit());
                        focusedButton->Reset();
                    }

                    focusedProfile->SetShipIdentity(nullptr);
                    ShipUnit::SetActivePlayerUnit(nullptr);
                    EntityManager::SetCameraFocus(nullptr);
                }
                else if(focusedShips.size() == 1)
                {
                    Ship* unit = *focusedShips.begin();
                    if(unit != nullptr && unit->GetShipType() == Ship::UnitShip && unit->GetTeam() == gamePlayerTeam)
                    {
                        ShipUnit::SetActivePlayerUnit((ShipUnit*)unit);
                        EntityManager::SetCameraFocus(unit);
                        focusedProfile->SetShipIdentity(GetShipIdentity(unit));
                        focusedButton->SetInactive();
                        updateInfo_identity = nullptr;
                        focusedShips.clear();

                        ShipIdentity* tempIdentity = ShipUnit::GetActivePlayerUnit()->GetIdentity();
                        if(tempIdentity != nullptr)
                        {
                            std::stringstream ss;
                            ss << tempIdentity;
                            session_gun_info->TransmitTo(ss.str());
                            session_gun_info->SetInactive();
                            gun_info_count = 0;

                            auto found = gun_info_displayVA.find(ss.str());
                            if(found != gun_info_displayVA.end())
                                lastGunInfoVA = found->second;

                            for(int c = 0; c < Gun::NumOfGuns; c++)
                                if(tempIdentity->GetShipInfo()->unlock_gun[c] <= tempIdentity->GetShipInfoLevel())
                                {
                                    if(ShipUnit::GetActivePlayerUnit()->GetCurrentGun() == c)
                                    selection_gun_info->SetViewCenter(sf::Vector2f(108 + 226 * gun_info_count, 55), true);
                                    gun_info_count++;
                                }
                        }
                    }
                }
            }

            //Base Highlight
            if(EntityManager::GetCameraFocus() == nullptr)
                TendTowards(baseHighlightTransparency, 60.f, 0.1, 0.1f, deltaTime);
            else TendTowards(baseHighlightTransparency, 0, 0.1, 0.1f, deltaTime);

            if(baseHighlightSelectTime > 0)
                baseHighlightSelectTime -= deltaTime;

            {
                sf::Vector2i mouseGrid = ShipUnit::GetActivePlayerUnit() != nullptr ? GetGridPosition(ShipUnit::GetActivePlayerUnit()->GetPosition()) : GetGridPosition(mouseWorldPosition);
                short mouseBaseId = -1;
                if(mouseGrid.x >= 0 && mouseGrid.y >= 0 && mouseGrid.x < MapTileManager::MAX_MAPTILE && mouseGrid.y < MapTileManager::MAX_MAPTILE)
                    mouseBaseId = GetGridInfo(mouseGrid).baseId;
                Base* base = GetBase(mouseBaseId);

                if(GetInput(sf::Mouse::Left, Hold) && mouseBaseId >= 0 && ShipUnit::GetActivePlayerUnit() == nullptr && GUI::IsCursorOnObject() == false)
                {
                    if(baseLeftClick == false)
                        baseLeftClickDuration += deltaTime;
                    if(baseLeftClickDuration >= 1.f)
                        baseLeftClick = true;
                }
                else
                {
                    baseLeftClick = false;
                    baseLeftClickDuration = 0;
                }

                if(baseFocusedId < 0 && mouseBaseId >= 0 && baseLeftClickDuration >= 1.f)
                {
                    baseFocusedId = mouseBaseId;
                    baseLeftClickDuration = 0.f;
                    baseInfoButton->SetShowOptionStatus(true);
                }
                if(baseFocusedId >= 0)
                {
                    if(GetInput(sf::Mouse::Left, Nothing) != true && ShipUnit::GetActivePlayerUnit() == nullptr && GUI::IsCursorOnObject() == false)
                    {
                        if(mouseBaseId != baseFocusedId || baseLeftClickDuration >= 1.f)
                        {
                            baseFocusedId = -1;
                            baseLeftClickDuration = 0.f;
                            baseInfoButton->SetShowOptionStatus(false);
                        }
                    }
                    else
                    {
                        mouseBaseId = baseFocusedId;
                        base = GetBase(mouseBaseId);
                        baseInfoButton->SetShowOptionStatus(true);
                    }
                }

                if(mouseBaseId >= 0)
                    baseInfoButton->UpdateInfo(base->GetTeam(), base->IsActive() ? base->GetNumOfBuildUnits() : -1, 250, mouseBaseId);
                else baseInfoButton->SetInactive();
            }

            if(baseHighlightTransparency > 0)
            {
                sf::FloatRect screenRect = EntityManager::GetCameraRect();
                int xStart = screenRect.left / MapTileManager::TILE_SIZE;
                int xEnd = (screenRect.left + screenRect.width) / MapTileManager::TILE_SIZE;
                int yStart = screenRect.top / MapTileManager::TILE_SIZE;
                int yEnd = (screenRect.top + screenRect.height) / MapTileManager::TILE_SIZE;
                short mouseBaseId = -1;
                sf::Vector2i mouseWorldGrid = GetGridPosition(mouseWorldPosition);
                if(mouseWorldGrid.x >= 0 && mouseWorldGrid.y >= 0 && mouseWorldGrid.x < MapTileManager::MAX_MAPTILE && mouseWorldGrid.y < MapTileManager::MAX_MAPTILE)
                    mouseBaseId = GetGridInfo(mouseWorldGrid).baseId;

                if(baseFocusedId >= 0)
                    mouseBaseId = baseFocusedId;

                for(int y = yStart; y <= yEnd; y++)
                    for(int x = xStart; x <= xEnd; x++)
                        if(x >= 0 && x < MapTileManager::MAX_MAPTILE && y >= 0 && y < MapTileManager::MAX_MAPTILE)
                        {
                            short baseId = GetGridInfo(y, x).baseId;
                            Base* base = GetBase(baseId);
                            if(base != nullptr)
                            {
                                short team_ = base->GetTeam();
                                sf::Color color_ = sf::Color(170, 170, 170);

                                if(team_ == Game::Alpha)
                                    color_ = Game::GetColor("Alpha_Light");
                                else if(team_ == Game::Delta)
                                    color_ = Game::GetColor("Delta_Light");
                                else if(team_ == Game::Vortex)
                                    color_ = Game::GetColor("Vortex_Light");
                                else if(team_ == Game::Omega)
                                    color_ = Game::GetColor("Omega_Light");

                                if(EntityManager::GetCameraFocus() == nullptr && baseId == mouseBaseId)
                                    color_.a = (baseFocusedId >= 0 && baseId == baseFocusedId ? 180 : 120) * baseHighlightTransparency / 60.f;
                                else color_.a = baseHighlightTransparency;

                                if(baseHighlightSelectId == baseId && baseHighlightSelectTime > 0)
                                    color_.a += (180 - (float)color_.a) * baseHighlightSelectTime / 0.5f * baseHighlightTransparency / 60.f;
                                AddRectVerticesToArray(va_baseHighlight, sf::FloatRect(x * MapTileManager::TILE_SIZE, y * MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE), color_);
                            }
                        }
            }

            //Gun HUD
            if(gun_info_time > 0)
            {
                gun_info_time -= deltaTime;
                if(gun_info_time <= 0)
                    selection_gun_info->SetInactive();
            }
            if(ShipUnit::GetActivePlayerUnit() != nullptr && ShipUnit::GetActivePlayerUnit()->GetIdentity() != nullptr &&
               ShipUnit::GetActivePlayerUnit()->GetIdentity()->GetPlayableStatus())
            {
                ShipIdentity* identity = ShipUnit::GetActivePlayerUnit()->GetIdentity();

                if(session_gun_sellection->GetCurrentLayer() != "" || gun_info_time > 0)
                    for(int b = 0; b < Gun::NumOfGuns; b++)
                    {
                        ref_gun_hud_ammo[b] = ConvertIntToString(identity->GetGunAmmo(b) +
                                                                 (identity->GetShipEntity() != nullptr && identity->GetShipEntity()->GetCurrentGun() == b ? identity->GetShipEntity()->GetGunMagazine() : identity->GetGunLastMagazine(b)));
                    }

                if(GetInput(sf::Keyboard::Period, Released))
                {
                    if(isGunHUDOpen == false)
                    {
                        std::stringstream ss;
                        ss << identity;
                        if(session_gun_sellection->GetCurrentLayer() == ss.str())
                            session_gun_sellection->ResetActiveLayers("default-phase-reset");
                        else session_gun_sellection->TransmitTo(ss.str());

                        if(boosterShouldCloseGunSelection && session_topLeft_tab->GetCurrentLayer() == "boosters")
                        {
                            session_topLeft->TransmitTo("focused units");
                            ref_showMoreOptions = "ShowMoreOption_Close";
                            button_showMoreOptions->SetIcon(texture_iconStates, sf::IntRect(180, 60, 60, 60), 0.5f);
                            focusedButton->SetInactive();
                        }

                        selection_gun_info->SetInactive();
                        isGunHUDOpen = true;
                    }
                    else
                    {
                        session_gun_sellection->SetInactive();
                        isGunHUDOpen = false;
                    }
                }

                if(session_gun_sellection->GetCurrentLayer() != "")
                    for(int c = 0; c < Gun::NumOfGuns; c++)
                        if(c != Gun::LaserGun)
                            ref_gun_hud[c] = ref_gun_hud_ammo[c] == "0" ? "Switch_Gun " + ConvertIntToString(c) : "";

                if(gun_info_time > 0)
                {
                    short index = 1;
                    short currentGunIndex = 1;
                    short currentGun = ShipUnit::GetActivePlayerUnit()->GetCurrentGun();
                    const sf::Vertex* tempVertex;
                    for(int c = 0; c < Gun::NumOfGuns; c++)
                        if(identity->GetShipInfo()->unlock_gun[c] <= identity->GetShipInfoLevel())
                        {
                            if(currentGun == c)
                                currentGunIndex = index;

                            for(int d = 0; d < 4; d++)
                            {
                                tempVertex = lastGunInfoVA->GetVertex(4 * 3 * index + d);
                                if(tempVertex != nullptr)
                                {
                                    lastGunInfoVA->EditVertext(4 * 3 * index + d, tempVertex->position, c != Gun::LaserGun && ref_gun_hud_ammo[c] == "0" ? sf::Color(207, 117, 117, 255.f * 0.7f) : rankTransClr, tempVertex->texCoords);
                                    if(index == gun_info_count - 1)
                                    {
                                        tempVertex = lastGunInfoVA->GetVertex(d);
                                        lastGunInfoVA->EditVertext(d, tempVertex->position, c != Gun::LaserGun && ref_gun_hud_ammo[c] == "0" ? sf::Color(207, 117, 117, 255.f * 0.7f) : rankTransClr, tempVertex->texCoords);
                                    }
                                }
                            }
                            index++;
                        }

                    if(selection_gun_info->GetViewCenter().x <= -118)
                    {
                        selection_gun_info->SetViewCenter(selection_gun_info->GetViewCenter() + sf::Vector2f(226 * gun_info_count, 0));
                        selection_gun_info->SetViewPositionOrigin(sf::Vector2f(-118 + 226 * currentGunIndex, 55));
                    }
                    else if(selection_gun_info->GetViewCenter().x >= 108 + 226 * gun_info_count)
                    {
                        selection_gun_info->SetViewCenter(selection_gun_info->GetViewCenter() + sf::Vector2f(-226 * gun_info_count, 0));
                        selection_gun_info->SetViewPositionOrigin(sf::Vector2f(-118 + 226 * currentGunIndex, 55));
                    }
                }

                /*if(selection_gun_info->GetViewCenter().x >= 108.f + 226.f * gun_info_count)
                {
                    selection_gun_info->SetViewCenter(selection_gun_info->GetViewCenter() + sf::Vector2f(-216 * gun_info_count, 0));
                    selection_gun_info->SetViewPositionOrigin(selection_gun_info->GetViewPositionOrigin() + sf::Vector2f(-216 * gun_info_count, 0));
                }
                else if(selection_gun_info->GetViewCenter().x <= 118)
                {
                    selection_gun_info->SetViewCenter(selection_gun_info->GetViewCenter() + sf::Vector2f(216 * gun_info_count, 0));
                    selection_gun_info->SetViewPositionOrigin(selection_gun_info->GetViewPositionOrigin() + sf::Vector2f(216 * gun_info_count, 0));
                }*/
            }
            else
            {
                session_gun_sellection->SetInactive();
                selection_gun_info->SetInactive();
                isGunHUDOpen = false;
            }
        }

        //Handling some inputs
        if(isMouseScrolled() && ShipUnit::GetActivePlayerUnit() != nullptr && ShipUnit::GetActivePlayerUnit()->GetIdentity() != nullptr &&
           GUI::GetMouseScrollDelta() != 0 && (GetMouseScrollDelta() >= 1.f || GetMouseScrollDelta() <= -1.f))
        {
            ShipIdentity* identity = ShipUnit::GetActivePlayerUnit()->GetIdentity();
            short direction = GetMouseScrollDelta() >= 1.f ? 1 : -1;
            short currentGun = ShipUnit::GetActivePlayerUnit()->GetCurrentGun();
            short gunToSwitch = -1;

            short tempIndex = 0;
            for(int b = 1; b < Gun::NumOfGuns; b++)
            {
                tempIndex = currentGun + b * direction;
                if(tempIndex >= Gun::NumOfGuns)
                    tempIndex -= Gun::NumOfGuns;
                else if(tempIndex < 0)
                    tempIndex += Gun::NumOfGuns;

                if(identity->GetShipInfo()->unlock_gun[tempIndex] <= identity->GetShipInfoLevel())
                {
                    gunToSwitch = tempIndex;
                    break;
                }
            }

            if(gunToSwitch >= 0 && gunToSwitch < Gun::NumOfGuns)
            {
                gun_info_time = 1.f;
                identity->SwitchUnitGun(gunToSwitch);
                selection_gun_info->SetViewPositionOrigin(selection_gun_info->GetViewPositionOrigin() + sf::Vector2f(226 * direction, 0));
                selection_gun_info->Reset("default-phase-reset");
                session_gun_sellection->SetInactive();
                isGunHUDOpen = false;
            }
        }

        //Updating
        EntityManager::UpdateAll(deltaTime);
        UpdateNotification(deltaTime);
        DisplayEffect::UpdateDisplayEffects(deltaTime);

        if(time_energyIncrease > 0)
            time_energyIncrease -= deltaTime;
        if(time_energyIncrease <= 0)
        {
            time_energyIncrease = 5.f;
            for(int b = 0; b < NumOfTeams; b++)
            {
                levelTeamEnergy[b] += 10;
                if(levelTeamEnergy[b] > 1000)
                    levelTeamEnergy[b] = 1000;
                else if(b == gamePlayerTeam)
                {
                    displayText_energy->EditPhase(GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 30, gameGuiScreenEdgeAllowance.y + 8), sf::Vector2f(0.7f, 0.7f), 255), GUI::DefaultPhase);
                    displayText_energy->SetPhase(GUI::DefaultPhase, true);
                    displayText_energy->EditPhase(GUI::DisplayObject::DisplayPhase(sf::Vector2f(gameGuiScreenEdgeAllowance.x + 30, gameGuiScreenEdgeAllowance.y + 10), sf::Vector2f(0.5f, 0.5f), 255), GUI::DefaultPhase);
                }
            }
        }
        ref_energy = ConvertIntToString(levelTeamEnergy[gamePlayerTeam]);
        ref_unitsLeft = ConvertIntToString(EntityManager::GetNumOfTeamAliveShipUnits(gamePlayerTeam));

        for(auto itr = focusedShips.begin(); itr != focusedShips.end();)
        {
            if((*itr)->GetAlive() == false)
                itr = focusedShips.erase(itr);
            else itr++;
        }

        for(int a = 0; a < gameBases.size(); a++)
            if(gameBases[a] != nullptr)
                gameBases[a]->Update(deltaTime);
        for(int a = 0; a < gameBases.size(); a++)
            if(gameBases[a] != nullptr)
                gameBases[a]->Refresh();
        Base::UpdateFrontLineBases();

        //Handle messages
        {
            //Remove Later
            if(message != "")
                std::cerr << message << "\n";

            std::stringstream ss_message;
            ss_message << message;
            ss_message >> message;

            if(message == "HideSubShips")
            {
                selection_hide = true;
                if(selection_subShips != nullptr)
                    selection_subShips->SetInactive();
                if(degree_subShips != nullptr)
                    degree_subShips->SetInactive();
            }
            else if(message == "ShowSubShips")
            {
                selection_hide = false;
                if(selection_subShips != nullptr)
                    selection_subShips->Reset();
                if(degree_subShips != nullptr)
                    degree_subShips->Reset();
            }
            else if(message == "FocusShipButton_Focus")
            {
                if(focusedShips.size() == 1)
                {
                    Ship* unit = *focusedShips.begin();
                    if(unit != nullptr && unit->GetShipType() == Ship::UnitShip && unit->GetTeam() == gamePlayerTeam)
                    {
                        ShipUnit::SetActivePlayerUnit((ShipUnit*)unit);
                        EntityManager::SetCameraFocus(unit);
                        focusedProfile->SetShipIdentity(GetShipIdentity(unit));
                        focusedButton->SetInactive();
                        updateInfo_identity = nullptr;
                        focusedShips.clear();

                        ShipIdentity* tempIdentity = ShipUnit::GetActivePlayerUnit()->GetIdentity();
                        if(tempIdentity != nullptr)
                        {
                            std::stringstream ss;
                            ss << tempIdentity;
                            session_gun_info->TransmitTo(ss.str());
                            session_gun_info->SetInactive();
                            gun_info_count = 0;

                            auto found = gun_info_displayVA.find(ss.str());
                            if(found != gun_info_displayVA.end())
                                lastGunInfoVA = found->second;

                            for(int c = 0; c < Gun::NumOfGuns; c++)
                                if(tempIdentity->GetShipInfo()->unlock_gun[c] <= tempIdentity->GetShipInfoLevel())
                                {
                                    if(ShipUnit::GetActivePlayerUnit()->GetCurrentGun() == c)
                                        selection_gun_info->SetViewCenter(sf::Vector2f(108 + 226 * gun_info_count, 55), true);
                                    gun_info_count++;
                                }
                        }
                    }
                }
            }
            else if(message == "FocusShipButton_Cancel")
            {
                for(Ship*& ship : focusedShips)
                    if(ship->GetShipType() == Ship::UnitShip)
                        ((ShipUnit*)ship)->CancleCurrentAction();
            }
            else if(message == "FocusShipButton_Close")
            {
                focusedButton->SetInactive();
                updateInfo_identity = nullptr;
                focusedShips.clear();
            }
            else if(message == "BaseInfoOptions")
            {
                if(ShipUnit::GetActivePlayerUnit() != nullptr ? baseFocusedId < 0 : true)
                baseInfoButton->SetShowOptionStatus(!baseInfoButton->GetShowOptionStatus());
            }
            else if(message == "ShowMoreOption")
            {
                button_showMoreOptions->SetIcon(texture_iconStates, sf::IntRect(240, 60, 60, 60), 0.5f);
                session_topLeft->TransmitTo("options");

                if(boosterShouldCloseGunSelection && session_topLeft_tab->GetCurrentLayer() == "boosters")
                {
                    session_gun_sellection->SetInactive();
                    isGunHUDOpen = false;
                }
            }
            else if(message == "ShowMoreOption_Close")
            {
                button_showMoreOptions->SetIcon(texture_iconStates, sf::IntRect(180, 60, 60, 60), 0.5f);
                session_topLeft->TransmitTo("focused units");

                if(!(focusedShips.size() > 0 || updateInfo_identity != nullptr))
                    focusedButton->SetInactive();
            }
            else if(message == "BehaviorTab")
                session_topLeft_tab->TransmitTo("behavior");
            else if(message == "BoosterTab")
            {
                session_topLeft_tab->TransmitTo("boosters");

                if(boosterShouldCloseGunSelection)
                {
                    session_gun_sellection->SetInactive();
                    isGunHUDOpen = false;
                }
            }
            else if(message == "Update_Booster_Status")
                updateBoosterStatus = true;
            else if(message == "Switch_Gun")
            {
                isGunHUDOpen = false;
                session_gun_sellection->SetInactive();

                short gunToSwitch;
                ss_message >> gunToSwitch;
                if(ShipUnit::GetActivePlayerUnit() != nullptr && ShipUnit::GetActivePlayerUnit()->GetIdentity() != nullptr)
                {
                    ShipIdentity* tempIdentity = ShipUnit::GetActivePlayerUnit()->GetIdentity();
                    tempIdentity->SwitchUnitGun(gunToSwitch);

                    short tempIndex = 0;
                    for(int c = 0; c < Gun::NumOfGuns; c++)
                        if(tempIdentity->GetShipInfo()->unlock_gun[c] <= tempIdentity->GetShipInfoLevel())
                        {
                            if(ShipUnit::GetActivePlayerUnit()->GetCurrentGun() == c)
                            {
                                selection_gun_info->SetViewCenter(sf::Vector2f(108 + 226 * tempIndex, 55), true);
                                break;
                            }
                            tempIndex++;
                        }
                }
            }

            message = "";
        }

        if(selection_subShips != nullptr)
        {
            float moveLength = 0;
            if(selection_hide)
                moveLength = selection_subShips_width + 10.f;

            sf::Vector2f curPos = selection_subShips->GetPosition();
            TendTowards(curPos.x, selection_position.x + moveLength, 0.1, 0.0001, deltaTime);
            selection_subShips->SetPosition(curPos);
        }

        //Drawing
        gameInstance->window.clear(sf::Color(20, 38, 69));
        gameInstance->window.setView(EntityManager::GetCamera());
        DrawEverything();

        gameInstance->window.setView(gameInstance->window.getDefaultView());
        if(highlighting)
            gameInstance->window.draw(highlightRectangle);
        gameInstance->window.draw(va_highlighedShips);
        va_highlighedShips.clear();

        if(text_commandTime > 0.f)
            gameInstance->window.draw(text_command);

        DrawFpsDisplay(); //Remove later
        guiSession.Draw(gameInstance->window);
        DrawCursorCentered(deltaTime);
        DrawNotification();
        //Remove later
        UpdateFpsDisplay(dtFPS, clock.getElapsedTime().asSeconds());
        dtFPS = 0;

        gameInstance->window.display();

        if(Game::GetInput(sf::Keyboard::Escape) == Released)
            gameInstance->window.close();
    }
}

void Game::UpdateWorldTiles()
{
    va_floor.resize(0);
    va_wall_lower.resize(0);
    va_wall_upper.resize(0);
    va_smallwall_lower.resize(0);
    va_smallwall_upper.resize(0);
    va_spawnpoint.resize(0);

    short int id = 0;
    MapTileManager::TileType type;
    std::string info = "";

    for(int y = 0; y < MapTileManager::MAX_MAPTILE; y++)
        for(int x = 0; x < MapTileManager::MAX_MAPTILE; x++)
        {
            if(GetGridInfo(y, x).tileId != -1)
            {
                id = GetGridInfo(y, x).tileId;
                type = GetGridInfoTile(y, x).type;
                info = MapTileManager::GetTile(GetGridInfo(y, x).tileId).info;

                if(info != "transparent")
                {
                    sf::Vertex vertices[4];
                    sf::Vector2f refPos(x * MapTileManager::TILE_SIZE, y * MapTileManager::TILE_SIZE);

                    sf::IntRect textrRect = MapTileManager::GetTileRect(id);
                    sf::Vector2f tileSize;
                    tileSize = MapTileManager::GetTileSize(type);

                    float reset = tileSize.y - MapTileManager::TILE_SIZE - 3;

                    vertices[0].position = sf::Vector2f(0, 0 - reset) + refPos;
                    vertices[1].position = sf::Vector2f(tileSize.x, 0 - reset) + refPos;
                    vertices[2].position = sf::Vector2f(tileSize.x, tileSize.y - reset) + refPos;
                    vertices[3].position = sf::Vector2f(0, tileSize.y - reset) + refPos;

                    vertices[0].texCoords = sf::Vector2f(textrRect.left, textrRect.top);
                    vertices[1].texCoords = sf::Vector2f(textrRect.left + textrRect.width, textrRect.top);
                    vertices[2].texCoords = sf::Vector2f(textrRect.left + textrRect.width, textrRect.top + textrRect.height);
                    vertices[3].texCoords = sf::Vector2f(textrRect.left, textrRect.top + textrRect.height);

                    if(info == "sp_alpha" || info == "sp_delta" || info == "sp_vortex" || info == "sp_omega" || info == "sp_all_teams")
                        for(int a = 0; a < 4; a++)
                            va_spawnpoint.append(vertices[a]);
                    else if(type == MapTileManager::Floor)
                        for(int a = 0; a < 4; a++)
                            va_floor.append(vertices[a]);
                    else if(type == MapTileManager::Wall || type == MapTileManager::SmallWall)
                    {
                        int transparency = 255;

                        if(y - 1 >= 0 && MapTileManager::GetTile(GetGridInfo(y - 1, x).tileId).type == MapTileManager::Floor && GetGridInfo(y - 1, x).tileId > -1)
                            transparency = 100;

                        sf::Vertex transPart[4];
                        for(int a = 0; a < 4; a++)
                        {
                            transPart[a] = vertices[a];
                            transPart[a].color.a = transparency;
                        }

                        vertices[0].texCoords.y += tileSize.y - MapTileManager::TILE_SIZE - 3;
                        vertices[1].texCoords.y += tileSize.y - MapTileManager::TILE_SIZE - 3;
                        vertices[0].position.y += tileSize.y - MapTileManager::TILE_SIZE - 3;
                        vertices[1].position.y += tileSize.y - MapTileManager::TILE_SIZE - 3;

                        transPart[2].texCoords.y -= MapTileManager::TILE_SIZE;
                        transPart[3].texCoords.y -= MapTileManager::TILE_SIZE;
                        transPart[2].position.y -= MapTileManager::TILE_SIZE;
                        transPart[3].position.y -= MapTileManager::TILE_SIZE;

                        if(type == MapTileManager::Wall)
                        {
                            for(int a = 0; a < 4; a++)
                                va_wall_lower.append(vertices[a]);
                            for(int a = 0; a < 4; a++)
                                va_wall_upper.append(transPart[a]);
                        }
                        else if(type == MapTileManager::SmallWall)
                        {
                            for(int a = 0; a < 4; a++)
                                va_smallwall_lower.append(vertices[a]);
                            for(int a = 0; a < 4; a++)
                                va_smallwall_upper.append(transPart[a]);
                        }
                    }
                }
            }
        }
}

void Game::UpdateWorldTiles(sf::IntRect range)
{
    va_floor.clear();
    va_wall_lower.clear();
    va_wall_upper.clear();
    va_smallwall_lower.clear();
    va_smallwall_upper.clear();
    va_spawnpoint.clear();

    short int id = 0;
    MapTileManager::TileType type;
    std::string info = "";

    for(int y = range.top; y <= range.top + range.height; y++)
        for(int x = range.left; x <= range.left + range.width; x++)
        {
            if(GetGridInfo(y, x).tileId != -1 &&
               x >= 0 && x < MapTileManager::MAX_MAPTILE &&
               y >= 0 && y < MapTileManager::MAX_MAPTILE)
            {
                id = GetGridInfo(y, x).tileId;
                type = GetGridInfoTile(y, x).type;
                info = MapTileManager::GetTile(GetGridInfo(y, x).tileId).info;

                if(info != "transparent")
                {
                    sf::Vertex vertices[4];
                    sf::Vector2f refPos(x * MapTileManager::TILE_SIZE, y * MapTileManager::TILE_SIZE);

                    sf::IntRect textrRect = MapTileManager::GetTileRect(id);
                    sf::Vector2f tileSize;
                    tileSize = MapTileManager::GetTileSize(type);

                    float reset = tileSize.y - MapTileManager::TILE_SIZE - 3;

                    vertices[0].position = sf::Vector2f(0, 0 - reset) + refPos;
                    vertices[1].position = sf::Vector2f(tileSize.x, 0 - reset) + refPos;
                    vertices[2].position = sf::Vector2f(tileSize.x, tileSize.y - reset) + refPos;
                    vertices[3].position = sf::Vector2f(0, tileSize.y - reset) + refPos;

                    vertices[0].texCoords = sf::Vector2f(textrRect.left, textrRect.top);
                    vertices[1].texCoords = sf::Vector2f(textrRect.left + textrRect.width, textrRect.top);
                    vertices[2].texCoords = sf::Vector2f(textrRect.left + textrRect.width, textrRect.top + textrRect.height);
                    vertices[3].texCoords = sf::Vector2f(textrRect.left, textrRect.top + textrRect.height);

                    if(info == "sp_alpha" || info == "sp_delta" || info == "sp_vortex" || info == "sp_omega" || info == "sp_all_teams")
                        for(int a = 0; a < 4; a++)
                            va_spawnpoint.append(vertices[a]);
                    else if(type == MapTileManager::Floor)
                        for(int a = 0; a < 4; a++)
                            va_floor.append(vertices[a]);
                    else if(type == MapTileManager::Wall || type == MapTileManager::SmallWall)
                    {
                        int transparency = 255;

                        if(y - 1 >= 0 && MapTileManager::GetTile(GetGridInfo(y - 1, x).tileId).type == MapTileManager::Floor && GetGridInfo(y - 1, x).tileId > -1)
                            transparency = 100;
                        if(y - 1 >= 0 && type == MapTileManager::Wall &&
                           MapTileManager::GetTile(GetGridInfo(y - 1, x).tileId).type == MapTileManager::SmallWall && GetGridInfo(y - 1, x).tileId > -1)
                            transparency = 100;

                        sf::Vertex transPart[4];
                        for(int a = 0; a < 4; a++)
                        {
                            transPart[a] = vertices[a];
                            transPart[a].color.a = transparency;
                        }

                        vertices[0].texCoords.y += tileSize.y - MapTileManager::TILE_SIZE - 3;
                        vertices[1].texCoords.y += tileSize.y - MapTileManager::TILE_SIZE - 3;
                        vertices[0].position.y += tileSize.y - MapTileManager::TILE_SIZE - 3;
                        vertices[1].position.y += tileSize.y - MapTileManager::TILE_SIZE - 3;

                        transPart[2].texCoords.y -= MapTileManager::TILE_SIZE;
                        transPart[3].texCoords.y -= MapTileManager::TILE_SIZE;
                        transPart[2].position.y -= MapTileManager::TILE_SIZE;
                        transPart[3].position.y -= MapTileManager::TILE_SIZE;

                        if(type == MapTileManager::Wall)
                        {
                            for(int a = 0; a < 4; a++)
                                va_wall_lower.append(vertices[a]);
                            for(int a = 0; a < 4; a++)
                                va_wall_upper.append(transPart[a]);
                        }
                        else if(type == MapTileManager::SmallWall)
                        {
                            for(int a = 0; a < 4; a++)
                                va_smallwall_lower.append(vertices[a]);
                            for(int a = 0; a < 4; a++)
                                va_smallwall_upper.append(transPart[a]);
                        }
                    }
                }
            }
        }
}

void Game::DrawFloors()
{
    gameInstance->window.draw(va_floor, tx_floor);
}

void Game::DrawWalls_Lower()
{
    gameInstance->window.draw(va_wall_lower, tx_wall);
}

void Game::DrawWalls_Upper()
{
    gameInstance->window.draw(va_wall_upper, tx_wall);
}

void Game::DrawSmallWalls_Lower()
{
    gameInstance->window.draw(va_smallwall_lower, tx_smallWall);
}

void Game::DrawSmallWalls_Upper()
{
    gameInstance->window.draw(va_smallwall_upper, tx_smallWall);
}

void Game::DrawSpawnPoints()
{
    gameInstance->window.draw(va_spawnpoint, tx_spawnpoint);
}

void Game::DrawEverything()
{
    DrawFloors();
    DrawSpawnPoints();

    gameInstance->window.draw(va_baseHighlight);
    va_baseHighlight.clear();

    sf::FloatRect screenRect = EntityManager::GetCameraRect();
    for(int a = 0; a < gameBases.size(); a++)
        if(gameBases[a] != nullptr)
        {
            gameBases[a]->DrawNotifyHighlight(gameInstance->window, screenRect);
            gameBases[a]->DrawOutline(gameInstance->window);
        }

    DE_SpecialRadiation::Draw(gameInstance->window);
    ShipUnit::Draw(gameInstance->window);
    BaseCore::DrawLower(gameInstance->window);
    DrawSmallWalls_Lower();
    DrawWalls_Lower();
    BaseCore::DrawUpper(gameInstance->window);
    DrawSmallWalls_Upper();
    Defence::Draw(gameInstance->window);
    ShipDefence::Draw(gameInstance->window);
    Bullet::Draw(gameInstance->window);
    DE_BulletSpeed::Draw(gameInstance->window);
    DE_PulseSpreaderBullet::Draw(gameInstance->window);
    DrawWalls_Upper();
    DE_Hit::Draw(gameInstance->window);
    DE_Shoot::Draw(gameInstance->window);
    DE_DamageDisplay::Draw(gameInstance->window);

    //HUD
    Ship::DrawHealthBars(gameInstance->window);

    BaseCore::ResetBaseCoresToDraw();
}

short Game::CreateBase(short _team, sf::Vector2i _spGrid, sf::IntRect _range)
{
    for(int y = _range.top; y < _range.top + _range.height; y++)
        for(int x = _range.left; x < _range.left + _range.width; x++)
            if(IsControlUnitSpawnPointHere(sf::Vector2i(x, y)))
                return -1;

    if(_spGrid.x >= 0 && _spGrid.x < MapTileManager::MAX_MAPTILE &&
       _spGrid.y >= 0 && _spGrid.y < MapTileManager::MAX_MAPTILE)
    {
        gameBases.push_back(new Base(gameBases.size(), _team, _spGrid, _range));
        return gameBases.size() - 1;
    }
    return -1;
}

Base* Game::GetBase(int id)
{
    if(id >= 0 && id < gameBases.size())
        return gameBases[id];
    return nullptr;
}

Base* Game::GetMainBase(int team)
{
    if(EntityManager::GetBaseCore(team) != nullptr)
        return GetBase(EntityManager::GetBaseCore(team)->GetBaseId());
    return nullptr;
}

void Game::DeletBase(int id)
{
    if(id >= 0 && id < gameBases.size())
    {
        if(gameBases[id]->GetSpawnPoint() != nullptr)
        {
            sf::Vector2i grid = gameBases[id]->GetSpawnPoint()->grid;
            gamegridInfo[grid.y][grid.x].tileId = -1;
        }
        //EntityManager::DeleteBaseCore(gameBases[id]->GetTeam());

        delete gameBases[id];
        gameBases.erase(gameBases.begin() + id);

        for(int a = id; a < gameBases.size(); a++)
            gameBases[a]->SetId(a);

        for(int y = 0; y < MapTileManager::MAX_MAPTILE; y++)
            for(int x = 0; x < MapTileManager::MAX_MAPTILE; x++)
            {
                if(GetGridInfo(y, x).baseId == id)
                    GetGridInfo(y, x).baseId = -1;
                else if(GetGridInfo(y, x).baseId > id)
                    GetGridInfo(y, x).baseId--;
            }
    }
}

void Game::DeletAllBases()
{
    for(int a = 0; a < gameBases.size(); a++)
    {
        if(gameBases[a]->GetSpawnPoint() != nullptr)
        {
            sf::Vector2i grid = gameBases[a]->GetSpawnPoint()->grid;
            gamegridInfo[grid.y][grid.x].tileId = -1;
        }
        delete gameBases[a];
    }
    gameBases.clear();
}

void Game::DrawAllBaseHighlight()
{
    for(int a = 0; a < gameBases.size(); a++)
        gameBases[a]->DrawHighlight(gameInstance->window);
}

void Game::DrawAllBaseOutline()
{
    for(int a = 0; a < gameBases.size(); a++)
        gameBases[a]->DrawOutline(gameInstance->window);
}

int Game::GetNumberOfBases()
{
    return gameBases.size();
}

bool Game::AddSpawnPointToControlUnit(sf::Vector2i spGrid_, short team_)
{
    if(team_ >= 0 && team_ < 4)
    {
        for(int a = 0; a < 4; a++)
            if(teamcontrolunits[a].GetSpawnPoint(spGrid_) != nullptr)
                return false;

        teamcontrolunits[team_].AddSpawnPoint(spGrid_);
        return true;
    }

    return false;
}

void Game::RemoveSpawnPointFromControlUnit(sf::Vector2i spGrid_)
{
    for(int a = 0; a < 4; a++)
        teamcontrolunits[a].RemoveSpawnPoint(spGrid_);
}

bool Game::IsControlUnitSpawnPointHere(sf::Vector2i grid_)
{
    for(int a = 0; a < 4; a++)
        if(teamcontrolunits[a].GetSpawnPoint(grid_) != nullptr)
            return true;
    return false;
}

bool Game::IsBaseSpawnPointHere(sf::Vector2i grid_)
{
    if(GetGridInfo(grid_.y, grid_.x).baseId >= 0 && GetBase(GetGridInfo(grid_.y, grid_.x).baseId) != nullptr)
        if(GetBase(GetGridInfo(grid_.y, grid_.x).baseId)->GetSpawnPoint()->grid == grid_)
            return true;
    return false;
}

bool Game::IsAnyBaseCoreHere(sf::Vector2i grid_)
{
    for(int a = 0; a < 4; a++)
        if(EntityManager::GetBaseCore(a) != nullptr)
        {
            sf::Vector2i diff = EntityManager::GetBaseCore(a)->GetGrid() - grid_;
            if(diff.x < 0) diff.x = -diff.x;
            if(diff.y < 0) diff.y = -diff.y;

            if(diff.x < 2 && diff.y < 2)
                return true;
        }

    return false;
}

void Game::ResetAllControlUnits()
{
    for(int a = 0; a < 4; a++)
        teamcontrolunits[a].Reset();
}

void Game::InitializeLevel()
{
    AStar::SetSearchRange(MapTileManager::MAX_MAPTILE, MapTileManager::MAX_MAPTILE);

    for(int a = 0; a < NumOfTeams; a++)
        if(EntityManager::GetBaseCore(a) != nullptr)
        {
            EntityManager::GetBaseCore(a)->InitializeForLevel();

            sf::Vector2i refGrid = EntityManager::GetBaseCore(a)->GetGrid();
            refGrid -= sf::Vector2i(1, 1);
            for(int y = 0; y < 3; y++)
                for(int x = 0; x < 3; x++)
                    gamegridInfo[refGrid.y + y][refGrid.x + x].tileId = MapTileManager::GetTransparentSmallWallId();
        }
    for(int y = -1; y < 2; y++)
        for(int x = -1; x < 2; x++)
            for(int a = 0; a < Game::NumOfTeams; a++)
                if(EntityManager::GetBaseCore(a) != nullptr)
                {
                    sf::Vector2i grid = EntityManager::GetBaseCore(a)->GetGrid();
                    Base::SpawnPointObject* spawnPoint = GetBase(GetGridInfo(grid.y, grid.x).baseId)->GetSpawnPoint();
                    for(int b = 0; b < spawnPoint->spawnGrids.size(); b++)
                        if(spawnPoint->spawnGrids[b] == grid + sf::Vector2i(x, y))
                        {
                            spawnPoint->spawnGrids.erase(spawnPoint->spawnGrids.begin() + b);
                            b--;
                        }
                }

    UpdateWorldTiles();
    Base::InitializeNetworkInfo();
    Base::InitializeBaseAreas();
    Base::UpdateFrontLineBases(true);

    if(levelSize.x <= 0 || levelSize.y <= 0)
        levelSize = sf::Vector2i(MapTileManager::MAX_MAPTILE, MapTileManager::MAX_MAPTILE);
    SetMapIconScale(1.f);
    UpdateMapLevelTexture(true);
    UpdateMapTexture();

    for(auto& base : gameBases)
        base->RefreshMainBaseStatus();

    for(int a = 0; a < GetShipIdentitySize(); a++)
        GetShipIdentity(a)->CreateShip(true);

    for(int a = 0; a < GetFormUnitSize(); a++)
        GetFormUnit(a)->Create();

    for(int a = 0; a < NumOfTeams; a++)
    {
        for(int b = 0; b < NumOfGameBoosters; b++)
            gameBoostersStatus[a][b] = false;

        levelTeamEnergy[a] = 700;
    }

    EntityManager::RefreshEnemyBaseCores();
    EntityManager::InitialiseDefenceGrids();
    EntityManager::CreateAllDefenceShips();
    EntityManager::SetCameraFocus(ShipUnit::GetActivePlayerUnit());
}

void Game::SetGameState(GameState state_)
{
    gameState = state_;
}

Game::GameState Game::GetGameState()
{
    return gameState;
}

void Game::SetTeamUpType(short team_, short type_)
{
    if(team_ >= 0 && team_ < NumOfTeams)
        teamUpType[team_] = type_;
}

short Game::GetTeamUpType(const short& team_)
{
    if(team_ >= 0 && team_ < NumOfTeams)
        return teamUpType[team_];

    return -1;
}

void Game::SetPlayerTeam(const short team_)
{
    gamePlayerTeam = team_;
}

short Game::GetPlayerTeam()
{
    return gamePlayerTeam;
}

const Game::DifficultyInfo& Game::GetDifficultyInfo(int difficultyLevel)
{
    return gameDifficultyInfo[difficultyLevel];
}

const Game::DifficultyInfo& Game::GetDifficultyInfo()
{
    return gameDifficultyInfo[currentDifficulty];
}

void Game::UpdateMapLevelTexture(bool updateBaseTexture)
{
    //float scale = 2 * (float)MapTileManager::MAX_MAPTILE / (levelSize.x > levelSize.y ? levelSize.x : levelSize.y);
    float scale = MapTileManager::MAX_MAPTILE / (levelSize.x > levelSize.y ? levelSize.x : levelSize.y);
    sf::Vector2i sizeCheck(levelSize);
    if(gameInstance->gameState == MapEdit)
        sizeCheck = sf::Vector2i(MapTileManager::MAX_MAPTILE, MapTileManager::MAX_MAPTILE);

    if((sf::Vector2i)mapLevelRenderTexture.getSize() != sizeCheck * 4 * (int)scale)
    {
        mapLevelRenderTexture.create(sizeCheck.x * 4 * scale, sizeCheck.y * 4 * scale);
        mapLevelRenderTexture.setSmooth(true);
    }

    sf::VertexArray va;
    va.setPrimitiveType(sf::Quads);

    //Base Texture
    if(updateBaseTexture)
    {
        if((sf::Vector2i)mapBaseRenderTexture.getSize() != sizeCheck * 4 * (int)scale)
        {
            mapBaseRenderTexture.create(sizeCheck.x * 4 * scale, sizeCheck.y * 4 * scale);
            mapBaseRenderTexture.setSmooth(true);
        }

        for(int y = 0; y < sizeCheck.y && y < MapTileManager::MAX_MAPTILE; y++)
            for(int x = 0; x < sizeCheck.x && x < MapTileManager::MAX_MAPTILE; x++)
            {
                //Base
                short baseId = GetGridInfo(y, x).baseId;
                if(baseId >= 0)
                {
                    short baseTeam = GetBase(baseId)->GetTeam();

                    sf::Color clr(0, 0, 0, 255 * 0.2f);
                    if(baseTeam == Alpha)
                        clr = sf::Color(220, 152, 152, 255 * 0.5f);
                    else if(baseTeam == Delta)
                        clr = sf::Color(195, 203, 139, 255 * 0.5f);
                    else if(baseTeam == Vortex)
                        clr = sf::Color(255, 223, 119, 255 * 0.5f);
                    else if(baseTeam == Omega)
                        clr = sf::Color(146, 171, 174, 255 * 0.5f);

                    AddRectVerticesToArray(va, sf::FloatRect(x * 4 * scale, y * 4 * scale, 4 * scale, 4 * scale), clr);
                    if(x - 1 >= 0 && x - 1 < MapTileManager::MAX_MAPTILE && GetGridInfo(y, x - 1).baseId > baseId)
                        AddRectVerticesToArray(va, sf::FloatRect(x * 4 * scale, y * 4 * scale, 2 * scale, 4 * scale), sf::Color::Transparent);
                    if(x + 1 >= 0 && x + 1 < MapTileManager::MAX_MAPTILE && GetGridInfo(y, x + 1).baseId > baseId)
                        AddRectVerticesToArray(va, sf::FloatRect((x * 4 + 2) * scale, y * 4 * scale, 2 * scale, 4 * scale), sf::Color::Transparent);
                    if(y - 1 >= 0 && y - 1 < MapTileManager::MAX_MAPTILE && GetGridInfo(y - 1, x).baseId > baseId)
                        AddRectVerticesToArray(va, sf::FloatRect(x * 4 * scale, y * 4 * scale, 4 * scale, 2 * scale), sf::Color::Transparent);
                    if(y + 1 >= 0 && y + 1 < MapTileManager::MAX_MAPTILE && GetGridInfo(y + 1, x).baseId > baseId)
                        AddRectVerticesToArray(va, sf::FloatRect(x * 4 * scale, (y * 4 + 2) * scale, 4 * scale, 2 * scale), sf::Color::Transparent);
                }
            }
        mapBaseRenderTexture.clear(sf::Color::Transparent);
        mapBaseRenderTexture.draw(va, sf::BlendMode(sf::BlendMode::One, sf::BlendMode::Zero));
        mapBaseRenderTexture.display();
        va.clear();
    }

    //Level Texture
    for(int y = 0; y < sizeCheck.y && y < MapTileManager::MAX_MAPTILE; y++)
        for(int x = 0; x < sizeCheck.x && x < MapTileManager::MAX_MAPTILE; x++)
        {
            //Walls
            MapTileManager::TileType tileType = GetGridInfoTile(y, x).type;
            if(tileType == MapTileManager::Wall)
                AddRectVerticesToArray(va, sf::FloatRect(x * 4 * scale, y * 4 * scale, 4 * scale, 4 * scale));
            else if(tileType == MapTileManager::SmallWall && GetGridInfoTile(y, x).info != "transparent")
            {
                //Conner Variables
                bool leftIsFree = false;
                bool leftIsSmallWall = false;
                if(x - 1 >= 0)
                {
                    leftIsFree = GetGridInfoTile(y, x - 1).type <= MapTileManager::Floor;
                    leftIsSmallWall = (GetGridInfoTile(y, x - 1).type == MapTileManager::SmallWall && GetGridInfoTile(y, x - 1).info != "transparent");
                }

                bool rightIsFree = false;
                bool rightIsSmallWall = false;
                if(x + 1 < MapTileManager::MAX_MAPTILE)
                {
                    rightIsFree = GetGridInfoTile(y, x + 1).type <= MapTileManager::Floor;
                    rightIsSmallWall = (GetGridInfoTile(y, x + 1).type == MapTileManager::SmallWall && GetGridInfoTile(y, x + 1).info != "transparent");
                }

                bool topIsFree = false;
                bool topIsSmallWall = false;
                if(y - 1 >= 0)
                {
                    topIsFree = GetGridInfoTile(y - 1, x).type <= MapTileManager::Floor;
                    topIsSmallWall = (GetGridInfoTile(y - 1, x).type == MapTileManager::SmallWall && GetGridInfoTile(y - 1, x).info != "transparent");
                }

                bool bottomIsFree = false;
                bool bottomIsSmallWall = false;
                if(y + 1 < MapTileManager::MAX_MAPTILE)
                {
                    bottomIsFree = GetGridInfoTile(y + 1, x).type <= MapTileManager::Floor;
                    bottomIsSmallWall = (GetGridInfoTile(y + 1, x).type == MapTileManager::SmallWall && GetGridInfoTile(y + 1, x).info != "transparent");
                }

                //Map Blocks
                //__Top-Left
                if((topIsFree || leftIsFree) ||
                   ((topIsSmallWall || leftIsSmallWall) ? (x - 1 >= 0 && y - 1 >= 0 ? GetGridInfoTile(y - 1, x - 1).type <= MapTileManager::Floor : false) : false))
                    AddRectVerticesToArray(va, sf::FloatRect((x * 4) * scale, (y * 4) * scale, 2 * scale, 2 * scale), sf::Color(220, 220, 220, 255));

                //__Top-Right
                if((topIsFree || rightIsFree) ||
                   ((topIsSmallWall || rightIsSmallWall) ? (x + 1 < MapTileManager::MAX_MAPTILE && y - 1 >= 0 ? GetGridInfoTile(y - 1, x + 1).type <= MapTileManager::Floor : false) : false))
                    AddRectVerticesToArray(va, sf::FloatRect((x * 4 + 2) * scale, (y * 4) * scale, 2 * scale, 2 * scale), sf::Color(220, 220, 220, 255));

                //__Bottom-Left
                if((bottomIsFree || leftIsFree) ||
                   ((bottomIsSmallWall || leftIsSmallWall) ? (x - 1 >= 0 && y + 1 < MapTileManager::MAX_MAPTILE ? GetGridInfoTile(y + 1, x - 1).type <= MapTileManager::Floor : false) : false))
                    AddRectVerticesToArray(va, sf::FloatRect((x * 4) * scale, (y * 4 + 2) * scale, 2 * scale, 2 * scale), sf::Color(220, 220, 220, 255));

                //__Bottom-Right
                if((bottomIsFree || rightIsFree) ||
                   ((bottomIsSmallWall || rightIsSmallWall) ? (x + 1 < MapTileManager::MAX_MAPTILE && y + 1 < MapTileManager::MAX_MAPTILE ? GetGridInfoTile(y + 1, x + 1).type <= MapTileManager::Floor : false) : false))
                    AddRectVerticesToArray(va, sf::FloatRect((x * 4 + 2) * scale, (y * 4 + 2) * scale, 2 * scale, 2 * scale), sf::Color(220, 220, 220, 255));
            }
        }
    mapLevelRenderTexture.clear(sf::Color::Transparent);
    mapLevelRenderTexture.draw(sf::Sprite(mapBaseRenderTexture.getTexture()), sf::BlendMode(sf::BlendMode::One, sf::BlendMode::Zero));
    mapLevelRenderTexture.draw(va, sf::BlendMode(sf::BlendMode::One, sf::BlendMode::Zero));
    mapLevelRenderTexture.display();

    //mapLevelRenderTexture.getTexture().copyToImage().saveToFile("tmp_map.png");
}

void Game::UpdateMapTexture(float dt)
{
    //float scale = mapScale * (float)MapTileManager::MAX_MAPTILE / (levelSize.x > levelSize.y ? levelSize.x : levelSize.y);
    float scale = MapTileManager::MAX_MAPTILE / (levelSize.x > levelSize.y ? levelSize.x : levelSize.y);
    sf::Vector2i sizeCheck(levelSize.x * 4 * scale + 60, levelSize.y * 4 * scale + 60);

    if((sf::Vector2i)mapRenderTexture.getSize() != sizeCheck)
    {
        mapRenderTexture.create(sizeCheck.x, sizeCheck.y);
        mapRenderTexture.setSmooth(true);
    }

    mapRenderTime -= dt;
    if(mapRenderTime <= 0.f)
    {
        mapRenderTime = 1.f / 30.f;//0.04f;
        for(int a = 0; a < NumOfTeams; a++)
        {
            map_va_smallUnits[a].clear();
            map_va_defenceColor[a].clear();
            map_va_defenceTrans[a].clear();
        }
        map_va_higherUnits.clear();
        map_va_basecore.clear();

        sf::Color clr[] = {sf::Color(220, 152, 152), sf::Color(195, 203, 139), sf::Color(255, 223, 119), sf::Color(146, 171, 174)};
        const std::list<Ship*>& ships = EntityManager::GetShipObjects();

        Ship* temp_ship = nullptr;
        ShipUnit* temp_unit = nullptr;
        ShipUnit* temp_unit_player = nullptr;
        ShipDefence* temp_defence = nullptr;
        std::vector<Ship*> temp_higherUnits;
        short temp_team = 0;
        sf::Vector2f temp_position;
        double time = GetGameExecutionTimeInSec();

        for(auto itr = ships.begin(); itr != ships.end(); itr++)
        {
            temp_ship = *itr;
            temp_position = temp_ship->GetPosition() / (float)MapTileManager::TILE_SIZE * scale * 4.f + (30.f * sf::Vector2f(1, 1));
            temp_team = temp_ship->GetTeam();

            if(temp_ship->GetShipType() == Ship::UnitShip)
            {
                temp_unit = (ShipUnit*)temp_ship;
                if(temp_unit->GetUnitType() == ShipUnit::MicroShip || temp_unit->GetUnitType() == ShipUnit::MiniShip)
                    AddRectVerticesToArray(map_va_smallUnits[temp_team], sf::FloatRect(temp_position.x - 4 * mapIconScale, temp_position.y - 4 * mapIconScale, 8 * mapIconScale, 8 * mapIconScale), clr[temp_team]);
                else if(temp_unit == ShipUnit::GetActivePlayerUnit())
                    temp_unit_player = temp_unit;
                else temp_higherUnits.push_back(temp_ship);
            }
            else if(temp_ship->GetShipType() == Ship::DefenceShip)
            {
                temp_defence = (ShipDefence*)temp_ship;
                temp_position += sf::Vector2f(0, 10 / (float)MapTileManager::TILE_SIZE * scale * 4.f);
                temp_position.x = (int)temp_position.x / 2 * 2;
                temp_position.y = (int)temp_position.y / 2 * 2;

                AddRectVerticesToArray(map_va_defenceColor[temp_team], sf::FloatRect(temp_position.x - 4 * mapIconScale, temp_position.y - 4 * mapIconScale, 8 * mapIconScale, 8 * mapIconScale), clr[temp_team]);
                AddRectVerticesToArray(map_va_defenceTrans[temp_team], sf::FloatRect(temp_position.x - 6 * mapIconScale, temp_position.y - 6 * mapIconScale, 12 * mapIconScale, 12 * mapIconScale), sf::Color::Transparent);
            }
        }

        if(temp_higherUnits.size() > 0)
        {
            std::vector<Ship*> temp_unitList;
            short firstIndex = ((int)time % (temp_higherUnits.size() * 2)) / 2;
            for(int a = 0; a < temp_higherUnits.size(); a++)
                temp_unitList.push_back(temp_higherUnits[(firstIndex + a) % temp_higherUnits.size()]);

            bool reverseList = (int)time % 2;
            sf::Vertex vertices[16];
            for(int a = reverseList ? temp_higherUnits.size() - 1 : 0; reverseList ? a >= 0 : a < temp_higherUnits.size(); reverseList ? a-- : a++)
            {
                temp_position = temp_higherUnits[a]->GetPosition() / (float)MapTileManager::TILE_SIZE * scale * 4.f + (30.f * sf::Vector2f(1, 1));
                temp_team = temp_higherUnits[a]->GetTeam();

                for(int b = 0; b < 16; b++)
                    vertices[b] = mapIconVertices_higherUnits[b];
                vertices[12].color = vertices[13].color = vertices[14].color = vertices[15].color = clr[temp_team];

                sf::Transformable transformable;
                transformable.setPosition(temp_position);
                transformable.setRotation(temp_higherUnits[a]->GetAngle());
                transformable.scale(0.4f * mapIconScale, 0.4f * mapIconScale);
                sf::Transform transformPoint = transformable.getTransform();
                for(int b = 0; b < 16; b++)
                {
                    vertices[b].position = transformPoint.transformPoint(vertices[b].position);
                    map_va_higherUnits.append(vertices[b]);
                }
            }
        }

        if(temp_unit_player != nullptr)
        {
                temp_position = temp_unit_player->GetPosition() / (float)MapTileManager::TILE_SIZE * scale * 4.f + (30.f * sf::Vector2f(1, 1));
                sf::Vertex vertices[16];

                for(int b = 0; b < 16; b++)
                    vertices[b] = mapIconVertices_higherUnits[b];
                vertices[12].color = vertices[13].color = vertices[14].color = vertices[15].color = sf::Color::White;

                sf::Transformable transformable;
                transformable.setPosition(temp_position);
                transformable.setRotation(temp_unit_player->GetAngle());
                transformable.scale(0.45f * mapIconScale, 0.45f * mapIconScale);
                sf::Transform transformPoint = transformable.getTransform();
                for(int b = 0; b < 16; b++)
                {
                    vertices[b].position = transformPoint.transformPoint(vertices[b].position);
                    map_va_higherUnits.append(vertices[b]);
                }
        }

        {
            sf::Vertex vertices[12];

            short firstTeam = ((int)time % (NumOfTeams * 2)) / 2;
            bool reversed = (int)time % 2;

            for(int a = 0; a < NumOfTeams; a++)
            {
                temp_team = (firstTeam + NumOfTeams + (reversed ? -a : a)) % NumOfTeams;
                BaseCore* baseCore = EntityManager::GetBaseCore(temp_team);

                if(baseCore != nullptr && baseCore->GetAlive())
                {
                    temp_position = baseCore->GetPosition() / (float)MapTileManager::TILE_SIZE * scale * 4.f + (30.f * sf::Vector2f(1, 1));

                    sf::Transformable transformable;
                    transformable.setScale(1.1f * mapIconScale, 1.1f * mapIconScale);
                    transformable.setPosition(temp_position);
                    transformable.setRotation(45);

                    for(int b = 0; b < 12; b++)
                    {
                        vertices[b].color = b >= 8 ? clr[temp_team] : mapIconVertices_basecore[b].color;
                        vertices[b].position = transformable.getTransform().transformPoint(mapIconVertices_basecore[b].position);
                        map_va_basecore.append(vertices[b]);
                    }
                }
            }
        }

        mapRenderTexture.clear(sf::Color::Transparent);

        sf::Sprite sprite(mapLevelRenderTexture.getTexture());
        sprite.move(30.f * sf::Vector2f(1, 1));
        mapRenderTexture.draw(sprite, sf::BlendMode(sf::BlendMode::One, sf::BlendMode::Zero));

        {
            short firstTeam = ((int)time % (NumOfTeams * 2)) / 2;
            bool reversed = (int)time % 2;

            for(int a = 0; a < NumOfTeams; a++)
            {
                mapRenderTexture.draw(map_va_smallUnits[(firstTeam + NumOfTeams + (reversed ? -a : a)) % NumOfTeams]);
                mapRenderTexture.draw(map_va_defenceTrans[(firstTeam + NumOfTeams + (reversed ? -a : a)) % NumOfTeams], sf::BlendMode(sf::BlendMode::One, sf::BlendMode::Zero));
                mapRenderTexture.draw(map_va_defenceColor[(firstTeam + NumOfTeams + (reversed ? -a : a)) % NumOfTeams]);
            }
        }

        if((int)time % 2)
        {
            mapRenderTexture.draw(map_va_basecore, sf::BlendMode(sf::BlendMode::One, sf::BlendMode::Zero));
            mapRenderTexture.draw(map_va_higherUnits, sf::BlendMode(sf::BlendMode::One, sf::BlendMode::Zero));
        }
        else
        {
            mapRenderTexture.draw(map_va_higherUnits, sf::BlendMode(sf::BlendMode::One, sf::BlendMode::Zero));
            mapRenderTexture.draw(map_va_basecore, sf::BlendMode(sf::BlendMode::One, sf::BlendMode::Zero));
        }
        mapRenderTexture.display();

        //mapRenderTexture.getTexture().copyToImage().saveToFile("tmp_map.png");
    }
}

void Game::SetMapIconScale(float scale_)
{
    mapIconScale = scale_;
}

const sf::Texture& Game::GetMapTexture()
{
    return mapRenderTexture.getTexture();
}
