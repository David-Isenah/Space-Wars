#ifndef MAPTILEMANAGER_H_INCLUDED
#define MAPTILEMANAGER_H_INCLUDED

#include "SFML/Graphics.hpp"
#include "entity.h"
#include "string"
#include "vector"

class MapTileManager
{
    public:
        static const int MAX_MAPTILE = 120;
        static const int TILE_SIZE = 97;

        enum TileType{Empty = 0, Floor, Wall, SmallWall};
        struct Tile
        {
            TileType type = Empty;
            short int textureIndex = 0;
            short int availableRank = 0;
            std::string info;

            Tile(TileType type_ = Empty, short int textureIndex_ = 0, short int availableRank_ = 0, std::string info_ = "") :
                type(type_),
                textureIndex(textureIndex_),
                availableRank(availableRank_),
                info(info_)
            {
            }
        };

        static void InitialiseTiles();
        static sf::Vector2f GetTileSize(TileType type);
        static Tile GetTile(short int id);
        static sf::IntRect GetTileRect(short int id);
        static int GetNumberOfTileId();
        static int AddTile(TileType type_, short int textureIndex_, short int availableRank_ = 3, std::string info_ = "");

        MapTileManager();

        static bool LoadMap(std::string const& file);
        static int& GetMapRows();
        static int& GetMapColoums();

        static sf::VertexArray& GetWallVertexArray();
        static sf::VertexArray& GetFloorVertexArray();

        static int GetTransparentSmallWallId();

        static std::vector<sf::FloatRect> GetEntityClosestWallRect(sf::Vector2f objPos, bool exceptSmallWalls = false);
        static std::vector<sf::FloatRect> GetRangedTileRect(sf::Vector2f objPos, TileType type, int WidthFactor = 1, int HeightFactor = 1);

    private:
        int MapRows;
        int MapColoums;

        sf::VertexArray va_floors;
        sf::VertexArray va_walls;

        static MapTileManager* instance;
};

sf::Vector2i GetGridPosition(sf::Vector2f objPos);

#endif // MAPTILEMANAGER_H_INCLUDED
