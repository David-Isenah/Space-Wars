#include <iostream>
#include <math.h>
#include <sstream>
#include "game.h"
#include "collision.h"
#include "public_functions.h"
#include "maptilemanager.h"

enum EditorMode {NoMode = -1, EditWorldMode, EditBaseMode, EditShipMode};
enum EditorTeamType {NoTeamType = -1, TeamTypeA, TeamTypeB, TeamTypeC, TeamTypeD};
enum EditorPlaceType {NoPlaceType = -1, SpawnPointPType, BaseCorePType, MainDefencePType, MiniDefencePType, ShipPType, BossPType};
enum EditorShipMode {NoShipMode = -1, MainShipMode, SubShipMode, MiniDefenceMode, MainDefenceMode, BossShipMode};

const float moveSpeed = 800.f;
const float scaleSpeed = 2.f;
const float maxScale = 3.f;
const float minScale = 0.5f;

void Game::MapEditior(std::string file)
{
    if(file != "");

    ClearEntireGridInfo();
    UpdateCharacterTexture(GetColor("Default_Light"));
    SetCursorTextrue(&AssetManager::GetTexture("Cursor_MapEditor"));
    SetLevelSize(sf::Vector2i(MapTileManager::MAX_MAPTILE, MapTileManager::MAX_MAPTILE));

    sf::Clock clock;
    sf::Vector2f worldViewPos;
    sf::Vector2i menuRelativePos;
    sf::Vector2i worldRelativePos;
    sf::Vector2f minWorldPos;
    sf::Vector2f highlightStart;
    sf::Vector2f highlightEnd;
    sf::IntRect baseRangeHighlight;
    sf::VertexArray va_grid;
    sf::VertexArray va_highlighted;
    sf::Texture& tileGrid = AssetManager::GetTexture("TileGrid");
    sf::Texture& ts_floor = AssetManager::GetTexture("TileSheet_Floor");
    sf::Texture& ts_smallWall = AssetManager::GetTexture("TileSheet_SmallWall");
    sf::Texture& ts_wall = AssetManager::GetTexture("TileSheet_Wall");
    sf::Texture& ts_spawnpoint = AssetManager::GetTexture("TileSheet_SpawnPoint");
    short int selectedTileId = -1;
    int selectedBaseId = -1;
    bool HideMenu = false;
    bool highlighting = false;
    float hideX = 300;
    float scale = 1;
    float worldViewScale = 1;
    std::string message = "";
    EditorMode mode = NoMode;
    GameMode gameMode = NoGameMode;
    EditorTeamType teamType[4] = {NoTeamType, NoTeamType, NoTeamType, NoTeamType};
    EditorPlaceType placeType = NoPlaceType;
    EditorShipMode shipMode = NoShipMode;
    short playerTeam = -1;
    short toUpdateTeamType = -1;
    short userType = 0;
    short tabTeam = -1;
    short gridType = 0;
    bool bs_update = false;
    bool updateTeamOption = false;
    bool updateTeamSetup = false;
    bool resetMode = false;
    bool toDrawHighlightSprite = false;
    int isThereChange_ts = false;
    ShipIdentity* singleSelect_identity = nullptr;
    FormUnit* singleSelect_formUnit = nullptr;

    //Ship Create Variable
    sf::Vector2i sc_position;
    std::string sc_name;
    short sc_skin = 0;
    float sc_speedFactor = 0;
    float sc_specialFactor = 0;
    int sc_characterIndex = -1;
    sf::Vector2i sc_highlightGrid;
    bool sc_highlighting = false;
    ShipIdentity* sc_shipIdentity = nullptr;
    FormUnit* sc_formUnit = nullptr;
    bool isCreatingShip = false;

    sf::RectangleShape hudModeBase;
    hudModeBase.setSize(sf::Vector2f(184, 36));
    hudModeBase.setFillColor(sf::Color(255, 255, 255, 200));
    hudModeBase.setOrigin(hudModeBase.getSize().x / 2.f, hudModeBase.getSize().y/ 2.f);
    hudModeBase.setPosition(Game::SCREEN_WIDTH - hudModeBase.getSize().x / 2, hudModeBase.getSize().y / 2);

    sf::Text hudModeText;
    hudModeText.setFillColor(Game::GetColor("Default_Light"));
    hudModeText.setFont(*Game::GetFont("Default"));
    hudModeText.setCharacterSize(19);
    hudModeText.setOrigin(hudModeText.getLocalBounds().width / 2, hudModeText.getLocalBounds().height / 2);
    hudModeText.setPosition(Game::SCREEN_WIDTH - hudModeBase.getSize().x / 2, hudModeBase.getSize().y / 2 - 4);

    sf::RectangleShape highlight;
    highlight.setOutlineThickness(-4);
    highlight.setOutlineColor(sf::Color(255, 255, 255, 100));
    highlight.setFillColor(sf::Color(255, 255, 255, 40));

    sf::RectangleShape highlightBase;
    highlightBase.setOutlineThickness(-5);
    highlightBase.setOutlineColor(sf::Color::White);
    highlightBase.setFillColor(sf::Color(255, 255, 255, 50));

    sf::Sprite highlightSprite;
    highlightSprite.setTexture(AssetManager::GetTexture("Editor_Highlight_Position"));

    sf::FloatRect selectedRegion(0, 0, 0, 0);
    va_grid.setPrimitiveType(sf::Quads);
    va_highlighted.setPrimitiveType(sf::Quads);

    sf::View worldView;
    worldView.setSize(Game::SCREEN_WIDTH , Game::SCREEN_HEIGHT);
    worldView.setCenter(worldView.getSize() / 2.f);

    sf::View hudView;
    hudView.setSize(Game::SCREEN_WIDTH , Game::SCREEN_HEIGHT);
    hudView.setCenter(worldView.getSize() / 2.f);

    sf::View menuView;
    menuView.setSize(310, Game::SCREEN_HEIGHT);
    menuView.setCenter(155, Game::SCREEN_HEIGHT / 2.f);
    menuView.setViewport(sf::FloatRect(0, 0, 310.f / SCREEN_WIDTH, 1));

    GUI::ResourcePack rp_editor_default(&AssetManager::GetTexture("Editor_Default"), GetFont("Default"));
    GUI::ResourcePack rp_editor_hider(nullptr, nullptr);
    GUI::ResourcePack rp_editor_exit(&AssetManager::GetTexture("Editor_Exit"), nullptr);
    GUI::ResourcePack rp_editor_save(&AssetManager::GetTexture("Editor_Save"), nullptr);
    GUI::ResourcePack rp_editor_mini(&AssetManager::GetTexture("Editor_Mini"), GetFont("Default"));
    GUI::ResourcePack rp_editor_tilemodes(&AssetManager::GetTexture("Editor_TileModes"), nullptr);
    GUI::ResourcePack rp_editor_arrow(&AssetManager::GetTexture("Editor_Arrow"), nullptr);
    GUI::ResourcePack rp_editor_selection(&AssetManager::GetTexture("Editor_Selection"), nullptr);
    GUI::ResourcePack rp_backArrow(&AssetManager::GetTexture("Editor_Mini"), nullptr);
    GUI::ResourcePack rp_editor_newmenu(&AssetManager::GetTexture("Editor_NewMenu"), GetFont("Default"));
    GUI::ResourcePack rp_editor_selectmode(&AssetManager::GetTexture("Editor_SelectMode"), GetFont("Default"));
    GUI::ResourcePack rp_editor_teamstatus(&AssetManager::GetTexture("Editor_TeamStatus"), nullptr);
    GUI::ResourcePack rp_editor_teamoption(&AssetManager::GetTexture("Editor_TeamOption"), GetFont("Conthrax"));
    GUI::ResourcePack rp_editor_plain(&AssetManager::GetTexture("Editor_Plain"), GetFont("Default"));
    GUI::ResourcePack rp_editor_smallarrow(&AssetManager::GetTexture("Editor_SmallArrow"), nullptr);
    GUI::ResourcePack rp_editor_shiptype(&AssetManager::GetTexture("Editor_ShipType"), GetFont("Default"));

    std::vector<GUI::Selection::ItemInfo> itemInfo_floor;
    std::vector<GUI::Selection::ItemInfo> itemInfo_smallWall;
    std::vector<GUI::Selection::ItemInfo> itemInfo_wall;
    std::vector<GUI::Selection::ItemInfo> itemInfo_sp_all_teams;
    std::vector<GUI::Selection::ItemInfo> itemInfo_sp_alpha;
    std::vector<GUI::Selection::ItemInfo> itemInfo_sp_delta;
    std::vector<GUI::Selection::ItemInfo> itemInfo_sp_vortex;
    std::vector<GUI::Selection::ItemInfo> itemInfo_sp_omega;
    std::vector<GUI::Selection::ItemInfo> itemInfo_subship_skin[NumOfTeams];
    std::vector<GUI::Selection::ItemInfo> itemInfo_mainDefence_skin[NumOfTeams];
    std::vector<GUI::Selection::ItemInfo> itemInfo_miniDefence_skin[NumOfTeams];
    std::vector<GUI::Selection::ItemInfo> itemInfo_character[NumOfTeams];

    for(int a = 0; a < MapTileManager::GetNumberOfTileId(); a++)
    {
        std::stringstream ss;
        ss << "TileId " << a;

        sf::IntRect rect = MapTileManager::GetTileRect(a);
        MapTileManager::TileType type = MapTileManager::GetTile(a).type;
        std::string info = MapTileManager::GetTile(a).info;

        if(info == "transparent")
            ;
        else if(info == "sp_all_teams")
        {
            itemInfo_sp_all_teams.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_spawnpoint, 1, rect, 0.05 * itemInfo_sp_all_teams.size()));
            itemInfo_sp_alpha.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_spawnpoint, 1, rect, 0.05 * itemInfo_sp_alpha.size()));
            itemInfo_sp_delta.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_spawnpoint, 1, rect, 0.05 * itemInfo_sp_delta.size()));
            itemInfo_sp_vortex.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_spawnpoint, 1, rect, 0.05 * itemInfo_sp_vortex.size()));
            itemInfo_sp_omega.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_spawnpoint, 1, rect, 0.05 * itemInfo_sp_omega.size()));
        }
        else if(info == "sp_alpha")
            itemInfo_sp_alpha.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_spawnpoint, 1, rect, 0.05 * itemInfo_sp_alpha.size()));
        else if(info == "sp_delta")
            itemInfo_sp_delta.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_spawnpoint, 1, rect, 0.05 * itemInfo_sp_delta.size()));
        else if(info == "sp_vortex")
            itemInfo_sp_vortex.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_spawnpoint, 1, rect, 0.05 * itemInfo_sp_vortex.size()));
        else if(info == "sp_omega")
            itemInfo_sp_omega.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_spawnpoint, 1, rect, 0.05 * itemInfo_sp_omega.size()));
        else if(type == MapTileManager::Floor)
            itemInfo_floor.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_floor, 1, rect, 0.05 * itemInfo_floor.size()));
        else if(type == MapTileManager::SmallWall)
            itemInfo_smallWall.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_smallWall, 1, rect, 0.05 * itemInfo_smallWall.size()));
        else if(type == MapTileManager::Wall)
            itemInfo_wall.push_back(GUI::Selection::ItemInfo(ss.str(), "TileId None", "", false, &ts_wall, 1, rect, 0.05 * itemInfo_wall.size()));
    }

    for(int a = 0; a < NumOfTeams; a++)
        for(int b = 0; b < GetCharacterTypeInfoSize(ShipInfo::MainCharacterType, a); b++)
            {
                std::stringstream ss;
                ss << "Info " << b;

                itemInfo_character[a].push_back(GUI::Selection::ItemInfo(ss.str(), "", "", false, &GetCharacterInfo(ShipInfo::MainCharacterType, b, a)->texture, 1, sf::IntRect()));
            }

    for(int a = 0; a < NumOfTeams; a++)
        for(int b = 0; b < GetSkinSize(SkinSubShip, a); b++)
        {
            std::stringstream ss;
            ss << "SkinSubShip " << b;

            itemInfo_subship_skin[a].push_back(GUI::Selection::ItemInfo(ss.str(), "", "", false, &GetSkin(SkinSubShip, b, a)->texture, 1, sf::IntRect(0, 0, 180, 180)));
        }

    for(int a = 0; a < NumOfTeams; a++)
        for(int b = 0; b < GetSkinSize(SkinMainDefence, a); b++)
        {
            std::stringstream ss;
            ss << "SkinMainDefence " << b;

            itemInfo_mainDefence_skin[a].push_back(GUI::Selection::ItemInfo(ss.str(), "", "", false, &GetSkin(SkinMainDefence, b, a)->texture, 0.5f, sf::IntRect(0, 0, 360, 360)));
        }

    for(int a = 0; a < NumOfTeams; a++)
        for(int b = 0; b < GetSkinSize(SkinMiniDefence, a); b++)
        {
            std::stringstream ss;
            ss << "SkinMiniDefence " << b;

            itemInfo_miniDefence_skin[a].push_back(GUI::Selection::ItemInfo(ss.str(), "", "", false, &GetSkin(SkinMiniDefence, b, a)->texture, 1, sf::IntRect(0, 0, 180, 180)));
        }

    GUI::Session menuMain;
    GUI::Session* menu = new GUI::Session();
    GUI::Session popUpMenu;

    menuMain.Create("main");
    menuMain.AddObject(new GUI::DisplayShape(sf::Vector2f(300, Game::SCREEN_HEIGHT), sf::Color::White,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(-150, Game::SCREEN_HEIGHT / 2)),
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(150, Game::SCREEN_HEIGHT / 2)),
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(-150, Game::SCREEN_HEIGHT / 2))), "main");
    menuMain.AddObject(new GUI::ShapeButton(GetGuiStyle("Editor_Hider"), sf::Vector2f(10, SCREEN_HEIGHT), sf::Color::White, sf::Color::White, GetColor("Default"), true, sf::Vector2f(305, SCREEN_HEIGHT / 2), rp_editor_hider, "Hide"), "main");    menuMain.AddObject(menu, "main");
    menuMain.AddInFront("main");

    menu->Create("main");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_Default"), sf::Vector2f(150, 125), 1, rp_editor_default, "Tile_Mode", "World", &AssetManager::GetTexture("Icon_EditTile"), 1, sf::IntRect()), "main");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_Default"), sf::Vector2f(150, 360), 1, rp_editor_default, "Setup", "Setup", &AssetManager::GetTexture("Icon_ShipVersus"), 1, sf::IntRect(), 0.1f, 0.1f), "main");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_Exit"), sf::Vector2f(249, Game::SCREEN_HEIGHT - 52), 1, rp_editor_exit, "ToExit", "", nullptr, 1, sf::IntRect(), 0.2f, 0.3f), "main");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_Save"), sf::Vector2f(109, Game::SCREEN_HEIGHT - 52), 1, rp_editor_save, "Save", "", nullptr, 1, sf::IntRect(), 0.3f, 0.2f), "main");

    //Tile Mode
    menu->Create("tile_mode");
    menu->AddObject(new GUI::DisplayShape(sf::Vector2f(60, Game::SCREEN_HEIGHT), Game::GetColor("Default_Lighter"),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(-30, Game::SCREEN_HEIGHT / 2)),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(30, Game::SCREEN_HEIGHT / 2)),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(-30, Game::SCREEN_HEIGHT / 2)), 0.f, 0.3f), "tile_mode");
    menu->AddObject(new GUI::DisplayShape(sf::Vector2f(5, Game::SCREEN_HEIGHT), sf::Color(53, 77, 89),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(-2.5, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 76),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(62.5, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 76),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(-2.5, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 76), 0.f, 0.3f), "tile_mode");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_Mini"), sf::Vector2f(30, Game::SCREEN_HEIGHT - 30), 1, rp_editor_mini, "Main", "", &AssetManager::GetTexture("Editor_Mini"), 1, sf::IntRect(120, 0, 60, 60), 0.f, 0.3f), "tile_mode");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_Mini"), sf::Vector2f(30, Game::SCREEN_HEIGHT - 90), 1, rp_editor_mini, "Help_World", "", &AssetManager::GetTexture("Editor_Mini"), 1, sf::IntRect(60, 0, 60, 60), 0.f, 0.3f), "tile_mode");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_Mini"), sf::Vector2f(30, Game::SCREEN_HEIGHT - 150), 1, rp_editor_mini, "World_Others", "", &AssetManager::GetTexture("Editor_Mini"), 1, sf::IntRect(60, 60, 60, 60), 0.f, 0.3f), "tile_mode");

    GUI::Group* group_tilemodes = new GUI::Group();
    group_tilemodes->AddObject(new GUI::ToggleButton(GetGuiStyle("Editor_TileModes_Floor"), GetGuiStyle("Editor_TileModes_Floor_On"), sf::Vector2f(30, 30), 1, rp_editor_tilemodes, "Floor_Mode", "No_TileType", "", nullptr, 1, sf::IntRect(), 0.f, 0.3f));
    group_tilemodes->AddObject(new GUI::ToggleButton(GetGuiStyle("Editor_TileModes_SmallWall"), GetGuiStyle("Editor_TileModes_SmallWall_On"), sf::Vector2f(30, 90), 1, rp_editor_tilemodes, "SmallWall_Mode", "No_TileType", "", nullptr, 1, sf::IntRect(), 0.f, 0.3f));
    group_tilemodes->AddObject(new GUI::ToggleButton(GetGuiStyle("Editor_TileModes_Wall"), GetGuiStyle("Editor_TileModes_Wall_On"), sf::Vector2f(30, 159), 1, rp_editor_tilemodes, "Wall_Mode", "No_TileType", "", nullptr, 1, sf::IntRect(), 0.f, 0.3f));
    group_tilemodes->AddObject(new GUI::ToggleButton(GetGuiStyle("Editor_TileModes_Empty"), GetGuiStyle("Editor_TileModes_Empty_On"), sf::Vector2f(30, 227), 1, rp_editor_tilemodes, "Empty_Mode", "No_TileType", "", nullptr, 1, sf::IntRect(), 0.f, 0.3f));
    menu->AddObject(group_tilemodes, "tile_mode");

    GUI::Session* session_tilemodes = new GUI::Session();
    session_tilemodes->Create("main");
    session_tilemodes->AddObject(new GUI::DisplayText("Please select a tile type.", GetFont("Default"), GetColor("Default_Light"), 15,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(180, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(180, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(180, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 0)), "main");
    session_tilemodes->Create("floor_list");
    session_tilemodes->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_Arrow_Up"), sf::Vector2f(183, 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Button(GetGuiStyle("Editor_Arrow_Down"), sf::Vector2f(183, Game::SCREEN_HEIGHT - 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Degree(GUI::Vertical, 581, 3, sf::Vector2f(294, Game::SCREEN_HEIGHT / 2), sf::Vector2f(5, 50), GetColor("Default_Lighter"), GetColor("Default"), sf::Color::Transparent, GUI::Degree::ClickNode, 0.5f),
                                                    new GUI::Selection(GetGuiStyle("Editor_Selection"), GetGuiStyle("Editor_Selection_On"), rp_editor_selection, sf::Vector2f(180, Game::SCREEN_HEIGHT / 2), 1, sf::Vector2f(235, Game::SCREEN_HEIGHT - 104 + 8),
                                                                       sf::FloatRect(0, 0, 200, 200), 200, 0.1f, GUI::Vertical, &itemInfo_floor)), "floor_list");
    session_tilemodes->Create("smallWall_list");
    session_tilemodes->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_Arrow_Up"), sf::Vector2f(183, 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Button(GetGuiStyle("Editor_Arrow_Down"), sf::Vector2f(183, Game::SCREEN_HEIGHT - 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Degree(GUI::Vertical, 581, 3, sf::Vector2f(294, Game::SCREEN_HEIGHT / 2), sf::Vector2f(5, 50), GetColor("Default_Lighter"), GetColor("Default"), sf::Color::Transparent, GUI::Degree::ClickNode, 0.5f),
                                                    new GUI::Selection(GetGuiStyle("Editor_Selection"), GetGuiStyle("Editor_Selection_On"), rp_editor_selection, sf::Vector2f(180, Game::SCREEN_HEIGHT / 2), 1, sf::Vector2f(235, Game::SCREEN_HEIGHT - 104),
                                                                       sf::FloatRect(0, 0, 200, 200), 200, 0.1f, GUI::Vertical, &itemInfo_smallWall)), "smallWall_list");
    session_tilemodes->Create("wall_list");
    session_tilemodes->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_Arrow_Up"), sf::Vector2f(183, 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Button(GetGuiStyle("Editor_Arrow_Down"), sf::Vector2f(183, Game::SCREEN_HEIGHT - 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Degree(GUI::Vertical, 581, 3, sf::Vector2f(294, Game::SCREEN_HEIGHT / 2), sf::Vector2f(5, 50), GetColor("Default_Lighter"), GetColor("Default"), sf::Color::Transparent, GUI::Degree::ClickNode, 0.5f),
                                                    new GUI::Selection(GetGuiStyle("Editor_Selection"), GetGuiStyle("Editor_Selection_On"), rp_editor_selection, sf::Vector2f(180, Game::SCREEN_HEIGHT / 2), 1, sf::Vector2f(235, Game::SCREEN_HEIGHT - 104),
                                                                       sf::FloatRect(0, 0, 200, 200), 200, 0.1f, GUI::Vertical, &itemInfo_wall)), "wall_list");
    session_tilemodes->Create("empty_info");
    session_tilemodes->AddObject(new GUI::DisplayText("This is an empty tile type.\nIt deletes or replaces any\nexisting tile on the grid with\nthis empty tile.", GetFont("Default"), GetColor("Default_Light"), 15,
                                                  GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 59), sf::Vector2f(1, 1), 0),
                                                  GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 59), sf::Vector2f(1, 1), 255),
                                                  GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 59), sf::Vector2f(1, 1), 0)), "empty_info");

    menu->AddObject(session_tilemodes, "tile_mode");

    //Setup
    menu->Create("setup");
    GUI::Button* button_setupBase = new GUI::Button(GetGuiStyle("Editor_Default"), sf::Vector2f(150, 125), 1, rp_editor_default, "Setup_Base", "Setup Base", &AssetManager::GetTexture("Icon_SetupBase"), 1, sf::IntRect());
    GUI::Button* button_setupShip = new GUI::Button(GetGuiStyle("Editor_Default"), sf::Vector2f(150, 360), 1, rp_editor_default, "Setup_Ship", "Setup Ship", &AssetManager::GetTexture("Icon_SetupShip"), 1, sf::IntRect(), 0.1f, 0.1f);
    button_setupBase->SetLock(true);
    button_setupShip->SetLock(true);
    menu->AddObject(button_setupBase, "setup");
    menu->AddObject(button_setupShip, "setup");

    menu->AddObject(new GUI::Button(GetGuiStyle("BackArrow"), sf::Vector2f(20, Game::SCREEN_HEIGHT - 20), 0.5f, rp_backArrow, "Main", ""), "setup");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_NewMenu"), sf::Vector2f(150, Game::SCREEN_HEIGHT - 166), 1, rp_editor_newmenu, "Setup_Teams", "Setup Teams", &AssetManager::GetTexture("Icon_Mini_White"), 1, sf::IntRect(0, 0, 60, 60), 0.2f, 0.2f), "setup");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_NewMenu"), sf::Vector2f(150, Game::SCREEN_HEIGHT - 85), 1, rp_editor_newmenu, "Select_Mode", "Select Mode", &AssetManager::GetTexture("Icon_Mini_White"), 1, sf::IntRect(0, 60, 60, 60), 0.3f, 0.3f), "setup");

    menu->AddInFront("main");
    menu->SetEmptyMessage("Exit");

    //Pop-up Menu
    popUpMenu.Create("select_mode");
    popUpMenu.AddObject(new GUI::DisplayShape(sf::Vector2f(Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT), sf::Color(0, 0, 0),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 0),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 25),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 0)), "select_mode");
    popUpMenu.AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 0, 880, 80),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 294), sf::Vector2f(1, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 294), sf::Vector2f(1, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 294), sf::Vector2f(1, 1), 0),
                                               0.2f), "select_mode");
    popUpMenu.AddObject(new GUI::DisplayShape(sf::Vector2f(880, 590), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 41), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 41), sf::Vector2f(1, 1), 230),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 41), sf::Vector2f(1, 1), 0),
                                              0.2f), "select_mode");
    popUpMenu.AddObject(new GUI::DisplayText("Please select a game mode", GetFont("Default"), GetColor("Default"), 24,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 300), sf::Vector2f(1, 1), 0),
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 300), sf::Vector2f(1, 1), 255),
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 300), sf::Vector2f(1, 1), 0),
                                             0.2f), "select_mode");
    popUpMenu.AddObject(new GUI::DisplayText("Mode", GetFont("Praetorian"), sf::Color::White, 50,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 227, SCREEN_HEIGHT / 2 - 308), sf::Vector2f(1, 1), 0),
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 227, SCREEN_HEIGHT / 2 - 308), sf::Vector2f(1, 1), 255),
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 227, SCREEN_HEIGHT / 2 - 308), sf::Vector2f(1, 1), 0), 0.2f), "select_mode");

    GUI::ToggleButton* button_battlemode = new GUI::ToggleButton(GetGuiStyle("Editor_SelectMode_Off"), GetGuiStyle("Editor_SelectMode_On"), sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 143), 1, rp_editor_selectmode, "SetTo_BattleMode", "Reset_SelectModeButtons",
                                                              "Battle against other teams and destroy the\nmain bases of the opposing teams to claim\nvictory.",
                                                              &AssetManager::GetTexture("Icon_SelectMode"), 1, sf::IntRect(0, 0, 138, 138), 0.2f);
    GUI::ToggleButton* button_defendmode = new GUI::ToggleButton(GetGuiStyle("Editor_SelectMode_Off"), GetGuiStyle("Editor_SelectMode_On"), sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 22), 1, rp_editor_selectmode, "SetTo_DefendMode", "Reset_SelectModeButtons",
                                                              "Defend your main base as the only base in\nthe level as the enemy comes in and attack\nfrom all sides.",
                                                              &AssetManager::GetTexture("Icon_SelectMode"), 1, sf::IntRect(0, 138, 138, 138), 0.2f);
    GUI::ToggleButton* button_bossmode = new GUI::ToggleButton(GetGuiStyle("Editor_SelectMode_Off"), GetGuiStyle("Editor_SelectMode_On"), sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 187), 1, rp_editor_selectmode, "SetTo_BossMode", "Reset_SelectModeButtons",
                                                              "Fight against a mighty boss with a very high\nhit point and its other ship units. Lead your\nteam to victory by defeating the boss.",
                                                              &AssetManager::GetTexture("Icon_SelectMode"), 1, sf::IntRect(0, 276, 138, 138), 0.2f);

    popUpMenu.AddObject(button_battlemode, "select_mode");
    popUpMenu.AddObject(button_defendmode, "select_mode");
    popUpMenu.AddObject(button_bossmode, "select_mode");
    popUpMenu.AddObject(new GUI::Button(GetGuiStyle("BackArrow"), sf::Vector2f(SCREEN_WIDTH / 2 + 423, Game::SCREEN_HEIGHT / 2 + 309), 0.5f, rp_backArrow, "Close_Popup", "", nullptr, 1, sf::IntRect(), 0.2f), "select_mode");

    //Setup Teams
    popUpMenu.Create("setup_teams");
    popUpMenu.AddObject(new GUI::DisplayShape(sf::Vector2f(Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT), sf::Color(0, 0, 0),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 0),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 25),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 0)), "setup_teams");
    popUpMenu.AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 0, 880, 80),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 160), sf::Vector2f(1, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 160), sf::Vector2f(1, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 160), sf::Vector2f(1, 1), 0),
                                               0.2f), "setup_teams");
    popUpMenu.AddObject(new GUI::DisplayShape(sf::Vector2f(880, 320), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40), sf::Vector2f(1, 1), 230),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40), sf::Vector2f(1, 1), 0),
                                              0.2f), "setup_teams");
   popUpMenu.AddObject(new GUI::DisplayText("Teams", GetFont("Praetorian"), sf::Color::White, 50,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 239, SCREEN_HEIGHT / 2 - 175), sf::Vector2f(1, 1), 0),
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 239, SCREEN_HEIGHT / 2 - 175), sf::Vector2f(1, 1), 255),
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 239, SCREEN_HEIGHT / 2 - 175), sf::Vector2f(1, 1), 0),
                                            0.2f), "setup_teams");
    GUI::DisplayText* displaytext_setupTeam = new GUI::DisplayText("Select a team to add+", GetFont("Default"), GetColor("Default_Light"), 25,
                                                                  GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40), sf::Vector2f(1, 1), 0),
                                                                  GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40), sf::Vector2f(1, 1), 255),
                                                                  GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40), sf::Vector2f(1, 1), 0), 0.2f);
    popUpMenu.AddObject(displaytext_setupTeam, "setup_teams");
    GUI::DisplayShape* displayshape_setupTeam = new GUI::DisplayShape(sf::Vector2f(744, 4), GetColor("Default_Light"),
                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 117), sf::Vector2f(1, 1), 0),
                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 117), sf::Vector2f(1, 1), 255),
                                                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 117), sf::Vector2f(1, 1), 0), 0.2f);
    popUpMenu.AddObject(displayshape_setupTeam, "setup_teams");
    popUpMenu.AddObject(new GUI::Button(GetGuiStyle("Editor_Close"), sf::Vector2f(SCREEN_WIDTH / 2 + 420, SCREEN_HEIGHT / 2 - 101), 0.5f, rp_backArrow, "Close_Popup", "", nullptr, 1, sf::IntRect(), 0.2f), "setup_teams");

    GUI::DisplaySprite* displaysprite_basearrow[4];
    GUI::DisplaySprite* displaysprite_statearrow[4];
    GUI::DisplayShape* displayshape_miniTeamLine[4];
    GUI::DisplayText* displaytext_miniTeamType[4];
    GUI::DisplayShape* displayshape_lineTeamType[4];
    GUI::DisplaySprite* displaysprite_team_ts[4];
    GUI::DisplaySprite* displaysprite_teamstate_ts[4];

    for(int a = 0; a < 4; a++)
    {
        displayshape_lineTeamType[a] = new GUI::DisplayShape(sf::Vector2f(130, 4), GetColor("Default_Light"),
                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0), 0.2f);
        popUpMenu.AddObject(displayshape_lineTeamType[a], "setup_teams");

        displaysprite_team_ts[a] = new GUI::DisplaySprite(&AssetManager::GetTexture("Team_Logo"), sf::IntRect(0, 200 * a, 188, 200),
                                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(0.585f, 0.585f), 0),
                                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(0.585f, 0.585f), 255),
                                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(0.585f, 0.585f), 0), 0.2f);
        popUpMenu.AddObject(displaysprite_team_ts[a], "setup_teams");

        displaysprite_basearrow[a] = new GUI::DisplaySprite(&AssetManager::GetTexture("Icon_TeamStatus"), sf::IntRect(20, 180, 20, 10),
                                                            GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                                                            GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                                                            GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0), 0.2f);
        popUpMenu.AddObject(displaysprite_basearrow[a], "setup_teams");

        displaysprite_statearrow[a] = new GUI::DisplaySprite(&AssetManager::GetTexture("Icon_TeamStatus"), sf::IntRect(0, 180, 20, 10),
                                                            GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0),
                                                            GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 255),
                                                            GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(1, 1), 0), 0.2f);
        popUpMenu.AddObject(displaysprite_statearrow[a], "setup_teams");

        displaysprite_teamstate_ts[a] = new GUI::DisplaySprite(&AssetManager::GetTexture("Icon_TeamStatus"), sf::IntRect(0, 0, 60, 60),
                                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(0.8f, 0.8f), 0),
                                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(0.8f, 0.8f), 255),
                                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(), sf::Vector2f(0.8f, 0.8f), 0), 0.2f);
        popUpMenu.AddObject(displaysprite_teamstate_ts[a], "setup_teams");

        displayshape_miniTeamLine[a] = new GUI::DisplayShape(sf::Vector2f(28, 3), GetColor("Default_Light"),
                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 409 + 40 * a, SCREEN_HEIGHT / 2 + 151), sf::Vector2f(1, 1), 0),
                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 409 + 40 * a, SCREEN_HEIGHT / 2 + 151), sf::Vector2f(1, 1), 255),
                                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 409 + 40 * a, SCREEN_HEIGHT / 2 + 151), sf::Vector2f(1, 1), 0), 0.2f);
        popUpMenu.AddObject(displayshape_miniTeamLine[a], "setup_teams");

        displaytext_miniTeamType[a] = new GUI::DisplayText("", GetFont("Conthrax"), GetColor("Default_Light"), 17,
                                                           GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 409 + 40 * a, SCREEN_HEIGHT / 2 + 135), sf::Vector2f(1, 1), 0),
                                                           GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 409 + 40 * a, SCREEN_HEIGHT / 2 + 135), sf::Vector2f(1, 1), 255),
                                                           GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 409 + 40 * a, SCREEN_HEIGHT / 2 + 135), sf::Vector2f(1, 1), 0), 0.2f);
        popUpMenu.AddObject(displaytext_miniTeamType[a], "setup_teams");
    }

    for(int a = 0; a < 4; a++)
    {
        std::stringstream ss;
        ss << "EditTeamType_" << a;
        popUpMenu.AddObject(new GUI::Button(GetGuiStyle("Editor_TeamOption"), sf::Vector2f(SCREEN_WIDTH / 2 - 409 + 40 * a, SCREEN_HEIGHT / 2 + 172), 0.5f, rp_backArrow, ss.str(), "", &AssetManager::GetTexture("Team_Logo"), 0.3f, sf::IntRect(0, 200 * a, 188, 200), 0.2), "setup_teams");
    }

    //Team Option
    popUpMenu.Create("team_option");
    popUpMenu.AddObject(new GUI::DisplayShape(sf::Vector2f(Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT), sf::Color(0, 0, 0),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 0),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 25),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 0)), "team_option");
    popUpMenu.AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 80, 400, 20),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 151), sf::Vector2f(1.165, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 151), sf::Vector2f(1.165, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 151), sf::Vector2f(1.165, 1), 0),
                                               0.2f), "team_option");
    popUpMenu.AddObject(new GUI::DisplayShape(sf::Vector2f(466, 302), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 10), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 10), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 10), sf::Vector2f(1, 1), 0),
                                              0.2f), "team_option");
    GUI::DisplaySprite* display_teamoption = new GUI::DisplaySprite(&AssetManager::GetTexture("Team_Logo"), sf::IntRect(188, 0, 161, 200),
                                                                    GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 116, SCREEN_HEIGHT / 2 + 3), sf::Vector2f(0.945f, 0.945f), 0),
                                                                    GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 116, SCREEN_HEIGHT / 2 + 3), sf::Vector2f(0.945f, 0.945f), 255),
                                                                    GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 - 116, SCREEN_HEIGHT / 2 + 3), sf::Vector2f(0.945f, 0.945f), 0),
                                                                    0.2f);
    GUI::DisplaySprite* display_teamoption_icon = new GUI::DisplaySprite(&AssetManager::GetTexture("Icon_TeamStatus"), sf::IntRect(0, 0, 60, 60),
                                                                         GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 124), sf::Vector2f(0.7f, 0.7f), 0),
                                                                         GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 124), sf::Vector2f(0.7f, 0.7f), 255),
                                                                         GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 124), sf::Vector2f(0.7f, 0.7f), 0),
                                                                         0.2f);
    popUpMenu.AddObject(display_teamoption, "team_option");
    popUpMenu.AddObject(display_teamoption_icon, "team_option");
    popUpMenu.AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Editor_TeamOption"), sf::IntRect(91, 0, 192, 192),
                                           GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 + 94, SCREEN_HEIGHT / 2 + 3), sf::Vector2f(1, 1), 0),
                                           GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 + 94, SCREEN_HEIGHT / 2 + 3), sf::Vector2f(1, 1), 255),
                                           GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2 + 94, SCREEN_HEIGHT / 2 + 3), sf::Vector2f(1, 1), 0),
                                           0.2f), "team_option");
    popUpMenu.AddObject(new GUI::DisplayShape(sf::Vector2f(126, 2), GetColor("Default_Light"),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 107), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 107), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 107), sf::Vector2f(1, 1), 0),
                                              0.2), "team_option");
    popUpMenu.AddObject(new GUI::DisplayText("Select a team up type for this team", GetFont("Default"), GetColor("Default_Light"), 20,
                                         GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 136), sf::Vector2f(1, 1), 0),
                                         GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 136), sf::Vector2f(1, 1), 255),
                                         GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 136), sf::Vector2f(1, 1), 0),
                                         0.2f), "team_option");
    popUpMenu.AddObject(new GUI::Button(GetGuiStyle("Editor_Close"), sf::Vector2f(SCREEN_WIDTH / 2 + 218, SCREEN_HEIGHT / 2 - 126), 0.5f, rp_backArrow, "Close_TeamOption", "", nullptr, 1, sf::IntRect(), 0.2f), "team_option");
    popUpMenu.AddObject(new GUI::Button(GetGuiStyle("Editor_Switch"), sf::Vector2f(SCREEN_WIDTH / 2 - 218, SCREEN_HEIGHT / 2 - 126), 0.5f, rp_backArrow, "Switch_TeamOption", "", nullptr, 1, sf::IntRect(), 0.2f), "team_option");

    GUI::ToggleButton* button_teamtype[4];
    button_teamtype[0] = new GUI::ToggleButton(GetGuiStyle("Editor_TeamType_Off"), GetGuiStyle("Editor_TeamType_On"), sf::Vector2f(SCREEN_WIDTH / 2 + 47, SCREEN_HEIGHT / 2 - 45), 1, rp_editor_teamoption, "TeamType_A", "No_TeamType", "A",
                                               nullptr, 1, sf::IntRect(), 0.2f);
    button_teamtype[1] = new GUI::ToggleButton(GetGuiStyle("Editor_TeamType_Off"), GetGuiStyle("Editor_TeamType_On"), sf::Vector2f(SCREEN_WIDTH / 2 + 142, SCREEN_HEIGHT / 2 - 45), 1, rp_editor_teamoption, "TeamType_B", "No_TeamType", "B",
                                               nullptr, 1, sf::IntRect(), 0.2f);
    button_teamtype[2] = new GUI::ToggleButton(GetGuiStyle("Editor_TeamType_Off"), GetGuiStyle("Editor_TeamType_On"), sf::Vector2f(SCREEN_WIDTH / 2 + 47, SCREEN_HEIGHT / 2 + 50), 1, rp_editor_teamoption, "TeamType_C", "No_TeamType", "C",
                                               nullptr, 1, sf::IntRect(), 0.2f);
    button_teamtype[3] = new GUI::ToggleButton(GetGuiStyle("Editor_TeamType_Off"), GetGuiStyle("Editor_TeamType_On"), sf::Vector2f(SCREEN_WIDTH / 2 + 142, SCREEN_HEIGHT / 2 + 50), 1, rp_editor_teamoption, "TeamType_D", "No_TeamType", "D",
                                               nullptr, 1, sf::IntRect(), 0.2f);
    GUI::Group* group_teamtype = new GUI::Group();
    for(int a = 0; a < 4; a++)
        group_teamtype->AddObject(button_teamtype[a]);
    popUpMenu.AddObject(group_teamtype, "team_option");

    //Setup Section
    menu->Create("setup_section");

    GUI::ToggleButton* button_teamtab[5];
    GUI::Group* group_teamtab = new GUI::Group();

    for(int a = 0; a < 4; a++)
    {
        std::stringstream ss;
        ss<< "Team_Tab_" << a;
        button_teamtab[a] = new GUI::ToggleButton(GetGuiStyle("Editor_Mini_Off"), GetGuiStyle("Editor_Mini_On"), sf::Vector2f(30.f, 30 + 60 * a), 1, rp_editor_mini, ss.str(), "No_Team_Tab", "", &AssetManager::GetTexture("Team_Logo"), 0.25f, sf::IntRect(0, 200 * a, 188, 200), 0, 0.3f);
        group_teamtab->AddObject(button_teamtab[a]);
    }
    button_teamtab[4] = new GUI::ToggleButton(GetGuiStyle("Editor_Mini_Off"), GetGuiStyle("Editor_Mini_On"), sf::Vector2f(30.f, 30 + 60 * 4), 1, rp_editor_mini, "Team_Tab_4", "No_Team_Tab", "", &AssetManager::GetTexture("Icon_NoTeam"), 0.5f, sf::IntRect(0, 0, 94, 100), 0, 0.3f);
    group_teamtab->AddObject(button_teamtab[4]);
    menu->AddObject(group_teamtab, "setup_section");

    menu->AddObject(new GUI::DisplayShape(sf::Vector2f(60, Game::SCREEN_HEIGHT), Game::GetColor("Default_Lighter"),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(-30, Game::SCREEN_HEIGHT / 2)),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(30, Game::SCREEN_HEIGHT / 2)),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(-30, Game::SCREEN_HEIGHT / 2)), 0.f, 0.3f), "setup_section");
    menu->AddObject(new GUI::DisplayShape(sf::Vector2f(5, Game::SCREEN_HEIGHT), sf::Color(53, 77, 89),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(-2.5, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 76),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(62.5, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 76),
                                     GUI::DisplayObject::DisplayPhase(sf::Vector2f(-2.5, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 76), 0.f, 0.3f), "setup_section");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_Mini"), sf::Vector2f(30, Game::SCREEN_HEIGHT - 30), 1, rp_editor_mini, "Setup", "", &AssetManager::GetTexture("Editor_Mini"), 1, sf::IntRect(120, 0, 60, 60), 0.f, 0.3f), "setup_section");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_Mini"), sf::Vector2f(30, Game::SCREEN_HEIGHT - 90), 1, rp_editor_mini, "Help_Setup", "", &AssetManager::GetTexture("Editor_Mini"), 1, sf::IntRect(60, 0, 60, 60), 0.f, 0.3f), "setup_section");
    menu->AddObject(new GUI::Button(GetGuiStyle("Editor_Mini"), sf::Vector2f(30, Game::SCREEN_HEIGHT - 150), 1, rp_editor_mini, "Others_Setup", "", &AssetManager::GetTexture("Editor_Mini"), 1, sf::IntRect(60, 60, 60, 60), 0.f, 0.3f), "tile_mode");

    //Setup Base
    GUI::Session* session_basesetup = new GUI::Session();
    menu->AddObject(session_basesetup, "setup_section");
    session_basesetup->Create("help_battle_mode");
    session_basesetup->AddObject(new GUI::DisplayText("Select a team you\nwant to create a\nbase for.", GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(167, 70), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(167, 70), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(167, 70), sf::Vector2f(1, 1), 0),
                                                      0.3f), "help_battle_mode");
    session_basesetup->AddObject(new GUI::DisplayShape(sf::Vector2f(208, 3), GetColor("Default_Light"),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 124), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 124), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 124), sf::Vector2f(1, 1), 0),
                                                       0.3f), "help_battle_mode");
    session_basesetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Editor_BS_SS_Help"), sf::IntRect(0, 0, 150, 160),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 232), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 232), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 232), sf::Vector2f(1, 1), 0),
                                                        0.3f), "help_battle_mode");
    session_basesetup->AddObject(new GUI::DisplayText("Highlight a portion\nfor the base.", GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(168, 338), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(168, 338), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(168, 338), sf::Vector2f(1, 1), 0),
                                                      0.3f), "help_battle_mode");
    session_basesetup->AddObject(new GUI::DisplayShape(sf::Vector2f(208, 3), GetColor("Default_Light"),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 383), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 383), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 383), sf::Vector2f(1, 1), 0),
                                                       0.3f), "help_battle_mode");
    session_basesetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Editor_BS_SS_Help"), sf::IntRect(150, 0, 150, 160),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 480), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 480), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 480), sf::Vector2f(1, 1), 0),
                                                        0.3f), "help_battle_mode");
    session_basesetup->AddObject(new GUI::DisplayText("Then select a spawn\npoint and place it\nwithin the highlighted\nportion to create a\nbase.", GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 614), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 614), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 614), sf::Vector2f(1, 1), 0),
                                                      0.3f), "help_battle_mode");
    session_basesetup->Create("help_defend_mode");
    session_basesetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Editor_BS_SS_Help"), sf::IntRect(0, 0, 150, 160),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 102), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 102), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 102), sf::Vector2f(1, 1), 0),
                                                        0.3f), "help_defend_mode");
    session_basesetup->AddObject(new GUI::DisplayText("In this mode, only one\nbase must be made\nwhich is the base to\ndefend.\nHighlight a portion\nfor the base.", GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 255), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 255), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 255), sf::Vector2f(1, 1), 0),
                                                      0.3f), "help_defend_mode");
    session_basesetup->AddObject(new GUI::DisplayShape(sf::Vector2f(208, 3), GetColor("Default_Light"),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 343), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 343), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 343), sf::Vector2f(1, 1), 0),
                                                       0.3f), "help_defend_mode");
    session_basesetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Editor_BS_SS_Help"), sf::IntRect(150, 0, 150, 160),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 441), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 441), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 441), sf::Vector2f(1, 1), 0),
                                                        0.3f), "help_defend_mode");
    session_basesetup->AddObject(new GUI::DisplayText("Spawn points for the\nplayer's team must\nbe placed in the base.\nOther spawn points\nfor other teams can\nbe placed anywhere\noutside the base\nwithout a base.",
                                                      GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 610), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 610), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 610), sf::Vector2f(1, 1), 0),
                                                      0.3f), "help_defend_mode");
    session_basesetup->Create("help_boss_mode");
    session_basesetup->AddObject(new GUI::DisplayText("In this mode, there\nare no bases, just\nspawn points.",
                                                      GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 78), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 78), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 78), sf::Vector2f(1, 1), 0),
                                                      0.3f), "help_boss_mode");
    session_basesetup->AddObject(new GUI::DisplayShape(sf::Vector2f(208, 3), GetColor("Default_Light"),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 139), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 139), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 139), sf::Vector2f(1, 1), 0),
                                                       0.3f), "help_boss_mode");
    session_basesetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Editor_BS_SS_Help"), sf::IntRect(150, 0, 150, 160),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 237), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 237), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 237), sf::Vector2f(1, 1), 0),
                                                        0.3f), "help_boss_mode");
    session_basesetup->AddObject(new GUI::DisplayText("Select a spawn point\nof any team and place\nit anywhere.",
                                                      GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 348), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 348), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 348), sf::Vector2f(1, 1), 0),
                                                      0.3f), "help_boss_mode");
    session_basesetup->Create("sp_select_alpha");
    session_basesetup->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_Arrow_Up"), sf::Vector2f(183, 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Button(GetGuiStyle("Editor_Arrow_Down"), sf::Vector2f(183, Game::SCREEN_HEIGHT - 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Degree(GUI::Vertical, 581, 3, sf::Vector2f(294, Game::SCREEN_HEIGHT / 2), sf::Vector2f(5, 50), GetColor("Default_Lighter"), GetColor("Default"), sf::Color::Transparent, GUI::Degree::ClickNode, 0.5f),
                                                    new GUI::Selection(GetGuiStyle("Editor_Selection"), GetGuiStyle("Editor_Selection_On"), rp_editor_selection, sf::Vector2f(180, Game::SCREEN_HEIGHT / 2), 1, sf::Vector2f(235, Game::SCREEN_HEIGHT - 104 + 8),
                                                                       sf::FloatRect(0, 0, 200, 200), 200, 0.1f, GUI::Vertical, &itemInfo_sp_alpha)), "sp_select_alpha");
    session_basesetup->Create("sp_select_delta");
    session_basesetup->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_Arrow_Up"), sf::Vector2f(183, 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Button(GetGuiStyle("Editor_Arrow_Down"), sf::Vector2f(183, Game::SCREEN_HEIGHT - 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Degree(GUI::Vertical, 581, 3, sf::Vector2f(294, Game::SCREEN_HEIGHT / 2), sf::Vector2f(5, 50), GetColor("Default_Lighter"), GetColor("Default"), sf::Color::Transparent, GUI::Degree::ClickNode, 0.5f),
                                                    new GUI::Selection(GetGuiStyle("Editor_Selection"), GetGuiStyle("Editor_Selection_On"), rp_editor_selection, sf::Vector2f(180, Game::SCREEN_HEIGHT / 2), 1, sf::Vector2f(235, Game::SCREEN_HEIGHT - 104 + 8),
                                                                       sf::FloatRect(0, 0, 200, 200), 200, 0.1f, GUI::Vertical, &itemInfo_sp_delta)), "sp_select_delta");
    session_basesetup->Create("sp_select_vortex");
    session_basesetup->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_Arrow_Up"), sf::Vector2f(183, 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Button(GetGuiStyle("Editor_Arrow_Down"), sf::Vector2f(183, Game::SCREEN_HEIGHT - 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Degree(GUI::Vertical, 581, 3, sf::Vector2f(294, Game::SCREEN_HEIGHT / 2), sf::Vector2f(5, 50), GetColor("Default_Lighter"), GetColor("Default"), sf::Color::Transparent, GUI::Degree::ClickNode, 0.5f),
                                                    new GUI::Selection(GetGuiStyle("Editor_Selection"), GetGuiStyle("Editor_Selection_On"), rp_editor_selection, sf::Vector2f(180, Game::SCREEN_HEIGHT / 2), 1, sf::Vector2f(235, Game::SCREEN_HEIGHT - 104 + 8),
                                                                        sf::FloatRect(0, 0, 200, 200), 200, 0.1f, GUI::Vertical, &itemInfo_sp_vortex)), "sp_select_vortex");
    session_basesetup->Create("sp_select_omega");
    session_basesetup->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_Arrow_Up"), sf::Vector2f(183, 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Button(GetGuiStyle("Editor_Arrow_Down"), sf::Vector2f(183, Game::SCREEN_HEIGHT - 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Degree(GUI::Vertical, 581, 3, sf::Vector2f(294, Game::SCREEN_HEIGHT / 2), sf::Vector2f(5, 50), GetColor("Default_Lighter"), GetColor("Default"), sf::Color::Transparent, GUI::Degree::ClickNode, 0.5f),
                                                    new GUI::Selection(GetGuiStyle("Editor_Selection"), GetGuiStyle("Editor_Selection_On"), rp_editor_selection, sf::Vector2f(180, Game::SCREEN_HEIGHT / 2), 1, sf::Vector2f(235, Game::SCREEN_HEIGHT - 104 + 8),
                                                                       sf::FloatRect(0, 0, 200, 200), 200, 0.1f, GUI::Vertical, &itemInfo_sp_omega)), "sp_select_omega");
    session_basesetup->Create("sp_select_all_teams");
    session_basesetup->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_Arrow_Up"), sf::Vector2f(183, 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Button(GetGuiStyle("Editor_Arrow_Down"), sf::Vector2f(183, Game::SCREEN_HEIGHT - 26), 1, rp_editor_arrow, "", ""),
                                                    new GUI::Degree(GUI::Vertical, 581, 3, sf::Vector2f(294, Game::SCREEN_HEIGHT / 2), sf::Vector2f(5, 50), GetColor("Default_Lighter"), GetColor("Default"), sf::Color::Transparent, GUI::Degree::ClickNode, 0.5f),
                                                    new GUI::Selection(GetGuiStyle("Editor_Selection"), GetGuiStyle("Editor_Selection_On"), rp_editor_selection, sf::Vector2f(180, Game::SCREEN_HEIGHT / 2), 1, sf::Vector2f(235, Game::SCREEN_HEIGHT - 104 + 8),
                                                                       sf::FloatRect(0, 0, 200, 200), 200, 0.1f, GUI::Vertical, &itemInfo_sp_all_teams)), "sp_select_all_teams");
    session_basesetup->Create("base_options");
    session_basesetup->AddObject(new GUI::DisplayText("Select an option you\nwish to perform on\nthe highlighted base.",
                                                      GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 60), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 60), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 60), sf::Vector2f(1, 1), 0),
                                                      0.3f), "base_options");
    session_basesetup->AddObject(new GUI::DisplayShape(sf::Vector2f(210, 3), GetColor("Default_Light"),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(180, SCREEN_HEIGHT - 51), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(180, SCREEN_HEIGHT - 51), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(180, SCREEN_HEIGHT - 51), sf::Vector2f(1, 1), 0),
                                                       0.3f), "base_options");
    session_basesetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Editor_StateDisplay"),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(135, SCREEN_HEIGHT - 24), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(135, SCREEN_HEIGHT - 24), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(135, SCREEN_HEIGHT - 24), sf::Vector2f(1, 1), 0),
                                                        0.3f), "base_options");
    session_basesetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Editor_StateDisplay"),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(230, SCREEN_HEIGHT - 24), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(230, SCREEN_HEIGHT - 24), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(230, SCREEN_HEIGHT - 24), sf::Vector2f(1, 1), 0),
                                                        0.3f), "base_options");
    session_basesetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Icon_States"), sf::IntRect(360, 0, 60, 60),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(106, SCREEN_HEIGHT - 24), sf::Vector2f(0.5f, 0.5f), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(106, SCREEN_HEIGHT - 24), sf::Vector2f(0.5f, 0.5f), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(106, SCREEN_HEIGHT - 24), sf::Vector2f(0.5f, 0.5f), 0),
                                                        0.3f), "base_options");
    session_basesetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Icon_States"), sf::IntRect(300, 0, 60, 60),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(201, SCREEN_HEIGHT - 24), sf::Vector2f(0.5f, 0.5f), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(201, SCREEN_HEIGHT - 24), sf::Vector2f(0.5f, 0.5f), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(201, SCREEN_HEIGHT - 24), sf::Vector2f(0.5f, 0.5f), 0),
                                                        0.3f), "base_options");

    GUI::DisplayText* displaytext_bsoptions_units = new GUI::DisplayText("", GetFont("Default"), GetColor("Default_Light"), 15,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(150, SCREEN_HEIGHT - 28), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(150, SCREEN_HEIGHT - 28), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(150, SCREEN_HEIGHT - 28), sf::Vector2f(1, 1), 0),
                                                      0.3f);
    GUI::DisplayText* displaytext_bsoptions_size = new GUI::DisplayText("", GetFont("Default"), GetColor("Default_Light"), 15,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(246, SCREEN_HEIGHT - 28), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(246, SCREEN_HEIGHT - 28), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(246, SCREEN_HEIGHT - 28), sf::Vector2f(1, 1), 0),
                                                      0.3f);
    session_basesetup->AddObject(displaytext_bsoptions_units, "base_options");
    session_basesetup->AddObject(displaytext_bsoptions_size, "base_options");

    GUI::Button* button_makemainbase = new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 263), 1, rp_editor_plain, "Make_MainBase", "Make Main Base", nullptr, 1, sf::IntRect(), 0.3f);

    session_basesetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 141), 1, rp_editor_plain, "Delete_Base", "Delete", nullptr, 1, sf::IntRect(), 0.3f), "base_options");
    session_basesetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 202), 1, rp_editor_plain, "Change_SpawnPoint", "Change S.P", nullptr, 1, sf::IntRect(), 0.3f), "base_options");
    session_basesetup->AddObject(button_makemainbase, "base_options");
    session_basesetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 324), 1, rp_editor_plain, "Base_Effectors", "Add Effectors", nullptr, 1, sf::IntRect(), 0.3f), "base_options");
    session_basesetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 385), 1, rp_editor_plain, "Set_Max_Units", "Set Max Units", nullptr, 1, sf::IntRect(), 0.3f), "base_options");
    session_basesetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 447), 1, rp_editor_plain, "Close_Base_Options", "Back", nullptr, 1, sf::IntRect(), 0.3f), "base_options");

    session_basesetup->Create("mainbase_option");
    session_basesetup->AddObject(new GUI::DisplayShape(sf::Vector2f(208, 3), GetColor("Default_Light"),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(185, 185), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(185, 185), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(185, 185), sf::Vector2f(1, 1), 0),
                                                       0.3f), "mainbase_option");
    session_basesetup->AddObject(new GUI::DisplayText("Place the base core\nin a suitable position\nwithin the highlighted\nbase. This will mark\nthis base as the\nmain base for this\nteam.",
                                                      GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(186, 285), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(186, 285), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(186, 285), sf::Vector2f(1, 1), 0),
                                                      0.3f), "mainbase_option");
    GUI::DisplaySprite* displaysprite_mainbaseoption_base = new GUI::DisplaySprite(nullptr, sf::IntRect(0, 0, 294, 306),
                                                                                   GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 94), sf::Vector2f(0.5, 0.5), 0),
                                                                                   GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 94), sf::Vector2f(0.5, 0.5), 255),
                                                                                   GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 94), sf::Vector2f(0.5, 0.5), 0),
                                                                                   0.3f);
    GUI::DisplaySprite* displaysprite_mainbaseoption_rotor = new GUI::DisplaySprite(nullptr, sf::IntRect(294, 0, 150, 150),
                                                                                   GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 93), sf::Vector2f(0.5, 0.5), 0),
                                                                                   GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 93), sf::Vector2f(0.5, 0.5), 255),
                                                                                   GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 93), sf::Vector2f(0.5, 0.5), 0),
                                                                                   0.3f);
    GUI::DisplaySprite* displaysprite_mainbaseoption_cover = new GUI::DisplaySprite(nullptr, sf::IntRect(444, 0, 174, 184),
                                                                                   GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 88), sf::Vector2f(0.5, 0.5), 0),
                                                                                   GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 88), sf::Vector2f(0.5, 0.5), 255),
                                                                                   GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 88), sf::Vector2f(0.5, 0.5), 0),
                                                                                   0.3f);
    session_basesetup->AddObject(displaysprite_mainbaseoption_base, "mainbase_option");
    session_basesetup->AddObject(displaysprite_mainbaseoption_rotor, "mainbase_option");
    session_basesetup->AddObject(displaysprite_mainbaseoption_cover, "mainbase_option");

    session_basesetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 402), 1, rp_editor_plain, "Close_MainBase_Options", "Back", nullptr, 1, sf::IntRect(), 0.3f), "mainbase_option");

    session_basesetup->Create("unit_option");
    session_basesetup->AddObject(new GUI::DisplayText("Set the maximum\nnumber of units this\nbase can hold.", GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 58), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 58), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 58), sf::Vector2f(1, 1), 0),
                                                      0.3f), "unit_option");
    session_basesetup->AddObject(new GUI::DisplayShape(sf::Vector2f(210, 3), GetColor("Default_Light"),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 109), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 109), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 109), sf::Vector2f(1, 1), 0),
                                                       0.3f), "unit_option");
    session_basesetup->AddObject(new GUI::DisplayText("You can let the game\nset this for you\nautomatically by\nusing the size of the\nbase.",
                                                      GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 191), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 191), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(181, 191), sf::Vector2f(1, 1), 0),
                                                      0.3f), "unit_option");
    session_basesetup->AddObject(new GUI::DisplayShape(sf::Vector2f(210, 3), GetColor("Default_Light"),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 329), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 329), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 329), sf::Vector2f(1, 1), 0),
                                                       0.3f), "unit_option");
    session_basesetup->AddObject(new GUI::DisplayText("Or you can set it\nmanually yourself.",
                                                      GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(167, 378), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(167, 378), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(167, 378), sf::Vector2f(1, 1), 0),
                                                      0.3f), "unit_option");
    session_basesetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Editor_StateDisplay"),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 440), sf::Vector2f(1.277f, 1.277f), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 440), sf::Vector2f(1.277f, 1.277f), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 440), sf::Vector2f(1.277f, 1.277f), 0),
                                                        0.3f), "unit_option");
    session_basesetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Icon_States"), sf::IntRect(360, 0, 60, 60),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(145, 440), sf::Vector2f(0.633f, 0.633f), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(145, 440), sf::Vector2f(0.633f, 0.633f), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(145, 440), sf::Vector2f(0.633f, 0.633f), 0),
                                                        0.3f), "unit_option");

    GUI::DisplayText* displaytext_unitoption_units = new GUI::DisplayText("", GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(202, 437), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(202, 437), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(202, 437), sf::Vector2f(1, 1), 0),
                                                      0.3f);

    session_basesetup->AddObject(displaytext_unitoption_units, "unit_option");
    session_basesetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 287), 1, rp_editor_plain, "Auto_Unit_Set", "Automatic Set", nullptr, 1, sf::IntRect(), 0.3f), "unit_option");
    session_basesetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, SCREEN_HEIGHT - 41), 1, rp_editor_plain, "Close_Unit_Option", "Back", nullptr, 1, sf::IntRect(), 0.3f), "unit_option");
    session_basesetup->AddObject(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(111, 440), 0.333f, rp_editor_smallarrow, "BaseSize--", "", nullptr, 1, sf::IntRect(), 0.3f), "unit_option");
    session_basesetup->AddObject(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(254, 440), 0.333f, rp_editor_smallarrow, "BaseSize++", "", nullptr, 1, sf::IntRect(), 0.3f), "unit_option");

    //Setup Ships
    GUI::Session* session_shipsetup = new GUI::Session();
    menu->AddObject(session_shipsetup, "setup_section");

    session_shipsetup->Create("ship_type_select");
    session_shipsetup->AddObject(new GUI::Button(GetGuiStyle("Editor_ShipType"), sf::Vector2f(184, 80), 1, rp_editor_shiptype, "Main_Ship", "Main Ship", &AssetManager::GetTexture("Icon_ShipType"), 1, sf::IntRect(0, 0, 165, 80), 0.3f), "ship_type_select");
    session_shipsetup->AddObject(new GUI::Button(GetGuiStyle("Editor_ShipType"), sf::Vector2f(184, 222), 1, rp_editor_shiptype, "Sub_Ship", "Sub-Ship", &AssetManager::GetTexture("Icon_ShipType"), 1, sf::IntRect(0, 80, 165, 80), 0.3f), "ship_type_select");
    session_shipsetup->AddObject(new GUI::Button(GetGuiStyle("Editor_ShipType"), sf::Vector2f(184, 364), 1, rp_editor_shiptype, "Mini_Defenses", "Mini-Defenses", &AssetManager::GetTexture("Icon_ShipType"), 1, sf::IntRect(0, 160, 165, 80), 0.3f), "ship_type_select");
    session_shipsetup->AddObject(new GUI::Button(GetGuiStyle("Editor_ShipType"), sf::Vector2f(184, 506), 1, rp_editor_shiptype, "Main_Defenses", "Main-Defenses", &AssetManager::GetTexture("Icon_ShipType"), 1, sf::IntRect(0, 240, 165, 80), 0.3f), "ship_type_select");
    session_shipsetup->AddObject(new GUI::Button(GetGuiStyle("Editor_ShipType"), sf::Vector2f(184, 647), 1, rp_editor_shiptype, "Boss_Ship", "Boss Ship", &AssetManager::GetTexture("Icon_ShipType"), 1, sf::IntRect(0, 320, 165, 80), 0.3f), "ship_type_select");

    session_shipsetup->Create("ship_placement");
    session_shipsetup->AddObject(new GUI::DisplayText("Place the mouse on\na valid grid in the\nlevel to place this\nship and set the\nattributes of this\nunit.",
                                                      GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(178, 89), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(178, 89), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(178, 89), sf::Vector2f(1, 1), 0), 0.3f), "ship_placement");
    session_shipsetup->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Editor_BS_SS_Help"), sf::IntRect(300, 0, 150, 160),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 248), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 248), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 248), sf::Vector2f(1, 1), 0), 0.3f), "ship_placement");

    session_shipsetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, SCREEN_HEIGHT - 41), 1, rp_editor_plain, "Ship_Type_Select", "Back", nullptr, 1, sf::IntRect(), 0.3f), "ship_placement");

    session_shipsetup->Create("ship_info");
    session_shipsetup->AddObject(new GUI::DisplayShape(sf::Vector2f(191, 3), GetColor("Default_Light"),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 326), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 326), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 326), sf::Vector2f(1, 1), 0),
                                                       0.3f), "ship_info");
    session_shipsetup->AddObject(new GUI::DisplayText("Select any of these\noptions.",
                                                      GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(179, 369), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(179, 369), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(179, 369), sf::Vector2f(1, 1), 0), 0.3f), "ship_info");
    GUI::DisplaySprite* displaysprite_shipinfo = new GUI::DisplaySprite(nullptr, sf::IntRect(),
                                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 163), sf::Vector2f(1, 1), 0),
                                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 163), sf::Vector2f(1, 1), 255),
                                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(182, 163), sf::Vector2f(1, 1), 0), 0.3f);
    session_shipsetup->AddObject(displaysprite_shipinfo, "ship_info");
    session_shipsetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 429), 1, rp_editor_plain, "Rotate_Ship", "Rotate", nullptr, 1, sf::IntRect(), 0.3f), "ship_info");
    session_shipsetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 486), 1, rp_editor_plain, "Delete_Ship", "Delete", nullptr, 1, sf::IntRect(), 0.3f), "ship_info");
    session_shipsetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 543), 1, rp_editor_plain, "Close_Ship_Info", "Back", nullptr, 1, sf::IntRect(), 0.3f), "ship_info");

    //Ship Setup PopUp Menu
    GUI::Session* session_shipcreate = new GUI::Session();

    popUpMenu.Create("sc_session");
    popUpMenu.AddObject(session_shipcreate, "sc_session");
    popUpMenu.AddObject(new GUI::DisplayShape(sf::Vector2f(Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT), sf::Color(255, 255, 255),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 0),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 100),
                                          GUI::DisplayObject::DisplayPhase(sf::Vector2f(Game::SCREEN_WIDTH / 2, Game::SCREEN_HEIGHT / 2), sf::Vector2f(1, 1), 0)), "sc_session");

    session_shipcreate->Create("sc_input_name");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(374, 301), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0), 0.3f), "sc_input_name");
    session_shipcreate->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 80, 400, 20),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 151), sf::Vector2f(0.935, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 151), sf::Vector2f(0.935, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 151), sf::Vector2f(0.935, 1), 0),
                                               0.3f), "sc_input_name");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(319, 3), GetColor("Default_Light"),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 13), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 13), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 13), sf::Vector2f(1, 1), 0), 0.3f), "sc_input_name");
    session_shipcreate->AddObject(new GUI::DisplayText("Input a name for this unit\nand press enter to\ncontinue.",
                                             GetFont("Default"), GetColor("Default_Light"), 25,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 64), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 64), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 64), sf::Vector2f(1, 1), 0), 0.3f), "sc_input_name");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Close"), sf::Vector2f(SCREEN_WIDTH / 2 + 169, SCREEN_HEIGHT / 2 - 120), 0.5f, rp_editor_mini, "Cancel_Ship_Create", "", nullptr, 1, sf::IntRect(), 0.3f), "sc_input_name");

    GUI::DisplayText* displaytext_nameInput = new GUI::DisplayText("", GetFont("Default"), GetColor("Default_Light"), 38,
                                                                  GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 45), sf::Vector2f(1, 1), 0),
                                                                  GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 45), sf::Vector2f(1, 1), 255),
                                                                  GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 45), sf::Vector2f(1, 1), 0));
    session_shipcreate->AddObject(displaytext_nameInput, "sc_input_name");

    session_shipcreate->Create("sc_subship_skin");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(374, 348), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0)), "sc_subship_skin");
    session_shipcreate->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 80, 400, 20),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 174), sf::Vector2f(0.935, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 174), sf::Vector2f(0.935, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 174), sf::Vector2f(0.935, 1), 0)), "sc_subship_skin");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Close"), sf::Vector2f(SCREEN_WIDTH / 2 + 169, SCREEN_HEIGHT / 2 - 143), 0.5f, rp_editor_mini, "Cancel_Ship_Create", "", nullptr, 1, sf::IntRect()), "sc_subship_skin");
    session_shipcreate->AddObject(new GUI::DisplayText("Select a skin you want for\nthis unit. This is what the\nunit will look like.",
                                             GetFont("Default"), GetColor("Default_Light"), 25,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 101), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 101), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 101), sf::Vector2f(1, 1), 0)), "sc_subship_skin");
    GUI::Session* session_skinsubship = new GUI::Session();
    session_skinsubship->Create("skinsubship_alpha");
    session_skinsubship->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_subship_skin[Alpha])), "skinsubship_alpha");
    session_skinsubship->Create("skinsubship_delta");
    session_skinsubship->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_subship_skin[Delta])), "skinsubship_delta");
    session_skinsubship->Create("skinsubship_vortex");
    session_skinsubship->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_subship_skin[Vortex])), "skinsubship_vortex");
    session_skinsubship->Create("skinsubship_omega");
    session_skinsubship->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_subship_skin[Omega])), "skinsubship_omega");
    session_shipcreate->AddObject(session_skinsubship, "sc_subship_skin");

    session_shipcreate->Create("sc_maindefence_skin");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(374, 348), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0)), "sc_maindefence_skin");
    session_shipcreate->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 80, 400, 20),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 174), sf::Vector2f(0.935, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 174), sf::Vector2f(0.935, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 174), sf::Vector2f(0.935, 1), 0)), "sc_maindefence_skin");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Close"), sf::Vector2f(SCREEN_WIDTH / 2 + 169, SCREEN_HEIGHT / 2 - 143), 0.5f, rp_editor_mini, "Cancel_Ship_Create", "", nullptr, 1, sf::IntRect()), "sc_maindefence_skin");
    session_shipcreate->AddObject(new GUI::DisplayText("Select a skin you want for\nthis unit. This is what the\nunit will look like.",
                                             GetFont("Default"), GetColor("Default_Light"), 25,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 101), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 101), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 101), sf::Vector2f(1, 1), 0)), "sc_maindefence_skin");
    GUI::Session* session_skinmaindefence = new GUI::Session();
    session_skinmaindefence->Create("alpha");
    session_skinmaindefence->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_mainDefence_skin[Alpha])), "alpha");
    session_skinmaindefence->Create("delta");
    session_skinmaindefence->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_mainDefence_skin[Delta])), "delta");
    session_skinmaindefence->Create("vortex");
    session_skinmaindefence->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_mainDefence_skin[Vortex])), "vortex");
    session_skinmaindefence->Create("omega");
    session_skinmaindefence->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_mainDefence_skin[Omega])), "omega");
    session_shipcreate->AddObject(session_skinmaindefence, "sc_maindefence_skin");

    session_shipcreate->Create("sc_minidefence_skin");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(374, 348), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0)), "sc_minidefence_skin");
    session_shipcreate->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 80, 400, 20),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 174), sf::Vector2f(0.935, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 174), sf::Vector2f(0.935, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 174), sf::Vector2f(0.935, 1), 0)), "sc_minidefence_skin");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Close"), sf::Vector2f(SCREEN_WIDTH / 2 + 169, SCREEN_HEIGHT / 2 - 143), 0.5f, rp_editor_mini, "Cancel_Ship_Create", "", nullptr, 1, sf::IntRect()), "sc_minidefence_skin");
    session_shipcreate->AddObject(new GUI::DisplayText("Select a skin you want for\nthis unit. This is what the\nunit will look like.",
                                             GetFont("Default"), GetColor("Default_Light"), 25,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 101), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 101), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 101), sf::Vector2f(1, 1), 0)), "sc_minidefence_skin");
    GUI::Session* session_skinminidefence = new GUI::Session();
    session_skinminidefence->Create("alpha");
    session_skinminidefence->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_miniDefence_skin[Alpha])), "alpha");
    session_skinminidefence->Create("delta");
    session_skinminidefence->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_miniDefence_skin[Delta])), "delta");
    session_skinminidefence->Create("vortex");
    session_skinminidefence->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_miniDefence_skin[Vortex])), "vortex");
    session_skinminidefence->Create("omega");
    session_skinminidefence->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 138, SCREEN_HEIGHT / 2.f - 38), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_SkinSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 38), 1, sf::Vector2f(194, 194),
                                                                       sf::FloatRect(0, 0, 194, 194), 194, 0.1f, GUI::Horizontal, &itemInfo_miniDefence_skin[Omega])), "omega");
    session_shipcreate->AddObject(session_skinminidefence, "sc_minidefence_skin");

    session_shipcreate->Create("sc_attributes");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(374, 639), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0)), "sc_attributes");
    session_shipcreate->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 80, 400, 20),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 320), sf::Vector2f(0.935, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 320), sf::Vector2f(0.935, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 320), sf::Vector2f(0.935, 1), 0)), "sc_attributes");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Close"), sf::Vector2f(SCREEN_WIDTH / 2 + 166, SCREEN_HEIGHT / 2 - 286), 0.5f, rp_editor_mini, "Cancel_Ship_Create", "", nullptr, 1, sf::IntRect()), "sc_attributes");
    session_shipcreate->AddObject(new GUI::DisplayText("Set the attribute of this\nunit. The hit point,\ndamage and defense\nattributes will be set\nautomatically by the\ndifficulty. You can only set\nthe speed and special\nattributes.",
                                             GetFont("Default"), GetColor("Default_Light"), 25,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 134), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 134), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 134), sf::Vector2f(1, 1), 0)), "sc_attributes");
    session_shipcreate->AddObject(new GUI::DisplayText("Or set manually.",
                                             GetFont("Default"), GetColor("Default_Light"), 25,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 66, SCREEN_HEIGHT / 2.f + 86), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 66, SCREEN_HEIGHT / 2.f + 86), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 66, SCREEN_HEIGHT / 2.f + 86), sf::Vector2f(1, 1), 0)), "sc_attributes");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(40, 40), GetColor("Default_Light"),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 136), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 136), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 136), sf::Vector2f(1, 1), 0)), "sc_attributes");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(40, 40), GetColor("Default_Light"),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 196), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 196), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 196), sf::Vector2f(1, 1), 0)), "sc_attributes");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(255, 40), GetColor("Default_Lighter"),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f + 20, SCREEN_HEIGHT / 2.f + 136), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f + 20, SCREEN_HEIGHT / 2.f + 136), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f + 20, SCREEN_HEIGHT / 2.f + 136), sf::Vector2f(1, 1), 0)), "sc_attributes");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(255, 40), sf::Color(207, 220, 227),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f + 20, SCREEN_HEIGHT / 2.f + 196), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f + 20, SCREEN_HEIGHT / 2.f + 196), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f + 20, SCREEN_HEIGHT / 2.f + 196), sf::Vector2f(1, 1), 0)), "sc_attributes");
    session_shipcreate->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("ShipInfo_Data"), sf::IntRect(0, 300, 100, 100),
                                                         GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 136), sf::Vector2f(0.4, 0.4), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 136), sf::Vector2f(0.4, 0.4), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 136), sf::Vector2f(0.4, 0.4), 0)), "sc_attributes");
    session_shipcreate->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("ShipInfo_Data"), sf::IntRect(0, 400, 100, 100),
                                                         GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 196), sf::Vector2f(0.4, 0.4), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 196), sf::Vector2f(0.4, 0.4), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 128, SCREEN_HEIGHT / 2.f + 196), sf::Vector2f(0.4, 0.4), 0)), "sc_attributes");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 29), 1, rp_editor_plain, "SC_Randomize_Att", "Randomize", nullptr, 1, sf::IntRect()), "sc_attributes");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 271), 1, rp_editor_plain, "SC_Form_Ship", "Continue", nullptr, 1, sf::IntRect()), "sc_attributes");
    session_shipcreate->AddObject(new GUI::Degree(GUI::Horizontal, 225, 21, sf::Vector2f(SCREEN_WIDTH / 2.f + 20, SCREEN_HEIGHT / 2.f + 136), sf::Vector2f(4, 28),sf::Color(165, 190, 203), GetColor("Default"), sf::Color::White, GUI::Degree::ClickAnyPart, 0, 0, 0.1f, &sc_speedFactor), "sc_attributes");
    session_shipcreate->AddObject(new GUI::Degree(GUI::Horizontal, 225, 21, sf::Vector2f(SCREEN_WIDTH / 2.f + 20, SCREEN_HEIGHT / 2.f + 196), sf::Vector2f(4, 28),sf::Color(165, 190, 203), GetColor("Default"), sf::Color::White, GUI::Degree::ClickAnyPart, 0, 0, 0.1f, &sc_specialFactor), "sc_attributes");


    session_shipcreate->Create("sc_mainship_option_1");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(374, 526), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0)), "sc_mainship_option_1");
    session_shipcreate->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 80, 400, 20),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 263), sf::Vector2f(0.935, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 263), sf::Vector2f(0.935, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 263), sf::Vector2f(0.935, 1), 0)), "sc_mainship_option_1");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Close"), sf::Vector2f(SCREEN_WIDTH / 2 + 166, SCREEN_HEIGHT / 2 - 230), 0.5f, rp_editor_mini, "Cancel_Ship_Create", "", nullptr, 1, sf::IntRect()), "sc_mainship_option_1");
    session_shipcreate->AddObject(new GUI::DisplayText("Select a character you\nwant to use as a main\nunit in this level.",
                                             GetFont("Default"), GetColor("Default_Light"), 25,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 24, SCREEN_HEIGHT / 2.f - 155), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 24, SCREEN_HEIGHT / 2.f - 155), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 24, SCREEN_HEIGHT / 2.f - 155), sf::Vector2f(1, 1), 0)), "sc_mainship_option_1");
    session_shipcreate->AddObject(new GUI::DisplayText("Or you can setup this unit\nthat isn't part of the\ndefault characters by using\ndifferent skins and\nattributes.",
                                             GetFont("Default"), GetColor("Default_Light"), 25,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 4, SCREEN_HEIGHT / 2.f + 78), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 4, SCREEN_HEIGHT / 2.f + 78), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 4, SCREEN_HEIGHT / 2.f + 78), sf::Vector2f(1, 1), 0)), "sc_mainship_option_1");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(342, 3), GetColor("Default_Light"),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 13), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 13), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 13), sf::Vector2f(1, 1), 0)), "sc_mainship_option_1");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 67), 1, rp_editor_plain, "SC_Character_Select", "Character", nullptr, 1, sf::IntRect()), "sc_mainship_option_1");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 195), 1, rp_editor_plain, "SC_Input_Name", "Setup Character", nullptr, 1, sf::IntRect()), "sc_mainship_option_1");

    session_shipcreate->Create("sc_mainship_option_1_pteam");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(374, 526), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0)), "sc_mainship_option_1_pteam");
    session_shipcreate->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 80, 400, 20),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 263), sf::Vector2f(0.935, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 263), sf::Vector2f(0.935, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 263), sf::Vector2f(0.935, 1), 0)), "sc_mainship_option_1_pteam");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Close"), sf::Vector2f(SCREEN_WIDTH / 2 + 166, SCREEN_HEIGHT / 2 - 230), 0.5f, rp_editor_mini, "Cancel_Ship_Create", "", nullptr, 1, sf::IntRect()), "sc_mainship_option_1_pteam");
    session_shipcreate->AddObject(new GUI::DisplayText("Select a character you\nwant to use as a main\nunit in this level.",
                                             GetFont("Default"), GetColor("Default_Light"), 25,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 24, SCREEN_HEIGHT / 2.f - 155), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 24, SCREEN_HEIGHT / 2.f - 155), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 24, SCREEN_HEIGHT / 2.f - 155), sf::Vector2f(1, 1), 0)), "sc_mainship_option_1_pteam");
    session_shipcreate->AddObject(new GUI::DisplayText("Or drop a ghost unit and\nlet the player choose\nwhich character he wants\nto use before he starts\nthe level.",
                                             GetFont("Default"), GetColor("Default_Light"), 25,
                                             GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 5, SCREEN_HEIGHT / 2.f + 78), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 5, SCREEN_HEIGHT / 2.f + 78), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 5, SCREEN_HEIGHT / 2.f + 78), sf::Vector2f(1, 1), 0)), "sc_mainship_option_1_pteam");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(342, 3), GetColor("Default_Light"),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 13), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 13), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 13), sf::Vector2f(1, 1), 0)), "sc_mainship_option_1_pteam");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 67), 1, rp_editor_plain, "SC_Character_Select", "Character", nullptr, 1, sf::IntRect()), "sc_mainship_option_1_pteam");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 195), 1, rp_editor_plain, "SC_Form_Ghost", "Ghost Unit", nullptr, 1, sf::IntRect()), "sc_mainship_option_1_pteam");

    session_shipcreate->Create("sc_character_select");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(678, 407), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0)), "sc_character_select");
    session_shipcreate->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 80, 400, 20),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 204), sf::Vector2f(1.695, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 204), sf::Vector2f(1.695, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 204), sf::Vector2f(1.695, 1), 0)), "sc_character_select");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Close"), sf::Vector2f(SCREEN_WIDTH / 2 + 316, SCREEN_HEIGHT / 2 - 170), 0.5f, rp_editor_mini, "Cancel_Ship_Create", "", nullptr, 1, sf::IntRect()), "sc_character_select");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(616, 3), GetColor("Default_Light"),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 144), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 144), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 144), sf::Vector2f(1, 1), 0)), "sc_character_select");
    session_shipcreate->AddObject(new GUI::DisplayText("Select a character you want to use.",
                                                       GetFont("Default"), GetColor("Default_Light"), 25,
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f , SCREEN_HEIGHT / 2.f + 170), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f , SCREEN_HEIGHT / 2.f + 170), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f , SCREEN_HEIGHT / 2.f + 170), sf::Vector2f(1, 1), 0)), "sc_character_select");

    GUI::Session* session_character_select = new GUI::Session;
    session_character_select->Create("characters_alpha");
    session_character_select->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 283, SCREEN_HEIGHT / 2.f - 17), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 283, SCREEN_HEIGHT / 2.f - 17), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_CharacterSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 8), 1, sf::Vector2f(490, 260),
                                                                       sf::FloatRect(0, 0, 160, 260), 165, 0.1f, GUI::Horizontal, &itemInfo_character[Alpha])), "characters_alpha");
    session_character_select->Create("characters_delta");
    session_character_select->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 283, SCREEN_HEIGHT / 2.f - 17), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 283, SCREEN_HEIGHT / 2.f - 17), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_CharacterSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 8), 1, sf::Vector2f(490, 260),
                                                                       sf::FloatRect(0, 0, 160, 260), 165, 0.1f, GUI::Horizontal, &itemInfo_character[Delta])), "characters_delta");
    session_character_select->Create("characters_vortex");
    session_character_select->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 283, SCREEN_HEIGHT / 2.f - 17), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 283, SCREEN_HEIGHT / 2.f - 17), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_CharacterSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 8), 1, sf::Vector2f(490, 260),
                                                                       sf::FloatRect(0, 0, 160, 260), 165, 0.1f, GUI::Horizontal, &itemInfo_character[Vortex])), "characters_vortex");
    session_character_select->Create("characters_omega");
    session_character_select->AddObject(new GUI::Navigator(new GUI::Button(GetGuiStyle("Editor_SmallArrow_Left"), sf::Vector2f(SCREEN_WIDTH / 2.f - 283, SCREEN_HEIGHT / 2.f - 17), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    new GUI::Button(GetGuiStyle("Editor_SmallArrow_Right"), sf::Vector2f(SCREEN_WIDTH / 2.f + 283, SCREEN_HEIGHT / 2.f - 17), 1, rp_editor_smallarrow, "", "", nullptr, 1, sf::IntRect()),
                                                    nullptr,
                                                    new GUI::Selection(GetGuiStyle("Editor_CharacterSelect"), GUI::ResourcePack(), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 8), 1, sf::Vector2f(490, 260),
                                                                       sf::FloatRect(0, 0, 160, 260), 165, 0.1f, GUI::Horizontal, &itemInfo_character[Omega])), "characters_omega");
    session_shipcreate->AddObject(session_character_select, "sc_character_select");

    session_shipcreate->Create("sc_character_playable");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(374, 679), sf::Color::White,
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 255),
                                              GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 10), sf::Vector2f(1, 1), 0)), "sc_character_playable");
    session_shipcreate->AddObject(new GUI::DisplaySprite(&AssetManager::GetTexture("Popup_Head"), sf::IntRect(0, 80, 400, 20),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 339), sf::Vector2f(0.935, 1), 0),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 339), sf::Vector2f(0.935, 1), 255),
                                               GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 339), sf::Vector2f(0.935, 1), 0)), "sc_character_playable");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Close"), sf::Vector2f(SCREEN_WIDTH / 2 + 166, SCREEN_HEIGHT / 2 - 306), 0.5f, rp_editor_mini, "Cancel_Ship_Create", "", nullptr, 1, sf::IntRect()), "sc_character_playable");
    session_shipcreate->AddObject(new GUI::DisplayText("How would you want this\nunit to be used? You can\nmake this unit playable by\nthe player.",
                                                       GetFont("Default"), GetColor("Default_Light"), 25,
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 7, SCREEN_HEIGHT / 2.f - 213), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 7, SCREEN_HEIGHT / 2.f - 213), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 7, SCREEN_HEIGHT / 2.f - 213), sf::Vector2f(1, 1), 0)), "sc_character_playable");
    session_shipcreate->AddObject(new GUI::DisplayText("Or you can lock this unit\nto be used by AI only.",
                                                       GetFont("Default"), GetColor("Default_Light"), 25,
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 15, SCREEN_HEIGHT / 2.f - 12), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 15, SCREEN_HEIGHT / 2.f - 12), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 15, SCREEN_HEIGHT / 2.f - 12), sf::Vector2f(1, 1), 0)), "sc_character_playable");
    session_shipcreate->AddObject(new GUI::DisplayText("You should note that you\ncan have more than one\nplayable unit in a level.\nThe player can switch\nbetween units while\nplaying.",
                                                       GetFont("Default"), GetColor("Default_Light"), 25,
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 14, SCREEN_HEIGHT / 2.f + 226), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 14, SCREEN_HEIGHT / 2.f + 226), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f - 14, SCREEN_HEIGHT / 2.f + 226), sf::Vector2f(1, 1), 0)), "sc_character_playable");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(342, 3), GetColor("Default_Light"),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 59), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 59), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 59), sf::Vector2f(1, 1), 0)), "sc_character_playable");
    session_shipcreate->AddObject(new GUI::DisplayShape(sf::Vector2f(342, 3), GetColor("Default_Light"),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 118), sf::Vector2f(1, 1), 0),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 118), sf::Vector2f(1, 1), 255),
                                                        GUI::DisplayObject::DisplayPhase(sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 118), sf::Vector2f(1, 1), 0)), "sc_character_playable");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 113), 1, rp_editor_plain, "SC_Make_Playable", "Playable", nullptr, 1, sf::IntRect()), "sc_character_playable");
    session_shipcreate->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 58), 1, rp_editor_plain, "SC_Make_Unplayable", "AI Only", nullptr, 1, sf::IntRect()), "sc_character_playable");

    session_shipsetup->Create("defence_option");
    session_shipsetup->AddObject(new GUI::DisplayShape(sf::Vector2f(210, 3), GetColor("Default_Light"),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 243), sf::Vector2f(1, 1), 0),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 243), sf::Vector2f(1, 1), 255),
                                                       GUI::DisplayObject::DisplayPhase(sf::Vector2f(183, 243), sf::Vector2f(1, 1), 0),
                                                       0.3f), "defence_option");
    session_shipsetup->AddObject(new GUI::DisplayText("Select an option for\nthe highlighted\ndefense.",
                                                      GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(179, 53), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(179, 53), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(179, 53), sf::Vector2f(1, 1), 0), 0.3f), "defence_option");
    session_shipsetup->AddObject(new GUI::DisplayText("Setup the defense\nship with these\noptions.",
                                                      GetFont("Default"), GetColor("Default_Light"), 18,
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(173, 303), sf::Vector2f(1, 1), 0),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(173, 303), sf::Vector2f(1, 1), 255),
                                                      GUI::DisplayObject::DisplayPhase(sf::Vector2f(173, 303), sf::Vector2f(1, 1), 0), 0.3f), "defence_option");
    session_shipsetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 195), 1, rp_editor_plain, "Close_Defence_Option", "Back", nullptr, 1, sf::IntRect(), 0.3f), "defence_option");
    session_shipsetup->AddObject(new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 138), 1, rp_editor_plain, "Delete_Defence", "Delete", nullptr, 1, sf::IntRect(), 0.3f), "defence_option");

    GUI::Button* button_do_create = new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 375), 1, rp_editor_plain, "Create_Defence_Unit", "Create", nullptr, 1, sf::IntRect(), 0.3f);
    GUI::Button* button_do_remove = new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 431), 1, rp_editor_plain, "Remove_Defence_Unit", "Remove", nullptr, 1, sf::IntRect(), 0.3f);
    GUI::Button* button_do_rotate = new GUI::Button(GetGuiStyle("Editor_Plain"), sf::Vector2f(183, 488), 1, rp_editor_plain, "Rotate_Defence_Unit", "Rotate", nullptr, 1, sf::IntRect(), 0.3f);
    session_shipsetup->AddObject(button_do_create, "defence_option");
    session_shipsetup->AddObject(button_do_remove, "defence_option");
    session_shipsetup->AddObject(button_do_rotate, "defence_option");

    //Moves world view
    auto handleViewMovement = [&](float dt)
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)) worldViewPos.y -= moveSpeed * dt * scale;
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)) worldViewPos.y += moveSpeed * dt * scale;
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)) worldViewPos.x -= moveSpeed * dt * scale;
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)) worldViewPos.x += moveSpeed * dt * scale;
        }

        if(highlighting)
        {
            if(worldRelativePos.x <= minWorldPos.x)
                worldViewPos.x -= moveSpeed * dt * scale;
            if(Game::GetMousePosition().x >= gameInstance->window.getSize().x - 2)
                worldViewPos.x += moveSpeed * dt * scale;
            if(worldRelativePos.y <= minWorldPos.y)
                worldViewPos.y -= moveSpeed * dt * scale;
            if(Game::GetMousePosition().y >= gameInstance->window.getSize().y - 2)
                worldViewPos.y += moveSpeed * dt * scale;
        }

        float maxMoveX = MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - worldView.getSize().x / 2.f;
        float maxMoveY = MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - worldView.getSize().y / 2.f;

        if(worldViewPos.x > maxMoveX)
            worldViewPos.x = maxMoveX;
        if(worldViewPos.y > maxMoveY)
            worldViewPos.y = maxMoveY;

        if(worldViewPos.x < worldView.getSize().x / 2.f)
            worldViewPos.x = worldView.getSize().x / 2.f;
        if(worldViewPos.y < worldView.getSize().y / 2.f)
            worldViewPos.y = worldView.getSize().y / 2.f;

        sf::Vector2f viewCenter = worldView.getCenter();
        TendTowards(viewCenter.x, worldViewPos.x, 0.1f, 0.1f, dt);
        TendTowards(viewCenter.y, worldViewPos.y, 0.1f, 0.1f, dt);

        worldView.setCenter(viewCenter);

        sf::Vector2f viewTopLeftPos = worldView.getCenter() - worldView.getSize() / 2.f;
        sf::Vector2f viewBottomRightPos = worldView.getCenter() + worldView.getSize() / 2.f;
        sf::Vector2f resetPos = worldView.getCenter();

        if(viewTopLeftPos.x < 0)
            resetPos.x = worldView.getSize().x / 2.f;
        if(viewTopLeftPos.y < 0)
            resetPos.y = worldView.getSize().y / 2.f;
        if(viewBottomRightPos.x > MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE)
            resetPos.x = MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - worldView.getSize().x / 2.f;
        if(viewBottomRightPos.y > MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE)
            resetPos.y = MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE - worldView.getSize().y / 2.f;

        worldView.setCenter(resetPos);
    };

    //Sets the respective view parameters right
    auto fitWorldViewSize = [&](float dt)
    {
        if(HideMenu)
            TendTowards(hideX, 0, 0.1f, 0.1f, dt);
        else TendTowards(hideX, 300, 0.1f, 0.1f, dt);

        menuView.setCenter(455 - hideX, menuView.getCenter().y);
        worldView.setViewport(sf::FloatRect((hideX) / SCREEN_WIDTH, 0, (SCREEN_WIDTH - hideX) / SCREEN_WIDTH, 1));
    };

    //Edit tiles and adjust viewable region
    auto editTile = [&](int yGrid, int xGrid, short int id, bool disableCheck = false)
    {
        if(yGrid >= 0 && yGrid <= MapTileManager::MAX_MAPTILE && xGrid >= 0 && xGrid <= MapTileManager::MAX_MAPTILE && GetGridInfo(yGrid, xGrid).tileId != id)
        {
            if(disableCheck == false)
            {
                if(IsAnyBaseCoreHere(sf::Vector2i(xGrid, yGrid)) == false)
                {
                    if(IsControlUnitSpawnPointHere(sf::Vector2i(xGrid, yGrid)) == true)
                    {
                        AddNotification("Spawn Point Deleted");
                        RemoveSpawnPointFromControlUnit(sf::Vector2i(xGrid, yGrid));
                    }
                    if(IsBaseSpawnPointHere(sf::Vector2i(xGrid, yGrid)) == true)
                    {
                        AddNotification("Base Deleted");
                        DeletBase(GetGridInfo(yGrid, xGrid).baseId);
                    }
                    GetGridInfo(yGrid, xGrid).tileId = id;
                }
                else AddNotification(SmartInsertNotif, "Can't place on a base core.");
            }
            else GetGridInfo(yGrid, xGrid).tileId = id;
        }
    };

    //Handle range highlighting and selection
    auto handleRangeSelection = [&](std::vector<sf::Vector2i>& highlightGrids)->bool
    {
        highlightGrids.clear();
        va_highlighted.clear();

        if(highlighting == false && GetInput(sf::Mouse::Left, Nothing) == false)
            highlightStart = highlightEnd = (sf::Vector2f)worldRelativePos;

        if(GetInput(sf::Mouse::Left, Nothing) == false &&
           (GetInput(sf::Keyboard::LShift, Nothing) == false ||
            (mode == EditWorldMode && (GetInput(sf::Keyboard::LControl, Nothing) == false || GetInput(sf::Keyboard::LAlt, Nothing) == false || GetInput(sf::Keyboard::RControl, Nothing) == false))))
        {
            sf::Color color = sf::Color(255, 255, 255, 100);
            //highlight.setFillColor(sf::Color(255, 255, 255, 40));
            //if(mode == EditBaseMode)
                highlight.setFillColor(sf::Color(0, 0, 0, 0));

            if(GetInput(sf::Mouse::Left, Pressed))
                    highlightStart = highlightEnd = (sf::Vector2f)worldRelativePos;

            if(worldRelativePos.x >= minWorldPos.x && worldRelativePos.y >= minWorldPos.y &&
               worldRelativePos.x < MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE &&
               worldRelativePos.y < MapTileManager::MAX_MAPTILE * MapTileManager::TILE_SIZE)
            {
                if(GetInput(sf::Mouse::Left, Hold))
                    highlightEnd = (sf::Vector2f)worldRelativePos;
            }
            else
            {
                if(worldRelativePos.x < minWorldPos.x)
                    highlightEnd.x = minWorldPos.x;
                else highlightEnd.x = worldRelativePos.x;

                if(worldRelativePos.y < minWorldPos.y)
                    highlightEnd.y = minWorldPos.y;
                else highlightEnd.y = worldRelativePos.y;
            }
            highlighting = true;

            sf::Vector2f highlightSize = highlightEnd - highlightStart;
            highlight.setPosition(highlightStart);
            highlight.setSize(highlightSize);

            sf::Vector2i startGrid = (sf::Vector2i)highlightStart / MapTileManager::TILE_SIZE;
            sf::Vector2i endGrid = (sf::Vector2i)highlightEnd / MapTileManager::TILE_SIZE;
            sf::Vector2i sizeGrid = endGrid - startGrid;

            sf::IntRect range((worldView.getCenter().x - worldView.getSize().x / 2.f) / MapTileManager::TILE_SIZE,
                              (worldView.getCenter().y - worldView.getSize().y / 2.f) / MapTileManager::TILE_SIZE,
                              (worldView.getSize().x / MapTileManager::TILE_SIZE) + 1,
                              (worldView.getSize().y / MapTileManager::TILE_SIZE) + 1);

            bool isOnTileMode = false;
            bool isOnEmptyInfo = false;
            if(menu->GetCurrentLayer() == "tile_mode")
                isOnTileMode = true;
            if(session_tilemodes->GetCurrentLayer() == "empty_info")
                isOnEmptyInfo = true;

            short xSign = 1;
            short ySign = 1;
            if(sizeGrid.x < 0) xSign = -1;
            if(sizeGrid.y < 0) ySign = -1;

            sf::FloatRect viewRect(worldView.getCenter().x - worldView.getSize().x / 2, worldView.getCenter().y - worldView.getSize().y / 2,
                                   worldView.getSize().x, worldView.getSize().y);

            for(int y = 0; y <= sizeGrid.y * ySign; y++)
                for(int x = 0; x <= sizeGrid.x * xSign; x++)
                {
                    bool skip = false;
                    sf::Vector2i grid(startGrid.x + x * xSign, startGrid.y + y * ySign);
                    sf::Vector2i gridPos(highlightStart.y + y * ySign, highlightStart.x + x * xSign);
                    sf::Vector2i startGrid = sf::Vector2i(highlightStart.x, highlightStart.y) / MapTileManager::TILE_SIZE;
                    sf::Vector2i endGrid = sf::Vector2i(highlightEnd.x, highlightEnd.y) / MapTileManager::TILE_SIZE;

                    if((isOnTileMode && isOnEmptyInfo) ||
                       (mode == EditWorldMode && GetInput(sf::Keyboard::RControl, Hold)))
                        color = sf::Color(237, 142, 142, 100);
                    else if(mode == EditWorldMode && (GetInput(sf::Keyboard::LControl, Hold) || GetInput(sf::Keyboard::LControl, Released)))
                    {
                        if(GetGridInfo(grid.y, grid.x).tileId != -1)
                            skip = true;
                    }
                    else if(mode == EditWorldMode && (GetInput(sf::Keyboard::LAlt, Hold) || GetInput(sf::Keyboard::LAlt, Released)))
                    {
                        if(grid.x == startGrid.x || grid.y == startGrid.y || grid.x == endGrid.x || grid.y == endGrid.y)
                            skip = false;
                        else skip = true;
                    }

                    if(placeType == NoPlaceType &&
                       (IsBaseSpawnPointHere(sf::Vector2i(startGrid.x + x * xSign, startGrid.y + y * ySign)) ||
                        IsControlUnitSpawnPointHere(sf::Vector2i(startGrid.x + x * xSign, startGrid.y + y * ySign)) ||
                        IsAnyBaseCoreHere(sf::Vector2i(startGrid.x + x * xSign, startGrid.y + y * ySign))))
                        skip = true;

                    if(skip == false)
                    {
                        if(startGrid.x + x * xSign >= range.left && startGrid.x + x * xSign <= range.left + range.width &&
                           startGrid.y + y * ySign >= range.top && startGrid.y + y * ySign <= range.top + range.height)
                        {
                            sf::FloatRect gridRect((startGrid.x + x * xSign) * MapTileManager::TILE_SIZE, (startGrid.y + y * ySign) * MapTileManager::TILE_SIZE,
                                                   MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE);
                            if(viewRect.intersects(gridRect))
                            {
                                sf::Vertex vertices[4];

                                sf::Vector2f refPos(grid.x * MapTileManager::TILE_SIZE, grid.y * MapTileManager::TILE_SIZE);
                                vertices[0].position = sf::Vector2f(0, 0) + refPos;
                                vertices[1].position = sf::Vector2f(MapTileManager::TILE_SIZE, 0) + refPos;
                                vertices[2].position = sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE) + refPos;
                                vertices[3].position = sf::Vector2f(0, MapTileManager::TILE_SIZE) + refPos;

                                for(int a = 0; a < 4; a++)
                                    vertices[a].color = color;
                                for(int a = 0; a < 4; a++)
                                    va_highlighted.append(vertices[a]);
                            }
                        }

                        highlightGrids.push_back(grid);
                    }
                }

            bool buttonReleased = false;
            if(mode == EditWorldMode && (GetInput(sf::Keyboard::RControl, Hold) || GetInput(sf::Keyboard::RControl, Released)))
            {
                if(GetInput(sf::Keyboard::RControl, Released))
                    buttonReleased = true;
            }
            else if(mode == EditWorldMode && (GetInput(sf::Keyboard::LControl, Hold) || GetInput(sf::Keyboard::LControl, Released)))
            {
                if(GetInput(sf::Keyboard::LControl, Released))
                    buttonReleased = true;
            }
            else if(mode == EditWorldMode && (GetInput(sf::Keyboard::LAlt, Hold) || GetInput(sf::Keyboard::LAlt, Released)))
            {
                if(GetInput(sf::Keyboard::LAlt, Released))
                    buttonReleased = true;
            }
            else if(GetInput(sf::Keyboard::LShift, Released))
                buttonReleased = true;

            if(GetInput(sf::Mouse::Left, Released))
                buttonReleased = true;

            if(buttonReleased)
            {
                highlighting = false;
                return true;
            }
        }
        else highlighting = false;

        return false;
    };

    auto handleRangeSelectionRect = [&](sf::IntRect& range) -> bool
    {
        std::vector<sf::Vector2i> grids;
        sf::Vector2i startGrid = (sf::Vector2i)highlightStart / MapTileManager::TILE_SIZE;
        sf::Vector2i endGrid = (sf::Vector2i)highlightEnd / MapTileManager::TILE_SIZE;
        sf::Vector2i diff = endGrid - startGrid;

        if(diff.x < 0)
        {
            endGrid.x = startGrid.x;
            startGrid.x += diff.x;
        }

        if(diff.y < 0)
        {
            endGrid.y = startGrid.y;
            startGrid.y += diff.y;
        }

        diff = endGrid - startGrid + sf::Vector2i(1, 1);

        if(handleRangeSelection(grids))
        {
            range = sf::IntRect(startGrid.x, startGrid.y, diff.x, diff.y);
            return true;
        }
        return false;
    };

    //Handle single selection
    auto handleSingleSelection = [&](sf::Vector2i& highlightGrid, bool onClick = false)->bool
    {
        if((highlighting == false && worldRelativePos.x >= minWorldPos.x && worldRelativePos.y >= minWorldPos.y) || isCreatingShip || sc_highlighting)
        {
            bool isPlaceValid = true;
            sf::Vector2i mouseGrid = worldRelativePos / MapTileManager::TILE_SIZE;

            if(sc_highlighting && sc_highlightGrid.x >= 0 && sc_highlightGrid.x < MapTileManager::MAX_MAPTILE &&
               sc_highlightGrid.y >= 0 && sc_highlightGrid.y < MapTileManager::MAX_MAPTILE)
                mouseGrid = sc_highlightGrid;

            if(isCreatingShip)
                highlight.setPosition((sf::Vector2f)(sc_position * MapTileManager::TILE_SIZE));
            else highlight.setPosition((sf::Vector2f)(mouseGrid * MapTileManager::TILE_SIZE));

            sf::Color color(255, 255, 255, 40);
            if((menu->GetCurrentLayer() == "tile_mode" && session_tilemodes->GetCurrentLayer() == "empty_info"))
                color = sf::Color(237, 142, 142, 100);
            if(mode == EditBaseMode || mode == EditShipMode)
                color = sf::Color(123, 161, 179, 150);
            if(mode == EditShipMode && shipMode == NoShipMode)
                color = sf::Color(255, 255, 255, 40);

            sf::IntRect pSize;
            EditorPlaceType pType = placeType;

            if(mode == EditShipMode)
            {
                singleSelect_formUnit = GetFormUnit(mouseGrid);
                singleSelect_identity = GetShipIdentity(mouseGrid);

                if(singleSelect_identity != nullptr)
                {
                    color = sf::Color(118, 222, 118, 150);
                    pType = ShipPType;
                }
                else if(singleSelect_formUnit != nullptr)
                {
                    if(singleSelect_formUnit->GetFormType() == FormUnit::FormMainDefence)
                    {
                        pType = MainDefencePType;
                        mouseGrid = singleSelect_formUnit->GetStartGrid();
                        highlight.setPosition((sf::Vector2f)(singleSelect_formUnit->GetStartGrid() * MapTileManager::TILE_SIZE));
                    }
                    else pType = ShipPType;

                    color = sf::Color(118, 222, 118, 150);
                }

                if(GetGridInfoTile(mouseGrid.y, mouseGrid.x).type == MapTileManager::SmallWall &&
                   (shipMode == MainDefenceMode || shipMode == MiniDefenceMode ||
                    (singleSelect_formUnit != nullptr && singleSelect_formUnit->GetFormType() == FormUnit::FormMainDefence)))
                {
                    highlight.move(0, -10);
                    highlightSprite.move(0, -10);
                }

                if(sc_highlighting)
                    color = sf::Color(118, 222, 118, 150);
            }
            else
            {
                sc_shipIdentity = nullptr;
                sc_formUnit = nullptr;
            }

            if(pType == BaseCorePType)
            {
                pSize = sf::IntRect(-1, -1, 3, 3);
                highlightSprite.setTextureRect(sf::IntRect(0, 194, 291, 291));
                highlightSprite.setOrigin(145.5f, 145.5f);
            }
            else if(pType == SpawnPointPType)
            {
                pSize = sf::IntRect(0, 0, 1, 1);
                highlightSprite.setTextureRect(sf::IntRect(0, 0, 97, 97));
                highlightSprite.setOrigin(48.5f, 48.5f);
            }
            else if(pType == MainDefencePType)
            {
                pSize = sf::IntRect(0, 0, 2, 2);
                highlightSprite.setTextureRect(sf::IntRect(97, 0, 194, 194));
                highlightSprite.setOrigin(48.5f, 48.5f);
            }
            else if(pType == MiniDefencePType)
            {
                pSize = sf::IntRect(0, 0, 1, 1);
                highlightSprite.setTextureRect(sf::IntRect(0, 0, 97, 97));
                highlightSprite.setOrigin(48.5f, 48.5f);
            }
            else if(pType == ShipPType)
            {
                pSize = sf::IntRect(0, 0, 1, 1);
                highlightSprite.setTextureRect(sf::IntRect(0, 0, 97, 97));
                highlightSprite.setOrigin(48.5f, 48.5f);
            }
            else if(pType == BossPType)
            {
                pSize = sf::IntRect(-2, -2, 5, 5);
                highlightSprite.setTextureRect(sf::IntRect(291, 0, 485, 485));
                highlightSprite.setOrigin(242.5f, 242.5f);
            }

            if((mouseGrid.x >= 0 && mouseGrid.x < MapTileManager::MAX_MAPTILE &&
               mouseGrid.y >= 0 && mouseGrid.y < MapTileManager::MAX_MAPTILE) || isCreatingShip || sc_highlighting)
            {
                if(pType != NoPlaceType)
                {
                    highlight.setSize(sf::Vector2f(MapTileManager::TILE_SIZE * pSize.width, MapTileManager::TILE_SIZE * pSize.height));
                    highlight.setOrigin(MapTileManager::TILE_SIZE * -pSize.left, MapTileManager::TILE_SIZE * -pSize.top);

                    highlightSprite.setPosition(highlight.getPosition().x + MapTileManager::TILE_SIZE / 2.f,
                                                highlight.getPosition().y + MapTileManager::TILE_SIZE / 2.f);
                    if((worldRelativePos.x >= minWorldPos.x && highlighting == false) || isCreatingShip || sc_highlighting)
                        toDrawHighlightSprite = true;
                }
                else if(GetGridInfoTile(mouseGrid.y, mouseGrid.x).type == MapTileManager::SmallWall)
                {
                    highlight.setSize(sf::Vector2f(100, 110));
                    highlight.setOrigin(0, 10);
                }
                else if(GetGridInfoTile(mouseGrid.y, mouseGrid.x).type == MapTileManager::Wall)
                {
                    highlight.setSize(sf::Vector2f(100, 148));
                    highlight.setOrigin(0, 48);
                }
                else if(pType == BaseCorePType)
                {
                    highlight.setSize(sf::Vector2f(291, 291));
                    highlight.setOrigin(97.f, 97.f);
                }
                else
                {
                    highlight.setSize(sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE));
                    highlight.setOrigin(0, 0);
                }

                if(pType != NoPlaceType && mode != NoMode && mode != EditWorldMode && isCreatingShip == false && sc_highlighting == false)
                {
                    for(int y = 0; y < pSize.height; y++)
                    {
                        bool breakIt = false;
                        for(int x = 0; x < pSize.width; x++)
                        {
                            sf::Vector2i grid(pSize.left + x + mouseGrid.x, pSize.top + y + mouseGrid.y);
                            if(grid.x < 0 || grid.x >= MapTileManager::MAX_MAPTILE || grid.y < 0 || grid.y >= MapTileManager::MAX_MAPTILE)
                            {
                                breakIt = true;
                                break;
                            }

                            if(mode == EditBaseMode)
                            {
                                if(pType != SpawnPointPType && selectedBaseId >= 0 && GetGridInfo(grid.y, grid.x).baseId != selectedBaseId)
                                {
                                    breakIt = true;
                                    break;
                                }
                                if(IsBaseSpawnPointHere(grid))
                                {
                                    breakIt = true;
                                    break;
                                }
                                if(pType == BaseCorePType || pType == SpawnPointPType)
                                    if(MapTileManager::GetTile(GetGridInfo(grid.y, grid.x).tileId).type == MapTileManager::Wall ||
                                       MapTileManager::GetTile(GetGridInfo(grid.y, grid.x).tileId).type == MapTileManager::SmallWall)
                                    {
                                        breakIt = true;
                                        break;
                                    }
                            }
                            else if(mode == EditShipMode)
                            {
                                if(MapTileManager::GetTile(GetGridInfo(grid.y, grid.x).tileId).type == MapTileManager::Wall ||
                                   IsAnyBaseCoreHere(grid) || IsBaseSpawnPointHere(grid) || IsControlUnitSpawnPointHere(grid))
                                {
                                    breakIt = true;
                                    break;
                                }
                                else if((shipMode == MainDefenceMode || shipMode == MiniDefenceMode) &&
                                        MapTileManager::GetTile(GetGridInfo(grid.y, grid.x).tileId).type != MapTileManager::SmallWall)
                                {
                                    breakIt = true;
                                    break;
                                }
                                else if(shipMode == MainDefenceMode && singleSelect_formUnit == nullptr && isOnFormUnit(sf::IntRect(mouseGrid.x, mouseGrid.y, 2, 2)))
                                {
                                    breakIt = true;
                                    break;
                                }
                            }
                        }
                        if(breakIt)
                        {
                            color = sf::Color(237, 142, 142, 150);
                            isPlaceValid = false;
                            break;
                        }
                    }
                }

                highlight.setFillColor(color);
            }

            if(mode == EditShipMode && sc_highlighting)
                mouseGrid = worldRelativePos / MapTileManager::TILE_SIZE;
            highlightGrid = mouseGrid;

            if(worldRelativePos.x >= minWorldPos.x && worldRelativePos.y >= minWorldPos.y && isCreatingShip == false && popUpMenu.GetCurrentLayer() == "")
            {
                if(placeType != NoPlaceType && isPlaceValid == false)
                    return false;
                else if(placeType == NoPlaceType && ((IsControlUnitSpawnPointHere(mouseGrid) || IsBaseSpawnPointHere(mouseGrid)) && GetInput(sf::Keyboard::RControl, Hold)))
                    return false;
                else if(onClick)
                {
                    if(mode == EditShipMode)
                    {
                        singleSelect_formUnit = GetFormUnit(worldRelativePos / MapTileManager::TILE_SIZE);
                        singleSelect_identity = GetShipIdentity(worldRelativePos / MapTileManager::TILE_SIZE);
                    }
                    return GetInput(sf::Mouse::Left, Pressed);
                }
                else return GetInput(sf::Mouse::Left, Hold);
            }
        }
        return false;
    };

    //Handles whatever each mode does respectively
    auto handleMode = [&]
    {
        if(mode == EditWorldMode)
        {
            std::vector<sf::Vector2i> highlightGrids;
            if(handleRangeSelection(highlightGrids))
            {
                if(GetInput(sf::Keyboard::RControl, Hold) || GetInput(sf::Keyboard::RControl, Released))
                    for(int a = 0; a < highlightGrids.size(); a++)
                        editTile(highlightGrids[a].y, highlightGrids[a].x, -1);
                else
                {
                    for(int a = 0; a < highlightGrids.size(); a++)
                        editTile(highlightGrids[a].y, highlightGrids[a].x, selectedTileId);
                }
            }

            sf::Vector2i highlightGrid;
            if(handleSingleSelection(highlightGrid))
            {
                //something was here - single selection delete tile
                if(IsControlUnitSpawnPointHere(highlightGrid) == false && IsBaseSpawnPointHere(highlightGrid) == false)
                    editTile(highlightGrid.y, highlightGrid.x, selectedTileId);
                else AddNotification(SmartInsertNotif, "Can't place tile here.");
            }
        }
        else if(mode == EditBaseMode)
        {
            if(gameMode != BossMode && placeType == SpawnPointPType && handleRangeSelectionRect(baseRangeHighlight))
            {
                if(gameMode == DefendMode)
                {
                    if(tabTeam != playerTeam || playerTeam == -1)
                    {
                        AddNotification("In this mode, only one base must of exist which should be of the player's team.", sf::Color(237, 142, 142));
                        baseRangeHighlight = sf::IntRect();
                    }
                    else if(GetNumberOfBases() >= 1)
                    {
                        AddNotification("Only one base must exist in this mode.", sf::Color(237, 142, 142));
                        baseRangeHighlight = sf::IntRect();
                    }
                }
                if(abs(baseRangeHighlight.width) < 4 || abs(baseRangeHighlight.height) < 4)
                    {
                        if(baseRangeHighlight.width > 1 || baseRangeHighlight.height > 1)
                            Game::AddNotification("Invalid base size. Minimum size is 4x4.", sf::Color(237, 142, 142));

                        baseRangeHighlight = sf::IntRect();
                    }

                if(selectedBaseId >= 0)
                    bs_update = true;
                selectedBaseId = -1;
            }

            else if(placeType == SpawnPointPType)
            {
                sf::Vector2i highlightGrid;
                sf::Vector2i mGrid(worldRelativePos.x / MapTileManager::TILE_SIZE, worldRelativePos.y / MapTileManager::TILE_SIZE);

                if(worldRelativePos.x >= minWorldPos.x && worldRelativePos.y >= minWorldPos.y &&
                   mGrid.x >= 0 && mGrid.x < MapTileManager::MAX_MAPTILE && mGrid.y >= 0 && mGrid.y < MapTileManager::MAX_MAPTILE &&
                   Game::GetInput(sf::Mouse::Left, Pressed) && GetGridInfo(mGrid.y, mGrid.x).baseId >= 0)
                {
                    selectedBaseId = GetGridInfo(mGrid.y, mGrid.x).baseId;
                    baseRangeHighlight = GetBase(selectedBaseId)->GetRange();
                    bs_update = true;
                }
                else if(selectedBaseId < 0 && handleSingleSelection(highlightGrid, true))
                {
                    if(tabTeam >= 0)
                    {
                        if(IsControlUnitSpawnPointHere(highlightGrid) == false)
                        {
                            if(GetGridInfo(highlightGrid.y, highlightGrid.x).baseId == -1 && IsAnyBaseCoreHere(highlightGrid) == false)
                            {
                                if(baseRangeHighlight != sf::IntRect() && selectedTileId >= 0)
                                {
                                    if(highlightGrid.x >= baseRangeHighlight.left && highlightGrid.x < baseRangeHighlight.left + baseRangeHighlight.width &&
                                        highlightGrid.y >= baseRangeHighlight.top && highlightGrid.y < baseRangeHighlight.top + baseRangeHighlight.height)
                                    {
                                        short baseId = Game::CreateBase(tabTeam, highlightGrid, baseRangeHighlight);

                                        if(baseId >= 0)
                                        {
                                            editTile(highlightGrid.y, highlightGrid.x, selectedTileId, true);
                                            selectedBaseId = baseId;
                                            baseRangeHighlight = GetBase(selectedBaseId)->GetRange();
                                            bs_update = true;
                                        }
                                        else
                                        {
                                            AddNotification("Unable to create base.", sf::Color(237, 142, 142));
                                            bs_update = true;
                                        }
                                    }
                                    else AddNotification("Unable to create base. Place Spawn Point within the highlighted region.", sf::Color(237, 142, 142));

                                    if(selectedBaseId < 0)
                                        baseRangeHighlight = sf::IntRect();
                                }
                                else if(gameMode != BattleMode && baseRangeHighlight == sf::IntRect() && selectedTileId >= 0)
                                {
                                    if((tabTeam != playerTeam && gameMode == DefendMode) || gameMode == BossMode)
                                    {
                                        if(AddSpawnPointToControlUnit(highlightGrid, tabTeam) == false)
                                            AddNotification("Unable to place Spawn Point.", sf::Color(237, 142, 142));
                                        else
                                        {
                                            editTile(highlightGrid.y, highlightGrid.x, selectedTileId, true);
                                            selectedTileId = -1;
                                            bs_update = true;
                                        }
                                    }
                                    else if(gameMode == DefendMode)
                                    {
                                        AddNotification("The player team cant have Spawn Points outside the base in this mode.", sf::Color(237, 142, 142));
                                        bs_update = true;
                                    }
                                    else
                                    {
                                        AddNotification("Can't do that.", sf::Color(237, 142, 142));
                                        bs_update = true;
                                    }
                                }
                                else  AddNotification("Can't do that.", sf::Color(237, 142, 142));
                            }
                            else AddNotification("Can't place Spawn Point in another base.", sf::Color(237, 142, 142));
                        }
                        else
                        {
                            AddNotification("Can't place Spawn Point on another Spawn Point.", sf::Color(237, 142, 142));
                            bs_update = true;
                        }
                    }
                    else AddNotification("Select a team first.", sf::Color(237, 142, 142));
                }
            }

            else if(placeType == BaseCorePType)
            {
                sf::Vector2i highlightGrid;

                if(handleSingleSelection(highlightGrid, true))
                {
                    EntityManager::CreateBaseCore(GetBase(selectedBaseId)->GetTeam(), highlightGrid);
                    bs_update = true;
                }
            }

            if(bs_update)
            {
                bs_update = false;
                placeType = SpawnPointPType;

                if(selectedBaseId >= 0)
                {
                    if(GetBase(selectedBaseId)->GetTeam() >= 4)
                        button_makemainbase->SetLock(true);
                    else button_makemainbase->SetLock(false);
                    session_basesetup->TransmitTo("base_options", 0.5f);

                    std::stringstream ss_size;
                    std::stringstream ss_units;
                    ss_size<< GetBase(selectedBaseId)->GetBaseSize();
                    ss_units<< GetBase(selectedBaseId)->GetMaxUnits();

                    displaytext_bsoptions_size->SetText(ss_size.str());
                    displaytext_bsoptions_units->SetText(ss_units.str());
                }
                else if(tabTeam >= 0)
                {
                    if(tabTeam == 0)
                        session_basesetup->TransmitTo("sp_select_alpha", 0.5f);
                    else if(tabTeam == 1)
                        session_basesetup->TransmitTo("sp_select_delta", 0.5f);
                    else if(tabTeam == 2)
                        session_basesetup->TransmitTo("sp_select_vortex", 0.5f);
                    else if(tabTeam == 3)
                        session_basesetup->TransmitTo("sp_select_omega", 0.5f);
                    else if(tabTeam == 4)
                        session_basesetup->TransmitTo("sp_select_all_teams", 0.5f);
                }
                else
                {
                    if(gameMode == BattleMode)
                        session_basesetup->TransmitTo("help_battle_mode", 0.5f);
                    else if(gameMode == DefendMode)
                        session_basesetup->TransmitTo("help_defend_mode", 0.5f);
                    else if(gameMode == BossMode)
                        session_basesetup->TransmitTo("help_boss_mode", 0.5f);
                    else session_basesetup->TransmitTo("");
                }
            }
        }
        else if(mode == EditShipMode)
        {
            sf::Vector2i positionGrid;
            if(handleSingleSelection(positionGrid, true) && isCreatingShip == false)
            {
                if(singleSelect_identity != nullptr)
                {
                    sc_shipIdentity = singleSelect_identity;
                    message = "Ship_Info";
                }
                else if(singleSelect_formUnit != nullptr)
                {
                    sc_formUnit = singleSelect_formUnit;
                    message = "Defence_Option";
                }

                else if(shipMode != NoShipMode && sc_highlighting == false)
                {
                    if(tabTeam >= 0 && tabTeam < NumOfTeams)
                    {
                        sc_position = positionGrid;

                        if(shipMode == MainShipMode)
                        {
                            message = "Create_MainShip";
                            isCreatingShip = true;
                        }
                        else if(shipMode == SubShipMode)
                        {
                            message = "Create_SubShip";
                            isCreatingShip = true;
                        }
                        else if(shipMode == MainDefenceMode)
                            message = "Create_MainDefence";
                        else if(shipMode == MiniDefenceMode)
                            message = "Create_MiniDefence";
                        else if(shipMode == BossMode)
                        {
                            message = "Create_Boss";
                            isCreatingShip = true;
                        }
                    }
                    else Game::AddNotification(SmartInsertNotif, "Can't create ship without a team.", sf::Color(237, 142, 142), sf::Color::White);
                }
            }

            if(session_shipcreate->GetCurrentLayer() == "sc_input_name")
            {
                displaytext_nameInput->SetText(GetTextInput());
                if(GetInput(sf::Keyboard::Return, Released))
                {
                    message = "SC_SubShip_Skin";
                    sc_name = GetTextInput();
                }
            }

            if((tabTeam < 0 || tabTeam >= NumOfTeams) && shipMode != NoShipMode)
                Game::AddNotification(SmartInsertNotif, "Select a team first.", sf::Color(237, 142, 142), sf::Color::White, 0.1);
        }
        else
        {
        }

        hudModeText.setOrigin(hudModeText.getGlobalBounds().width / 2.f, hudModeText.getGlobalBounds().height / 2.f);
    };

    //Handle pop-up menu activities
    auto handlePopupMenu = [&]
    {
        if(popUpMenu.GetCurrentLayer() == "team_option")
        {
            if(updateTeamOption)
            {
                display_teamoption->setTextureRect(sf::IntRect(188, 200 * toUpdateTeamType, 161, 200));

                if(teamType[toUpdateTeamType] != NoTeamType)
                {
                    if(toUpdateTeamType == playerTeam)
                        userType = 0;
                    else userType = 1;
                }
                else if(playerTeam == NoTeamType)
                    userType = 0;
                else userType = 1;

                if(userType == 0)
                    display_teamoption_icon->setTextureRect(sf::IntRect(0, 0, 60, 60));
                else
                {
                    if(teamType[toUpdateTeamType] == NoTeamType || teamType[toUpdateTeamType] == teamType[playerTeam] || playerTeam == NoTeam)
                        display_teamoption_icon->setTextureRect(sf::IntRect(0, 60, 60, 60));
                    else display_teamoption_icon->setTextureRect(sf::IntRect(0, 120, 60, 60));
                }
                updateTeamOption = false;
            }
        }

        if(popUpMenu.GetCurrentLayer() == "setup_teams")
        {
            if(updateTeamSetup)
            {
                updateTeamSetup = false;

                int numOfTeams = 0;
                int lastType = -1;
                float lastPos = 0;
                int order[4] = {-1, -1, -1, -1};
                float orderPos[4] = {0, 0, 0, 0};

                for(int a = 0; a < 4; a++)
                {
                    displaysprite_team_ts[a]->SetInactive();
                    displayshape_lineTeamType[a]->SetInactive();
                    displaysprite_basearrow[a]->SetInactive();
                    displaysprite_statearrow[a]->SetInactive();
                    displaysprite_teamstate_ts[a]->SetInactive();
                    displayshape_miniTeamLine[a]->SetInactive();
                    displaytext_miniTeamType[a]->SetInactive();
                }

                for(int a = 0; a < 4; a++)
                {
                    for(int b = 0; b < 4; b++)
                        if(teamType[b] == a)
                            {
                                numOfTeams++;
                                order[numOfTeams - 1] = b;

                                if(lastType >= 0 && teamType[b] == lastType)
                                {
                                    lastPos += 123;
                                    orderPos[numOfTeams - 1] = lastPos;
                                }
                                else if(lastType >= 0 && teamType[b] != lastType)
                                {
                                    lastPos += 162;
                                    orderPos[numOfTeams - 1] = lastPos;
                                }
                                lastType = a;
                            }
                }

                for(int a = 0; a < numOfTeams; a++)
                {
                    displaysprite_team_ts[order[a]]->SetSinglePhasePosition(sf::Vector2f(SCREEN_WIDTH / 2 - lastPos / 2 + orderPos[a], SCREEN_HEIGHT / 2 + 32));
                    displayshape_lineTeamType[order[a]]->SetSinglePhasePosition(sf::Vector2f(SCREEN_WIDTH / 2 - lastPos / 2 + orderPos[a], SCREEN_HEIGHT / 2 + 116));
                    displaysprite_basearrow[order[a]]->SetSinglePhasePosition(sf::Vector2f(SCREEN_WIDTH / 2 - lastPos / 2 + orderPos[a], SCREEN_HEIGHT / 2 + 111));
                    displaysprite_statearrow[order[a]]->SetSinglePhasePosition(sf::Vector2f(SCREEN_WIDTH / 2 - lastPos / 2 + orderPos[a], SCREEN_HEIGHT / 2 - 40));
                    displaysprite_teamstate_ts[order[a]]->SetSinglePhasePosition(sf::Vector2f(SCREEN_WIDTH / 2 - lastPos / 2 + orderPos[a], SCREEN_HEIGHT / 2 - 65));

                    if(order[a] == playerTeam)
                        displaysprite_teamstate_ts[order[a]]->setTextureRect(sf::IntRect(0, 0, 60, 60));
                    else if(playerTeam != NoTeam && teamType[order[a]] != teamType[playerTeam])
                        displaysprite_teamstate_ts[order[a]]->setTextureRect(sf::IntRect(0, 120, 60, 60));
                    else displaysprite_teamstate_ts[order[a]]->setTextureRect(sf::IntRect(0, 60, 60, 60));

                    if(teamType[order[a]] == TeamTypeA)
                        displaytext_miniTeamType[order[a]]->SetText("A");
                    else if(teamType[order[a]] == TeamTypeB)
                        displaytext_miniTeamType[order[a]]->SetText("B");
                    else if(teamType[order[a]] == TeamTypeC)
                        displaytext_miniTeamType[order[a]]->SetText("C");
                    else if(teamType[order[a]] == TeamTypeD)
                        displaytext_miniTeamType[order[a]]->SetText("D");

                    if(isThereChange_ts >= 0)
                    {
                        GUI::PhaseStage phase = GUI::DefaultPhase;
                        if(isThereChange_ts == 1)
                            phase = GUI::StartPhase;

                        displaysprite_team_ts[order[a]]->SetPhase(phase, true);
                        displayshape_lineTeamType[order[a]]->SetPhase(phase, true);
                        displaysprite_basearrow[order[a]]->SetPhase(phase, true);
                        displaysprite_statearrow[order[a]]->SetPhase(phase, true);
                        displaysprite_teamstate_ts[order[a]]->SetPhase(phase, true);
                        displayshape_miniTeamLine[order[a]]->SetPhase(phase, true);
                        displaytext_miniTeamType[order[a]]->SetPhase(phase, true);
                    }
                    else
                    {
                        displaysprite_team_ts[order[a]]->Reset();
                        displayshape_lineTeamType[order[a]]->Reset();
                        displaysprite_basearrow[order[a]]->Reset();
                        displaysprite_statearrow[order[a]]->Reset();
                        displaysprite_teamstate_ts[order[a]]->Reset();
                        displayshape_miniTeamLine[order[a]]->Reset();
                        displaytext_miniTeamType[order[a]]->Reset();
                    }

                }
                isThereChange_ts = false;

                if(numOfTeams > 0)
                {
                    displayshape_setupTeam->SetInactive();
                    displaytext_setupTeam->SetInactive();
                }
                else
                {
                    displayshape_setupTeam->SetPhase(GUI::DefaultPhase);
                    displaytext_setupTeam->SetPhase(GUI::DefaultPhase);
                }
            }
        }
    };

    //Draws everything concerning the Mode HUD
    auto drawModeHUD = [&]
    {
        if(mode != NoMode)
        {
            if(mode == EditWorldMode)
                hudModeText.setString("Edit World Mode");
            else if(mode == EditBaseMode)
                hudModeText.setString("Edit Base Mode");
            else if(mode == EditShipMode)
                hudModeText.setString("Edit Ship Mode");

            gameInstance->window.draw(hudModeBase);
            gameInstance->window.draw(hudModeText);
        }
    };

    auto drawMode = [&]
    {
        if(mode == EditWorldMode)
        {
        }
        else if(mode == EditBaseMode)
        {
            highlightBase.setPosition(baseRangeHighlight.left * MapTileManager::TILE_SIZE, baseRangeHighlight.top * MapTileManager::TILE_SIZE);
            highlightBase.setSize(sf::Vector2f(baseRangeHighlight.width * MapTileManager::TILE_SIZE, baseRangeHighlight.height * MapTileManager::TILE_SIZE));
            gameInstance->window.draw(highlightBase);

            Game::DrawAllBaseHighlight();
            Game::DrawAllBaseOutline();
        }

        for(int a = 0; a < GetShipIdentitySize(); a++)
            GetShipIdentity(a)->DrawInfo(gameInstance->window,
                                         sf::FloatRect(worldView.getCenter().x - worldView.getSize().x / 2.f, worldView.getCenter().y - worldView.getSize().y / 2.f, worldView.getSize().x, worldView.getSize().y));

        for(int a = 0; a < GetFormUnitSize(); a++)
            GetFormUnit(a)->Draw(gameInstance->window,
                                 sf::FloatRect(worldView.getCenter().x - worldView.getSize().x / 2.f, worldView.getCenter().y - worldView.getSize().y / 2.f, worldView.getSize().x, worldView.getSize().y));
    };

    auto drawHighlight = [&]
    {
        if(highlighting)
            gameInstance->window.draw(va_highlighted);
        gameInstance->window.draw(highlight);

        if(toDrawHighlightSprite)
        {
            gameInstance->window.draw(highlightSprite);
            toDrawHighlightSprite = false;
        }

        va_highlighted.clear();
        highlight.setSize(sf::Vector2f(0, 0));
    };

    auto drawEditorWorld = [&]
    {
        DrawFloors();
        DrawSpawnPoints();
        BaseCore::DrawLower(gameInstance->window);

        if(true)
            Game::DrawAllBaseHighlight();

        DrawSmallWalls_Lower();
        DrawWalls_Lower();
        BaseCore::DrawUpper(gameInstance->window);
        BaseCore::ResetBaseCoresToDraw();
        DrawSmallWalls_Upper();

        for(int a = 0; a < GetShipIdentitySize(); a++)
            GetShipIdentity(a)->DrawInfo(gameInstance->window,
                                         sf::FloatRect(worldView.getCenter().x - worldView.getSize().x / 2.f, worldView.getCenter().y - worldView.getSize().y / 2.f, worldView.getSize().x, worldView.getSize().y));

        for(int a = 0; a < GetFormUnitSize(); a++)
            GetFormUnit(a)->Draw(gameInstance->window,
                                 sf::FloatRect(worldView.getCenter().x - worldView.getSize().x / 2.f, worldView.getCenter().y - worldView.getSize().y / 2.f, worldView.getSize().x, worldView.getSize().y));

        DrawWalls_Upper();
        drawHighlight();

        if(true)
        {
            highlightBase.setPosition(baseRangeHighlight.left * MapTileManager::TILE_SIZE, baseRangeHighlight.top * MapTileManager::TILE_SIZE);
            highlightBase.setSize(sf::Vector2f(baseRangeHighlight.width * MapTileManager::TILE_SIZE, baseRangeHighlight.height * MapTileManager::TILE_SIZE));
            gameInstance->window.draw(highlightBase);

            Game::DrawAllBaseOutline();
        }
    };

    auto updateVertexArray = [&]
    {
        va_grid.clear();
        sf::IntRect range((worldView.getCenter().x - worldView.getSize().x / 2.f) / MapTileManager::TILE_SIZE,
                          (worldView.getCenter().y - worldView.getSize().y / 2.f) / MapTileManager::TILE_SIZE,
                          (worldView.getSize().x / MapTileManager::TILE_SIZE) + 1,
                          (worldView.getSize().y / MapTileManager::TILE_SIZE) + 1);

        if(gridType >= 0)
            for(int y = range.top; y <= range.top + range.height; y++)
                for(int x = range.left; x <= range.left + range.width; x++)
                {
                    if(GetGridInfo(y, x).tileId < 0 &&
                       x >= 0 && x < MapTileManager::MAX_MAPTILE &&
                       y >= 0 && y < MapTileManager::MAX_MAPTILE)
                    {
                        sf::Vertex vertices[4];
                        int refPos_tx = MapTileManager::TILE_SIZE * gridType;

                        sf::Vector2f refPos(x * MapTileManager::TILE_SIZE, y * MapTileManager::TILE_SIZE);
                        vertices[0].position = sf::Vector2f(0, 0) + refPos;
                        vertices[1].position = sf::Vector2f(MapTileManager::TILE_SIZE, 0) + refPos;
                        vertices[2].position = sf::Vector2f(MapTileManager::TILE_SIZE, MapTileManager::TILE_SIZE) + refPos;
                        vertices[3].position = sf::Vector2f(0, MapTileManager::TILE_SIZE) + refPos;

                        vertices[0].texCoords = sf::Vector2f(0 + refPos_tx, 0);
                        vertices[1].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE  + refPos_tx, 0);
                        vertices[2].texCoords = sf::Vector2f(MapTileManager::TILE_SIZE  + refPos_tx, MapTileManager::TILE_SIZE);
                        vertices[3].texCoords = sf::Vector2f(0  + refPos_tx, MapTileManager::TILE_SIZE);

                        for(int a = 0; a < 4; a++)
                            va_grid.append(vertices[a]);
                    }
            }
        UpdateWorldTiles(range);
    };

    auto updateRelativePositions = [&]
    {
        menuRelativePos = (sf::Vector2i)Game::GetMousePosition() + sf::Vector2i(300 - hideX, 0);

        sf::Vector2f portPos(worldView.getViewport().left * Game::SCREEN_WIDTH, worldView.getViewport().top * Game::SCREEN_HEIGHT);
        sf::Vector2f portSize(worldView.getViewport().width * Game::SCREEN_WIDTH, worldView.getViewport().height * Game::SCREEN_HEIGHT);
        sf::Vector2f ratioSize(worldView.getSize().x / portSize.x, worldView.getSize().y / portSize.y);
        sf::Vector2f relativeMousePos = (sf::Vector2f)Game::GetMousePosition();
        relativeMousePos.x *= ratioSize.x;
        relativeMousePos.y *= ratioSize.y;
        portPos.x *= ratioSize.x;
        portPos.y *= ratioSize.y;

        worldRelativePos = (sf::Vector2i)(relativeMousePos + (worldView.getCenter() - worldView.getSize() / 2.f - portPos));

        minWorldPos = worldView.getCenter() - worldView.getSize() / 2.f + sf::Vector2f(10 * ratioSize.x, 0);
    };

    auto handleMessages = [&]
    {
        //Menu session messages

        while(message != "")
        {
            std::string nextMessage = "";

            if(message == "Hide")
                HideMenu = !HideMenu;
            else if(message == "ToExit")
            {
                menu->TransmitTo("", 0.5f);
                HideMenu = true;
            }
            else if(message == "Exit")
                gameInstance->window.close();
            else if(message == "Save")
            {
                gameInstance->gameState = Prototype;
                SetLevelMode(gameMode);
                SetPlayerTeam(playerTeam);
                for(int a = 0; a < 4; a++)
                    SetTeamUpType(a, teamType[a]);

                //Determine base size
                sf::Vector2i levelSize;
                for(int y = 0; y < MapTileManager::MAX_MAPTILE; y++)
                    for(int x = 0; x < MapTileManager::MAX_MAPTILE; x++)
                        if(GetGridInfoTile(y, x).type > MapTileManager::Empty)
                        {
                            if(x + 1 > levelSize.x)
                                levelSize.x = x + 1;
                            if(y + 1 > levelSize.y)
                                levelSize.y = y + 1;
                        }
                SetLevelSize(levelSize);
            }
            else if(message == "Main")
            {
                menu->TransmitTo("main", 0.5f);
                session_tilemodes->TransmitTo("main", 0.5f);
                mode = NoMode;
            }

            else if(message == "Tile_Mode")
            {
                menu->TransmitTo("tile_mode", 0.5f);
                session_tilemodes->TransmitTo("main");
                mode = EditWorldMode;
                placeType = NoPlaceType;
            }
            else if(message == "No_TileType")
                session_tilemodes->TransmitTo("main", 0.5f);
            else if(message == "Floor_Mode")
                session_tilemodes->TransmitTo("floor_list", 0.5f);
            else if(message == "SmallWall_Mode")
                session_tilemodes->TransmitTo("smallWall_list", 0.5f);
            else if(message == "Wall_Mode")
                session_tilemodes->TransmitTo("wall_list", 0.5f);
            else if(message == "Empty_Mode")
            {
                session_tilemodes->TransmitTo("empty_info", 0.5f);
                selectedTileId = -1;
            }
            else if(message == "TileId None")
                selectedTileId = -1;

            else if(message == "Setup")
            {
                mode = NoMode;
                menu->TransmitTo("setup", 0.5f);
                session_basesetup->TransmitTo("", 0.5f);
                session_shipsetup->TransmitTo("", 0.5f);
                highlight.setOutlineColor(sf::Color(255, 255, 255, 100));
                highlight.setFillColor(sf::Color(255, 255, 255, 40));
                sc_highlighting = false;
                isCreatingShip = false;
                selectedBaseId = -1;
                selectedTileId = -1;
                baseRangeHighlight = sf::IntRect();
            }
            else if(message == "Select_Mode")
            {
                popUpMenu.TransmitTo("select_mode");

                if(gameMode != NoGameMode)
                    AddNotification("Note: Changing mode will reset the previous ship and base setup.");

                if(gameMode == BattleMode)
                    button_battlemode->Reset("on-style-reset");
                else if(gameMode == DefendMode)
                    button_defendmode->Reset("on-style-reset");
                else if(gameMode == BossMode)
                    button_bossmode->Reset("on-style-reset");
            }
            else if(message == "SetTo_BattleMode")
            {
                if(gameMode != NoGameMode)
                    resetMode = true;
                gameMode = Game::BattleMode;

                button_bossmode->Reset("default-phase-reset");
                button_defendmode->Reset("default-phase-reset");
            }
            else if(message == "SetTo_DefendMode")
            {
                if(gameMode != NoGameMode)
                    resetMode = true;
                gameMode = Game::DefendMode;

                button_bossmode->Reset("default-phase-reset");
                button_battlemode->Reset("default-phase-reset");
            }
            else if(message == "SetTo_BossMode")
            {
                if(gameMode != NoGameMode)
                    resetMode = true;
                gameMode = Game::BossMode;

                button_battlemode->Reset("default-phase-reset");
                button_defendmode->Reset("default-phase-reset");
            }
            else if(message == "Reset_SelectModeButtons")
            {
                if(gameMode != NoGameMode)
                    resetMode = true;
                gameMode = Game::NoGameMode;

                button_battlemode->Reset("default-phase-reset");
                button_bossmode->Reset("default-phase-reset");
                button_defendmode->Reset("default-phase-reset");
            }

            else if(message == "Setup_Teams")
            {
                popUpMenu.TransmitTo("setup_teams");
                updateTeamSetup = true;
                isThereChange_ts = -1;
            }
            else if(message == "EditTeamType_0")
            {
                popUpMenu.AddInFront("team_option");
                toUpdateTeamType = 0;
                if(teamType[toUpdateTeamType] != NoTeamType)
                    button_teamtype[teamType[toUpdateTeamType]]->Reset("on-style-reset");
                updateTeamOption = true;
            }
            else if(message == "EditTeamType_1")
            {
                popUpMenu.AddInFront("team_option");
                toUpdateTeamType = 1;
                if(teamType[toUpdateTeamType] != NoTeamType)
                    button_teamtype[teamType[toUpdateTeamType]]->Reset("on-style-reset");
                updateTeamOption = true;
            }
            else if(message == "EditTeamType_2")
            {
                popUpMenu.AddInFront("team_option");
                toUpdateTeamType = 2;
                if(teamType[toUpdateTeamType] != NoTeamType)
                    button_teamtype[teamType[toUpdateTeamType]]->Reset("on-style-reset");
                updateTeamOption = true;
            }
            else if(message == "EditTeamType_3")
            {
                popUpMenu.AddInFront("team_option");
                toUpdateTeamType = 3;
                if(teamType[toUpdateTeamType] != NoTeamType)
                    button_teamtype[teamType[toUpdateTeamType]]->Reset("on-style-reset");
                updateTeamOption = true;
            }
            else if(message == "TeamType_A")
            {
                teamType[toUpdateTeamType] = TeamTypeA;
                if(userType == 0)
                    playerTeam = toUpdateTeamType;
                updateTeamOption = true;
                isThereChange_ts = true;
            }
            else if(message == "TeamType_B")
            {
                teamType[toUpdateTeamType] = TeamTypeB;
                if(userType == 0)
                    playerTeam = toUpdateTeamType;
                updateTeamOption = true;
                isThereChange_ts = true;
            }
            else if(message == "TeamType_C")
            {
                teamType[toUpdateTeamType] = TeamTypeC;
                if(userType == 0)
                    playerTeam = toUpdateTeamType;
                updateTeamOption = true;
                isThereChange_ts = true;
            }
            else if(message == "TeamType_D")
            {
                teamType[toUpdateTeamType] = TeamTypeD;
                if(userType == 0)
                    playerTeam = toUpdateTeamType;
                updateTeamOption = true;
                isThereChange_ts = true;
            }
            else if(message == "No_TeamType")
            {
                teamType[toUpdateTeamType] = NoTeamType;
                if(toUpdateTeamType == playerTeam)
                    playerTeam = NoTeam;
                updateTeamOption = true;
                isThereChange_ts = true;
            }
            else if(message == "Switch_TeamOption")
            {
                if(userType == 0)
                {
                    userType = 1;
                    if(toUpdateTeamType == playerTeam)
                        playerTeam = NoTeam;
                }
                else
                {
                    userType = 0;
                    if(teamType[toUpdateTeamType] != NoTeamType)
                        playerTeam = toUpdateTeamType;
                }

                if(userType == 0)
                    display_teamoption_icon->setTexture(&AssetManager::GetTexture("Icon_TeamStatus"), sf::IntRect(0, 0, 60, 60));
                else
                {
                    if(teamType[toUpdateTeamType] == NoTeamType || teamType[toUpdateTeamType] == teamType[playerTeam] || playerTeam == NoTeam)
                        display_teamoption_icon->setTexture(&AssetManager::GetTexture("Icon_TeamStatus"), sf::IntRect(0, 60, 60, 60));
                    else display_teamoption_icon->setTexture(&AssetManager::GetTexture("Icon_TeamStatus"), sf::IntRect(0, 120, 60, 60));
                }
                isThereChange_ts = true;
            }
            else if(message == "Close_TeamOption")
            {
                popUpMenu.MoveBack(0.5f);
                updateTeamSetup = true;
            }

            else if(message == "Back_Popup")
                popUpMenu.MoveBack(0.5f);
            else if(message == "Close_Popup")
            {
                popUpMenu.TransmitTo("", 0.5f);
                nextMessage = "Prepare_Setup";
            }
            else if(message == "Prepare_Setup")
            {
                if(resetMode)
                {
                    resetMode = false;
                    ClearEntireBaseIdGridInfo();
                    DeletAllBases();
                    ResetAllControlUnits();
                }

                if(gameMode != NoGameMode)
                {
                    button_setupBase->SetLock(false);
                    button_setupShip->SetLock(false);
                }
                else
                {
                    button_setupBase->SetLock(true);
                    button_setupShip->SetLock(true);
                }
                button_setupBase->Reset("default-phase-reset");
                button_setupShip->Reset("default-phase-reset");

                int index = 0;
                for(int a = 0; a < 4; a++)
                {
                    if(teamType[a] != NoTeamType)
                    {
                        button_teamtab[a]->SetLock(false);
                        button_teamtab[a]->SetPosition(sf::Vector2f(30.f, 30 + 60 * index));
                        index++;
                    }
                    else button_teamtab[a]->SetLock(true);
                }
            }

            else if(message == "Setup_Base")
            {
                mode = EditBaseMode;
                selectedBaseId = -1;
                selectedTileId = -1;
                baseRangeHighlight = sf::IntRect();

                int index = 0;
                for(int a = 0; a < NumOfTeams; a++)
                    if(teamType[a] != NoTeamType)
                        index++;

                if(gameMode == BattleMode)
                {
                    button_teamtab[4]->SetPosition(sf::Vector2f(30.f, 30 + 60 * index));
                    button_teamtab[4]->SetLock(false);
                }
                else button_teamtab[4]->SetLock(true);

                menu->TransmitTo("setup_section", 0.5f);
                session_shipsetup->TransmitTo("");
                if(gameMode == BattleMode)
                    session_basesetup->TransmitTo("help_battle_mode");
                else if(gameMode == DefendMode)
                    session_basesetup->TransmitTo("help_defend_mode");
                else if(gameMode == BossMode)
                    session_basesetup->TransmitTo("help_boss_mode");
                else session_basesetup->TransmitTo("");
            }
            else if(message == "Team_Tab_0")
            {
                tabTeam = 0;
                if(selectedBaseId < 0)
                {
                    bs_update = true;
                    baseRangeHighlight = sf::IntRect();
                }
            }
            else if(message == "Team_Tab_1")
            {
                tabTeam = 1;
                if(selectedBaseId < 0)
                {
                    bs_update = true;
                    baseRangeHighlight = sf::IntRect();
                }
            }
            else if(message == "Team_Tab_2")
            {
                tabTeam = 2;
                if(selectedBaseId < 0)
                {
                    bs_update = true;
                    baseRangeHighlight = sf::IntRect();
                }
            }
            else if(message == "Team_Tab_3")
            {
                tabTeam = 3;
                if(selectedBaseId < 0)
                {
                    bs_update = true;
                    baseRangeHighlight = sf::IntRect();
                }
            }
            else if(message == "Team_Tab_4")
            {
                tabTeam = 4;
                if(selectedBaseId < 0)
                {
                    bs_update = true;
                    baseRangeHighlight = sf::IntRect();
                }
            }
            else if(message == "No_Team_Tab")
            {
                tabTeam = -1;
                if(selectedBaseId < 0)
                {
                    bs_update = true;
                    baseRangeHighlight = sf::IntRect();
                }
            }

            else if(message == "Delete_Base")
            {
                AddNotification("Base Deleted");
                Game::DeletBase(selectedBaseId);
                selectedBaseId = -1;
                bs_update = true;
                baseRangeHighlight = sf::IntRect();
            }
            else if(message == "Make_MainBase")
            {

                sf::Texture* textr = nullptr;
                if(GetBase(selectedBaseId)->GetTeam() == Alpha)
                    textr = &AssetManager::GetTexture("BaseCore_Alpha");
                else if(GetBase(selectedBaseId)->GetTeam() == Delta)
                    textr = &AssetManager::GetTexture("BaseCore_Delta");
                else if(GetBase(selectedBaseId)->GetTeam() == Vortex)
                    textr = &AssetManager::GetTexture("BaseCore_Vortex");
                else if(GetBase(selectedBaseId)->GetTeam() == Omega)
                    textr = &AssetManager::GetTexture("BaseCore_Omega");

                displaysprite_mainbaseoption_base->setTexture(textr, sf::IntRect(0, 0, 294, 306));
                displaysprite_mainbaseoption_rotor->setTexture(textr, sf::IntRect(294, 0, 150, 150));
                displaysprite_mainbaseoption_cover->setTexture(textr, sf::IntRect(444, 0, 174, 184));

                placeType = BaseCorePType;
                session_basesetup->TransmitTo("mainbase_option", 0.5f);
            }
            else if(message == "Close_MainBase_Options")
            {
                selectedBaseId = -1;
                selectedTileId = -1;
                baseRangeHighlight = sf::IntRect();
                bs_update = true;
            }

            else if(message == "Set_Max_Units")
            {
                session_basesetup->TransmitTo("unit_option", 0.5f);

                std::stringstream ss;
                if(selectedBaseId >= 0)
                    ss<< GetBase(selectedBaseId)->GetMaxUnits();
                displaytext_unitoption_units->SetText(ss.str());
            }
            else if(message == "Auto_Unit_Set")
            {
                std::stringstream ss;
                if(selectedBaseId >= 0)
                {
                    GetBase(selectedBaseId)->SetDefaultMaxUnits();
                    ss<< GetBase(selectedBaseId)->GetMaxUnits();
                }
                displaytext_unitoption_units->SetText(ss.str());
            }
            else if(message == "BaseSize--")
            {
                std::stringstream ss;
                if(selectedBaseId >= 0)
                {
                    GetBase(selectedBaseId)->SetMaxUnits(GetBase(selectedBaseId)->GetMaxUnits() - 5);
                    ss<< GetBase(selectedBaseId)->GetMaxUnits();
                }
                displaytext_unitoption_units->SetText(ss.str());
            }
            else if(message == "BaseSize++")
            {
                std::stringstream ss;
                if(selectedBaseId >= 0)
                {
                    GetBase(selectedBaseId)->SetMaxUnits(GetBase(selectedBaseId)->GetMaxUnits() + 5);
                    ss<< GetBase(selectedBaseId)->GetMaxUnits();
                }
                displaytext_unitoption_units->SetText(ss.str());
            }
            else if(message == "Close_Unit_Option")
                bs_update = true;

            else if(message == "Close_Base_Options")
            {
                selectedBaseId = -1;
                selectedTileId = -1;
                baseRangeHighlight = sf::IntRect();
                bs_update = true;
            }

            else if(message == "Setup_Ship")
            {
                button_teamtab[4]->SetLock(true);

                menu->TransmitTo("setup_section", 0.5f);
                session_basesetup->TransmitTo("");
                session_shipsetup->TransmitTo("ship_type_select");
                mode = EditShipMode;
                sc_highlighting = false;
            }
            else if(message == "Main_Ship")
            {
                shipMode = MainShipMode;
                placeType = ShipPType;

                highlightSprite.setTextureRect(sf::IntRect(0, 0, 97, 97));
                highlightSprite.setOrigin(48.5f, 48.5f);

                session_shipsetup->TransmitTo("ship_placement", 0.5f);
            }
            else if(message == "Sub_Ship")
            {
                shipMode = SubShipMode;
                placeType = ShipPType;

                session_shipsetup->TransmitTo("ship_placement", 0.5f);
            }
            else if(message == "Mini_Defenses")
            {
                shipMode = MiniDefenceMode;
                placeType = ShipPType;

                highlightSprite.setTextureRect(sf::IntRect(0, 0, 97, 97));
                highlightSprite.setOrigin(48.5f, 48.5f);

                session_shipsetup->TransmitTo("ship_placement", 0.5f);
            }
            else if(message == "Main_Defenses")
            {
                shipMode = MainDefenceMode;
                placeType = MainDefencePType;

                highlightSprite.setTextureRect(sf::IntRect(97, 0, 194, 194));
                highlightSprite.setOrigin(48.5f, 48.5f);

                session_shipsetup->TransmitTo("ship_placement", 0.5f);
            }
            else if(message == "Boss_Ship")
            {
                shipMode = BossShipMode;
                placeType = BossPType;

                highlightSprite.setTextureRect(sf::IntRect(291, 0, 485, 485));
                highlightSprite.setOrigin(242.5f, 242.5f);

                session_shipsetup->TransmitTo("ship_placement", 0.5f);
            }
            else if(message == "Ship_Type_Select")
            {
                session_shipsetup->TransmitTo("ship_type_select", 0.5f);
                session_shipcreate->TransmitTo("");
                placeType = NoPlaceType;
                shipMode = NoShipMode;
                sc_highlighting = false;
            }
            else if(message == "Ship_Info")
            {
                if(sc_shipIdentity != nullptr && session_shipsetup->GetCurrentLayer() != "ship_info")
                    session_shipsetup->TransmitTo("ship_info", 0.5f);

                if(sc_shipIdentity != nullptr)
                    displaysprite_shipinfo->setTexture(&sc_shipIdentity->GetShipInfo()->texture);

                sc_highlighting = true;
                sc_highlightGrid = sc_shipIdentity->GetStartGrid();
            }
            else if(message == "Defence_Option")
            {
                if(sc_formUnit != nullptr)
                {
                    if(session_shipsetup->GetCurrentLayer() != "defence_option")
                    session_shipsetup->TransmitTo("defence_option", 0.5f);

                    if(sc_formUnit->GetSkinIndex() >= 0)
                    {
                        button_do_create->SetLock(true);
                        button_do_remove->SetLock(false);
                        button_do_rotate->SetLock(false);
                    }
                    else
                    {
                        button_do_create->SetLock(false);
                        button_do_remove->SetLock(true);
                        button_do_rotate->SetLock(true);
                    }

                    sc_highlighting = true;
                    sc_highlightGrid = sc_formUnit->GetStartGrid();
                }
            }
            else if(message == "Close_Ship_Info" || message == "Close_Defence_Option")
            {
                if(shipMode != NoShipMode)
                    session_shipsetup->TransmitTo("ship_placement", 0.5f);
                else nextMessage = "Ship_Type_Select";

                sc_highlighting = false;
            }
            else if(message == "Rotate_Ship")
            {
                if(sc_shipIdentity != nullptr)
                    sc_shipIdentity->Rotate();
            }
            else if(message == "Delete_Ship")
            {
                RemoveShipIdentity(sc_shipIdentity);

                sc_shipIdentity = nullptr;
                sc_highlighting = false;
                displaysprite_shipinfo->setTexture(nullptr);
                nextMessage = "Close_Ship_Info";
            }
            else if(message == "Create_Defence_Unit")
            {
                popUpMenu.TransmitTo("sc_session");
                if(sc_formUnit->GetFormType() == FormUnit::FormMainDefence)
                    session_shipcreate->TransmitTo("sc_maindefence_skin");
                else if(sc_formUnit->GetFormType() == FormUnit::FormMiniDefence)
                    session_shipcreate->TransmitTo("sc_minidefence_skin");

                if(sc_formUnit->GetFormType() == FormUnit::FormMainDefence)
                {
                    if(sc_formUnit->GetTeam() == Alpha)
                        session_skinmaindefence->TransmitTo("alpha");
                    else if(sc_formUnit->GetTeam() == Delta)
                        session_skinmaindefence->TransmitTo("delta");
                    else if(sc_formUnit->GetTeam() == Vortex)
                        session_skinmaindefence->TransmitTo("vortex");
                    else if(sc_formUnit->GetTeam() == Omega)
                        session_skinmaindefence->TransmitTo("omega");
                }
                else if(sc_formUnit->GetFormType() == FormUnit::FormMiniDefence)
                    {
                    if(sc_formUnit->GetTeam() == Alpha)
                        session_skinminidefence->TransmitTo("alpha");
                    else if(sc_formUnit->GetTeam() == Delta)
                        session_skinminidefence->TransmitTo("delta");
                    else if(sc_formUnit->GetTeam() == Vortex)
                        session_skinminidefence->TransmitTo("vortex");
                    else if(sc_formUnit->GetTeam() == Omega)
                        session_skinminidefence->TransmitTo("omega");
                }
            }
            else if(message == "Remove_Defence_Unit")
            {
                sc_formUnit->SetSkinIndex(-1);
                nextMessage = "Defence_Option";
            }
            else if(message == "Rotate_Defence_Unit")
            {
                sc_formUnit->Rotate();
            }
            else if(message == "Delete_Defence")
            {
                RemoveFormUnit(sc_formUnit);
                nextMessage = "Close_Defence_Option";
                sc_formUnit = nullptr;
            }

            else if(message == "Cancel_Ship_Create")
            {
                isCreatingShip = false;
                popUpMenu.TransmitTo("", 0.5f);
            }
            else if(message == "Create_MainShip")
            {
                popUpMenu.TransmitTo("sc_session");
                if(tabTeam == playerTeam)
                    session_shipcreate->TransmitTo("sc_mainship_option_1_pteam");
                else session_shipcreate->TransmitTo("sc_mainship_option_1");
            }
            else if(message == "Create_SubShip")
            {
                SetTextInputState(WordInputState);
                popUpMenu.TransmitTo("sc_session");
                session_shipcreate->TransmitTo("sc_input_name");
            }
            else if(message == "Create_MainDefence")
            {
                nextMessage = "Defence_Option";
                sc_formUnit = AddFormUnit(new FormUnit(FormUnit::FormMainDefence, sc_position, tabTeam, -1));
            }
            else if(message == "Create_MiniDefence")
            {
                nextMessage = "Defence_Option";
                sc_formUnit = AddFormUnit(new FormUnit(FormUnit::FormMiniDefence, sc_position, tabTeam, -1));
            }

            else if(message == "SC_SubShip_Skin")
            {
                session_shipcreate->TransmitTo("sc_subship_skin", 0.5f);
                if(tabTeam == Alpha)
                    session_skinsubship->TransmitTo("skinsubship_alpha");
                else if(tabTeam == Delta)
                    session_skinsubship->TransmitTo("skinsubship_delta");
                else if(tabTeam == Vortex)
                    session_skinsubship->TransmitTo("skinsubship_vortex");
                else if(tabTeam == Omega)
                    session_skinsubship->TransmitTo("skinsubship_omega");
                else session_skinsubship->TransmitTo("");
            }
            else if(message == "SC_Randomize_Att")
            {
                sc_specialFactor = GenerateRandomNumber(101) / 100.f;
                sc_speedFactor = GenerateRandomNumber(101) / 100.f;
            }
            else if(message == "SC_Form_Ship")
            {
                if(shipMode == MainShipMode)
                {
                    sc_shipIdentity = AddShipIdentity(AddCharacterInfo(new ShipInfo(sc_name,ShipInfo::NoShipClass, 0, sc_skin, tabTeam, SkinSubShip, 300, 300, 300, sc_speedFactor * 100, sc_specialFactor * 100), ShipInfo::TemporaryCharacterType), sc_position, true);
                    UpdateCharacterTexture(ShipInfo::TemporaryCharacterType, GetCharacterTypeInfoSize(ShipInfo::TemporaryCharacterType, tabTeam) - 1, tabTeam, GetColor("Default_Light"));
                }
                else if(shipMode == SubShipMode)
                {
                    sc_shipIdentity = AddShipIdentity(AddCharacterInfo(new ShipInfo(sc_name,ShipInfo::NoShipClass, 0, sc_skin, tabTeam, SkinSubShip, 300, 300, 300, sc_speedFactor * 100, sc_specialFactor * 100), ShipInfo::TemporaryCharacterType), sc_position, false);
                    UpdateCharacterTexture(ShipInfo::TemporaryCharacterType, GetCharacterTypeInfoSize(ShipInfo::TemporaryCharacterType, tabTeam) - 1, tabTeam, GetColor("Default_Light"));
                }

                isCreatingShip = false;
                popUpMenu.TransmitTo("", 0.5f);
                nextMessage = "Ship_Info";
            }
            else if(message == "SC_Input_Name")
            {
                session_shipcreate->TransmitTo("sc_input_name", 0.5f);
                SetTextInputState(WordInputState);
            }
            else if(message == "SC_Character_Select")
            {
                session_shipcreate->TransmitTo("sc_character_select", 0.5f);
                if(tabTeam == Alpha)
                    session_character_select->TransmitTo("characters_alpha");
                else if(tabTeam == Delta)
                    session_character_select->TransmitTo("characters_delta");
                else if(tabTeam == Vortex)
                    session_character_select->TransmitTo("characters_vortex");
                else if(tabTeam == Omega)
                    session_character_select->TransmitTo("characters_omega");
                else session_character_select->TransmitTo("");
            }
            else if(message == "SC_Make_Playable")
            {
                sc_shipIdentity = AddShipIdentity(GetCharacterInfo(ShipInfo::MainCharacterType, sc_characterIndex, tabTeam), sc_position, true, true);
                isCreatingShip = false;
                popUpMenu.TransmitTo("", 0.5f);
                nextMessage = "Ship_Info";
            }
            else if(message == "SC_Make_Unplayable")
            {
                sc_shipIdentity = AddShipIdentity(GetCharacterInfo(ShipInfo::MainCharacterType, sc_characterIndex, tabTeam), sc_position, true);
                isCreatingShip = false;
                popUpMenu.TransmitTo("", 0.5f);
                sc_highlightGrid = sc_position;
                nextMessage = "Ship_Info";
            }

            //Mode messages
            if(mode != NoMode)
            {
                std::stringstream ss;
                std::string str;

                ss << message;
                ss >> str;

                if(str == "TileId")
                    ss >> selectedTileId;
                else if(str == "SkinSubShip")
                {
                    ss >> sc_skin;
                    session_shipcreate->TransmitTo("sc_attributes", 0.5f);
                    sc_specialFactor = sc_speedFactor = 0.5f;
                }
                else if(str == "SkinMainDefence" || str == "SkinMiniDefence")
                {
                    ss >> sc_skin;
                    sc_formUnit->SetSkinIndex(sc_skin);

                    isCreatingShip = false;
                    popUpMenu.TransmitTo("", 0.5f);
                    nextMessage = "Defence_Option";
                }
                else if(str == "Info")
                {
                    ss >> sc_characterIndex;
                    session_shipcreate->TransmitTo("sc_character_playable", 0.5f);
                }
            }

            message = nextMessage;
        }
    };

    float deltaTime = 0.f;
    float dtFPS = 0.f;

    Game::AddNotification("Welcome to the level editor. You can create levels of your own here.");
    clock.restart();

    //Remove later
    teamType[Alpha] = TeamTypeA;
    teamType[Delta] = TeamTypeB;
    teamType[Vortex] = TeamTypeC;
    teamType[Omega] = TeamTypeD;
    playerTeam = Alpha;
    gameMode = BattleMode;
    message = "Prepare_Setup";

    //editTile(59, 59, 5);

    /*{
        for(int y = 0; y < 13; y++)
            for(int x = 0; x < 13; x++)
            {
                editTile(1 + y, 1 + x, 1);
                editTile(1 + 13 + 6 + y, 1 + x, 1);
                editTile(1 + 13 + 6 + y, 1 + 13 + 6 + x, 1);
                editTile(1 + y, 1 + 13 + 6 + x, 1);
            }
        for(int y = 0; y < 12; y++)
            for(int x = 0; x < 12; x++)
                editTile(11 + y, 11 + x, -1);
        for(int y = 0; y < 10; y++)
            for(int x = 0; x < 10; x++)
                editTile(12 + y, 12 + x, 1);
        for(int y = 0; y < 4; y++)
            for(int x = 0; x < 13 * 2 + 6; x++)
            {
                editTile(1 + 14 + y, 1 + x, 1);
                editTile(1 + x, 1 + 14 + y, 1);
            }
        for(int a = 0; a < 4; a++)
        {
            editTile(1 + a, 14, 1);
            editTile(1 + 13 + 6 + 9 + a, 19, 1);
            editTile(14 + 5, 1 + a, 1);
            editTile(14, 1 + 13 + 6 + 9 + a, 1);
        }
        for(int y = 0; y < 13 * 2 + 8; y++)
            for(int x = 0; x < 13 * 2 + 8; x++)
                if(GetGridInfo(y, x).tileId == -1)
                    editTile(y, x, 5);
        for(int y = 0; y < 2; y++)
            for(int x = 0; x < 2; x++)
            {
                editTile(16 + y, 16 + x, 14);
                editTile(2 + y, 2 + x, 14);
                editTile(2 + y, 14 + 6 + 10 + x, 14);
                editTile(14 + 6 + 10 + y, 2 + x, 14);
                editTile(14 + 6 + 10 + y, 14 + 6 + 10 + x, 14);
            }
        editTile(9, 12, 14);
        editTile(24, 12, 14);
        editTile(9, 21, 14);
        editTile(24, 21, 14);
        editTile(12, 9, 14);
        editTile(21, 9, 14);
        editTile(12, 24, 14);
        editTile(21, 24, 14);

        Game::GetBase(Game::CreateBase(0, sf::Vector2i(7, 7), sf::IntRect(1, 1, 14, 14)))->SetMaxUnits(120);
        Game::GetBase(Game::CreateBase(1, sf::Vector2i(27, 7), sf::IntRect(20, 1, 14, 14)))->SetMaxUnits(120);
        Game::GetBase(Game::CreateBase(2, sf::Vector2i(7, 27), sf::IntRect(1, 20, 14, 14)))->SetMaxUnits(120);
        Game::GetBase(Game::CreateBase(3, sf::Vector2i(27, 27), sf::IntRect(20, 20, 14, 14)))->SetMaxUnits(120);

        editTile(7, 7, 22, true);
        editTile(7, 26, 23, true);
        editTile(26, 7, 24, true);
        editTile(26, 26, 25, true);

        EntityManager::CreateBaseCore(0, sf::Vector2i(3, 11));
        EntityManager::CreateBaseCore(1, sf::Vector2i(22, 3));
        EntityManager::CreateBaseCore(2, sf::Vector2i(11, 30));
        EntityManager::CreateBaseCore(3, sf::Vector2i(30, 22));

        AddFormUnit(new FormUnit(FormUnit::FormMainDefence, sf::Vector2i(2, 2), 0, 0, 3));
        AddFormUnit(new FormUnit(FormUnit::FormMainDefence, sf::Vector2i(2, 30), 2, 0, 1));
        AddFormUnit(new FormUnit(FormUnit::FormMainDefence, sf::Vector2i(30, 2), 1, 0, 5));
        AddFormUnit(new FormUnit(FormUnit::FormMainDefence, sf::Vector2i(30, 30), 3, 0, 7));
        AddFormUnit(new FormUnit(FormUnit::FormMiniDefence, sf::Vector2i(9, 12), 0, 0, 7));
        AddFormUnit(new FormUnit(FormUnit::FormMiniDefence, sf::Vector2i(24, 12), 1, 0, 1));
        AddFormUnit(new FormUnit(FormUnit::FormMiniDefence, sf::Vector2i(9, 21), 2, 0, 5));
        AddFormUnit(new FormUnit(FormUnit::FormMiniDefence, sf::Vector2i(24, 21), 3, 0, 3));
        AddFormUnit(new FormUnit(FormUnit::FormMiniDefence, sf::Vector2i(12, 9), 0, 0, 7));
        AddFormUnit(new FormUnit(FormUnit::FormMiniDefence, sf::Vector2i(21, 9), 1, 0, 1));
        AddFormUnit(new FormUnit(FormUnit::FormMiniDefence, sf::Vector2i(12, 24), 2, 0, 5));
        AddFormUnit(new FormUnit(FormUnit::FormMiniDefence, sf::Vector2i(21, 24), 3, 0, 3));

        AddShipIdentity(GetCharacterInfo(ShipInfo::MainCharacterType, 0, 0), sf::Vector2i(5, 5), true, true, 1);
        AddShipIdentity(AddCharacterInfo(new ShipInfo("Duke", ShipInfo::NoShipClass, 0, 1, 0, SkinSubShip, 300, 300, 300, 70, 60), ShipInfo::TemporaryCharacterType), sf::Vector2i(9, 9), false, false, 1);
        AddShipIdentity(AddCharacterInfo(new ShipInfo("Volt", ShipInfo::NoShipClass, 0, 0, 1, SkinSubShip, 300, 300, 300, 70, 60), ShipInfo::TemporaryCharacterType), sf::Vector2i(24, 9), false, false, 3);
        AddShipIdentity(AddCharacterInfo(new ShipInfo("Fluffy", ShipInfo::NoShipClass, 0, 1, 1, SkinSubShip, 300, 300, 300, 70, 60), ShipInfo::TemporaryCharacterType), sf::Vector2i(28, 5), false, false, 3);
        AddShipIdentity(AddCharacterInfo(new ShipInfo("Dave", ShipInfo::NoShipClass, 0, 0, 2, SkinSubShip, 300, 300, 300, 70, 60), ShipInfo::TemporaryCharacterType), sf::Vector2i(9, 24), false, false, 7);
        AddShipIdentity(AddCharacterInfo(new ShipInfo("Kattie", ShipInfo::NoShipClass, 0, 1, 2, SkinSubShip, 300, 300, 300, 70, 60), ShipInfo::TemporaryCharacterType), sf::Vector2i(5, 28), false, false, 7);
        AddShipIdentity(AddCharacterInfo(new ShipInfo("Fraction", ShipInfo::NoShipClass, 0, 0, 3, SkinSubShip, 300, 300, 300, 70, 60), ShipInfo::TemporaryCharacterType), sf::Vector2i(24, 24), false, false, 5);
        AddShipIdentity(AddCharacterInfo(new ShipInfo("Zack", ShipInfo::NoShipClass, 1, 1, 3, SkinSubShip, 300, 300, 300, 70, 60), ShipInfo::TemporaryCharacterType), sf::Vector2i(28, 28), false, false, 5);

        UpdateCharacterTexture(GetColor("Default_Light"));
        message = "Save";
        handleMessages();
    }*/
    //AddShipIdentity(GetCharacterInfo(ShipInfo::MainCharacterType, 0, 0), sf::Vector2i(1, 1), true, true);
    /*for(int a = 0; a < 10; a++)
        AddFormUnit(new FormUnit(FormUnit::FormMainDefence, sf::Vector2i(5 + 8 * a, 5), a >= 5 ? 1 : 0, 0));

    for(int a = 0; a < 10; a++)
        AddFormUnit(new FormUnit(FormUnit::FormMiniDefence, sf::Vector2i(5 + 6 * a, 15), a >= 5 ? 1 : 0, 0));*/

    //Editor loop
    while(gameInstance->window.isOpen() && gameInstance->gameState == MapEdit)
    {
        dtFPS = clock.getElapsedTime().asSeconds();
        deltaTime = clock.restart().asSeconds();

        if(deltaTime > MIN_FRAMERATE)
            deltaTime = MIN_FRAMERATE;

        if(true)
        {
            //Handle inputs
            HandleInputs();

            //Update
            updateRelativePositions();

            if(Game::GetInput(sf::Keyboard::Space, Released))
                HideMenu = !HideMenu;
            if(Game::GetInput(sf::Keyboard::RShift, Released))
            {
                gridType++;
                if(gridType > 1)
                    gridType = -1;
            }

            for(int a = 0; a < 4; a++)
                if(EntityManager::GetBaseCore(a) != nullptr)
                    EntityManager::GetBaseCore(a)->Update(deltaTime,
                                                          sf::FloatRect(worldView.getCenter().x - worldView.getSize().x / 2.f,
                                                                        worldView.getCenter().y - worldView.getSize().y / 2.f,
                                                                        worldView.getSize().x, worldView.getSize().y));

            worldView.setSize((SCREEN_WIDTH - hideX) * scale, SCREEN_HEIGHT * scale);
            handleViewMovement(deltaTime);
            fitWorldViewSize(deltaTime);

            handleMode();
            updateVertexArray();
            Game::UpdateNotification(deltaTime);

            //Handle Message
            handleMessages();

            if(popUpMenu.GetCurrentLayer() != "")
            {
                if(popUpMenu.HandleInput((sf::Vector2i)GetMousePosition(), Game::GetInput(sf::Mouse::Left, Pressed), deltaTime))
                    message = popUpMenu.GetMessage();
                handlePopupMenu();

                menuMain.HandleInput(menuRelativePos, Game::GetInput(sf::Mouse::Left, Pressed), deltaTime, false);
            }
            else if(menuMain.HandleInput(menuRelativePos, Game::GetInput(sf::Mouse::Left, Pressed), deltaTime))
                message = menuMain.GetMessage();

            //Handle View Scaling
            if(Game::isMouseScrolled() && GUI::GetMouseScrollDelta() != 0.f)
                worldViewScale += -Game::GetMouseScrollDelta() * 0.4f;
            if(Game::GetInput(sf::Keyboard::PageUp, Hold))
                TendTowards(worldViewScale, minScale, scaleSpeed, deltaTime);
            if(Game::GetInput(sf::Keyboard::PageDown, Hold))
                TendTowards(worldViewScale, maxScale, scaleSpeed, deltaTime);

            if(worldViewScale < minScale) worldViewScale = minScale;
            if(worldViewScale > maxScale) worldViewScale = maxScale;
            TendTowards(scale, worldViewScale, 0.1f, 0, deltaTime);

            //Draw
            gameInstance->window.clear(sf::Color(163, 190, 203));

            gameInstance->window.setView(worldView);
            gameInstance->window.draw(va_grid, &tileGrid);

            drawEditorWorld();
            //drawMode();

            gameInstance->window.setView(menuView);
            menuMain.Draw(gameInstance->window);

            gameInstance->window.setView(hudView);
            drawModeHUD();

            if(popUpMenu.GetCurrentLayer() != "")
            {
                gameInstance->window.setView(hudView);
                popUpMenu.Draw(gameInstance->window);
            }

            Game::DrawNotification();
            Game::DrawFpsDisplay();
            Game::DrawCursor();
            //Update FPS Display
            UpdateFpsDisplay(dtFPS, clock.getElapsedTime().asSeconds());
            dtFPS = 0;

            gameInstance->window.display();
        }
    }
}
