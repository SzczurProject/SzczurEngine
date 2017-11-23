#include "Widget.hpp"

#include <iostream>

namespace rat {
    Widget::Widget() :
    _parent(nullptr),
    _isHovered(false),
    _isPressed(false),
    _aboutToRecalculate(false),
    _size(0u,0u) {
        ;
    }

    void Widget::setParent(Widget* parent) {
        _parent = parent;
    }

    Widget* Widget::setCallback(CallbackType key, Function_t value) {
        _callback.insert_or_assign(key, value);

        return this;
    }

    void Widget::callback(CallbackType type) {
        if(auto it = _callback.find(type); it != _callback.end())
            std::invoke(std::get<1>(*it), this);
    }

    Widget* Widget::add(Widget* object) {
        if(object) {
            _children.push_back(object);
            object->setParent(this);
            object->calculateSize();
            //calculateSize();
        }
        return object;
    }

    sf::Vector2u Widget::getSize() const {
        return _size;
    }

    void Widget::input(const sf::Event& event) {
        _input(event);

        switch(event.type) {
            case sf::Event::MouseMoved: {
                auto thisSize = getSize();
                if(
                    event.mouseMove.x >= 0 &&
                    event.mouseMove.x <= thisSize.x &&
                    event.mouseMove.y >= 0 &&
                    event.mouseMove.y <= thisSize.y
                ) {
                    if(!_isHovered) {
                        callback(CallbackType::onHoverIn);
                        _isHovered = true;
                    }
                }
                else {
                    if(_isHovered) {
                        callback(CallbackType::onHoverOut);
                        _isHovered = false;
                    }
                }
                break;
            }

            case sf::Event::MouseButtonPressed: {
                if(_isHovered) {
                    callback(CallbackType::onPress);
                    _isPressed = true;
                }
                break;
            }

            case sf::Event::MouseButtonReleased: {
                if(_isPressed) {
                    _isPressed = false;
                    if(_isHovered)
                        callback(CallbackType::onRelease);         
                }
                break;
            }

            default: break;
        }
        for(Widget* it : _children) {
            if(event.type == sf::Event::MouseMoved) {
                auto itPosition = it->getPosition();
                sf::Event tempEvent(event);
                tempEvent.mouseMove.x -= itPosition.x;
                tempEvent.mouseMove.y -= itPosition.y;
                it->input(tempEvent);
            }
            else
                it->input(event);
        }
    }

    void Widget::update(float deltaTime) {
        _update(deltaTime);

        if(_isHovered) 
            callback(CallbackType::onHover);

        if(_isPressed)
            callback(CallbackType::onHold);

        for(Widget* it : _children)
            it->update(deltaTime);
        
        if(_aboutToRecalculate && _parent) {
            calculateSize();
        }
        
    }

    void Widget::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();

        for(Widget* it : _children)
            target.draw(*it, states);
        
        _draw(target, states);
    }

    void Widget::calculateSize() {
        _aboutToRecalculate = false;
        _size = {0u,0u};
        for(Widget* it : _children) {
            auto itSize = it->getSize();
            auto itPosition = static_cast<sf::Vector2i>(it->getPosition());
            if(itPosition.x + itSize.x > _size.x)
                _size.x = itPosition.x + itSize.x;
            if(itPosition.y + itSize.y > _size.y)
                _size.y = itPosition.y + itSize.y;
        }
        auto ownSize = _getSize();
        if(ownSize.x > _size.x)
            _size.x = ownSize.x;
        if(ownSize.y > _size.y)
            _size.y = ownSize.y;

        if(_parent != nullptr)
            _parent->calculateSize();
    }

    sf::Vector2u Widget::_getSize() const {
        return {0u, 0u};
    }

    void Widget::move(const sf::Vector2f& offset) {
        sf::Transformable::move(offset);
        _aboutToRecalculate = true;
    }
    void Widget::move(float offsetX, float offsetY) {
        sf::Transformable::move(offsetX, offsetY);
        _aboutToRecalculate = true;
    }
    void Widget::setPosition(const sf::Vector2f& offset) {
        sf::Transformable::setPosition(offset);
        _aboutToRecalculate = true;
    }
    void Widget::setPosition(float x, float y) {
        sf::Transformable::setPosition(x, y);
        _aboutToRecalculate = true;
    }
}