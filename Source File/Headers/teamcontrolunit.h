#ifndef TEAMCONTROLUNIT_H_INCLUDED
#define TEAMCONTROLUNIT_H_INCLUDED

#include "base.h"

class TeamControlUnit
{
public:
    TeamControlUnit();
    ~TeamControlUnit();

    void AddSpawnPoint(sf::Vector2i spGrid_);
    void RemoveSpawnPoint(sf::Vector2i spGrid_);
    void Reset();
    int GetSpawnPointSize();
    Base::SpawnPointObject* GetSpawnPoint(sf::Vector2i spGrid_);

private:
    std::vector<Base::SpawnPointObject> spawnPoints;
};

#endif // TEAMCONTROLUNIT_H_INCLUDED
