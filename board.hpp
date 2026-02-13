#pragma once
#include <string>
#include <vector>

struct Card {
    int id;
    std::string title;
    std::string description;

    Card(int i, const std::string& t) : id(i), title(t) {}
};

struct List {
    int id;
    std::string name;
    std::vector<Card> cards;

    List(int i, const std::string& n) : id(i), name(n) {}
};

struct Board {
    int id;
    std::string name;
    std::vector<List> lists;

    Board(int i, const std::string& n) : id(i), name(n) {}
};
