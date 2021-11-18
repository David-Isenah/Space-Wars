#include "ship.h"
#include "iostream"

std::vector<Ship::UnitGroup*> unitGroups;

void Ship::UnitGroup::AddMember(Ship* unit)
{
    if(members.size() < Ship::MAX_GROUP_MEMBERS)
        members.push_back(unit);
}

void Ship::UnitGroup::LeaveGroup(Ship* unit)
{
    if(leader == unit)
        leader = nullptr;
    for(int a = 0; a < members.size(); a++)
        if(members[a] == unit)
        {
            members.erase(members.begin() + a);
            a--;
        }
}

Ship::UnitGroup* Ship::UnitGroup::CreatUnitGroups(/*Ship* leader_*/)
{
    UnitGroup* group = new UnitGroup;
    //group->leader = leader_;
    unitGroups.push_back(group);
    return group;
}

void Ship::UnitGroup::DeleteGroup(UnitGroup* group_)
{
    if(group_->leader != nullptr)
        group_->leader->LeaveGroup();
    std::vector<Ship*> tempMembers = group_->members;
    for(auto& unit : tempMembers)
        unit->LeaveGroup();

    for(int a = 0; a < unitGroups.size(); a++)
        if(unitGroups[a] == group_)
        {
            unitGroups.erase(unitGroups.begin() + a);
            break;
        }
    delete group_;
}

void Ship::UnitGroup::RefreshUnitGroups()
{
    //Checking units to see if they are still in the group
    for(UnitGroup*& group : unitGroups)
    {
        if(group->leader != nullptr)
        {
            if(group->leader->GetGroup() != group)
                group->leader = nullptr;
            else if(group->leader->GetAlive() == false)
                group->leader->LeaveGroup();
        }
        for(int a = 0; a < group->members.size(); a++)
        {
            if(group->members[a]->GetGroup() != group)
            {
                group->members.erase(group->members.begin() + a);
                a--;
            }
            else if(group->members[a]->GetAlive() == false)
            {
                Ship* tempShip = group->members[a];
                group->members.erase(group->members.begin() + a);
                a--;

                tempShip->LeaveGroup();
            }
        }
    }

    //Choosing new leader
    for(int a = 0; a < unitGroups.size(); a++)
    {
        if((unitGroups[a]->leader != nullptr ? unitGroups[a]->leader->GetAlive() == false : true) && unitGroups[a]->members.size() > 0)
        {
            int highestHierarchy = -1;
            int highestHealth = -1;
            Ship* newLeader = nullptr;
            int indexToErase = -1;

            for(int b = 0; b < unitGroups[a]->members.size(); b++)
            {
                Ship* member = unitGroups[a]->members[b];
                if(highestHierarchy < 0 || member->GetHierarchy() > highestHierarchy)
                {
                    highestHierarchy = member->GetHierarchy();
                    highestHealth = member->GetHealth();
                    newLeader = member;
                    indexToErase = b;
                }
                else if(member->GetHierarchy() == highestHierarchy && member->GetHealth() > highestHealth)
                {
                    highestHealth = member->GetHealth();
                    newLeader = member;
                    indexToErase = b;
                }
            }

            if(unitGroups[a]->leader != nullptr)
                unitGroups[a]->leader->LeaveGroup();
            unitGroups[a]->leader = newLeader;
            if(indexToErase >= 0)
                unitGroups[a]->members.erase(unitGroups[a]->members.begin() + indexToErase);
        }

        //Checking group if to delete it
        if(((unitGroups[a]->leader != nullptr ? unitGroups[a]->leader->GetAlive() == false : true) && unitGroups[a]->members.size() == 0) ||
           (unitGroups[a]->leader != nullptr && unitGroups[a]->leader->GetAlive() && unitGroups[a]->members.size() == 0 && unitGroups[a]->leader->GetHierarchy() <= 1))
        {
            if(unitGroups[a]->leader != nullptr)
                unitGroups[a]->leader->LeaveGroup();
            std::vector<Ship*> tempMembers = unitGroups[a]->members;
            for(auto& unit : tempMembers)
                unit->LeaveGroup();

            delete unitGroups[a];
            unitGroups.erase(unitGroups.begin() + a);
            a--;
        }
    }

    /*for(int a = 0; a < unitGroups.size(); a++)
    {
        std::cout << "UG " << a << " - " << unitGroups[a]->members.size() << (unitGroups[a]->leader == nullptr ? " No Leader\n\n" : "Has Leader\n\n");
    }*/
}
