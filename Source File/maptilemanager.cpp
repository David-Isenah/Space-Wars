#include "maptilemanager.h"
#include "game.h"
#include "AssetManager.h"
#include "astarpathfinding.h"
#include "collision.h"
#include "fstream"
#include "iostream"
#include <assert.h>

const sf::Vector2f FLOOR_SIZE_IMG(100, 100);
const sf::Vector2f SMALL_WALL_SIZE_IMG(100, 110);
const sf::Vector2f WALL_SIZE_IMG(100, 148);
const int TILESHEET_PADDING = 2;
const float LINE_SIGHT_WIDTH = 4;
const int numberOfTilesPerLine = 8;

int transparentSmallWallId = 0;

std::vector<MapTileManager::Tile> gameTiles;

MapTileManager* MapTileManager::instance = nullptr;
MapTileManager mapTileManagerInstance;

int MapTileManager::AddTile(TileType type_, short int textureIndex_, short int availableRank_, std::string info_)
{
    gameTiles.push_back(Tile(type_, textureIndex_, availableRank_, info_));
    return gameTiles.size() - 1;
}

void MapTileManager::InitialiseTiles()
{
    //Empty Tile
    AddTile(MapTileManager::Empty, 0);

    //Floor Tiles
    AddTile(Floor, 0);
    AddTile(Floor, 1);
    AddTile(Floor, 2, 4);

    //Wall Tiles
    AddTile(Wall, 0);
    AddTile(Wall, 1);
    AddTile(Wall, 2);
    AddTile(Wall, 3);
    AddTile(Wall, 4);
    AddTile(Wall, 5);
    AddTile(Wall, 6);
    AddTile(Wall, 7);
    AddTile(Wall, 8, 4);

    //Small Wall Tiles
    transparentSmallWallId = AddTile(SmallWall, 0, 0, "transparent");
    AddTile(SmallWall, 0);
    AddTile(SmallWall, 1);
    AddTile(SmallWall, 2);
    AddTile(SmallWall, 3);

    //Spawn Tiles
    AddTile(Floor, 0, 0, "sp_alpha");
    AddTile(Floor, 1, 0, "sp_delta");
    AddTile(Floor, 2, 0, "sp_vortex");
    AddTile(Floor, 3, 0, "sp_omega");
    AddTile(Floor, 4, 4, "sp_alpha");
    AddTile(Floor, 5, 4, "sp_delta");
    AddTile(Floor, 6, 4, "sp_vortex");
    AddTile(Floor, 7, 4, "sp_omega");
    AddTile(Floor, 8, 0, "sp_all_teams");
    AddTile(Floor, 9, 4, "sp_all_teams");
}

MapTileManager::Tile MapTileManager::GetTile(short int id)
{
    if(id >= 0 && id < gameTiles.size())
        return gameTiles[id];

    return gameTiles[0];
}

int MapTileManager::GetTransparentSmallWallId()
{
    return transparentSmallWallId;
}

sf::Vector2f MapTileManager::GetTileSize(TileType type)
{
    sf::Vector2f size_;

    if(type == Floor)
        size_ = FLOOR_SIZE_IMG;
    else if(type == SmallWall)
        size_ = SMALL_WALL_SIZE_IMG;
    else if(type == Wall)
        size_ = WALL_SIZE_IMG;

    return size_;
}

int MapTileManager::GetNumberOfTileId()
{
    return gameTiles.size();
}

sf::IntRect MapTileManager::GetTileRect(short int id)
{
    sf::IntRect rect;

    if(gameTiles[id].type == Floor)
    {
        rect.width = FLOOR_SIZE_IMG.x;
        rect.height = FLOOR_SIZE_IMG.y;
    }
    else if(gameTiles[id].type == SmallWall)
    {
        rect.width = SMALL_WALL_SIZE_IMG.x;
        rect.height = SMALL_WALL_SIZE_IMG.y;
    }
    else if(gameTiles[id].type == Wall)
    {
        rect.width = WALL_SIZE_IMG.x;
        rect.height = WALL_SIZE_IMG.y;
    }

    rect.left = TILESHEET_PADDING + (gameTiles[id].textureIndex % numberOfTilesPerLine) * (rect.width + TILESHEET_PADDING * 2);
    rect.top = TILESHEET_PADDING + (gameTiles[id].textureIndex / numberOfTilesPerLine) * (rect.height + TILESHEET_PADDING * 2);

    return rect;
}

