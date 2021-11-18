#include "astarpathfinding.h"
#include "entitymanager.h"
#include "game.h"
#include "math.h"
#include "iostream"
#include "public_functions.h"

namespace AStar
{
    enum WalkArea{Unwalkable = 0, Walkable};
    enum OnList{OnNoList = 0, OnOpenList, OnClosedList};

    //const long MAX_F_COST = MapTileManager::TILE_SIZE * MapTileManager::TILE_SIZE * 14 + MapTileManager::TILE_SIZE * 2 * 10;
    const int REACH_RANGE = 5;

    int AstarWidth = 0;
    int AstarHeight = 0;
    bool fastSearchStatus = true;
    bool progressiveSearchStatus = true;
    bool isProgressing = false;

    short prog_teamUpType = 0;
    int prog_numOfOpenList = 0;
    int prog_newOpenListId = 0;
    sf::Vector2i prog_target;
    sf::Vector2i prog_start;
    SearchType prog_searchType = NoSearchType;
    bool prog_useReachSearch = false;

    //Normal Search Variables(Progression)
    int openList[MapTileManager::MAX_MAPTILE * MapTileManager::MAX_MAPTILE + 2] = {};
    int openListIndex[MapTileManager::MAX_MAPTILE + 1][MapTileManager::MAX_MAPTILE + 1] = {};
    int onWhichList[MapTileManager::MAX_MAPTILE + 1][MapTileManager::MAX_MAPTILE + 1] = {};
    int xOpenList[MapTileManager::MAX_MAPTILE * MapTileManager::MAX_MAPTILE + 2] = {};
    int yOpenList[MapTileManager::MAX_MAPTILE * MapTileManager::MAX_MAPTILE + 2] = {};
    int xParent[MapTileManager::MAX_MAPTILE + 1][MapTileManager::MAX_MAPTILE + 1] = {};
    int yParent[MapTileManager::MAX_MAPTILE + 1][MapTileManager::MAX_MAPTILE + 1] = {};
    int costF[MapTileManager::MAX_MAPTILE * MapTileManager::MAX_MAPTILE + 2] = {};
    int costH[MapTileManager::MAX_MAPTILE * MapTileManager::MAX_MAPTILE + 2] = {};
    int costG[MapTileManager::MAX_MAPTILE + 1][MapTileManager::MAX_MAPTILE + 1] = {};
    int numOfGridsIterated = 0;
    int perFramLimit = LIMIT_PER_FRAME / 2.f;
    bool isLimited = false;
    std::list<sf::Vector2i> onListToReset;
    std::list<sf::Vector2i> onListToReset_reach;

    //Fast Search Variables
    int fast_openList[MapTileManager::MAX_MAPTILE * MapTileManager::MAX_MAPTILE + 2] = {};
    int fast_openListIndex[MapTileManager::MAX_MAPTILE + 1][MapTileManager::MAX_MAPTILE + 1] = {};
    int fast_onWhichList[MapTileManager::MAX_MAPTILE + 1][MapTileManager::MAX_MAPTILE + 1] = {};
    int fast_xOpenList[MapTileManager::MAX_MAPTILE * MapTileManager::MAX_MAPTILE + 2] = {};
    int fast_yOpenList[MapTileManager::MAX_MAPTILE * MapTileManager::MAX_MAPTILE + 2] = {};
    int fast_xParent[MapTileManager::MAX_MAPTILE + 1][MapTileManager::MAX_MAPTILE + 1] = {};
    int fast_yParent[MapTileManager::MAX_MAPTILE + 1][MapTileManager::MAX_MAPTILE + 1] = {};
    int fast_costF[MapTileManager::MAX_MAPTILE * MapTileManager::MAX_MAPTILE + 2] = {};
    int fast_costH[MapTileManager::MAX_MAPTILE * MapTileManager::MAX_MAPTILE + 2] = {};
    int fast_costG[MapTileManager::MAX_MAPTILE + 1][MapTileManager::MAX_MAPTILE + 1] = {};
    int fast_numOfGridsIterated = 0;
    int fast_perFrameLimit = LIMIT_PER_FRAME / 2.f;
    bool fast_isLimited = false;
    std::list<sf::Vector2i> fast_onListToReset;
    std::list<sf::Vector2i> fast_onListToReset_reach;

    //Fast Continue Variables
    short fast_teamUpType = 0;
    int fast_numOfOpenList = 0;
    int fast_newOpenListId = 0;
    sf::Vector2i fast_target;
    sf::Vector2i fast_start;
    SearchType fast_searchType = NoSearchType;
    bool fast_useReachSearch = false;
    bool fast_isContinuing = false;

    void SetSearchRange(int yRange, int xRange)
    {
        AstarWidth = xRange;
        AstarHeight = yRange;
    }

    void SetFastSearchStatus(bool status)
    {
        fastSearchStatus = status;
    }

    void SetProgressiveSearchStatus(bool status)
    {
        progressiveSearchStatus = status;
    }

    bool IsReadyToSearch(bool isFastSearch)
    {
        if(isFastSearch)
            return fast_isLimited == false && fastSearchStatus;
        return isLimited == false && progressiveSearchStatus;
    }

    bool GetLimitedState()
    {
        return isLimited;
    }

    bool GetFastSearchLimitedState()
    {
        return fast_isLimited;
    }

    void UpdateLimiter()
    {
        /* Not useful
        if(numOfGridsIterated >= perFramLimit)
            isLimited = true;*/
        if(fast_numOfGridsIterated >= fast_perFrameLimit)
            fast_isLimited = true;
    }

    void ResetLimiter()
    {
        numOfGridsIterated = 0;
        fast_numOfGridsIterated = 0;
        isLimited = false;
        fast_isLimited = false;

        if(isProgressing)
        {
            perFramLimit = LIMIT_PER_FRAME - LIMIT_BALANCE_PER_FRAME;
            fast_perFrameLimit = LIMIT_BALANCE_PER_FRAME;
        }
        else
        {
            fast_perFrameLimit = LIMIT_PER_FRAME - LIMIT_BALANCE_PER_FRAME;
            perFramLimit = LIMIT_BALANCE_PER_FRAME;
        }
    }

