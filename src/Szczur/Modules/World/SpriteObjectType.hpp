#pragma once

/** @file SpriteObjectType.hpp
 ** @description Header file with SpriteObjectType class as shared game sprite object type information.
 ** @author Patryk (PsychoX) Ludwikowski <psychoxivi+basementstudios@gmail.com>
 **/

#include <array>
#include <vector>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include "Szczur/Modules/World/World.hpp"
#include "Szczur/Modules/World/Object.hpp"
#include "Szczur/Modules/World/ObjectType.hpp"

namespace rat
{

class SpriteObjectType : public ObjectType
{
	/* Types */
protected:
	struct StateDetails
	{
		World::Vector_t 			origin		{0, 0};
		std::array<sf::Vertex, 4>	vertices;
	};

	
	
	/* Constants */
protected:
	constexpr static auto	assetsPath		= "Assets/Objects/";
	constexpr static auto 	textureFileName	= "texture.png";
	constexpr static auto	configFileName	= "object.json";
	

	
	/* Fields */
protected:
	sf::Texture 				texture;
	std::vector<StateDetails> 	statesDetails;



	/* Properties */
public:
	/** @property Texture
	 ** @description Texture used to draw the object.
	 ** @access const get
	 **/
	const sf::Texture& getTexture() const;

	/** @property Origin
	 ** @description Origin on texture for the object center.
	 ** @access const get
	 ** @argument stateID - since returns for specific state
	 **/
	const World::Vector_t getOrigin(ObjectType::StateID_t stateID) const;

	/** @property Vertices
	 ** @description Vertices used to render the texture as the object.
	 ** @access const get
	 ** @argument stateID - since returns for specific state
	 **/
	const std::array<sf::Vertex, 4> getVertices(ObjectType::StateID_t stateID) const;

	

	/* Operators */
public:
	/** @constructor
	 ** @description Loads sprite object type.
	 ** @argument name - name of the sprite object type to load.
	 **/
	SpriteObjectType(const std::string& name);



	/* Functions */
protected:
	std::array<sf::Vertex, 4> calculateVertices(const sf::IntRect& rect);
};

}