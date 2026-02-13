#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "ui.hpp"

struct DropdownMenu {
    std::vector<Button> options;
    bool visible = false;
    sf::Vector2f position;

    void open(const sf::Font& font, sf::Vector2f pos);
    void draw(sf::RenderWindow& window);
    int handleClick(sf::Vector2f mouse);
};
