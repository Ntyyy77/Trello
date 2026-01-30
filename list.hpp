#ifndef LIST_HPP
#define LIST_HPP

#include <vector>
#include "card.hpp"

class List {
public:
    int id;
    std::string name;
    std::vector<Card> cards;

    List(int id, const std::string& name) : id(id), name(name) {}
};

#endif
