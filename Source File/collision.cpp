#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED

#include<SFML/Graphics.hpp>
#include<collision.h>
#include<math.h>
#include<vector>
#include<iostream>

std::vector<sf::Vector2f> collisionAxis;
float collisionAxesDotProduct = 0.f;
float collisionEntryAxisSqr = 0.f;

/*sf::Vector2f collisionCircleAxes[4] =
{
    sf::Vector2f(cos(22.f / 7.f / 8.f), sin(22.f / 7.f / 8.f)),
    sf::Vector2f(cos(22.f / 7.f / 8.f * 3.f), sin(22.f / 7.f / 8.f * 3.f)),
    sf::Vector2f(cos(22.f / 7.f / 8.f * 5.f), sin(22.f / 7.f / 8.f * 5.f)),
    sf::Vector2f(cos(22.f / 7.f / 8.f * 7.f), sin(22.f / 7.f / 8.f * 7.f))
};*/

float DotProduct(const sf::Vector2f& vec1, const sf::Vector2f& vec2)
{
    return (vec1.x * vec2.x + vec1.y * vec2.y);
}

bool AxisCollision(sf::Vector2f& axisVec,
                   const std::vector<sf::Vector2f>* objPointPos, const std::vector<sf::Vector2f>* obstPointPos,
                   const sf::Vector2f* objCircleCenter = nullptr, const float* objCircleRadius = nullptr, const sf::Vector2f* obstCircleCenter = nullptr, const float* obstCircleRadius = nullptr,
                   sf::Vector2f* pushVec = nullptr, float* pushMagnSqr = nullptr, const sf::Vector2f* pushEntryDir = nullptr)
{
    float obj1MaxProj = 0.f;
    float obj1MinProj = 0.f;
    float obj2MaxProj = 0.f;
    float obj2MinProj = 0.f;
    float proj = 0.f;

    bool isPushEntryCollision = pushVec != nullptr && pushMagnSqr != nullptr && pushEntryDir != nullptr;
    if(isPushEntryCollision)
    {
        bool isPerpendicularToEntry = false;
        if(pushEntryDir->y == 0 && axisVec.x == 0 || pushEntryDir->x == 0 && axisVec.y == 0)
            isPerpendicularToEntry = true;
        else if(axisVec.x != 0 && axisVec.y != 0)
        {
            obj1MaxProj = pushEntryDir->y / pushEntryDir->x;
            obj1MinProj = axisVec.x / axisVec.y;
            obj2MaxProj = (obj1MaxProj < 0 ? -obj1MaxProj : obj1MaxProj) - (obj1MinProj < 0 ? -obj1MinProj : obj1MinProj);
            if((obj2MaxProj < 0 ? -obj2MaxProj : obj2MaxProj) < 0.001f)
                isPerpendicularToEntry = true;
            obj1MaxProj = obj1MinProj = obj2MaxProj = 0.f;
        }

        if(isPerpendicularToEntry)
            collisionAxesDotProduct = 0;
        else collisionAxesDotProduct = DotProduct(axisVec, *pushEntryDir);
        collisionEntryAxisSqr = DotProduct(*pushEntryDir, *pushEntryDir);

        if(collisionAxesDotProduct > 0)
        {
            axisVec *= -1.f;
            collisionAxesDotProduct *= -1.f;
        }
    }

    //for object
    if(objCircleCenter != nullptr && objCircleRadius != nullptr)
    {
        //axisVec should be a unit vector because of radius
        obj1MaxProj = obj1MinProj = DotProduct(*objCircleCenter, axisVec);
        obj1MaxProj += *objCircleRadius;
        obj1MinProj -= *objCircleRadius;
    }
    else if(objPointPos != nullptr)
    {
        obj1MaxProj = obj1MinProj = DotProduct(objPointPos->at(1), axisVec);

        for(int b = 2; b < objPointPos->size(); b++)
        {
            proj = DotProduct(objPointPos->at(b), axisVec);
            if(proj < obj1MinProj)
                obj1MinProj = proj;

            if(proj > obj1MaxProj)
                obj1MaxProj = proj;
        }
    }

    //for obstacle
    if(obstCircleCenter != nullptr && obstCircleRadius != nullptr)
    {
        //axisVec should be a unit vector because of radius
        obj2MaxProj = obj2MinProj = DotProduct(*obstCircleCenter, axisVec);
        obj2MaxProj += *obstCircleRadius;
        obj2MinProj -= *obstCircleRadius;
    }
    else if(obstPointPos != nullptr)
    {
        obj2MaxProj = obj2MinProj = DotProduct(obstPointPos->at(1), axisVec);

        for(int b = 2; b < obstPointPos->size(); b++)
        {
            proj = DotProduct(obstPointPos->at(b), axisVec);
            if(proj < obj2MinProj)
                obj2MinProj = proj;

            if(proj > obj2MaxProj)
                obj2MaxProj = proj;
        }
    }

    if(obj1MaxProj < obj2MinProj || obj2MaxProj < obj1MinProj)//add equals sign to make touching count as colliding
        return false;

    if(isPushEntryCollision)
    {
        if(collisionAxesDotProduct != 0)
        {
            sf::Vector2f tempPushVec = (obj2MaxProj - obj1MinProj) / collisionAxesDotProduct * *pushEntryDir;
            float tempPushMagnSqr = DotProduct(tempPushVec, tempPushVec);

            if(*pushMagnSqr < 0 || tempPushMagnSqr < *pushMagnSqr)
            {
                *pushMagnSqr = tempPushMagnSqr;
                *pushVec = tempPushVec;
            }
        }
    }
    else if(pushVec != nullptr && pushMagnSqr != nullptr)
    {
        float diff = 0.f;
        bool isInverted = false;

        if(obj1MaxProj - obj2MinProj < obj2MaxProj - obj1MinProj)
        {
            diff = obj1MaxProj - obj2MinProj;
            isInverted = !isInverted;
        }
        else diff = obj2MaxProj - obj1MinProj;

        sf::Vector2f tempPushVec = axisVec * diff * (isInverted ? -1.f : 1.f) / DotProduct(axisVec, axisVec);
        float tempPushMagnSqr = DotProduct(tempPushVec, tempPushVec);

        if(*pushMagnSqr < 0 || tempPushMagnSqr < *pushMagnSqr)
        {
            *pushMagnSqr = tempPushMagnSqr;
            *pushVec = tempPushVec;
        }
    }


    return true;
}

