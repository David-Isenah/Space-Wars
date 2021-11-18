#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <SFML/Graphics.hpp>
#include <map> //header for map

class AssetManager
{
public:
	AssetManager();

    static void LoadTexture(std::string const& id, sf::Texture const& texture);
    //function to load into map

    static void LoadTexture(std::string const& id, std::string const& filename, bool setSmooth = true);
    //function overload to load into map as well

	static sf::Texture& GetTexture(std::string const& id);
	//function to get texture from the map

private:
	std::map <std::string, sf::Texture> m_Textures;
	//map to keep textures

	static AssetManager* sInstance;
};

#endif
