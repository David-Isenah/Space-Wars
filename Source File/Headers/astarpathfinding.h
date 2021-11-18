#ifndef ASTARPATHFINDING_H_INCLUDED
#define ASTARPATHFINDING_H_INCLUDED

#include "SFML/System/Vector2.hpp"
#include "vector"

namespace AStar
{
    /*
    Note:
    -   Path find always returns a found path from start to target whether with reaching or not.
    -   Continuing search continues where last search stops with a different start location.
    -   Path find with no reaching begins search from target to start.
    -   Path find with reaching begins search from start to target.
    -   Start and target should always be on a floor or empty tile in the case of normal search. In the case of reach
        search, target should always be on a floor or empty tile while start can be on floor, empty or small wall.
    -   Reaching tries to reach the nearest spot from a set range that can see the start location from target if it
        is block by smalls walls and walls. It will return a normal search path if its distance is smaller.
    -   Use search with reaching very carefully. Always use normal search over reach search where possible.
    -   Fast search on iterates for a short number of time and stops if path a path is not found. Normal searches the
        entire AStar search range until there is no possible path.
    -   Progression is...complicated. Helps set search limit per frame. Will be useless once incorporated in threads.
    */

    const int LIMIT_PER_FRAME = 1200;
    const int LIMIT_FAST_SEARCH = 40;
    const int LIMIT_BALANCE_PER_FRAME = 120;

    enum SearchType{NoSearchType = -1, AvoidEnemySearchType, BaseRegionSearchType};

    bool FindPath(sf::Vector2i start, sf::Vector2i target, std::vector<sf::Vector2i>& path, SearchType searchType = NoSearchType, short teamUpType = -1, bool ignoreLimiter = false, bool useReachSearch = false);
    bool FindFastPath(sf::Vector2i start, sf::Vector2i target, std::vector<sf::Vector2i>& path, SearchType searchType = NoSearchType, short teamUpType = -1, bool ignoreLimiter = false, bool useReachSearch = false);
    void ContinueLastTargetSearch(sf::Vector2i start_);
    void ContinueLastTargetFastSearch(sf::Vector2i start_);
    void SetSearchRange(int yRange, int xRange);
    void SetProgressiveSearchStatus(bool status);
    void SetFastSearchStatus(bool status);
    bool IsReadyToSearch(bool isFastSerach);
    bool GetLimitedState();
    bool GetFastSearchLimitedState();
    bool GetProgressionState();
    void CancleProgression();
    void UpdateLimiter();
    void ResetLimiter();
}

#endif // ASTARPATHFINDING_H_INCLUDED
