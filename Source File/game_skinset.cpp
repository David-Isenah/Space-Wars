#include "game.h"
#include <iostream>

std::vector<Skin> skinSet[Game::NumOfSkinTypes][Game::NumOfTeams];

std::string folderNameType[] = {"Main-Ship/", "Sub-Ship/", "Mini-Ship/", "Micro-Ship/", "Main-Defense/", "Mini-Defense/", "Boss/"};
std::string folderNameTeam[] = {"Alpha/", "Delta/", "Vortex/", "Omega/"};

void AddSkin(Game::SkinType skinType, std::string points, sf::Vector2f shootPoint, short team)
{
    if(team >= 0 && team < Game::NumOfTeams && skinType >= 0 && skinType < Game::NumOfSkinTypes)
    {
        Skin skin;
        std::stringstream fileName;
        std::stringstream ssPoints;

        fileName << "Resources/Images/Skins/" << folderNameType[skinType] << folderNameTeam[team] << '#';
        if(skinSet[skinType][team].size() + 1 < 10)
            fileName << '0';
        fileName << skinSet[skinType][team].size() + 1 << ".png";

        skin.texture.loadFromFile(fileName.str());
        skin.texture.setSmooth(true);

        ssPoints << points;
        while(ssPoints.eof() == false)
        {
            sf::Vector2f readPoint;
            ssPoints >> readPoint.x;
            ssPoints >> readPoint.y;
            skin.points.push_back(readPoint);

            if(ssPoints.eof() == false)
                ssPoints.ignore(2);
        }

        skin.shootPoint = shootPoint;

        if(skin.points.size() > 2)
        {
            float xMin = skin.points[1].x;
            float xMax = skin.points[1].x;
            float yMin = skin.points[1].y;
            float yMax = skin.points[1].y;

            for(int a = 2; a < skin.points.size(); a++)
            {
                if(skin.points[a].x < xMin)
                    xMin = skin.points[a].x;

                if(skin.points[a].x > xMax)
                    xMax = skin.points[a].x;

                if(skin.points[a].y < yMin)
                    yMin = skin.points[a].y;

                if(skin.points[a].y > yMax)
                    yMax = skin.points[a].y;
            }

            skin.collisionRect.left = xMin;
            skin.collisionRect.top = yMin;
            skin.collisionRect.width = xMax - xMin;
            skin.collisionRect.height = yMax - yMin;
        }

        skinSet[skinType][team].push_back(skin);
    }
}

void AddSkinToTeams(Game::SkinType skinType, std::string points, sf::Vector2f shootPoint)
{
    for(int a = 0; a < Game::NumOfTeams; a++)
        AddSkin(skinType, points, shootPoint, a);
}

void Game::InitializeSkin()
{
    //____Main-Ship
    //Alpha
    AddSkin(Game::SkinMainShip, "90 90 , 77 31 , 103 31 , 170 82 , 164 129 , 100 150 , 80 150 , 16 129 , 10 82", sf::Vector2f(90, 31), Game::Alpha);

    //Delta
    AddSkin(Game::SkinMainShip, "90 95 , 82 35 , 98 35 , 143 56 , 172 115 , 172 124 , 134 144, 46 144 , 8 124 , 8 115 , 37 56", sf::Vector2f(90, 35), Game::Delta);

    //____Sub-Ship
    AddSkinToTeams(Game::SkinSubShip, "90 90 , 75 36 , 105 36 , 173 73 , 165 126 , 142 137 , 103 143 , 77 143 , 38 137 , 15 126 , 7 73", sf::Vector2f(90, 36));
    AddSkinToTeams(Game::SkinSubShip, "90 90 , 83 37 , 97 37 , 171 79 , 172 108 , 161 122 , 99 143 , 81 143 , 19 122 , 8 108 , 9 79", sf::Vector2f(90, 37));

    //____Mini-Ship
    AddSkinToTeams(Game::SkinMiniShip, "90 101 , 59 60 , 121 60 , 150 93 , 139 120 , 41 120 , 30 93", sf::Vector2f(90, 60));

    //___Micro-Ship
    AddSkinToTeams(Game::SkinMicroShip, "90 90 , 76 66 , 104 66 , 125 79 , 127 82 , 128 108 , 125 111 , 103 114 , 77 114 , 55 111 , 52 108 , 53 82 , 55 79", sf::Vector2f(90, 66));

    //____Main-Defense
    AddSkinToTeams(SkinMainDefence, "180 192 , 180 72 , 286 88 , 350 128 , 350 240 , 304 270 , 214 286 , 146 286 , 56 270 , 10 240 , 10 128", sf::Vector2f(180, 72));
    AddSkinToTeams(SkinMainDefence, "180 186 , 148 72 , 212 72 , 253 91 , 337 141 , 351 248 , 273 288 , 87 288 , 9 248 , 23 141 , 107 91", sf::Vector2f(180, 72));

    //____Mini-Defense
    AddSkinToTeams(SkinMiniDefence, "90 90 , 48 44 , 132 44 , 172 82 , 148 136 , 32 136 , 8 82", sf::Vector2f(90, 44));
}

Skin* Game::GetSkin(SkinType skinType, short index, short team)
{
    if(index < skinSet[skinType][team].size())
        return &skinSet[skinType][team][index];
    return nullptr;
}

int Game::GetSkinSize(SkinType skinType, short team)
{
    return skinSet[skinType][team].size();
}
