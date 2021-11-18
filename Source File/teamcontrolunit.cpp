#include "teamcontrolunit.h"
#include "game.h"

TeamControlUnit::TeamControlUnit()
{
}

TeamControlUnit::~TeamControlUnit()
{
    Reset();
}

void TeamControlUnit::AddSpawnPoint(sf::Vector2i spGrid_)
{
    spawnPoints.resize(spawnPoints.size() + 1);
    spawnPoints[spawnPoints.size() - 1].grid = spGrid_;
}

void TeamControlUnit::RemoveSpawnPoint(sf::Vector2i spGrid_)
{
    for(int a = 0; a < spawnPoints.size(); a++)
        if(spawnPoints[a].grid == spGrid_)
        {
            spawnPoints.erase(spawnPoints.begin() + a);
            a--;
        }
}

int TeamControlUnit::GetSpawnPointSize()
{
    return spawnPoints.size();
}

Base::SpawnPointObject* TeamControlUnit::GetSpawnPoint(sf::Vector2i spGrid_)
{
    for(int a = 0; a < spawnPoints.size(); a ++)
        if(spawnPoints[a].grid == spGrid_)
            return &spawnPoints[a];
    return nullptr;
}

void TeamControlUnit::Reset()
{
    for(int a = 0; a < spawnPoints.size(); a++)
        GetGridInfo(spawnPoints[a].grid.y, spawnPoints[a].grid.x).tileId = -1;
    spawnPoints.clear();
}
