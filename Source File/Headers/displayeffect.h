#ifndef DISPLAYEFFECT_H_INCLUDED
#define DISPLAYEFFECT_H_INCLUDED

#include "SFML/Graphics.hpp"

class DisplayEffect
{
protected:
    bool isActive;

public:
    const float REACTION_TIME = 0.08f;

    DisplayEffect():
        isActive(true)
    {
    }

    bool IsActive()
    {
        return isActive;
    }

    virtual void RefreshEntities()
    {
    }

    virtual void Update(float dt) = 0;

    static void Initialise();
    static void AddDisplayEffect(DisplayEffect* effect_);
    static void UpdateDisplayEffects(float dt);
    static void RefreshDisplayEffects();
    static void ClearDisplayEffects();
};

#endif // DISPLAYEFFECT_H_INCLUDED
