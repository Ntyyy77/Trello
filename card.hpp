#ifndef CARD_HPP
#define CARD_HPP

#include <string>

class Card {
public:
    int id;
    std::string title;
    std::string description;

    Card(int id, const std::string& title, const std::string& description = "")
        : id(id), title(title), description(description) {}
};

#endif
