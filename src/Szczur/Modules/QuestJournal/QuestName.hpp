#pragma once
#include <Szczur/Modules/GUI/TextWidget.hpp>
#include <Szczur/Modules/GUI/GUI.hpp>

#include "Quest.hpp"

namespace rat
{

class TextWidget;

namespace journal
{
    class QuestName
    {
        public:
            QuestName(sf::Font *font,Widget *interface);
            ~QuestName();

            void clear();

            void setQuest(std::shared_ptr<Quest> quest);
        private:
            sf::Font* _font;
            std::shared_ptr<Quest> _quest;
            std::string _questName;
            TextWidget * _widget;
            
    };
}
}