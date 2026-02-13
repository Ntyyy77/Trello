#include "menu.hpp"

void DropdownMenu::open(const sf::Font& font, sf::Vector2f pos) {
    visible = true;
    position = pos;
    options.clear();

    options.push_back(createButton(font, "Rename", pos, {120.f, 30.f}));
    options.push_back(createButton(font, "Delete", {pos.x, pos.y + 35.f}, {120.f, 30.f}));
    options.push_back(createButton(font, "Add Description", {pos.x, pos.y + 70.f}, {120.f, 30.f}));
}

void DropdownMenu::draw(sf::RenderWindow& window) {
    if (!visible) return;
    for (auto& opt : options) {
        window.draw(opt.shape);
        window.draw(opt.text);
    }
}

int DropdownMenu::handleClick(sf::Vector2f mouse) {
    if (!visible) return -1;

    for (size_t i = 0; i < options.size(); ++i) {
        if (options[i].isHovered(mouse)) {
            visible = false;
            return static_cast<int>(i); // 0 = Rename, 1 = Delete, 2 = Add description
        }
    }
    return -1;
}
