#include "game.h"
#include "public_functions.h"
#include <sstream>

const int NotifMaxDisplay = 8;

struct NotificationInfo
{
    sf::Text text;
    sf::RectangleShape boarder;
    float duration = 0.f;
    bool isActivated = false;
};

std::vector<NotificationInfo> notificationList;

void Game::AddNotification(NotificationStyle style_, std::string text_, sf::Color boarderColor_, sf::Color textColor_, float duration_)
{
    NotificationInfo notif;
    notif.text.setString(text_);
    notif.text.setFillColor(textColor_);
    notif.text.setFont(*GetFont("Default"));
    notif.text.setCharacterSize(23);
    notif.text.setOrigin(notif.text.getGlobalBounds().width / 2, notif.text.getGlobalBounds().height / 2);

    boarderColor_.a = 150;
    notif.boarder.setFillColor(boarderColor_);
    notif.boarder.setSize(sf::Vector2f(145 + notif.text.getGlobalBounds().width, 45));
    notif.boarder.setOrigin(notif.boarder.getGlobalBounds().width / 2, notif.boarder.getGlobalBounds().height / 2);

    notif.duration = duration_;

    if(style_ == InsertNotif)
        notificationList.push_back(notif);
    else if(style_ == SmartInsertNotif)
    {
        bool addIt = true;

        for(int a = 0; a < notificationList.size(); a++)
        {
            if(notificationList[a].boarder.getFillColor() == boarderColor_ &&
               notificationList[a].isActivated == true &&
               notificationList[a].text.getString() == text_)
            {
                notificationList[a].duration = duration_;
                addIt = false;
            }
        }

        if(addIt)
            notificationList.push_back(notif);
    }
    else if(style_ == ReplaceNotif)
    {
        if(notificationList.size() > 0)
            for(int a = 0; a < notificationList.size(); a++)
            {
                if(notificationList[a].boarder.getFillColor() == boarderColor_ &&
                   notificationList[a].isActivated == true &&
                   notificationList[a].text.getString() == text_)
                   notificationList[a].duration = 0;
            }
        notificationList.push_back(notif);
    }
}

void Game::AddNotification(std::string text_, sf::Color boarderColor_, sf::Color textColor_, float duration_)
{
    AddNotification(InsertNotif, text_, boarderColor_, textColor_, duration_);
}

void Game::RemoveNotification(std::string text_)
{
    for(int a = 0; a < notificationList.size(); a++)
        if(notificationList[a].text.getString() == text_)
            notificationList[a].duration = 0;
}

void Game::UpdateNotification(float dt)
{
    for(int a = 0; a < notificationList.size() && a < NotifMaxDisplay; a++)
    {
        if(notificationList[a].isActivated == false)
        {
            notificationList[a].boarder.setPosition(SCREEN_WIDTH + notificationList[a].boarder.getSize().x / 2, SCREEN_HEIGHT - 150 - 55 * a);
            notificationList[a].isActivated = true;
        }

        if(notificationList[a].duration > 0)
        {
            notificationList[a].duration -= dt;
            if(notificationList[a].duration < 0)
               notificationList[a].duration = 0;
        }

        sf::Vector2f pos = notificationList[a].boarder.getPosition();
        sf::Vector2f realPos = sf::Vector2f(SCREEN_WIDTH - notificationList[a].boarder.getSize().x / 2, SCREEN_HEIGHT - 150 - 55 * a);
        if(notificationList[a].duration == 0)
            realPos = sf::Vector2f(SCREEN_WIDTH + notificationList[a].boarder.getSize().x / 2, SCREEN_HEIGHT - 150 - 55 * a);

        TendTowards(pos.x, realPos.x, 0.1, 1, dt);
        TendTowards(pos.y, realPos.y, 0.1, 1, dt);
        notificationList[a].boarder.setPosition(pos);

        if(notificationList[a].duration == 0 && pos == realPos)
        {
            notificationList.erase(notificationList.begin() + a);
            a -= 1;
        }
    }
}

void Game::DrawNotification()
{
    gameInstance->window.setView(gameInstance->window.getDefaultView());
    for(int a = 0; a < notificationList.size() && a < NotifMaxDisplay; a++)
        if(notificationList[a].isActivated)
        {
            notificationList[a].text.setPosition(notificationList[a].boarder.getPosition().x + notificationList[a].boarder.getGlobalBounds().width / 2 - 40 - notificationList[a].text.getGlobalBounds().width / 2,
                                                 notificationList[a].boarder.getPosition().y);

            gameInstance->window.draw(notificationList[a].boarder);
            gameInstance->window.draw(notificationList[a].text);
        }
}