    bool getWalkableStatus(const int& y, const int& x, const AStar::SearchType& type, const short& teamUpType, const sf::Vector2i& startGrid, const sf::Vector2i& targetGrid, const bool& allowSmallWalls)
    {
        if(GetGridInfoTile(y, x).type == MapTileManager::Wall || (allowSmallWalls ? false : GetGridInfoTile(y, x).type == MapTileManager::SmallWall))
            return Unwalkable;

        if(type == AStar::AvoidEnemySearchType)
        {
            int baseId = GetGridInfo(y, x).baseId;
            if(baseId >= 0 && Game::GetBase(baseId)->IsActive() &&
               Game::GetTeamUpType(Game::GetBase(baseId)->GetTeam()) != teamUpType)
                return Unwalkable;
        }
        else if(type == AStar::BaseRegionSearchType)
        {
            int startBaseId = GetGridInfo(startGrid.y, startGrid.x).baseId;
            if(startBaseId >= 0 && GetGridInfo(y, x).baseId != startBaseId)
                return Unwalkable;
        }

        return Walkable;
    }

    bool GetProgressionState()
    {
        return isProgressing;
    }

    void CancleProgression()
    {
        isProgressing = false;
    }

    void ContinueLastTargetSearch(sf::Vector2i start_)
    {
        isProgressing = true;
        if(prog_useReachSearch)
        {
            prog_target = start_;

            for(sf::Vector2i& grid : onListToReset_reach)
                onWhichList[grid.y][grid.x] = OnNoList;
            onListToReset_reach.clear();

            if(onWhichList[prog_target.y][prog_target.x] == OnClosedList && costG[prog_target.y][prog_target.x] == 1)
                ;
            else
            {
                prog_numOfOpenList = 1;
                prog_newOpenListId = 0;

                costG[start_.y][start_.x] = 0;
                openList[1] = 1;
                openListIndex[start_.y][start_.x] = 1;
                xOpenList[1] = start_.x;
                yOpenList[1] = start_.y;
                onListToReset_reach.push_back(start_);
            }
        }
        else
        {
            prog_start = start_;
            std::vector<int> id;

            for(int a = 1; a <= prog_numOfOpenList; a++)
            {
                id.push_back(openList[a]);

                int y = yOpenList[openList[a]];
                int x = xOpenList[openList[a]];

                costH[openList[a]] = 10 * (abs(y - prog_start.y) + abs(x - prog_start.x));
                costF[openList[a]] = costG[y][x] + costH[openList[a]];
            }

            for(int a = 0; a < id.size(); a++)
            {
                openList[a + 1] = id[a];
                openListIndex[yOpenList[id[a]]][xOpenList[id[a]]] = a + 1;

                int index = a + 1;
                while(index != 1)
                {
                    if(costF[openList[index]] <= costF[openList[index / 2]])
                    {
                        int idCopy = openList[index / 2];
                        openList[index / 2] = openList[index];
                        openList[index] = idCopy;

                        openListIndex[yOpenList[openList[index]]][xOpenList[openList[index]]] = index;
                        openListIndex[yOpenList[openList[index / 2]]][xOpenList[openList[index / 2]]] = index / 2;

                        index = index / 2;
                    }
                    else break;
                }
            }
        }
    }

    void ContinueLastTargetFastSearch(sf::Vector2i start_)
    {
        fast_isContinuing = true;

        if(fast_useReachSearch)
        {
            fast_target = start_;

            for(sf::Vector2i& grid : fast_onListToReset_reach)
                fast_onWhichList[grid.y][grid.x] = OnNoList;
            fast_onListToReset_reach.clear();

            if(fast_onWhichList[fast_target.y][fast_target.x] == OnClosedList && fast_costG[fast_target.y][fast_target.x] == 1)
                ;
            else
            {
                fast_numOfOpenList = 1;
                fast_newOpenListId = 0;

                fast_costG[fast_target.y][fast_target.x] = 0;
                fast_openList[1] = 1;
                fast_openListIndex[fast_target.y][fast_target.x] = 1;
                fast_xOpenList[1] = fast_target.x;
                fast_yOpenList[1] = fast_target.y;
                fast_onListToReset_reach.push_back(fast_target);
            }
        }
        else
        {
            fast_start = start_;
            std::vector<int> id;

            for(int a = 1; a <= fast_numOfOpenList; a++)
            {
                id.push_back(fast_openList[a]);

                int y = fast_yOpenList[fast_openList[a]];
                int x = fast_xOpenList[fast_openList[a]];

                fast_costH[fast_openList[a]] = 10 * (abs(y - fast_target.y) + abs(x - fast_target.x));
                fast_costF[fast_openList[a]] = fast_costG[y][x] + fast_costH[fast_openList[a]];
            }

            for(int a = 0; a < id.size(); a++)
            {
                fast_openList[a + 1] = id[a];
                fast_openListIndex[fast_yOpenList[id[a]]][fast_xOpenList[id[a]]] = a + 1;

                int index = a + 1;
                while(index != 1)
                {
                    if(fast_costF[fast_openList[index]] <= fast_costF[fast_openList[index / 2]])
                    {
                        int idCopy = fast_openList[index / 2];
                        fast_openList[index / 2] = fast_openList[index];
                        fast_openList[index] = idCopy;

                        fast_openListIndex[fast_yOpenList[fast_openList[index]]][fast_xOpenList[fast_openList[index]]] = index;
                        fast_openListIndex[fast_yOpenList[fast_openList[index / 2]]][fast_xOpenList[fast_openList[index / 2]]] = index / 2;

                        index = index / 2;
                    }
                    else break;
                }
            }
        }
    }

