#ifndef MULTIDRAWER_H_INCLUDED
#define MULTIDRAWER_H_INCLUDED

#include "SFML/Graphics.hpp"
#include <vector>

class MultiDrawer
{
private:
    sf::Texture* texture;
    sf::VertexArray vertexArray;

public:
    MultiDrawer() :
        texture(nullptr)
    {
        vertexArray.setPrimitiveType(sf::Quads);
    }

    MultiDrawer(sf::Texture* texture_) :
        texture(texture_)
    {
        vertexArray.setPrimitiveType(sf::Quads);
    }

    void SetTexture(sf::Texture* texture_)
    {
        texture = texture_;
    }

    sf::Texture* GetTexture()
    {
        return texture;
    }

    void Draw(sf::RenderTarget& target)
    {
        if(texture == nullptr)
            target.draw(vertexArray);
        else target.draw(vertexArray, texture);

        vertexArray.clear();
    }

    void AddVertex(sf::Vertex vertex_[], int n = 4);
    void AddVertex(const std::vector<sf::Vertex>& vertices_);
};

#endif // MULTIDRAWER_H_INCLUDED
