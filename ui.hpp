#pragma once
#include <SFML/Graphics.hpp>
#include <string>

struct Button {
    sf::RectangleShape shape;
    sf::Text text;

    bool isHovered(sf::Vector2f mouse) const {
        return shape.getGlobalBounds().contains(mouse);
    }
};

Button createButton(const sf::Font& font,
                    const std::string& label,
                    sf::Vector2f pos,
                    sf::Vector2f size = {220.f, 40.f});
