#pragma once

#include "Widget.hpp"

#include <vector>

namespace rat
{
    class ListWidget : public Widget
    {
    public:
        void setBetweenPadding(float padding);
    protected:
        virtual void _addWidget(Widget* widget) override;
        virtual void _calculateSize() override;
        virtual sf::Vector2u _getChildrenSize() override;
        virtual sf::Vector2f _getChildShiftByIndex(size_t index) const override;

        virtual void _drawChildren(sf::RenderTarget& target, sf::RenderStates states) const override;
    private:
        std::vector<float> _shifts;
        bool _areShiftsCurrent{false};
        float _betweenWidgetsPadding{0.f};

        void _updateShifts();
    };
}