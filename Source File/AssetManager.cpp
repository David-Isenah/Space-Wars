#include "AssetManager.h"
#include <assert.h>

AssetManager* AssetManager::sInstance = nullptr;

AssetManager::AssetManager()
{
    assert(sInstance == nullptr);
    sInstance = this;
}

void AssetManager::LoadTexture(std::string const& id, sf::Texture const& texture)
{
    auto& texMap = sInstance->m_Textures;

    if(texMap.find(id) != texMap.end())
        return;
    else
    {
        texMap.insert(std::make_pair(id, texture));
    }
}

void AssetManager::LoadTexture(std::string const& id, std::string const& filename, bool setSmooth)
{
    auto& texMap = sInstance->m_Textures;

    if(texMap.find(id) != texMap.end())
        return;
    else
    {
        auto& texture = texMap[id];
        texture.loadFromFile(filename);
        if(setSmooth)
            texture.setSmooth(true);
    }
}

sf::Texture& AssetManager::GetTexture(std::string const& id)
{
    auto& texMap = sInstance->m_Textures;
    auto pairFound = texMap.find(id);

    if(pairFound != texMap.end())
        return pairFound->second;
    else
    {
        auto& texture = texMap[id];
        texture.loadFromFile(id);
        return texture;
    }
}
