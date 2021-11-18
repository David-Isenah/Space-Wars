#include "multidrawer.h"
#include "entitymanager.h"

void MultiDrawer::AddVertex(sf::Vertex vertex_[], int n)
{
    sf::FloatRect screenRect = EntityManager::GetCameraRect();

    for(int a = 0; a < n; a++)
        if((vertex_[a].position.x >= screenRect.left && vertex_[a].position.x <= screenRect.left + screenRect.width) ||
           (vertex_[a].position.y >= screenRect.top && vertex_[a].position.y <= screenRect.top + screenRect.height))
        {
            for(int b = 0; b < n; b++)
                vertexArray.append(vertex_[b]);
            break;
        }
}

void MultiDrawer::AddVertex(const std::vector<sf::Vertex>& vertices_)
{
    sf::FloatRect screenRect = EntityManager::GetCameraRect();

    for(int a = 0; a < vertices_.size(); a++)
        if((vertices_[a].position.x >= screenRect.left && vertices_[a].position.x <= screenRect.left + screenRect.width) ||
           (vertices_[a].position.y >= screenRect.top && vertices_[a].position.y <= screenRect.top + screenRect.height))
        {
            for(int b = 0; b < vertices_.size(); b++)
                vertexArray.append(vertices_[b]);
            break;
        }
}