MapTileManager::MapTileManager()
{
    assert(instance == nullptr);
    instance = this;

    instance->MapRows = instance->MapColoums = MAX_MAPTILE;
    InitialiseTiles();
}

sf::Vector2i GetGridPosition(sf::Vector2f objPos)
{
    int posX = objPos.x / MapTileManager::TILE_SIZE;
    int posY = objPos.y / MapTileManager::TILE_SIZE;

    return sf::Vector2i(posX, posY);
}

bool MapTileManager::LoadMap(std::string const& file)
{
    int rows = 0;
    int coloums = 0;
    int numOfWalls = 0;
    int numOfFloors = 0;
    std::ifstream fileReader(file);

    if(fileReader.is_open())
    {
        if(!(fileReader >> rows))
            return false;

        if(!(fileReader >> coloums))
            return false;

        instance->MapRows = rows;
        instance->MapColoums = coloums;

        ClearEntireGridInfo(coloums, rows);
        AStar::SetSearchRange(coloums, rows);

        short int tileId[coloums][rows];

        for(int y = 0; y < coloums; y++)
        {
            for(int x = 0; x < rows; x++)
            {
                if(!(fileReader >> tileId[y][x]))
                    return false;
            }
        }

        for(int y = 0; y < coloums; y++)
        {
            for(int x = 0; x < rows; x++)
            {
                if(tileId[y][x] != 0)
                {
                    GetGridInfo(y, x).tileId = tileId[y][x];

                    if(GetGridInfoTile(y, x).type == Floor || GetGridInfoTile(y, x).type == SmallWall)
                        numOfFloors++;
                    else if(GetGridInfoTile(y, x).type == Wall)
                        numOfWalls++;
                }
            }
        }

        int numOfVertices = 4;
        instance->va_floors.resize(numOfFloors * numOfVertices);
        instance->va_walls.resize(numOfWalls * numOfVertices);

        instance->va_floors.setPrimitiveType(sf::Quads);
        instance->va_walls.setPrimitiveType(sf::Quads);

        int current_floor = 0;
        int current_wall = 0;

        for(int y = 0; y < instance->MapColoums; y++)
            for(int x = 0; x < instance->MapRows; x++)
            {
                TileType type = GetGridInfoTile(y, x).type;

                if(type != Empty)
                {
                    sf::FloatRect rect;
                    sf::FloatRect texrRect;
                    int textrIndex = GetGridInfoTile(y, x).textureIndex;

                    if(type == Floor)
                    {
                        rect.width = texrRect.width = 100;
                        rect.height = texrRect.height = 100;
                        rect.left = 0;
                        rect.top = 0;

                        texrRect.left = 0;
                    }
                    else if(type == SmallWall)
                    {
                        rect.width = texrRect.width = 100;
                        rect.height = texrRect.height = 110;
                        rect.left = 0;
                        rect.top = -10;

                        texrRect.left = 100;
                    }
                    else if(type == Wall)
                    {
                        rect.width = texrRect.width = 100;
                        rect.height = texrRect.height = 148;
                        rect.left = 0;
                        rect.top = -48;

                        texrRect.left = 200;
                    }

                    rect.left += x * TILE_SIZE - TILE_SIZE / 2.f;
                    rect.top += y * TILE_SIZE - TILE_SIZE / 2.f;

                    texrRect.top = texrRect.height * textrIndex;

                    if(type == Wall)
                    {
                        if(y + 1 < instance->MapColoums)
                            if(GetGridInfoTile(y + 1, x).type == SmallWall)
                                rect.height = texrRect.height = 135;
                        if(y - 1 >= 0)
                            if(GetGridInfoTile(y - 1, x).type == Floor || GetGridInfoTile(y - 1, x).type == SmallWall)
                                texrRect.left += 100;
                    }

                    if(type == SmallWall || type == Floor)
                    {
                        instance->va_floors[current_floor + 0].position = sf::Vector2f(rect.left, rect.top);
                        instance->va_floors[current_floor + 1].position = sf::Vector2f(rect.left + rect.width, rect.top);
                        instance->va_floors[current_floor + 2].position = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);
                        instance->va_floors[current_floor + 3].position = sf::Vector2f(rect.left, rect.top + rect.height);

                        instance->va_floors[current_floor + 0].texCoords = sf::Vector2f(texrRect.left, texrRect.top);
                        instance->va_floors[current_floor + 1].texCoords = sf::Vector2f(texrRect.left + texrRect.width, texrRect.top);
                        instance->va_floors[current_floor + 2].texCoords = sf::Vector2f(texrRect.left + texrRect.width, texrRect.top + texrRect.height);
                        instance->va_floors[current_floor + 3].texCoords = sf::Vector2f(texrRect.left, texrRect.top + texrRect.height);

                        current_floor += numOfVertices;
                    }
                    else if(type == Wall)
                    {
                        instance->va_walls[current_wall + 0].position = sf::Vector2f(rect.left, rect.top);
                        instance->va_walls[current_wall + 1].position = sf::Vector2f(rect.left + rect.width, rect.top);
                        instance->va_walls[current_wall + 2].position = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);
                        instance->va_walls[current_wall + 3].position = sf::Vector2f(rect.left, rect.top + rect.height);

                        instance->va_walls[current_wall + 0].texCoords = sf::Vector2f(texrRect.left, texrRect.top);
                        instance->va_walls[current_wall + 1].texCoords = sf::Vector2f(texrRect.left + texrRect.width, texrRect.top);
                        instance->va_walls[current_wall + 2].texCoords = sf::Vector2f(texrRect.left + texrRect.width, texrRect.top + texrRect.height);
                        instance->va_walls[current_wall + 3].texCoords = sf::Vector2f(texrRect.left, texrRect.top + texrRect.height);

                        current_wall += numOfVertices;
                    }
                }
            }
    }

    return true;
}

