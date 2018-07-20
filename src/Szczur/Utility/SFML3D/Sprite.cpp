#include "Sprite.hpp"

#include <glm/vec2.hpp>

#include "RenderTarget.hpp"
#include "Texture.hpp"

namespace sf3d
{

/* Properties */
void Sprite::setTexture(const Texture& texture)
{
	_texture = &texture;

	const glm::vec2 size = texture.getSize();

	_vertices[0].position = {0.f, 0.f, 0.f};
	_vertices[0].texCoord = {0.f, 0.f};

	_vertices[1].position = {size.x, 0.f, 0.f};
	_vertices[1].texCoord = {1.f, 0.f};

	_vertices[2].position = {size.x, -size.y, 0.f};
	_vertices[2].texCoord = {1.f, 1.f};
	
	_vertices[3].position = {0.f, -size.y, 0.f};
	_vertices[3].texCoord = {0.f, 1.f};
}

void Sprite::setTextureRect(const glm::uvec2& position, const glm::uvec2& size)
{
	const glm::vec2 textureSize = _texture.getSize();
	const float startX = static_cast<float>(position.x) / textureSize.x;
	const float startY = static_cast<float>(position.y) / textureSize.y;
	const float endX = startX + static_cast<float>(size.x) / textureSize.x;
	const float endY = startY + static_cast<float>(size.y) / textureSize.y;

	_vertices[0].position = {0.f, 0.f, 0.f};
	_vertices[0].texCoord = {startX, startY};

	_vertices[1].position = {size.x, 0.f, 0.f};
	_vertices[1].texCoord = {endX, startY};

	_vertices[2].position = {size.x, -size.y, 0.f};
	_vertices[2].texCoord = {endX, endY};
	
	_vertices[3].position = {0.f, -size.y, 0.f};
	_vertices[3].texCoord = {startY, endY};
}



/* Operators */
Sprite::Sprite()
{
	;
}

Sprite::Sprite(const Texture& texture) 
{
	setTexture(texture);
}



/* Methods */
void Sprite::draw(RenderTarget & target, RenderStates states) const
{
	states.transform *= getTransform();
	states.texture = _texture;
	target.draw(_vertices, states);
}

}
