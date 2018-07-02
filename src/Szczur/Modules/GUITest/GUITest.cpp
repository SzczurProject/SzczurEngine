#include "GUITest.hpp"

#include "StressTester.hpp"

#include "Szczur/Modules/GUI/Widget.hpp"
#include "Szczur/Modules/GUI/ImageWidget.hpp"
#include "Szczur/Modules/GUI/ScrollAreaWidget.hpp"
#include "Szczur/Modules/GUI/WindowWidget.hpp"
#include "Szczur/Modules/GUI/ListWidget.hpp"
#include "Szczur/Modules/GUI/TextWidget.hpp"

#include "Szczur/Modules/GUI/Animation/Anim.hpp"

namespace rat
{
    GUITest::GUITest()
    {
        auto& mainWindow = getModule<Window>();
        auto& window = mainWindow.getWindow();

        mainWindow.pushGLStates();
        _canvas.create(window.getSize().x, window.getSize().y);
        mainWindow.popGLStates();

        init();
    }
    void GUITest::init()
    {
        auto& gui = getModule<GUI>();


        gui.addAsset<sf::Texture>("Assets/Test/NinePatchTest.png");
        gui.addAsset<sf::Texture>("Assets/Test/Scroller.png");
        gui.addAsset<sf::Texture>("Assets/Test/ScrollerBar.png");
        gui.addAsset<sf::Texture>("Assets/Test/ScrollerBound.png");
        gui.addAsset<sf::Texture>("Assets/Test/NinePatchTest.png");

        gui.addAsset<sf::Texture>("Assets/GUITest/Blue.png");
        gui.addAsset<sf::Texture>("Assets/GUITest/Red.png");
        gui.addAsset<sf::Font>("Assets/GUITest/lumos.ttf");
        gui.addTexture("Assets/GUITest/Cubic.png");

        _widget = gui.addInterface();

        _widget->setSizingWidthToHeightProportion(1.f);
        //_widget->setWidthToHeightProp(16.f/9.f);

        auto* scroll = new ScrollAreaWidget;
        _widget->add(scroll);

        scroll->setScrollerTexture(gui.getAsset<sf::Texture>("Assets/Test/Scroller.png"));
        scroll->setPathTexture(gui.getTexture("Assets/Test/ScrollerBar.png"));
        scroll->setBoundsTexture(gui.getTexture("Assets/Test/ScrollerBound.png"));

        scroll->setPropSize(0.5f, 0.5f);
        scroll->setPropPosition(1.0f, 1.0f);

        //auto gui::AnimData{4.f, gui::Easing::EaseInQuad, [](){}};

        // scroll->setPropPosition({0.5f, 0.f}, {3.f, gui::Easing::EaseInBack, [scroll](){
        //     scroll->setPropPosition({0.5f, 0.5f}, {3.f, gui::Easing::EaseInOutBack, [scroll](){
        //         scroll->setPropPosition({0.f, 0.5f}, {3.f, gui::Easing::EaseOutBack, [scroll](){
        //             scroll->setPropPosition({0.f, 0.f}, {3.f, gui::Easing::EaseInBack, [scroll](){
        //                     scroll->setPropPosition({0.5f, 0.f}, {3.f, gui::Easing::EaseInBack, [scroll](){
            
        //                 }});
        //             }});
        //         }});
        //     }});
        // }});

        
        image = new ImageWidget;
        image->setTexture(gui.getAsset<sf::Texture>("Assets/GUITest/Cubic.png"));
        image->setPropSize(0.3f, 0.3f);
        image->setFullyTexSizing();
        image->setStaticTexPositing();
        image->setPropPosition(0.5f, 0.5f);
        _widget->add(image);
        
        list = new ListWidget;
        list->makeReversed();
        //list->setBetweenPadding(20.f);
        scroll->add(list);
        //list->setPropPosition(0.f, 1.f);
        list->setPropSize(0.3f, 1.f);
        list->setAutoBetweenPadding();

        

        float size = 0.1f;
        for(int i = 0; i < 4; i++)
        {
            auto* w = new ImageWidget;
            list->add(w);
            w->setPropSize(size, size);
            size += 0.05f;
            w->setTexture(gui.getAsset<sf::Texture>("Assets/GUITest/Blue.png"));
            
            w->setCallback(Widget::CallbackType::onHoverIn, [w](auto){
                w->setColorInTime({0, 0, 0}, {1.f, gui::Easing::EaseInCubic});
            });
            w->setCallback(Widget::CallbackType::onHoverOut, [w](auto){
                w->setColorInTime({255, 255, 255}, {1.f, gui::Easing::EaseInCubic});
            });
        }

        fps = new TextWidget;
        _widget->add(fps);
        fps->setFont(gui.getAsset<sf::Font>("Assets/GUITest/lumos.ttf"));
        fps->setCharacterSize(40u);
        fps->setColor({255, 255, 255});
        fps->setString("ABCDEFG");
        image->setCallback(Widget::CallbackType::onRelease, [this](auto){
            fps->setStringInTime("Ten tekst pojawi sie po anmacji wiec no", 4.f);
        });
    }
    
    
    void GUITest::update(float deltaTime)
    {
        const auto& window = getModule<Window>().getWindow();

        [[maybe_unused]] auto mousePos = sf::Mouse::getPosition(window);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        {
            _scale-= deltaTime * 0.4f;
            if(_scale < 0.2f) _scale = 0.2f;
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        {
            _scale+= deltaTime * 0.4f;
            //if(_scale > 1.f) _scale = 1.f;
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            _prop+= deltaTime * 0.4f;
            if(_prop > 1.f) _prop = 1.f;            
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            _prop-= deltaTime * 0.4f;
            if(_prop < 0.f) _prop = 0.f;
        }
        /*
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            _shift+= deltaTime * 0.4f;
            if(_shift > 1.f) _shift = 1.f;            
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            _shift-= deltaTime * 0.4f;
            if(_shift < 0.f) _shift = 0.f;
        }
        */
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            _size.x -= deltaTime * 150.f;
            if(_size.x < 0.f) _size.x = 0.f;
            //if(_shift > 1.f) _shift = 1.f;            
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            _size.x += deltaTime * 150.f;
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            _size.y += deltaTime * 150.f;
            //if(_shift > 1.f) _shift = 1.f;            
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            _size.y -= deltaTime * 150.f;
            if(_size.y < 0.f) _size.y = 0.f;
        }

        //fps->setString(std::to_string(int(1.f / deltaTime)) + " fps");

        image->setPropTextureRect({{_prop * 0.5f, _prop * 0.5f}, {1.f - _prop, 1.f - _prop}});
    }
    void GUITest::render()
    {
        auto& mainWindow = getModule<Window>();

        mainWindow.pushGLStates();

        _canvas.clear(sf::Color::Transparent);
        _canvas.display();

        mainWindow.getWindow().draw(sf::Sprite(_canvas.getTexture()));

        mainWindow.popGLStates();
    }
}