int& MapTileManager::GetMapRows()
{
    return instance->MapRows;
}

int& MapTileManager::GetMapColoums()
{
    return instance->MapColoums;
}

std::vector<sf::FloatRect> MapTileManager::GetEntityClosestWallRect(sf::Vector2f objPos, bool exceptSmallWalls)
{
    std::vector<sf::FloatRect> rangedTile;
    sf::Vector2i objPosIndex = GetGridPosition(objPos);

    auto addRect = [&](int X, int Y)
    {
        if(X >= 0 && X <= MAX_MAPTILE - 1 && Y >= 0 && Y <= MAX_MAPTILE - 1)
        {
            if(GetGridInfoTile(Y, X).type == Wall || (GetGridInfoTile(Y, X).type == SmallWall && exceptSmallWalls == false))
                rangedTile.push_back(sf::FloatRect(X * TILE_SIZE, Y * TILE_SIZE, TILE_SIZE, TILE_SIZE));
        }
        else rangedTile.push_back(sf::FloatRect(X * TILE_SIZE, Y * TILE_SIZE, TILE_SIZE, TILE_SIZE));
    };

    addRect(objPosIndex.x + 0, objPosIndex.y + 0);
    addRect(objPosIndex.x + 0, objPosIndex.y - 1);
    addRect(objPosIndex.x + 0, objPosIndex.y + 1);
    addRect(objPosIndex.x - 1, objPosIndex.y + 0);
    addRect(objPosIndex.x + 1, objPosIndex.y + 0);
    addRect(objPosIndex.x - 1, objPosIndex.y + 1);
    addRect(objPosIndex.x + 1, objPosIndex.y + 1);
    addRect(objPosIndex.x - 1, objPosIndex.y - 1);
    addRect(objPosIndex.x + 1, objPosIndex.y - 1);

    return rangedTile;
}

std::vector<sf::FloatRect> MapTileManager::GetRangedTileRect(sf::Vector2f objPos, TileType type, int WidthFactor, int HeightFactor)
{
    std::vector<sf::FloatRect> rangedTile;
    sf::Vector2i objPosIndex = GetGridPosition(objPos);

    for(int y = -HeightFactor; y <= HeightFactor; y++)
        for(int x = -WidthFactor; x <= WidthFactor; x++)
        {
            int X = objPosIndex.x + x;
            int Y = objPosIndex.y + y;

            if(X >= 0 && X <= MAX_MAPTILE - 1 && Y >= 0 && Y <= MAX_MAPTILE - 1)
                if(GetGridInfoTile(Y, X).type == type)
                    rangedTile.push_back(sf::FloatRect(X * TILE_SIZE - TILE_SIZE / 2.f, Y * TILE_SIZE - TILE_SIZE / 2.f, TILE_SIZE, TILE_SIZE));
        }

    return rangedTile;
}

sf::VertexArray& MapTileManager::GetWallVertexArray()
{
    return instance->va_walls;
}

sf::VertexArray& MapTileManager::GetFloorVertexArray()
{
    return instance->va_floors;
}