bool GetAxis(const std::vector<sf::Vector2f>* obj1PointPos, const std::vector<sf::Vector2f>* obj2PointPos,
             const sf::Vector2f* obj1CircleCenter = nullptr, const float* obj1CircleRadius = nullptr, const sf::Vector2f* obj2CircleCenter = nullptr, const float* obj2CircleRadius = nullptr,
             sf::Vector2f* pushVec = nullptr, float* pushMagnSqr = nullptr, sf::Vector2f* pushEntryDir = nullptr)
{
    float tempValue = 0;
    sf::Vector2f tempAxisVec;
    bool flipPush = false;

    bool isObj1Circle = obj1CircleCenter != nullptr && obj1CircleRadius != nullptr;
    bool isObj2Circle = obj2CircleCenter != nullptr && obj2CircleRadius != nullptr;
    if(isObj2Circle && isObj1Circle == false)
    {
        obj1CircleCenter = obj2CircleCenter;
        obj1CircleRadius = obj2CircleRadius;
        obj2PointPos = obj1PointPos;
        obj1PointPos = nullptr;
        isObj1Circle = true;
        isObj2Circle = false;
        flipPush = true;
        if(pushEntryDir != nullptr)
            *pushEntryDir *= -1.f;
    }


    if(isObj1Circle && isObj2Circle) //circle vs circle
    {
        if(pushVec != nullptr && pushMagnSqr != nullptr)
        {
            if(pushEntryDir != nullptr)
                tempAxisVec = *pushEntryDir / static_cast<float>(sqrt(pushEntryDir->x * pushEntryDir->x + pushEntryDir->y * pushEntryDir->y));
            else if(*obj1CircleCenter == *obj2CircleCenter)
                tempAxisVec = sf::Vector2f(1, 0);
            else
            {
                tempAxisVec = *obj1CircleCenter - *obj2CircleCenter;
                tempAxisVec /= static_cast<float>(sqrt(tempAxisVec.x * tempAxisVec.x + tempAxisVec.y * tempAxisVec.y));
            }

            if(AxisCollision(tempAxisVec, nullptr, nullptr, obj1CircleCenter, obj1CircleRadius, obj2CircleCenter, obj2CircleRadius, pushVec, pushMagnSqr, pushEntryDir) == false)
                return false;

            if(pushEntryDir != nullptr)
            {
                tempAxisVec = *obj1CircleCenter + *pushVec - *obj2CircleCenter;
                float dist = tempAxisVec.x * tempAxisVec.x + tempAxisVec.y * tempAxisVec.y;

                if(dist > (*obj1CircleRadius + *obj2CircleRadius + 2.f) * (*obj1CircleRadius + *obj2CircleRadius + 2.f))
                {
                    dist = sqrt(dist);
                    tempAxisVec /= dist;
                    sf::Vector2f alignmentVec = (dist - *obj1CircleRadius - *obj2CircleRadius) / DotProduct(*pushEntryDir, tempAxisVec) * *pushEntryDir;
                    *pushVec -= alignmentVec;
                }
            }
        }
        else
        {
            tempAxisVec = *obj1CircleCenter - *obj2CircleCenter;
            return tempAxisVec.x * tempAxisVec.x + tempAxisVec.y * tempAxisVec.y < (*obj1CircleRadius + *obj2CircleRadius) * (*obj1CircleRadius + *obj2CircleRadius);
        }
    }
    else if(isObj1Circle && obj2PointPos != nullptr) //circle vs polygon
    {
        int closestIndex = -1;
        float closestDistSqr = -1.f;

        for(int a = 1; a < obj2PointPos->size(); a++)
        {
            tempAxisVec = *obj1CircleCenter - obj2PointPos->at(a);
            tempValue = tempAxisVec.x * tempAxisVec.x + tempAxisVec.y * tempAxisVec.y;
            if(closestDistSqr < 0 || tempValue < closestDistSqr)
            {
                closestIndex = a;
                closestDistSqr = tempValue;
            }
        }

        tempAxisVec = *obj1CircleCenter - obj2PointPos->at(closestIndex);

        sf::Vector2f firstConnerAxis = obj2PointPos->at(closestIndex - 1 < 1 ? obj2PointPos->size() - 1 : closestIndex - 1) - obj2PointPos->at(closestIndex);
        sf::Vector2f secondConnerAxis = obj2PointPos->at(closestIndex + 1 > obj2PointPos->size() - 1 ? 1 : closestIndex + 1) - obj2PointPos->at(closestIndex);
        bool checkFirstConnerAxis = firstConnerAxis != sf::Vector2f() && DotProduct(firstConnerAxis, tempAxisVec) > 0;
        bool checkSecondConnerAxis = secondConnerAxis != sf::Vector2f() && DotProduct(secondConnerAxis, tempAxisVec) > 0;

        if(pushVec != nullptr && pushMagnSqr != nullptr)
        {
            if(pushEntryDir != nullptr)
            {
               sf::Vector2f* connerAxisChecked = nullptr;

                if(checkFirstConnerAxis && checkSecondConnerAxis)
                    ;
                else if(checkFirstConnerAxis)
                {
                    firstConnerAxis = sf::Vector2f(-firstConnerAxis.y, firstConnerAxis.x);
                    if(DotProduct(firstConnerAxis, *pushEntryDir) < 0)
                    {
                        connerAxisChecked = &firstConnerAxis;
                        firstConnerAxis /= static_cast<float>(sqrt(firstConnerAxis.x * firstConnerAxis.x + firstConnerAxis.y * firstConnerAxis.y));
                        if(AxisCollision(firstConnerAxis, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr, pushVec, pushMagnSqr, pushEntryDir) == false)
                            return false;
                    }
                    else
                    {
                        firstConnerAxis /= static_cast<float>(sqrt(firstConnerAxis.x * firstConnerAxis.x + firstConnerAxis.y * firstConnerAxis.y));
                        if(AxisCollision(firstConnerAxis, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr) == false)
                            return false;
                    }
                }
                else if(checkSecondConnerAxis)
                {
                    secondConnerAxis = sf::Vector2f(secondConnerAxis.y, -secondConnerAxis.x);
                    if(DotProduct(secondConnerAxis, *pushEntryDir) < 0)
                    {
                        connerAxisChecked = &secondConnerAxis;
                        secondConnerAxis /= static_cast<float>(sqrt(secondConnerAxis.x * secondConnerAxis.x + secondConnerAxis.y * secondConnerAxis.y));
                        if(AxisCollision(secondConnerAxis, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr, pushVec, pushMagnSqr, pushEntryDir) == false)
                            return false;
                    }
                    else
                    {
                        secondConnerAxis /= static_cast<float>(sqrt(secondConnerAxis.x * secondConnerAxis.x + secondConnerAxis.y * secondConnerAxis.y));
                        if(AxisCollision(secondConnerAxis, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr) == false)
                            return false;
                    }
                }
                else
                {
                    tempAxisVec /= static_cast<float>(sqrt(tempAxisVec.x * tempAxisVec.x + tempAxisVec.y * tempAxisVec.y));
                    if(AxisCollision(tempAxisVec, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr) == false)
                        return false;
                }

                for(int a = 1; a < obj2PointPos->size(); a++)
                {
                    if(a == obj2PointPos->size() - 1)
                        tempAxisVec = obj2PointPos->at(1) - obj2PointPos->at(a);
                    else tempAxisVec = obj2PointPos->at(a + 1) - obj2PointPos->at(a);
                    tempAxisVec = sf::Vector2f(tempAxisVec.y, -tempAxisVec.x);

                    if(DotProduct(tempAxisVec, *pushEntryDir) < 0 &&
                       (connerAxisChecked != nullptr ? tempAxisVec != *connerAxisChecked && tempAxisVec != -*connerAxisChecked : true))
                    {
                        tempAxisVec /= static_cast<float>(sqrt(tempAxisVec.x * tempAxisVec.x + tempAxisVec.y * tempAxisVec.y));
                        if(AxisCollision(tempAxisVec, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr, pushVec, pushMagnSqr, pushEntryDir) == false)
                            return false;
                    }
                }

                if(*pushMagnSqr >= 0) //alignment of push vector if it aligns with closest point
                {
                    closestIndex = -1;
                    closestDistSqr = -1.f;
                    for(int a = 1; a < obj2PointPos->size(); a++)
                    {
                        tempAxisVec = *obj1CircleCenter + *pushVec - obj2PointPos->at(a);
                        tempValue = tempAxisVec.x * tempAxisVec.x + tempAxisVec.y * tempAxisVec.y;
                        if(closestDistSqr < 0 || tempValue < closestDistSqr)
                        {
                            closestIndex = a;
                            closestDistSqr = tempValue;
                        }
                    }

                    tempAxisVec = *obj1CircleCenter + *pushVec - obj2PointPos->at(closestIndex);

                    firstConnerAxis = obj2PointPos->at(closestIndex - 1 < 1 ? obj2PointPos->size() - 1 : closestIndex - 1) - obj2PointPos->at(closestIndex);
                    secondConnerAxis = obj2PointPos->at(closestIndex + 1 > obj2PointPos->size() - 1 ? 1 : closestIndex + 1) - obj2PointPos->at(closestIndex);
                    checkFirstConnerAxis = firstConnerAxis != sf::Vector2f() && DotProduct(firstConnerAxis, tempAxisVec) > 0;
                    checkSecondConnerAxis = secondConnerAxis != sf::Vector2f() && DotProduct(secondConnerAxis, tempAxisVec) > 0;

                    if(checkFirstConnerAxis == false && checkSecondConnerAxis == false)
                    {
                        float dist = tempAxisVec.x * tempAxisVec.x + tempAxisVec.y * tempAxisVec.y;
                        if(dist > (*obj1CircleRadius + 2.f) * (*obj1CircleRadius + 2.f))
                        {
                            dist = sqrt(dist);
                            tempAxisVec /= dist;
                            sf::Vector2f alignmentVec = (dist - *obj1CircleRadius) / DotProduct(*pushEntryDir, tempAxisVec) * *pushEntryDir;
                            *pushVec -= alignmentVec;
                        }
                    }
                }
            }
            else if(checkFirstConnerAxis == false && checkSecondConnerAxis == false)
            {
                tempAxisVec /= static_cast<float>(sqrt(tempAxisVec.x * tempAxisVec.x + tempAxisVec.y * tempAxisVec.y));
                if(AxisCollision(tempAxisVec, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr, pushVec, pushMagnSqr, pushEntryDir) == false)
                    return false;
            }
            else
            {
                if(checkFirstConnerAxis)
                {
                    firstConnerAxis = sf::Vector2f(-firstConnerAxis.y, firstConnerAxis.x);
                    firstConnerAxis /= static_cast<float>(sqrt(firstConnerAxis.x * firstConnerAxis.x + firstConnerAxis.y * firstConnerAxis.y));
                    if(AxisCollision(firstConnerAxis, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr, pushVec, pushMagnSqr, pushEntryDir) == false)
                        return false;
                }
                if(checkSecondConnerAxis)
                {
                    secondConnerAxis = sf::Vector2f(secondConnerAxis.y, -secondConnerAxis.x);
                    secondConnerAxis /= static_cast<float>(sqrt(secondConnerAxis.x * secondConnerAxis.x + secondConnerAxis.y * secondConnerAxis.y));
                    if(AxisCollision(secondConnerAxis, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr, pushVec, pushMagnSqr, pushEntryDir) == false)
                        return false;
                }
            }
        }
        else
        {
            if(checkFirstConnerAxis && checkSecondConnerAxis)
                return true;
            else if(checkFirstConnerAxis)
            {
                firstConnerAxis = sf::Vector2f(-firstConnerAxis.y, firstConnerAxis.x);
                firstConnerAxis /= static_cast<float>(sqrt(firstConnerAxis.x * firstConnerAxis.x + firstConnerAxis.y * firstConnerAxis.y));
                return AxisCollision(firstConnerAxis, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr, pushVec, pushMagnSqr, pushEntryDir);
            }
            else if(checkSecondConnerAxis)
            {
                secondConnerAxis = sf::Vector2f(secondConnerAxis.y, -secondConnerAxis.x);
                secondConnerAxis /= static_cast<float>(sqrt(secondConnerAxis.x * secondConnerAxis.x + secondConnerAxis.y * secondConnerAxis.y));
                return AxisCollision(secondConnerAxis, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr, pushVec, pushMagnSqr, pushEntryDir);
            }
            else
            {
                tempAxisVec /= static_cast<float>(sqrt(tempAxisVec.x * tempAxisVec.x + tempAxisVec.y * tempAxisVec.y));
                return AxisCollision(tempAxisVec, nullptr, obj2PointPos, obj1CircleCenter, obj1CircleRadius, nullptr, nullptr, pushVec, pushMagnSqr, pushEntryDir);
            }
        }
    }
    else if(obj1PointPos != nullptr && obj2PointPos != nullptr) //polygon vs polygon
    {
        bool hasDoneAxis = false;

        //first object points
        if(obj1PointPos != nullptr)
        {
            for(int a = 1; a < obj1PointPos->size(); a++)
            {
                if(a == obj1PointPos->size() - 1)
                    tempAxisVec = obj1PointPos->at(1) - obj1PointPos->at(a);
                else tempAxisVec = obj1PointPos->at(a + 1) - obj1PointPos->at(a);
                tempAxisVec = sf::Vector2f(tempAxisVec.y, -tempAxisVec.x);

                hasDoneAxis = tempAxisVec == sf::Vector2f();
                for(int b = 0; hasDoneAxis == false && b < collisionAxis.size(); b++)
                {
                    if(tempAxisVec.x == 0)
                        hasDoneAxis = collisionAxis[b].x == 0;
                    else if(tempAxisVec.y == 0)
                        hasDoneAxis == collisionAxis[b].y == 0;
                    else if(collisionAxis[b].x != 0 && collisionAxis[b].x != 0)
                    {
                        tempValue = tempAxisVec.y / tempAxisVec.x - collisionAxis[b].y / collisionAxis[b].x;
                        hasDoneAxis = (tempValue < 0 ? -tempValue : tempValue) < 0.001f; //should be zero but floats cant always be exact

                    }
                }

                if(hasDoneAxis == false)
                {
                    collisionAxis.push_back(tempAxisVec);

                    if(AxisCollision(tempAxisVec, obj1PointPos, obj2PointPos, obj1CircleCenter, obj1CircleRadius, obj2CircleCenter, obj2CircleRadius, pushVec, pushMagnSqr, pushEntryDir) == false)
                        return false;
                }
            }
        }

        //second object points
        if(obj2PointPos != nullptr)
        {
            for(int a = 1; a < obj2PointPos->size(); a++)
            {
                if(a == obj2PointPos->size() - 1)
                    tempAxisVec = obj2PointPos->at(1) - obj2PointPos->at(a);
                else tempAxisVec = obj2PointPos->at(a + 1) - obj2PointPos->at(a);
                tempAxisVec = sf::Vector2f(tempAxisVec.y, -tempAxisVec.x);

                hasDoneAxis = tempAxisVec == sf::Vector2f();
                for(int b = 0; hasDoneAxis == false && b < collisionAxis.size(); b++)
                {
                    if(tempAxisVec.x == 0)
                        hasDoneAxis = collisionAxis[b].x == 0;
                    else if(tempAxisVec.y == 0)
                        hasDoneAxis == collisionAxis[b].y == 0;
                    else if(collisionAxis[b].x != 0 && collisionAxis[b].x != 0)
                    {
                        tempValue = tempAxisVec.y / tempAxisVec.x - collisionAxis[b].y / collisionAxis[b].x;
                        hasDoneAxis = (tempValue < 0 ? -tempValue : tempValue) < 0.001f; //should be zero but floats cant always be exact
                    }
                }

                if(hasDoneAxis == false)
                {
                    collisionAxis.push_back(tempAxisVec);

                    if(AxisCollision(tempAxisVec, obj1PointPos, obj2PointPos, obj1CircleCenter, obj1CircleRadius, obj2CircleCenter, obj2CircleRadius, pushVec, pushMagnSqr, pushEntryDir) == false)
                        return false;
                }
            }
        }
    }
    else return false;

    if(flipPush)
    {
        if(pushVec != nullptr)
            *pushVec *= -1.f;
        if(pushEntryDir != nullptr)
            *pushEntryDir *= -1.f;
    }

    return true;
}

bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const std::vector<sf::Vector2f>& obj2DotsPos)
{
    collisionAxis.clear();
    if(GetAxis(&obj1DotsPos, &obj2DotsPos) == false)
        return false;

    return true;
}

bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const std::vector<sf::Vector2f>& obj2DotsPos, sf::Vector2f& pushVector)
{
    float pushMagnSqr = -1;

    collisionAxis.clear();
    if(GetAxis(&obj1DotsPos, &obj2DotsPos, nullptr, nullptr, nullptr, nullptr, &pushVector, &pushMagnSqr) == false)
        return false;

    return true;
}

bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const std::vector<sf::Vector2f>& obj2DotsPos, sf::Vector2f& obj1EntryDir, sf::Vector2f& pushVector)
{
    float pushMagnSqr = -1;

    collisionAxis.clear();
    if(GetAxis(&obj1DotsPos, &obj2DotsPos, nullptr, nullptr, nullptr, nullptr, &pushVector, &pushMagnSqr, &obj1EntryDir) == false)
        return false;

    return true;
}

bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const sf::Vector2f& obj2CircleCenter, const float& obj2CircleRadius, sf::Vector2f& pushVector)
{
     float pushMagnSqr = -1;

    collisionAxis.clear();
    if(GetAxis(&obj1DotsPos, nullptr, nullptr, nullptr, &obj2CircleCenter, &obj2CircleRadius, &pushVector, &pushMagnSqr) == false)
        return false;

    return true;
}

bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const sf::Vector2f& obj2CircleCenter, const float& obj2CircleRadius, sf::Vector2f& obj1EntryDir, sf::Vector2f& pushVector)
{
     float pushMagnSqr = -1;

    collisionAxis.clear();
    if(GetAxis(&obj1DotsPos, nullptr, nullptr, nullptr, &obj2CircleCenter, &obj2CircleRadius, &pushVector, &pushMagnSqr, &obj1EntryDir) == false)
        return false;

    return true;
}

/*
old implementation(deprecated)

bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const sf::Vector2f& obj2Pos, const float& radius, sf::Vector2f& pushVector)
{
    sf::Vector2f axisVec = obj1DotsPos[0] - obj2Pos;
    axisVec /= (float)sqrt(axisVec.x * axisVec.x + axisVec.y * axisVec.y);

    float obj2MaxProj = DotProduct(obj2Pos, axisVec) + radius;
    float obj2MinProj = obj2MaxProj - 2 * radius;

    float obj1MaxProj = DotProduct(obj1DotsPos[1], axisVec);
    float obj1MinProj = DotProduct(obj1DotsPos[1], axisVec);

    float proj = 0.f;

    for(int b = 2; b < obj1DotsPos.size(); b++)
    {
        proj = DotProduct(obj1DotsPos[b], axisVec);
        if(proj < obj1MinProj)
            obj1MinProj = proj;

        if(proj > obj1MaxProj)
            obj1MaxProj = proj;
    }

    if(obj1MaxProj < obj2MinProj || obj2MaxProj < obj1MinProj)//add equals sign to make touching count as colliding
        return false;

    float pushMagn = obj1MaxProj - obj2MinProj;
    float tempDist = obj1MinProj - obj2MaxProj;
    if(tempDist * tempDist < pushMagn * pushMagn)
        pushMagn = tempDist;

    pushVector = axisVec * pushMagn;
    return true;
}

bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const std::vector<sf::Vector2f>& obj2DotsPos, const sf::Vector2f& obj1EntryDir, sf::Vector2f& pushVector, float displacement)
{
    if(obj1DotsPos.size() >= 3 && obj2DotsPos.size() >= 3)
    {
        sf::Vector2f obj1LineStartPoint;
        sf::Vector2f obj1LineEndPoint;
        sf::Vector2f obj2LineStartPoint;
        sf::Vector2f obj2LineEndPoint;
        float obj1MaxProj = 0.f;
        float obj1MinProj = 0.f;
        float obj2MaxProj = 0.f;
        float obj2MinProj = 0.f;
        float obj1PerpLineAxis = 0.f;
        float obj2PerpLineAxis = 0.f;
        float axesDotProduct = 0.f;
        float maxPushMagn = -1.f;

        float tempValue = 0.f;
        sf::Vector2f tempVec = 0.f;
        bool hasCheckedLineCollsion = false;

        for(int a = 1; a < obj2DotsPos.size(); a++)
        {
            obj2LineStartPoint = obj2DotsPos[a];
            obj2LineEndPoint = obj2DotsPos[a == obj2DotsPos.size() - 1 ? 1 : a + 1];
            obj2PerpLineAxis = obj2LineEndPoint - obj2LineStartPoint;
            obj2PerpLineAxis = sf::Vector2f(-obj2PerpLineAxis.y, obj2PerpLineAxis.x);
            axesDotProduct = DotProduct(obj2PerpLineAxis, obj1EntryDir);

            for(int b = 1; b < obj1DotsPos.size(); b++)
            {
                obj1LineStartPoint = obj1DotsPos[a];
                obj1LineEndPoint = obj1DotsPos[a == obj1DotsPos.size() - 1 ? 1 : b + 1];
                obj1PerpLineAxis = obj1LineEndPoint - obj1LineStartPoint;
                obj1PerpLineAxis = sf::Vector2f(-obj1PerpLineAxis.y, obj1PerpLineAxis.x);
                hasCheckedLineCollsion = false;

                //checking obj1 first point
                float projOnEntryAxis = DotProduct(obj1LineStartPoint - obj2LineStartPoint, obj2PerpLineAxis) / axesDotProduct;
                if(projOnEntryAxis < 0)
                {
                    if(hasCheckedLineCollsion == false)
                    {
                        //checking for space in first axis
                        obj1MaxProj = obj1MinProj = DotProduct(obj1LineStartPoint, obj1PerpLineAxis);
                        tempValue = DotProduct(obj1LineEndPoint, obj1PerpLineAxis);
                        if(tempValue < obj1MinProj)
                            obj1MinProj = tempValue;
                        else obj1MaxProj = tempValue;

                        obj2MaxProj = obj2MinProj = DotProduct(obj2LineStartPoint, obj1PerpLineAxis);
                        tempValue = DotProduct(obj2LineEndPoint, obj1PerpLineAxis);
                        if(tempValue < obj2MinProj)
                            obj2MinProj = tempValue;
                        else obj2MaxProj = tempValue;
                        if(obj1MaxProj < obj2MinProj || obj2MaxProj < obj1MinProj)
                        {
                            if(displacement > 0)
                            {
                                tempValue = obj1MaxProj - obj2MinProj;
                                if(tempValue > obj2MaxProj - obj1MinProj)
                                    tempValue = obj2MaxProj - obj1MinProj;
                                tempValue /= DotProduct(obj1PerpLineAxis, obj1EntryDir);
                                tempValue *= tempValue < 0 ? -1 : 1;

                                if(tempValue > displacement)
                                    continue;
                            }
                            else continue;
                        }

                        //checking for space in second axis
                        obj1MaxProj = obj1MinProj = DotProduct(obj1LineStartPoint, obj2PerpLineAxis);
                        tempValue = DotProduct(obj1LineEndPoint, obj2PerpLineAxis);
                        if(tempValue < obj1MinProj)
                            obj1MinProj = tempValue;
                        else obj1MaxProj = tempValue;

                        obj2MaxProj = obj2MinProj = DotProduct(obj2LineStartPoint, obj2PerpLineAxis);
                        tempValue = DotProduct(obj2LineEndPoint, obj2PerpLineAxis);
                        if(tempValue < obj2MinProj)
                            obj2MinProj = tempValue;
                        else obj2MaxProj = tempValue;
                        if(obj1MaxProj < obj2MinProj || obj2MaxProj < obj1MinProj)
                        {
                            if(displacement > 0)
                            {
                                tempValue = obj1MaxProj - obj2MinProj;
                                if(tempValue > obj2MaxProj - obj1MinProj)
                                    tempValue = obj2MaxProj - obj1MinProj;
                                tempValue /= DotProduct(obj2PerpLineAxis, obj1EntryDir);
                                tempValue *= tempValue < 0 ? -1 : 1;

                                if(tempValue > displacement)
                                    continue;
                            }
                            else continue;
                        }

                        hasCheckedLineCollsion = true;
                    }

                    if(hasCheckedLineCollsion)
                        if(maxPushMagn < 0 || projOnEntryAxis < maxPushMagn)
                            maxPushMagn = projOnEntryAxis;
                }

                //checking obj1 first point
                projOnEntryAxis = DotProduct(obj1LineEndPoint - obj2LineStartPoint, obj2PerpLineAxis) / axesDotProduct;
                if(projOnEntryAxis < 0)
                {
                    if(hasCheckedLineCollsion == false)
                    {
                        //checking for space in first axis
                        obj1MaxProj = obj1MinProj = DotProduct(obj1LineStartPoint, obj1PerpLineAxis);
                        tempValue = DotProduct(obj1LineEndPoint, obj1PerpLineAxis);
                        if(tempValue < obj1MinProj)
                            obj1MinProj = tempValue;
                        else obj1MaxProj = tempValue;

                        obj2MaxProj = obj2MinProj = DotProduct(obj2LineStartPoint, obj1PerpLineAxis);
                        tempValue = DotProduct(obj2LineEndPoint, obj1PerpLineAxis);
                        if(tempValue < obj2MinProj)
                            obj2MinProj = tempValue;
                        else obj2MaxProj = tempValue;
                        if(obj1MaxProj < obj2MinProj || obj2MaxProj < obj1MinProj)
                        {
                            if(displacement > 0)
                            {
                                tempValue = obj1MaxProj - obj2MinProj;
                                if(tempValue > obj2MaxProj - obj1MinProj)
                                    tempValue = obj2MaxProj - obj1MinProj;
                                tempValue /= DotProduct(obj1PerpLineAxis, obj1EntryDir);
                                tempValue *= tempValue < 0 ? -1 : 1;

                                if(tempValue > displacement)
                                    continue;
                            }
                            else continue;
                        }

                        //checking for space in second axis
                        obj1MaxProj = obj1MinProj = DotProduct(obj1LineStartPoint, obj2PerpLineAxis);
                        tempValue = DotProduct(obj1LineEndPoint, obj2PerpLineAxis);
                        if(tempValue < obj1MinProj)
                            obj1MinProj = tempValue;
                        else obj1MaxProj = tempValue;

                        obj2MaxProj = obj2MinProj = DotProduct(obj2LineStartPoint, obj2PerpLineAxis);
                        tempValue = DotProduct(obj2LineEndPoint, obj2PerpLineAxis);
                        if(tempValue < obj2MinProj)
                            obj2MinProj = tempValue;
                        else obj2MaxProj = tempValue;
                        if(obj1MaxProj < obj2MinProj || obj2MaxProj < obj1MinProj)
                        {
                            if(displacement > 0)
                            {
                                tempValue = obj1MaxProj - obj2MinProj;
                                if(tempValue > obj2MaxProj - obj1MinProj)
                                    tempValue = obj2MaxProj - obj1MinProj;
                                tempValue /= DotProduct(obj2PerpLineAxis, obj1EntryDir);
                                tempValue *= tempValue < 0 ? -1 : 1;

                                if(tempValue > displacement)
                                    continue;
                            }
                            else continue;
                        }

                        hasCheckedLineCollsion = true;
                    }

                    if(hasCheckedLineCollsion)
                        if(maxPushMagn < 0 || projOnEntryAxis < maxPushMagn)
                            maxPushMagn = projOnEntryAxis;
                }
            }
        }

        pushVector =
    }
    return false;
}*/