    bool FindPath(sf::Vector2i start, sf::Vector2i target, std::vector<sf::Vector2i>& path, AStar::SearchType searchType, short teamUpType, bool ignoreLimiter, bool useReachSearch)
    {
        if(isLimited == false || ignoreLimiter)
        {
            int MapWidth = AstarWidth;
            int MapHeight = AstarHeight;
            int numOfOpenList = 0;
            int newOpenListId = 0;
            int numOfCurrentIterated = 0;

            if(isProgressing)
            {
                numOfOpenList = prog_numOfOpenList;
                newOpenListId = prog_newOpenListId;
                target = prog_target;
                start = prog_start;
                searchType = prog_searchType;
                isProgressing = false;
                teamUpType = prog_teamUpType;
                useReachSearch = prog_useReachSearch;

                if(useReachSearch && onWhichList[prog_target.y][prog_target.x] == OnClosedList && costG[prog_target.y][prog_target.x] == 1)
                {
                    std::vector<sf::Vector2i> tempPath;
                    sf::Vector2i index(target.x, target.y);

                    while(index != start)
                    {
                        tempPath.push_back(index);
                        int x = index.x;
                        int y = index.y;
                        index.x = xParent[y][x];
                        index.y = yParent[y][x];
                    }
                    tempPath.push_back(start);

                    for(int b = tempPath.size() - 1; b >= 0; b--)
                        path.push_back(tempPath[b]);

                    for(sf::Vector2i& grid : onListToReset_reach)
                            onWhichList[grid.y][grid.x] = OnNoList;
                        onListToReset_reach.clear();

                    return true;
                }

                if(onWhichList[start.y][start.x] != OnNoList && start != target && (useReachSearch ? costG[start.y][start.x] != 1 : true))
                {
                    sf::Vector2i index = start;
                    bool isPathTruncated = false;

                    while(index != target)
                    {
                        if(GetGridInfoTile(index.y, index.x).type == MapTileManager::SmallWall || GetGridInfoTile(index.y, index.x).type == MapTileManager::Wall)
                        {
                            isPathTruncated = true;
                            break;
                        }
                        path.push_back(index);
                        int x = index.x;
                        int y = index.y;
                        index.x = xParent[y][x];
                        index.y = yParent[y][x];
                    }
                    if(isPathTruncated == false && GetGridInfoTile(target.y, target.x).type != MapTileManager::SmallWall && GetGridInfoTile(target.y, target.x).type != MapTileManager::Wall)
                        path.push_back(target);

                    if(useReachSearch)
                    {
                        for(sf::Vector2i& grid : onListToReset_reach)
                            onWhichList[grid.y][grid.x] = OnNoList;
                        onListToReset_reach.clear();

                        for(int b = 0; b < path.size(); b++)
                        {
                            onWhichList[path[b].y][path[b].x] = OnClosedList;
                            costG[path[b].y][path[b].x] = 1; //marking it
                            if(b != 0)
                            {
                                xParent[path[b].y][path[b].x] = path[b - 1].x;
                                yParent[path[b].y][path[b].x] = path[b - 1].y;
                            }
                            onListToReset.push_back(path[b]);
                        }
                    }

                    return true;
                }
            }
            else
            {
                if(useReachSearch) //flip start and target
                {
                    sf::Vector2i temp_start = start;
                    start = target;
                    target = temp_start;
                }

                prog_start = start;
                prog_target = target;
                prog_searchType = searchType;
                prog_teamUpType = teamUpType;
                prog_newOpenListId = newOpenListId;
                prog_numOfOpenList = numOfOpenList;
                prog_useReachSearch = useReachSearch;

                if(start == target || getWalkableStatus(start.y, start.x, searchType, teamUpType, start, target, false) == Unwalkable || getWalkableStatus(target.y, target.x, searchType, teamUpType, start, target, useReachSearch) == Unwalkable)
                    return false;

                if(start.x > AstarWidth || start.y > AstarHeight || target.x > AstarWidth || target.y > AstarHeight ||
                   start.x < 0 || start.y < 0 || target.x < 0 || target.y < 0)
                    return false;

                for(sf::Vector2i& grid : onListToReset)
                    onWhichList[grid.y][grid.x] = OnNoList;
                onListToReset.clear();

                for(sf::Vector2i& grid : onListToReset_reach)
                    onWhichList[grid.y][grid.x] = OnNoList;
                onListToReset_reach.clear();

                costG[target.y][target.x] = 0;
                numOfOpenList = 1;
                openList[1] = 1;
                openListIndex[target.y][target.x] = 1;
                xOpenList[1] = target.x;
                yOpenList[1] = target.y;
                if(useReachSearch)
                    onListToReset_reach.push_back(target);
                else onListToReset.push_back(target);

                prog_newOpenListId = newOpenListId;
                prog_numOfOpenList = numOfOpenList;
            }

            //Reach variables
            sf::Vector2i temp_reachAbsDiff;
            sf::Vector2i temp_parentAbsDiff;
            bool temp_isConnerSmallWall = false;
            bool temp_isParentSmallWall = false;
            bool temp_isParentMarked = false;
            bool temp_isAlignedWithParent = false;
            bool temp_reachIsDiagnonal = false;
            bool temp_reachIsWithinRange = false;
            int temp_reachRange = 0;

            while(1)
            {
                if(numOfOpenList != 0)
                {
                    int xVal = xOpenList[openList[1]];
                    int yVal = yOpenList[openList[1]];

                    if(useReachSearch)
                    {
                        temp_parentAbsDiff.x = abs(xVal - target.x);
                        temp_parentAbsDiff.y = abs(yVal - target.y);

                        temp_isParentSmallWall = GetGridInfoTile(yVal, xVal).type == MapTileManager::SmallWall;
                        temp_isParentMarked = (costH[openList[openListIndex[yVal][xVal]]] % 10 != 0) || (temp_parentAbsDiff.x == 0 && temp_parentAbsDiff.y == 0);
                    }

                    onWhichList[yVal][xVal] = OnClosedList;
                    openListIndex[yVal][xVal] = 0;
                    openList[1] = openList[numOfOpenList];
                    if(onWhichList[yOpenList[openList[1]]][xOpenList[openList[1]]] == OnOpenList)
                        openListIndex[yOpenList[openList[1]]][xOpenList[openList[1]]] = 1;
                    numOfOpenList--;

                    int listId = 1;

                    while(1)
                    {
                        int initialListId = listId;

                        if(2 * initialListId + 1 <= numOfOpenList)
                        {
                            if(costF[openList[listId]] >= costF[openList[2 * initialListId]])
                                listId = 2 * initialListId;
                            if(costF[openList[listId]] >= costF[openList[2 * initialListId + 1]])
                                listId = 2 * initialListId + 1;
                        }
                        else
                        {
                            if(2 * initialListId <= numOfOpenList)
                                if(costF[openList[listId]] >= costF[openList[2 * initialListId]])
                                        listId = 2 * initialListId;
                        }

                        if(listId != initialListId)
                        {
                            int idCopy = openList[initialListId];
                            openList[initialListId] = openList[listId];
                            openList[listId] = idCopy;

                            openListIndex[yOpenList[openList[listId]]][xOpenList[openList[listId]]] = listId;
                            openListIndex[yOpenList[openList[initialListId]]][xOpenList[openList[initialListId]]] = initialListId;
                        }
                        else break;
                    }

                    for(int y = yVal - 1; y <= yVal + 1; y++)
                        for(int x = xVal - 1; x <= xVal + 1; x++)
                        {
                            int conner = Walkable;

                            if(y >= 0 && y < MapHeight && x >= 0 && x < MapWidth)
                            {
                                if(onWhichList[y][x] != OnClosedList)
                                {
                                    if(useReachSearch)
                                    {
                                        temp_reachAbsDiff.x = abs(x - target.x);
                                        temp_reachAbsDiff.y = abs(y - target.y);

                                        temp_reachIsDiagnonal = temp_reachAbsDiff.x == temp_reachAbsDiff.y;
                                        temp_reachRange = temp_reachAbsDiff.x > temp_reachAbsDiff.y ? temp_reachAbsDiff.x : temp_reachAbsDiff.y;
                                        temp_reachIsWithinRange = temp_reachRange <= REACH_RANGE * (temp_reachIsDiagnonal ? 0.7f : 1);
                                        temp_isConnerSmallWall = GetGridInfoTile(y, x).type == MapTileManager::SmallWall;
                                        temp_isAlignedWithParent = (temp_reachAbsDiff.x == 0 && temp_parentAbsDiff.x == 0) || (temp_reachAbsDiff.y == 0 && temp_parentAbsDiff.y == 0) || (temp_reachAbsDiff.x == temp_reachAbsDiff.y && temp_parentAbsDiff.x == temp_parentAbsDiff.y);

                                        if(temp_isConnerSmallWall && !((temp_isParentMarked && temp_isAlignedWithParent && temp_reachIsWithinRange)))
                                            conner = Unwalkable;

                                        if(temp_isParentSmallWall && !temp_isAlignedWithParent)
                                            conner = Unwalkable;
                                    }

                                    if(conner == Unwalkable)
                                        ;
                                    else if(getWalkableStatus(y, x, searchType, teamUpType, start, target, useReachSearch && temp_reachIsWithinRange))
                                    {
                                        if(x - xVal != 0 && y - yVal != 0)
                                            if(getWalkableStatus(yVal, x, searchType, teamUpType, start, target, useReachSearch && temp_reachIsWithinRange) == Unwalkable ||
                                               getWalkableStatus(y, xVal, searchType, teamUpType, start, target, useReachSearch && temp_reachIsWithinRange) == Unwalkable)
                                                conner = Unwalkable;
                                    }
                                    else conner = Unwalkable;

                                    if(conner == Walkable)
                                    {
                                        if(onWhichList[y][x] != OnOpenList) //only those on no list
                                        {
                                            newOpenListId++;
                                            int newIdOnList = numOfOpenList + 1;
                                            openList[newIdOnList] = newOpenListId;
                                            openListIndex[y][x] = newIdOnList;
                                            xOpenList[newOpenListId] = x;
                                            yOpenList[newOpenListId] = y;

                                            costG[y][x] = costG[yVal][xVal] + (x - xVal != 0 && y - yVal != 0 ? 14 : 10);
                                            costH[openList[newIdOnList]] = 10 * (abs(y - start.y) + abs(x - start.x));

                                            if(useReachSearch && temp_reachRange <= REACH_RANGE && temp_isAlignedWithParent && temp_isParentMarked)
                                                costH[openList[newIdOnList]]++; //marking it

                                            costF[openList[newIdOnList]] = costG[y][x] + costH[openList[newIdOnList]];

                                            xParent[y][x] = xVal;
                                            yParent[y][x] = yVal;

                                            int index = newIdOnList;
                                            while(index != 1)
                                            {
                                                if(costF[openList[index]] <= costF[openList[index / 2]])
                                                {
                                                    int idCopy = openList[index / 2];
                                                    openList[index / 2] = openList[index];
                                                    openList[index] = idCopy;

                                                    openListIndex[yOpenList[openList[index]]][xOpenList[openList[index]]] = index;
                                                    openListIndex[yOpenList[openList[index / 2]]][xOpenList[openList[index / 2]]] = index / 2;

                                                    index = index / 2;
                                                }
                                                else break;
                                            }
                                            numOfOpenList++;
                                            onWhichList[y][x] = OnOpenList;
                                            if(useReachSearch)
                                                onListToReset_reach.push_back(sf::Vector2i(x, y));
                                            else onListToReset.push_back(sf::Vector2i(x, y));
                                        }
                                        else if(useReachSearch ? temp_isConnerSmallWall == false : true) // only those on open list
                                        {
                                            int totalGCost = costG[yVal][xVal] + (x - xVal != 0 && y - yVal != 0 ? 14 : 10);

                                            if(totalGCost < costG[y][x])
                                            {
                                                xParent[y][x] = xVal;
                                                yParent[y][x] = yVal;
                                                costG[y][x] = totalGCost;

                                                int index = openListIndex[y][x];
                                                if(useReachSearch && temp_reachRange <= REACH_RANGE && temp_isParentMarked && temp_isAlignedWithParent)
                                                    costH[openList[index]]++; //marking it
                                                costF[openList[index]] = costH[openList[index]] + costG[y][x];

                                                while(index != 1)
                                                {
                                                    if(costF[openList[index]] < costF[openList[index / 2]])
                                                    {
                                                        int idCopy = openList[index / 2];
                                                        openList[index / 2] = openList[index];
                                                        openList[index] = idCopy;

                                                        openListIndex[yOpenList[openList[index]]][xOpenList[openList[index]]] = index;
                                                        openListIndex[yOpenList[openList[index / 2]]][xOpenList[openList[index / 2]]] = index / 2;

                                                        index /= 2;
                                                    }
                                                    else break;
                                                }
                                            }
                                        }
                                    }
                                }
                                else if(useReachSearch && costG[y][x] == 1)
                                {
                                    if((x - xVal != 0 && y - yVal != 0) ?
                                       (!(onWhichList[yVal][x] == OnClosedList && costG[yVal][x] == 1) && !(onWhichList[y][xVal] == OnClosedList && costG[y][xVal] == 1) &&
                                        GetGridInfoTile(yVal, x).type != MapTileManager::Wall && GetGridInfoTile(yVal, x).type != MapTileManager::SmallWall &&
                                        GetGridInfoTile(y, xVal).type != MapTileManager::Wall && GetGridInfoTile(y, xVal).type != MapTileManager::SmallWall) : true)
                                    {
                                        std::vector<sf::Vector2i> tempPath;
                                        sf::Vector2i index(x, y);

                                        while(index != start)
                                        {
                                            tempPath.push_back(index);
                                            int x = index.x;
                                            int y = index.y;
                                            index.x = xParent[y][x];
                                            index.y = yParent[y][x];
                                        }
                                        tempPath.push_back(start);

                                        for(int b = tempPath.size() - 1; b >= 0; b--)
                                            path.push_back(tempPath[b]);

                                        index.x = xVal;
                                        index.y = yVal;
                                        tempPath.clear();
                                        bool isPathTruncated = false;
                                        while(index != target)
                                        {
                                            if(GetGridInfoTile(index.y, index.x).type == MapTileManager::SmallWall || GetGridInfoTile(index.y, index.x).type == MapTileManager::Wall)
                                            {
                                                isPathTruncated = true;
                                                break;
                                            }
                                            tempPath.push_back(index);
                                            path.push_back(index);
                                            int x = index.x;
                                            int y = index.y;
                                            index.x = xParent[y][x];
                                            index.y = yParent[y][x];
                                        }
                                        if(isPathTruncated == false && GetGridInfoTile(target.y, target.x).type != MapTileManager::SmallWall && GetGridInfoTile(target.y, target.x).type != MapTileManager::Wall)
                                        {
                                            path.push_back(target);
                                            tempPath.push_back(target);
                                        }

                                        for(sf::Vector2i& grid : onListToReset_reach)
                                            onWhichList[grid.y][grid.x] = OnNoList;
                                        onListToReset_reach.clear();

                                        for(int b = 0; b < tempPath.size(); b++)
                                        {
                                            onWhichList[tempPath[b].y][tempPath[b].x] = OnClosedList;
                                            costG[tempPath[b].y][tempPath[b].x] = 1; //marking it
                                            if(b != 0)
                                            {
                                                xParent[tempPath[b].y][tempPath[b].x] = tempPath[b - 1].x;
                                                yParent[tempPath[b].y][tempPath[b].x] = tempPath[b - 1].y;
                                            }
                                            else
                                            {
                                                xParent[tempPath[b].y][tempPath[b].x] = x;
                                                yParent[tempPath[b].y][tempPath[b].x] = y;
                                            }
                                            onListToReset.push_back(tempPath[b]);
                                        }

                                        return true;
                                    }
                                }
                            }
                        }
                    prog_newOpenListId = newOpenListId;
                    prog_numOfOpenList = numOfOpenList;
                }
                else return false;

                if(onWhichList[start.y][start.x] != OnNoList && (useReachSearch ? costG[start.y][start.x] != 1 : true))
                {
                    sf::Vector2i index = start;

                    bool isPathTruncated = false;
                    while(index != target)
                    {
                        if(GetGridInfoTile(index.y, index.x).type == MapTileManager::SmallWall || GetGridInfoTile(index.y, index.x).type == MapTileManager::Wall)
                        {
                            isPathTruncated = true;
                            break;
                        }
                        path.push_back(index);
                        int x = index.x;
                        int y = index.y;
                        index.x = xParent[y][x];
                        index.y = yParent[y][x];
                    }
                    if(isPathTruncated == false && GetGridInfoTile(target.y, target.x).type != MapTileManager::SmallWall && GetGridInfoTile(target.y, target.x).type != MapTileManager::Wall)
                        path.push_back(target);

                    if(useReachSearch)
                    {
                        for(sf::Vector2i& grid : onListToReset_reach)
                            onWhichList[grid.y][grid.x] = OnNoList;
                        onListToReset_reach.clear();

                        for(int b = 0; b < path.size(); b++)
                        {
                            onWhichList[path[b].y][path[b].x] = OnClosedList;
                            costG[path[b].y][path[b].x] = 1; //marking it
                            if(b != 0)
                            {
                                xParent[path[b].y][path[b].x] = path[b - 1].x;
                                yParent[path[b].y][path[b].x] = path[b - 1].y;
                            }
                            onListToReset.push_back(path[b]);
                        }
                    }

                    return true;
                }

                numOfGridsIterated++;
                if(numOfGridsIterated >= perFramLimit && ignoreLimiter == false)
                {
                    isProgressing = true;
                    isLimited = true;
                    break;
                }
            }
        }

        return false;
    }

