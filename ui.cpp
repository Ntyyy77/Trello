#include "ui.hpp"

Button createButton(const sf::Font& font,
                    const std::string& label,
                    sf::Vector2f pos,
                    sf::Vector2f size) {

    Button b;
    b.shape.setSize(size);
    b.shape.setFillColor(sf::Color(70, 70, 70));
    b.shape.setPosition(pos);

    b.text.setFont(font);
    b.text.setString(label);
    b.text.setCharacterSize(16);
    b.text.setFillColor(sf::Color::White);
    b.text.setPosition(pos.x + 8.f, pos.y + 8.f);

    return b;
}
