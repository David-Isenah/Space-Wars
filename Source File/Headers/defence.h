#ifndef DEFENCE_H_INCLUDED
#define DEFENCE_H_INCLUDED
#include "entity.h"
#include "shipdefence.h"
#include "skin.h"

class Defence
{
public:
    enum DefenceType{MainDefence = 0, MiniDefence};

    Defence(sf::Vector2f locationPos, DefenceType type_, short team_, short rotation_, int skinIndex_);

     static void InitialiseMultiDrawer();
     static void Draw(sf::RenderTarget& target);
     void Update(float dt);
     void RefreshEntities();
     bool IsShipDefenceAlive();
     sf::FloatRect GetDefenceSpotRect();
     ShipDefence* BuildEntity(bool isSpawning_ = true);

private:
    ShipDefence* shipDefence;
    short team;
    int baseId;

    int rebuildCost;
    short rotation;
    DefenceType type;
    sf::Vector2f location;
    sf::FloatRect spotRect;
    int skinIndex;
    sf::Transformable transformation;
    sf::Vertex vertices[4];
};

#endif // DEFENCE_H_INCLUDED