    bool FindFastPath(sf::Vector2i start, sf::Vector2i target, std::vector<sf::Vector2i>& path, AStar::SearchType searchType, short teamUpType, bool ignoreLimiter, bool useReachSearch)
    {
        if(fast_isLimited == false || ignoreLimiter)
        {
            int MapWidth = AstarWidth;
            int MapHeight = AstarHeight;
            int numOfOpenList = 0;
            int newOpenListId = 0;
            int numOfCurrentIterated = 0;

            if(fast_isContinuing)
            {
                fast_isContinuing = false;
                numOfOpenList = fast_numOfOpenList;
                newOpenListId = fast_newOpenListId;
                target = fast_target;
                start = fast_start;
                searchType = fast_searchType;
                teamUpType = fast_teamUpType;
                useReachSearch = fast_useReachSearch;

                if(useReachSearch && fast_onWhichList[fast_target.y][fast_target.x] == OnClosedList && fast_costG[fast_target.y][fast_target.x] == 1)
                {
                    std::vector<sf::Vector2i> tempPath;
                    sf::Vector2i index(target.x, target.y);

                    while(index != start)
                    {
                        tempPath.push_back(index);
                        int x = index.x;
                        int y = index.y;
                        index.x = fast_xParent[y][x];
                        index.y = fast_yParent[y][x];
                    }
                    tempPath.push_back(start);

                    for(int b = tempPath.size() - 1; b >= 0; b--)
                        path.push_back(tempPath[b]);

                    for(sf::Vector2i& grid : fast_onListToReset_reach)
                        fast_onWhichList[grid.y][grid.x] = OnNoList;
                    fast_onListToReset_reach.clear();

                    return true;
                }

                if(fast_onWhichList[start.y][start.x] != OnNoList && start != target && (useReachSearch ? fast_costG[start.y][start.x] != 1 : true))
                {
                    sf::Vector2i index = start;
                    bool isPathTruncated = false;

                    while(index != target)
                    {
                        if(GetGridInfoTile(index.y, index.x).type == MapTileManager::SmallWall || GetGridInfoTile(index.y, index.x).type == MapTileManager::Wall)
                        {
                            isPathTruncated = true;
                            break;
                        }
                        path.push_back(index);
                        int x = index.x;
                        int y = index.y;
                        index.x = fast_xParent[y][x];
                        index.y = fast_yParent[y][x];
                    }
                    if(isPathTruncated == false && GetGridInfoTile(target.y, target.x).type != MapTileManager::SmallWall && GetGridInfoTile(target.y, target.x).type != MapTileManager::Wall)
                        path.push_back(target);

                    if(useReachSearch)
                    {
                        for(sf::Vector2i& grid : fast_onListToReset_reach)
                            fast_onWhichList[grid.y][grid.x] = OnNoList;
                        fast_onListToReset_reach.clear();

                        for(int b = 0; b < path.size(); b++)
                        {
                            fast_onWhichList[path[b].y][path[b].x] = OnClosedList;
                            fast_costG[path[b].y][path[b].x] = 1; //marking it
                            if(b != 0)
                            {
                                fast_xParent[path[b].y][path[b].x] = path[b - 1].x;
                                fast_yParent[path[b].y][path[b].x] = path[b - 1].y;
                            }
                            fast_onListToReset.push_back(path[b]);
                        }
                    }

                    return true;
                }
            }
            else
            {
                if(useReachSearch) //flip start and target
                {
                    sf::Vector2i temp_start = start;
                    start = target;
                    target = temp_start;
                }

                fast_start = start;
                fast_target = target;
                fast_searchType = searchType;
                fast_teamUpType = teamUpType;
                fast_newOpenListId = newOpenListId;
                fast_numOfOpenList = numOfOpenList;
                fast_useReachSearch = useReachSearch;

                if(start == target || getWalkableStatus(start.y, start.x, searchType, teamUpType, start, target, false) == Unwalkable ||
                   getWalkableStatus(target.y, target.x, searchType, teamUpType, start, target, useReachSearch) == Unwalkable)
                    return false;

                if(start.x > AstarWidth || start.y > AstarHeight || target.x > AstarWidth || target.y > AstarHeight ||
                   start.x < 0 || start.y < 0 || target.x < 0 || target.y < 0)
                    return false;

                for(sf::Vector2i& grid : fast_onListToReset)
                    fast_onWhichList[grid.y][grid.x] = OnNoList;
                fast_onListToReset.clear();

                for(sf::Vector2i& grid : fast_onListToReset_reach)
                    fast_onWhichList[grid.y][grid.x] = OnNoList;
                fast_onListToReset_reach.clear();

                fast_costG[target.y][target.x] = 0;
                numOfOpenList = 1;
                fast_openList[1] = 1;
                fast_openListIndex[target.y][target.x] = 1;
                fast_xOpenList[1] = target.x;
                fast_yOpenList[1] = target.y;
                if(useReachSearch)
                    fast_onListToReset_reach.push_back(target);
                else fast_onListToReset.push_back(target);

                fast_newOpenListId = newOpenListId;
                fast_numOfOpenList = numOfOpenList;
            }

            //Reach variables
            sf::Vector2i temp_reachAbsDiff;
            sf::Vector2i temp_parentAbsDiff;
            bool temp_isConnerSmallWall = false;
            bool temp_isParentSmallWall = false;
            bool temp_isParentMarked = false;
            bool temp_isAlignedWithParent = false;
            bool temp_reachIsDiagnonal = false;
            bool temp_reachIsWithinRange = false;
            int temp_reachRange = 0;

            while(1)
            {
                if(numOfOpenList != 0)
                {
                    int xVal = fast_xOpenList[fast_openList[1]];
                    int yVal = fast_yOpenList[fast_openList[1]];

                    if(useReachSearch)
                    {
                        temp_parentAbsDiff.x = abs(xVal - target.x);
                        temp_parentAbsDiff.y = abs(yVal - target.y);

                        temp_isParentSmallWall = GetGridInfoTile(yVal, xVal).type == MapTileManager::SmallWall;
                        temp_isParentMarked = (fast_costH[fast_openList[fast_openListIndex[yVal][xVal]]] % 10 != 0) || (temp_parentAbsDiff.x == 0 && temp_parentAbsDiff.y == 0);
                    }

                    fast_onWhichList[yVal][xVal] = OnClosedList;
                    fast_openListIndex[yVal][xVal] = 0;
                    fast_openList[1] = fast_openList[numOfOpenList];
                    if(fast_onWhichList[fast_yOpenList[fast_openList[1]]][fast_xOpenList[fast_openList[1]]] == OnOpenList)
                        fast_openListIndex[fast_yOpenList[fast_openList[1]]][fast_xOpenList[fast_openList[1]]] = 1;
                    numOfOpenList--;

                    int listId = 1;

                    while(1)
                    {
                        int initialListId = listId;

                        if(2 * initialListId + 1 <= numOfOpenList)
                        {
                            if(fast_costF[fast_openList[listId]] >= fast_costF[fast_openList[2 * initialListId]])
                                listId = 2 * initialListId;
                            if(fast_costF[fast_openList[listId]] >= fast_costF[fast_openList[2 * initialListId + 1]])
                                listId = 2 * initialListId + 1;
                        }
                        else
                        {
                            if(2 * initialListId <= numOfOpenList)
                                if(fast_costF[fast_openList[listId]] >= fast_costF[fast_openList[2 * initialListId]])
                                        listId = 2 * initialListId;
                        }

                        if(listId != initialListId)
                        {
                            int idCopy = fast_openList[initialListId];
                            fast_openList[initialListId] = fast_openList[listId];
                            fast_openList[listId] = idCopy;

                            fast_openListIndex[fast_yOpenList[fast_openList[listId]]][fast_xOpenList[fast_openList[listId]]] = listId;
                            fast_openListIndex[fast_yOpenList[fast_openList[initialListId]]][fast_xOpenList[fast_openList[initialListId]]] = initialListId;
                        }
                        else break;
                    }

                    for(int y = yVal - 1; y <= yVal + 1; y++)
                        for(int x = xVal - 1; x <= xVal + 1; x++)
                        {
                            int conner = Walkable;

                            if(y >= 0 && y < MapHeight && x >= 0 && x < MapWidth)
                            {
                                if(fast_onWhichList[y][x] != OnClosedList)
                                {
                                    if(useReachSearch)
                                    {
                                        temp_reachAbsDiff.x = abs(x - target.x);
                                        temp_reachAbsDiff.y = abs(y - target.y);

                                        temp_reachIsDiagnonal = temp_reachAbsDiff.x == temp_reachAbsDiff.y;
                                        temp_reachRange = temp_reachAbsDiff.x > temp_reachAbsDiff.y ? temp_reachAbsDiff.x : temp_reachAbsDiff.y;
                                        temp_reachIsWithinRange = temp_reachRange <= REACH_RANGE * (temp_reachIsDiagnonal ? 0.7 : 1);
                                        temp_isConnerSmallWall = GetGridInfoTile(y, x).type == MapTileManager::SmallWall;
                                        temp_isAlignedWithParent = (temp_reachAbsDiff.x == 0 && temp_parentAbsDiff.x == 0) || (temp_reachAbsDiff.y == 0 && temp_parentAbsDiff.y == 0) || (temp_reachAbsDiff.x == temp_reachAbsDiff.y && temp_parentAbsDiff.x == temp_parentAbsDiff.y);

                                        if(temp_isConnerSmallWall && !((temp_isParentMarked && temp_isAlignedWithParent && temp_reachIsWithinRange)))
                                            conner = Unwalkable;

                                        if(temp_isParentSmallWall && !temp_isAlignedWithParent)
                                            conner = Unwalkable;
                                    }

                                    if(conner == Unwalkable)
                                        ;
                                    else if(getWalkableStatus(y, x, searchType, teamUpType, start, target, useReachSearch && temp_reachIsWithinRange))
                                    {
                                        if(x - xVal != 0 && y - yVal != 0)
                                            if(getWalkableStatus(yVal, x, searchType, teamUpType, start, target, useReachSearch && temp_reachIsWithinRange) == Unwalkable ||
                                               getWalkableStatus(y, xVal, searchType, teamUpType, start, target, useReachSearch && temp_reachIsWithinRange) == Unwalkable)
                                                conner = Unwalkable;
                                    }
                                    else conner = Unwalkable;

                                    if(conner == Walkable)
                                    {
                                        if(fast_onWhichList[y][x] != OnOpenList)
                                        {
                                            newOpenListId++;
                                            int newIdOnList = numOfOpenList + 1;
                                            fast_openList[newIdOnList] = newOpenListId;
                                            fast_openListIndex[y][x] = newIdOnList;
                                            fast_xOpenList[newOpenListId] = x;
                                            fast_yOpenList[newOpenListId] = y;

                                            fast_costG[y][x] = fast_costG[yVal][xVal] + (x - xVal != 0 && y - yVal != 0 ? 14 : 10);
                                            fast_costH[fast_openList[newIdOnList]] = 10 * (abs(y - start.y) + abs(x - start.x));

                                            if(useReachSearch && temp_reachRange <= REACH_RANGE && temp_isAlignedWithParent && temp_isParentMarked)
                                                fast_costH[fast_openList[newIdOnList]]++; //marking it

                                            fast_costF[fast_openList[newIdOnList]] = fast_costG[y][x] + fast_costH[fast_openList[newIdOnList]];

                                            fast_xParent[y][x] = xVal;
                                            fast_yParent[y][x] = yVal;

                                            int index = newIdOnList;
                                            while(index != 1)
                                            {
                                                if(fast_costF[fast_openList[index]] <= fast_costF[fast_openList[index / 2]])
                                                {
                                                    int idCopy = fast_openList[index / 2];
                                                    fast_openList[index / 2] = fast_openList[index];
                                                    fast_openList[index] = idCopy;

                                                    fast_openListIndex[fast_yOpenList[fast_openList[index]]][fast_xOpenList[fast_openList[index]]] = index;
                                                    fast_openListIndex[fast_yOpenList[fast_openList[index / 2]]][fast_xOpenList[fast_openList[index / 2]]] = index / 2;

                                                    index = index / 2;
                                                }
                                                else break;
                                            }
                                            numOfOpenList++;
                                            fast_onWhichList[y][x] = OnOpenList;
                                            if(useReachSearch)
                                                fast_onListToReset_reach.push_back(sf::Vector2i(x, y));
                                            else fast_onListToReset.push_back(sf::Vector2i(x, y));
                                        }
                                        else if(useReachSearch ? temp_isConnerSmallWall == false : true)
                                        {
                                            int totalGCost = fast_costG[yVal][xVal] + (x - xVal != 0 && y - yVal != 0 ? 14 : 10);

                                            if(totalGCost < fast_costG[y][x])
                                            {
                                                fast_xParent[y][x] = xVal;
                                                fast_yParent[y][x] = yVal;
                                                fast_costG[y][x] = totalGCost;

                                                int index = fast_openListIndex[y][x];
                                                if(useReachSearch && temp_reachRange <= REACH_RANGE && temp_isParentMarked && temp_isAlignedWithParent)
                                                    fast_costH[fast_openList[index]]++; //marking it
                                                fast_costF[fast_openList[index]] = fast_costH[fast_openList[index]] + fast_costG[y][x];

                                                while(index != 1)
                                                {
                                                    if(fast_costF[fast_openList[index]] < fast_costF[fast_openList[index / 2]])
                                                    {
                                                        int idCopy = fast_openList[index / 2];
                                                        fast_openList[index / 2] = fast_openList[index];
                                                        fast_openList[index] = idCopy;

                                                        fast_openListIndex[fast_yOpenList[fast_openList[index]]][fast_xOpenList[fast_openList[index]]] = index;
                                                        fast_openListIndex[fast_yOpenList[fast_openList[index / 2]]][fast_xOpenList[fast_openList[index / 2]]] = index / 2;

                                                        index /= 2;
                                                    }
                                                    else break;
                                                }
                                            }
                                        }
                                    }
                                }
                                else if(useReachSearch && fast_costG[y][x] == 1)
                                {
                                    if((x - xVal != 0 && y - yVal != 0) ?
                                       (!(fast_onWhichList[yVal][x] == OnClosedList && fast_costG[yVal][x] == 1) && !(fast_onWhichList[y][xVal] == OnClosedList && fast_costG[y][xVal] == 1) &&
                                        GetGridInfoTile(yVal, x).type != MapTileManager::Wall && GetGridInfoTile(yVal, x).type != MapTileManager::SmallWall &&
                                        GetGridInfoTile(y, xVal).type != MapTileManager::Wall && GetGridInfoTile(y, xVal).type != MapTileManager::SmallWall) : true)
                                    {
                                        std::vector<sf::Vector2i> tempPath;
                                        sf::Vector2i index(x, y);

                                        while(index != start)
                                        {
                                            tempPath.push_back(index);
                                            int x = index.x;
                                            int y = index.y;
                                            index.x = fast_xParent[y][x];
                                            index.y = fast_yParent[y][x];
                                        }
                                        tempPath.push_back(start);

                                        for(int b = tempPath.size() - 1; b >= 0; b--)
                                            path.push_back(tempPath[b]);

                                        index.x = xVal;
                                        index.y = yVal;
                                        tempPath.clear();
                                        bool isPathTruncated = false;
                                        while(index != target)
                                        {
                                            if(GetGridInfoTile(index.y, index.x).type == MapTileManager::SmallWall || GetGridInfoTile(index.y, index.x).type == MapTileManager::Wall)
                                            {
                                                isPathTruncated = true;
                                                break;
                                            }
                                            tempPath.push_back(index);
                                            path.push_back(index);
                                            int x = index.x;
                                            int y = index.y;
                                            index.x = fast_xParent[y][x];
                                            index.y = fast_yParent[y][x];
                                        }
                                        if(isPathTruncated == false && GetGridInfoTile(target.y, target.x).type != MapTileManager::SmallWall && GetGridInfoTile(target.y, target.x).type != MapTileManager::Wall)
                                        {
                                            path.push_back(target);
                                            tempPath.push_back(target);
                                        }

                                        for(sf::Vector2i& grid : fast_onListToReset_reach)
                                            fast_onWhichList[grid.y][grid.x] = OnNoList;
                                        fast_onListToReset_reach.clear();

                                        for(int b = 0; b < tempPath.size(); b++)
                                        {
                                            fast_onWhichList[tempPath[b].y][tempPath[b].x] = OnClosedList;
                                            fast_costG[tempPath[b].y][tempPath[b].x] = 1; //marking it
                                            if(b != 0)
                                            {
                                                fast_xParent[tempPath[b].y][tempPath[b].x] = tempPath[b - 1].x;
                                                fast_yParent[tempPath[b].y][tempPath[b].x] = tempPath[b - 1].y;
                                            }
                                            else
                                            {
                                                fast_xParent[tempPath[b].y][tempPath[b].x] = x;
                                                fast_yParent[tempPath[b].y][tempPath[b].x] = y;
                                            }
                                            fast_onListToReset.push_back(tempPath[b]);
                                        }

                                        return true;
                                    }
                                }
                            }
                        }
                    fast_newOpenListId = newOpenListId;
                    fast_numOfOpenList = numOfOpenList;
                }
                else return false;

                if(fast_onWhichList[start.y][start.x] == OnOpenList && (useReachSearch ? fast_costG[start.y][start.x] != 1 : true))
                {
                    sf::Vector2i index = start;

                    bool isPathTruncated = false;
                    while(index != target)
                    {
                        if(GetGridInfoTile(index.y, index.x).type == MapTileManager::SmallWall || GetGridInfoTile(index.y, index.x).type == MapTileManager::Wall)
                        {
                            isPathTruncated = true;
                            break;
                        }
                        path.push_back(index);
                        int x = index.x;
                        int y = index.y;
                        index.x = fast_xParent[y][x];
                        index.y = fast_yParent[y][x];
                    }
                    if(isPathTruncated == false && GetGridInfoTile(target.y, target.x).type != MapTileManager::SmallWall && GetGridInfoTile(target.y, target.x).type != MapTileManager::Wall)
                        path.push_back(target);

                    if(useReachSearch)
                    {
                        for(sf::Vector2i& grid : fast_onListToReset_reach)
                            fast_onWhichList[grid.y][grid.x] = OnNoList;
                        fast_onListToReset_reach.clear();

                        for(int b = 0; b < path.size(); b++)
                        {
                            fast_onWhichList[path[b].y][path[b].x] = OnClosedList;
                            fast_costG[path[b].y][path[b].x] = 1; //marking it
                            if(b != 0)
                            {
                                fast_xParent[path[b].y][path[b].x] = path[b - 1].x;
                                fast_yParent[path[b].y][path[b].x] = path[b - 1].y;
                            }
                            fast_onListToReset.push_back(path[b]);
                        }
                    }

                    return true;
                }

                fast_numOfGridsIterated++;
                numOfCurrentIterated++;
                if(numOfCurrentIterated >= LIMIT_FAST_SEARCH)
                    break;
            }
        }
        return false;
    }
}
