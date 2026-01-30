#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>
#include <string>
#include "list.hpp"

class Board {
public:
    int id;
    std::string name;
    std::vector<List> lists;

    Board(int id, const std::string& name) : id(id), name(name) {}
};

#endif