/*bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const std::vector<sf::Vector2f>& obj2DotsPos, sf::Vector2f& pushVector)
{
    collisionAxis.clear();
    if(GetAxis(obj1DotsPos, obj2DotsPos) == false)
        return false;
    if(GetAxis(obj2DotsPos, obj1DotsPos) == false)
        return false;

    sf::Vector2f minPushVector(0.f, 0.f);
    float minDotP = 0.f;

    for(int a = 0; a < collisionAxis.size(); a++)
    {
        float obj1MaxProj = 0.f;
        float obj1MinProj = 0.f;
        float obj2MaxProj = 0.f;
        float obj2MinProj = 0.f;

        obj1MaxProj = DotProduct(obj1DotsPos[1], collisionAxis[a]);
        obj1MinProj = DotProduct(obj1DotsPos[1], collisionAxis[a]);

        for(int b = 2; b < obj1DotsPos.size(); b++)
        {
            float proj = DotProduct(obj1DotsPos[b], collisionAxis[a]);
            if(proj < obj1MinProj)
                obj1MinProj = proj;

            if(proj > obj1MaxProj)
                obj1MaxProj = proj;
        }

        obj2MaxProj = DotProduct(obj2DotsPos[1], collisionAxis[a]);
        obj2MinProj = DotProduct(obj2DotsPos[1], collisionAxis[a]);

        for(int b = 2; b < obj2DotsPos.size(); b++)
        {
            float proj = DotProduct(obj2DotsPos[b], collisionAxis[a]);
            if(proj < obj2MinProj)
                obj2MinProj = proj;

            if(proj > obj2MaxProj)
                obj2MaxProj = proj;
        }

        float distance1 = obj1MaxProj - obj2MinProj;
        float distance2 = obj2MaxProj - obj1MinProj;
        float depth = 0.f;

        if(distance1 < distance2)
            depth = distance1;
        else
            depth = distance2;

        float axisLenghtSquare = DotProduct(collisionAxis[a], collisionAxis[a]);

        sf::Vector2f curPushVec = (collisionAxis[a] * depth / axisLenghtSquare);
        float dotP = DotProduct(curPushVec, curPushVec);

        if(a == 0)
        {
            minPushVector = curPushVec;
            minDotP = dotP;
        }
        else if(dotP < minDotP)
        {
            minPushVector = curPushVec;
            minDotP = dotP;
        }
    }

    pushVector = minPushVector;

    sf::Vector2f distance = obj1DotsPos[0] - obj2DotsPos[0];
    if(DotProduct(distance, pushVector) < 0.f)
        pushVector = -pushVector;

    return true;
}*/

