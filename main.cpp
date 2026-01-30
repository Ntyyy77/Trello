#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include "board.hpp"

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
                    sf::Vector2f size = {220.f, 40.f}) {
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

void saveBoards(const std::vector<Board>& boards) {
    for (const auto& b : boards) {
        std::ofstream file("board_" + std::to_string(b.id) + ".cfg");
        file << "BOARD " << b.id << " " << b.name << "\n";
        for (const auto& l : b.lists)
            file << "LIST " << l.id << " " << l.name << "\n";
    }
}

enum class AppState {
    BOARD_SELECTION,
    BOARD_VIEW
};

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 700), "SFML Board UI");
    window.setFramerateLimit(60);

    sf::Font font;
    font.loadFromFile("arial.ttf");

    std::vector<Board> boards;
    int nextBoardId = 1;
    int selectedBoard = -1;

    AppState state = AppState::BOARD_SELECTION;

    bool renaming = false;
    sf::String inputText;

    // -------- Context menus
    bool listMenuOpen = false;
    int listMenuIndex = -1;
    sf::Vector2f listMenuPos;

    bool boardMenuOpen = false;
    int boardMenuIndex = -1;
    sf::Vector2f boardMenuPos;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // -------- TEXT INPUT (rename board)
            if (renaming && event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b' && inputText.getSize() > 0)
                    inputText.erase(inputText.getSize() - 1, 1);
                else if (event.text.unicode == '\r') {
                    boards[selectedBoard].name = inputText;
                    renaming = false;
                    saveBoards(boards);
                }
                else if (event.text.unicode < 128)
                    inputText += static_cast<char>(event.text.unicode);
            }

            // -------- MOUSE CLICK
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {

                sf::Vector2f mouse = window.mapPixelToCoords(
                    sf::Mouse::getPosition(window));

                // Close menus if click elsewhere
                if (listMenuOpen && !sf::FloatRect(listMenuPos, {120.f, 40.f}).contains(mouse)) {
                    listMenuOpen = false;
                }
                if (boardMenuOpen && !sf::FloatRect(boardMenuPos, {140.f, 80.f}).contains(mouse)) {
                    boardMenuOpen = false;
                }

                // -------- BOARD SELECTION
                if (state == AppState::BOARD_SELECTION) {
                    Button addBtn = createButton(font, "+ Ajouter un board", {100.f, 80.f});
                    if (addBtn.isHovered(mouse)) {
                        boards.emplace_back(nextBoardId, "Board " + std::to_string(nextBoardId));
                        nextBoardId++;
                        saveBoards(boards);
                    }

                    float y = 150.f;
                    for (size_t i = 0; i < boards.size(); ++i) {
                        std::string displayName = (renaming && i == static_cast<size_t>(selectedBoard)) ? inputText.toAnsiString() : boards[i].name;

                        Button b = createButton(font, displayName, {100.f, y});
                        Button dots = createButton(font, "...", {340.f, y + 5.f}, {40.f, 30.f});

                        // Clic sur le board → ouvrir le board
                        if (b.isHovered(mouse)) {
                            selectedBoard = i;
                            state = AppState::BOARD_VIEW;
                        }

                        // Clic sur "..." → ouvrir menu contextuel
                        if (dots.isHovered(mouse)) {
                            boardMenuOpen = true;
                            boardMenuIndex = i;
                            boardMenuPos = {340.f, y + 35.f};
                        }

                        y += 60.f;
                    }
                }

                // -------- BOARD VIEW
                else if (state == AppState::BOARD_VIEW) {
                    Board& board = boards[selectedBoard];

                    if (createButton(font, "Retour", {20.f, 20.f}).isHovered(mouse)) {
                        state = AppState::BOARD_SELECTION;
                        saveBoards(boards);
                    }

                    if (createButton(font, "+ Ajouter une liste", {20.f, 90.f}).isHovered(mouse)) {
                        int id = board.lists.size() + 1;
                        board.lists.emplace_back(id, "Liste " + std::to_string(id));
                        saveBoards(boards);
                    }

                    float x = 250.f;
                    for (size_t i = 0; i < board.lists.size(); ++i) {
                        sf::FloatRect dotsZone({x + 130.f, 155.f}, {40.f, 30.f});
                        if (dotsZone.contains(mouse)) {
                            listMenuOpen = true;
                            listMenuIndex = i;
                            listMenuPos = {x + 130.f, 185.f};
                        }
                        x += 200.f;
                    }
                }
            }
        }

        // ================= RENDER =================
        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        window.clear(sf::Color(30, 30, 30));

        if (state == AppState::BOARD_SELECTION) {
            // Titre
            sf::Text title("Mes Boards", font, 36);
            title.setFillColor(sf::Color::White);
            title.setPosition(100.f, 20.f);
            window.draw(title);

            // Add board button
            Button addBtn = createButton(font, "+ Ajouter un board", {100.f, 80.f});
            addBtn.shape.setFillColor(sf::Color(100, 180, 100));
            if (addBtn.isHovered(mouse)) addBtn.shape.setFillColor(sf::Color(150, 220, 150));
            window.draw(addBtn.shape);
            window.draw(addBtn.text);

            // Boards list
            float y = 150.f;
            for (size_t i = 0; i < boards.size(); ++i) {
                std::string displayName = (renaming && i == static_cast<size_t>(selectedBoard)) ? inputText.toAnsiString() : boards[i].name;

                Button b = createButton(font, displayName, {100.f, y});
                b.shape.setFillColor(sf::Color(60, 120, 180));
                if (b.isHovered(mouse)) b.shape.setFillColor(sf::Color(80, 150, 220));

                Button dots = createButton(font, "...", {340.f, y + 5.f}, {40.f, 30.f});
                dots.shape.setFillColor(sf::Color(180, 180, 180));
                if (dots.isHovered(mouse)) dots.shape.setFillColor(sf::Color(220, 220, 100));

                window.draw(b.shape);
                window.draw(b.text);
                window.draw(dots.shape);
                window.draw(dots.text);

                y += 60.f;
            }

            // Board context menu
            if (boardMenuOpen && boardMenuIndex >= 0) {
                sf::RectangleShape menu({140.f, 80.f});
                menu.setPosition(boardMenuPos);
                menu.setFillColor(sf::Color(50, 50, 50));
                window.draw(menu);

                sf::Text rename("Renommer", font, 14);
                rename.setFillColor(sf::Color::White);
                rename.setPosition(boardMenuPos.x + 10.f, boardMenuPos.y + 10.f);
                window.draw(rename);

                sf::Text del("Supprimer", font, 14);
                del.setFillColor(sf::Color::White);
                del.setPosition(boardMenuPos.x + 10.f, boardMenuPos.y + 40.f);
                window.draw(del);

                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    if (menu.getGlobalBounds().contains(mouse)) {
                        if (mouse.y < boardMenuPos.y + 35.f) {
                            selectedBoard = boardMenuIndex;
                            renaming = true;
                            inputText = boards[selectedBoard].name;
                        } else {
                            boards.erase(boards.begin() + boardMenuIndex);
                        }
                        boardMenuOpen = false;
                        saveBoards(boards);
                    }
                }
            }
        }

        else if (state == AppState::BOARD_VIEW) {
            Board& board = boards[selectedBoard];

            sf::Text title(board.name, font, 36);
            title.setFillColor(sf::Color::White);
            title.setPosition(350.f, 20.f);
            window.draw(title);

            Button backBtn = createButton(font, "Retour", {20.f, 20});
            backBtn.shape.setFillColor(sf::Color(180, 180, 180));
            if (backBtn.isHovered(mouse)) backBtn.shape.setFillColor(sf::Color(220, 220, 100));
            window.draw(backBtn.shape);
            window.draw(backBtn.text);

            Button addListBtn = createButton(font, "+ Ajouter une liste", {20.f, 90.f});
            addListBtn.shape.setFillColor(sf::Color(100, 180, 100));
            if (addListBtn.isHovered(mouse)) addListBtn.shape.setFillColor(sf::Color(150, 220, 150));
            window.draw(addListBtn.shape);
            window.draw(addListBtn.text);

            float x = 250.f;
            for (const auto& l : board.lists) {
                sf::RectangleShape col({180.f, 400.f});
                col.setPosition(x, 150.f);
                col.setFillColor(sf::Color(70, 70, 100));
                window.draw(col);

                sf::Text name(l.name, font, 18);
                name.setFillColor(sf::Color::White);
                name.setPosition(x + 10.f, 160.f);
                window.draw(name);

                Button dots = createButton(font, "...", {x + 130.f, 155.f}, {40.f, 30.f});
                dots.shape.setFillColor(sf::Color(180,180,180));
                if (dots.isHovered(mouse)) dots.shape.setFillColor(sf::Color(220,220,100));
                window.draw(dots.shape);
                window.draw(dots.text);

                x += 200.f;
            }

            // List context menu
            if (listMenuOpen && listMenuIndex >= 0) {
                sf::RectangleShape menu({120.f, 40.f});
                menu.setPosition(listMenuPos);
                menu.setFillColor(sf::Color(50, 50, 50));
                window.draw(menu);

                sf::Text del("Supprimer", font, 14);
                del.setFillColor(sf::Color::White);
                del.setPosition(listMenuPos.x + 10.f, listMenuPos.y + 10.f);
                window.draw(del);

                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    if (menu.getGlobalBounds().contains(mouse)) {
                        board.lists.erase(board.lists.begin() + listMenuIndex);
                        listMenuOpen = false;
                        saveBoards(boards);
                    }
                }
            }
        }

        window.display();
    }

    return 0;
}