/*bool IsColliding(const std::vector<sf::Vector2f>& obj1DotsPos, const std::vector<sf::Vector2f>& obj2DotsPos, const sf::Vector2f& obj1EntryDir, sf::Vector2f& pushVector)
{
    if(obj1DotsPos.size() >= 3 && obj2DotsPos.size() >= 3)
    {
        float obj1MaxProj = 0.f;
        float obj1MinProj = 0.f;
        float obj1EntryAxisMagn = 0.f;

        sf::Vector2f entryAxis = obj1EntryDir != sf::Vector2f(0, 0) ? obj1EntryDir / (float)sqrt(obj1EntryDir.x * obj1EntryDir.x + obj1EntryDir.y * obj1EntryDir.y) : sf::Vector2f(0, 0);
        sf::Vector2f inversEntryAxis = entryAxis * -1.f;
        sf::Vector2f perpEntryAxis(-entryAxis.y, entryAxis.x);

        {
            float obj1EntryAxisMaxProj = 0.f;
            float obj1EntryAxisMinProj = 0.f;

            obj1EntryAxisMinProj = obj1EntryAxisMaxProj = DotProduct(obj1DotsPos[1], entryAxis);
            for(int a = 2; a < obj1DotsPos.size(); a++)
            {
                float proj = DotProduct(obj1DotsPos[a], entryAxis);
                if(proj < obj1EntryAxisMinProj)
                    obj1EntryAxisMinProj = proj;
                else if(proj > obj1EntryAxisMaxProj)
                    obj1EntryAxisMaxProj = proj;
            }
            obj1EntryAxisMagn = obj1EntryAxisMaxProj - obj1EntryAxisMinProj;
        }

        obj1MinProj = obj1MaxProj = DotProduct(obj1DotsPos[1], perpEntryAxis);
        for(int a = 2; a < obj1DotsPos.size(); a++)
        {
            float proj = DotProduct(obj1DotsPos[a], perpEntryAxis);
            if(proj < obj1MinProj)
                obj1MinProj = proj;
            else if(proj > obj1MaxProj)
                obj1MaxProj = proj;
        }

        sf::Vector2f lineStartPos;
        sf::Vector2f lineEndPos;
        float maxEntryMagn = 0.f;
        float lineMaxProj = 0.f;
        float lineMinProj = 0.f;

        for(int a = 1; a < (obj2DotsPos.size() == 3 ? a == 1 : obj2DotsPos.size()); a++)
        {
            lineStartPos = obj2DotsPos[a];
            if(a == obj2DotsPos.size() - 1)
                lineEndPos = obj2DotsPos[1];
            else lineEndPos = obj2DotsPos[a + 1];

            lineMaxProj = lineMinProj = DotProduct(lineStartPos, perpEntryAxis);
            {
                float proj = DotProduct(lineEndPos, perpEntryAxis);
                if(proj < lineMinProj)
                    lineMinProj = proj;
                else if(proj > lineMaxProj)
                    lineMaxProj = proj;
            }

            if((obj1MaxProj >= lineMinProj && obj1MinProj <= lineMaxProj) || (lineMaxProj >= obj1MinProj && lineMinProj <= obj1MaxProj))//add equals sign to make touching count as colliding
            {
                sf::Vector2f lineAxis = lineEndPos - lineStartPos;
                lineAxis = lineAxis != sf::Vector2f(0, 0) ? lineAxis / (float)sqrt(lineAxis.x * lineAxis.x + lineAxis.y * lineAxis.y) : sf::Vector2f(0, 0);
                sf::Vector2f perpLineAxis(-lineAxis.y, lineAxis.x);

                for(int b = 1; b < obj1DotsPos.size(); b++)
                {
                    float pointProj = DotProduct(obj1DotsPos[b], perpEntryAxis);
                    float perpLineAxisProj = DotProduct(obj1DotsPos[b] - lineStartPos, perpLineAxis);

                    if(pointProj > lineMinProj && pointProj < lineMaxProj && DotProduct(perpLineAxisProj * perpLineAxis, entryAxis) > 0)
                    {
                        float absPerpLineAxisProj = (perpLineAxisProj < 0 ? -perpLineAxisProj : perpLineAxisProj);
                        float absPerpLineAxisEntryProj = DotProduct(obj1EntryAxisMagn * entryAxis, perpLineAxis);
                        if(absPerpLineAxisEntryProj < 0)
                            absPerpLineAxisEntryProj = -absPerpLineAxisEntryProj;

                        float curEntryMagn = 0.f;
                        if(absPerpLineAxisEntryProj != 0)
                            curEntryMagn = absPerpLineAxisProj / absPerpLineAxisEntryProj * obj1EntryAxisMagn;

                        if(curEntryMagn > maxEntryMagn)
                            maxEntryMagn = curEntryMagn;
                    }
                }
            }
        }

        for(int a = 1; a < (obj1DotsPos.size() == 3 ? a == 1 : obj1DotsPos.size()); a++)
        {
            lineStartPos = obj1DotsPos[a];
            if(a == obj1DotsPos.size() - 1)
                lineEndPos = obj1DotsPos[1];
            else lineEndPos = obj1DotsPos[a + 1];

            lineMaxProj = lineMinProj = DotProduct(lineStartPos, perpEntryAxis);
            {
                float proj = DotProduct(lineEndPos, perpEntryAxis);
                if(proj < lineMinProj)
                    lineMinProj = proj;
                else if(proj > lineMaxProj)
                    lineMaxProj = proj;
            }

            if((obj1MaxProj >= lineMinProj && obj1MinProj <= lineMaxProj) || (lineMaxProj >= obj1MinProj && lineMinProj <= obj1MaxProj))//add equals sign to make touching count as colliding
            {
                sf::Vector2f lineAxis = lineEndPos - lineStartPos;
                lineAxis = lineAxis != sf::Vector2f(0, 0) ? lineAxis / (float)sqrt(lineAxis.x * lineAxis.x + lineAxis.y * lineAxis.y) : sf::Vector2f(0, 0);
                sf::Vector2f perpLineAxis(-lineAxis.y, lineAxis.x);

                for(int b = 1; b < obj2DotsPos.size(); b++)
                {
                    float pointProj = DotProduct(obj2DotsPos[b], perpEntryAxis);
                    float perpLineAxisProj = DotProduct(obj2DotsPos[b] - lineStartPos, perpLineAxis);

                    if(pointProj > lineMinProj && pointProj < lineMaxProj && DotProduct(perpLineAxisProj * perpLineAxis, entryAxis) < 0)
                    {
                        float absPerpLineAxisProj = (perpLineAxisProj < 0 ? -perpLineAxisProj : perpLineAxisProj);
                        float absPerpLineAxisEntryProj = DotProduct(obj1EntryAxisMagn * entryAxis, perpLineAxis);
                        if(absPerpLineAxisEntryProj < 0)
                            absPerpLineAxisEntryProj = -absPerpLineAxisEntryProj;

                        float curEntryMagn = 0.f;
                        if(absPerpLineAxisEntryProj != 0)
                            curEntryMagn = absPerpLineAxisProj / absPerpLineAxisEntryProj * obj1EntryAxisMagn;

                        if(curEntryMagn > maxEntryMagn)
                            maxEntryMagn = curEntryMagn;
                    }
                }
            }
        }

        if(maxEntryMagn > 0)
        {
            pushVector = inversEntryAxis * maxEntryMagn;
            return true;
        }
    }
    return false;
}*/

#endif // COLLISION_H_INCLUDED
