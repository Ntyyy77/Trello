#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include "ui.hpp"
#include "board.hpp"
#include "menu.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

#include <cstdio>
#include <memory>
#include <array>

std::string openFileDialog() {
    std::array<char, 256> buffer;
    std::string result;

    std::shared_ptr<FILE> pipe(
        popen("zenity --file-selection --file-filter='Config files | *.cfg'", "r"),
        pclose
    );

    if (!pipe) return "";

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if (!result.empty() && result.back() == '\n')
        result.pop_back();

    return result;
}


Board loadBoardFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier");
    }

    Board loadedBoard(0, "");
    List* currentList = nullptr;

    std::string word;

    while (file >> word) {

        if (word == "BOARD") {
            int id;
            std::string name;
            file >> id >> std::quoted(name);
            loadedBoard = Board(id, name);
        }

        else if (word == "LIST") {
            int id;
            std::string name;
            file >> id >> std::quoted(name);
            loadedBoard.lists.emplace_back(id, name);
            currentList = &loadedBoard.lists.back();
        }

        else if (word == "CARD" && currentList) {
            int id;
            std::string title, desc;
            file >> id >> std::quoted(title) >> std::quoted(desc);
            currentList->cards.emplace_back(id, title);
            currentList->cards.back().description = desc;
        }
    }

    file.close();
    return loadedBoard;
}


// ================== SAVE ==================
void saveBoards(const std::vector<Board>& boards) {

    for (const auto& board : boards) {

        std::ofstream file("board_" + std::to_string(board.id) + ".cfg");

        file << "BOARD "
             << board.id << " "
             << std::quoted(board.name) << "\n";

        for (const auto& list : board.lists) {

            file << "LIST "
                 << list.id << " "
                 << std::quoted(list.name) << "\n";

            for (const auto& card : list.cards) {

                file << "CARD "
                     << card.id << " "
                     << std::quoted(card.title) << " "
                     << std::quoted(card.description) << "\n";
            }

            file << "ENDLIST\n";
        }

        file << "ENDBOARD\n";
        file.close();
    }
}


struct CardEditor {
    bool visible = false;
    Card* card = nullptr;
    List* parentList = nullptr;

    sf::String titleInput;
    sf::String descInput;

    bool editingTitle = false;
    bool editingDesc = false;

    void open(Card& c, List& list) {
        card = &c;
        parentList = &list;
        titleInput = c.title;
        descInput = c.description;
        visible = true;
        editingTitle = false;
        editingDesc = false;
    }

    void close() {
        visible = false;
        card = nullptr;
        parentList = nullptr;
    }

    void handleEvent(const sf::Event& event, std::vector<Board>& boards) {
        if (!visible || !card) return;

        if (event.type == sf::Event::TextEntered) {
            if (event.text.unicode == '\b') {
                if (editingTitle && titleInput.getSize() > 0)
                    titleInput.erase(titleInput.getSize() - 1, 1);
                if (editingDesc && descInput.getSize() > 0)
                    descInput.erase(descInput.getSize() - 1, 1);
            }
            else if (event.text.unicode < 128) {
                if (editingTitle)
                    titleInput += static_cast<char>(event.text.unicode);
                if (editingDesc)
                    descInput += static_cast<char>(event.text.unicode);
            }
        }

        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left) {

            sf::Vector2f mouse(event.mouseButton.x, event.mouseButton.y);

            // Zones boutons
            sf::FloatRect titleBox(350, 250, 300, 40);
            sf::FloatRect descBox(350, 310, 300, 80);
            sf::FloatRect renameBtn(350, 410, 90, 35);
            sf::FloatRect deleteBtn(460, 410, 90, 35);
            sf::FloatRect saveBtn(570, 410, 120, 35);

            if (titleBox.contains(mouse)) {
                editingTitle = true;
                editingDesc = false;
            }
            else if (descBox.contains(mouse)) {
                editingDesc = true;
                editingTitle = false;
            }
            else if (renameBtn.contains(mouse)) {
                card->title = titleInput;
            }
            else if (deleteBtn.contains(mouse)) {
                auto& cards = parentList->cards;
                cards.erase(std::remove_if(cards.begin(), cards.end(),
                    [&](const Card& c) { return c.id == card->id; }),
                    cards.end());
                close();
            }
            else if (saveBtn.contains(mouse)) {
                card->title = titleInput;
                card->description = descInput;
                close();
            }
        }
    }

    void draw(sf::RenderWindow& window, sf::Font& font) {
        if (!visible || !card) return;

        // Fond sombre semi transparent
        sf::RectangleShape overlay({1000, 700});
        overlay.setFillColor(sf::Color(0,0,0,150));
        window.draw(overlay);

        // Carré noir central
        sf::RectangleShape panel({400, 300});
        panel.setPosition(300, 200);
        panel.setFillColor(sf::Color(30,30,30));
        panel.setOutlineThickness(2);
        panel.setOutlineColor(sf::Color::White);
        window.draw(panel);

        // Titre affiché
        sf::Text header("Card Editor", font, 20);
        header.setFillColor(sf::Color::White);
        header.setPosition(420, 210);
        window.draw(header);

        // Title Box
        sf::RectangleShape titleBox({300, 40});
        titleBox.setPosition(350, 250);
        titleBox.setFillColor(sf::Color::White);
        window.draw(titleBox);

        sf::Text titleText(titleInput, font, 16);
        titleText.setFillColor(sf::Color::Black);
        titleText.setPosition(360, 260);
        window.draw(titleText);

        // Description Box
        sf::RectangleShape descBox({300, 80});
        descBox.setPosition(350, 310);
        descBox.setFillColor(sf::Color::White);
        window.draw(descBox);

        sf::Text descText(descInput, font, 14);
        descText.setFillColor(sf::Color::Black);
        descText.setPosition(360, 320);
        window.draw(descText);

        // Rename Button
        sf::RectangleShape renameBtn({90, 35});
        renameBtn.setPosition(350, 410);
        renameBtn.setFillColor(sf::Color(100,180,100));
        window.draw(renameBtn);

        sf::Text renameText("Rename", font, 14);
        renameText.setFillColor(sf::Color::Black);
        renameText.setPosition(365, 418);
        window.draw(renameText);

        // Delete Button
        sf::RectangleShape deleteBtn({90, 35});
        deleteBtn.setPosition(460, 410);
        deleteBtn.setFillColor(sf::Color(180,100,100));
        window.draw(deleteBtn);

        sf::Text deleteText("Delete", font, 14);
        deleteText.setFillColor(sf::Color::Black);
        deleteText.setPosition(480, 418);
        window.draw(deleteText);

        // Save Button
        sf::RectangleShape saveBtn({120, 35});
        saveBtn.setPosition(570, 410);
        saveBtn.setFillColor(sf::Color(100,100,220));
        window.draw(saveBtn);

        sf::Text saveText("Save Changes", font, 14);
        saveText.setFillColor(sf::Color::White);
        saveText.setPosition(585, 418);
        window.draw(saveText);
    }
};


// ================== APP ==================
enum class AppState { BOARD_SELECTION, BOARD_VIEW };


int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 700), "SFML Trello");
    window.setFramerateLimit(60);

    sf::Font font;
    font.loadFromFile("arial.ttf");

    std::vector<Board> boards;
    int nextBoardId = 1, nextListId = 1, nextCardId = 1;
    int selectedBoard = -1;

    AppState state = AppState::BOARD_SELECTION;

    CardEditor cardEditor;

    bool isDragging = false;
    Card* draggedCard = nullptr;
    List* sourceList = nullptr;
    sf::Vector2f dragOffset;

     // ===== Context menu board =====
    bool boardContextVisible = false;
    int boardContextIndex = -1;
    sf::Vector2f boardContextPos;

    // Rename board mode
    bool renamingBoard = false;
    sf::String renameInput;


    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            if (event.type == sf::Event::Closed)
                window.close();

            cardEditor.handleEvent(event, boards);

            // ===== CLIC DROIT SUR CARTE =====
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Right &&
                state == AppState::BOARD_VIEW &&
                !cardEditor.visible)
            {
                Board& board = boards[selectedBoard];
                float x = 250.f;

                for (auto& list : board.lists) {
                    float cardY = 190.f;
                    for (auto& card : list.cards) {
                        sf::FloatRect rect({x + 10.f, cardY}, {160.f, 30.f});
                        if (rect.contains(mouse)) {
                            cardEditor.open(card, list); // ← Ouvre le CardEditor
                            break;
                        }
                        cardY += 40.f;
                    }
                    x += 200.f;
                }
            }



            // ===== Rename board text input =====
            if (renamingBoard && event.type == sf::Event::TextEntered) {

                if (event.text.unicode == '\b') {
                    if (renameInput.getSize() > 0)
                        renameInput.erase(renameInput.getSize() - 1, 1);
                }
                else if (event.text.unicode == '\r') { // Enter
                    boards[boardContextIndex].name = renameInput;
                    renamingBoard = false;
                }
                else if (event.text.unicode < 128) {
                    renameInput += static_cast<char>(event.text.unicode);
                }
            }

            // ================== CLIC GAUCHE ==================
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left &&
                !cardEditor.visible) {

                // ================== BOARD SELECTION ==================
                if (state == AppState::BOARD_SELECTION) {

                    // Ajouter board
                    if (createButton(font, "+ Ajouter un board", {100.f, 80.f}).isHovered(mouse)) {
                        boards.emplace_back(nextBoardId, "Board " + std::to_string(nextBoardId));
                        nextBoardId++;
                    }

                    float yCheck = 180.f;
                    for (size_t i = 0; i < boards.size(); ++i) {
                        sf::FloatRect delRect(360.f, yCheck, 30.f, 30.f);
                        if (delRect.contains(mouse)) {
                            boards.erase(boards.begin() + i);
                            break; // sortir de la boucle pour éviter crash
                        }
                        yCheck += 60.f;
                    }


                    // Load board (Linux zenity)
                    if (createButton(font, "Load Board", {100.f, 130.f}).isHovered(mouse)) {

                        std::string path = openFileDialog();

                        if (!path.empty()) {
                            try {
                                Board loaded = loadBoardFromFile(path);
                                boards.push_back(loaded);

                                if (loaded.id >= nextBoardId)
                                    nextBoardId = loaded.id + 1;
                            }
                            catch (...) {
                                std::cout << "Erreur chargement fichier\n";
                            }
                        }
                    }

                    // Sélection board
                    float y = 180.f; // ↓ DESCENDU
                    for (size_t i = 0; i < boards.size(); ++i) {
                        if (createButton(font, boards[i].name, {100.f, y}).isHovered(mouse)) {
                            selectedBoard = i;
                            state = AppState::BOARD_VIEW;
                        }
                        y += 60.f;
                    }
                }

                // ================== BOARD VIEW ==================
                else if (state == AppState::BOARD_VIEW) {

                    Board& board = boards[selectedBoard];

                    if (createButton(font, "Retour", {20.f, 20.f}).isHovered(mouse))
                        state = AppState::BOARD_SELECTION;

                    if (createButton(font, "+ Ajouter une liste", {20.f, 90.f}).isHovered(mouse)) {
                        board.lists.emplace_back(nextListId++, "Liste " + std::to_string(nextListId - 1));
                    }

                    if (createButton(font, "+ Ajouter une carte", {20.f, 140.f}).isHovered(mouse)) {
                        if (!board.lists.empty()) {
                            board.lists[0].cards.emplace_back(nextCardId++, "Carte " + std::to_string(nextCardId - 1));
                        }
                    }

                    if (createButton(font, "Save Board", {20.f, 190.f}).isHovered(mouse)) {
                        saveBoards(boards);
                    }

                    // Drag start
                    float x = 250.f;
                    for (auto& list : board.lists) {
                        float cardY = 190.f;

                        for (auto& card : list.cards) {
                            sf::FloatRect rect({x + 10.f, cardY}, {160.f, 30.f});

                            if (rect.contains(mouse)) {
                                isDragging = true;
                                draggedCard = &card;
                                sourceList = &list;
                                dragOffset = mouse - sf::Vector2f(rect.left, rect.top);
                            }

                            cardY += 40.f;
                        }
                        x += 200.f;
                    }
                    float xCheck = 250.f;
                    for (size_t i = 0; i < board.lists.size(); ++i) {
                        sf::FloatRect delRect(xCheck + 140.f, 155.f, 30.f, 30.f);
                        if (delRect.contains(mouse)) {
                            board.lists.erase(board.lists.begin() + i);
                            break; // On sort pour éviter crash
                        }
                        xCheck += 200.f;
                    }

                }
            }

            // ================== CLIC DROIT ==================
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Right &&
                state == AppState::BOARD_SELECTION &&
                !cardEditor.visible)
            {
                float y = 180.f;

                for (size_t i = 0; i < boards.size(); ++i) {

                    sf::FloatRect boardRect({100.f, y}, {250.f, 40.f});

                    if (boardRect.contains(mouse)) {
                        boardContextVisible = true;
                        boardContextIndex = i;
                        boardContextPos = mouse;
                        renamingBoard = false;
                        break;
                    }

                    y += 60.f;
                }
            }


            // ================== DROP ==================
            if (event.type == sf::Event::MouseButtonReleased &&
                event.mouseButton.button == sf::Mouse::Left) {

                if (isDragging && draggedCard) {

                    Board& board = boards[selectedBoard];
                    float x = 250.f;

                    for (auto& list : board.lists) {

                        sf::FloatRect zone(x, 150.f, 180.f, 400.f);

                        if (zone.contains(mouse) && &list != sourceList) {

                            list.cards.push_back(*draggedCard);

                            auto& cards = sourceList->cards;
                            cards.erase(std::remove_if(cards.begin(), cards.end(),
                                [&](const Card& c) {
                                    return c.id == draggedCard->id;
                                }),
                                cards.end());

                            break;
                        }

                        x += 200.f;
                    }
                }

                isDragging = false;
                draggedCard = nullptr;
                sourceList = nullptr;
            }
        }

        // ================== RENDER ==================
        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        window.clear(sf::Color(30, 30, 30));

        if (state == AppState::BOARD_SELECTION) {
            // ===== Click on context menu ======
            sf::Text title("Mes Boards", font, 36);
            title.setFillColor(sf::Color::White);
            title.setPosition(100.f, 20.f);
            window.draw(title);

            Button addBtn = createButton(font, "+ Ajouter un board", {100.f, 80.f});
            addBtn.shape.setFillColor(sf::Color(100, 180, 100));
            if (addBtn.isHovered(mouse))
                addBtn.shape.setFillColor(sf::Color(150, 220, 150));
            window.draw(addBtn.shape);
            window.draw(addBtn.text);

            Button loadBtn = createButton(font, "Load Board", {100.f, 130.f});
            loadBtn.shape.setFillColor(sf::Color(100, 100, 220));
            if (loadBtn.isHovered(mouse))
                loadBtn.shape.setFillColor(sf::Color(150, 150, 255));
            window.draw(loadBtn.shape);
            window.draw(loadBtn.text);

            float y = 180.f; // ↓ DESCENDU
            for (size_t i = 0; i < boards.size(); ++i) {

                Button b = createButton(font, boards[i].name, {100.f, y});
                b.shape.setFillColor(sf::Color(60, 120, 180));
                if (b.isHovered(mouse))
                    b.shape.setFillColor(sf::Color(80, 150, 220));

                window.draw(b.shape);
                window.draw(b.text);

                // ===== Petit bouton delete rouge =====
                sf::RectangleShape delBtn({30.f, 30.f});
                delBtn.setPosition(360.f, y); // juste à droite du board
                delBtn.setFillColor(sf::Color(180, 50, 50));
                window.draw(delBtn);

                sf::Text delText("X", font, 16);
                delText.setFillColor(sf::Color::White);
                delText.setPosition(367.f, y + 5.f);
                window.draw(delText);

                // Si bouton survolé, on peut changer la couleur
                if (delBtn.getGlobalBounds().contains(mouse)) {
                    delBtn.setFillColor(sf::Color(220, 80, 80));
                }

                y += 60.f;
            }
            if (boardContextVisible) {

                sf::FloatRect renameRect(boardContextPos, {120.f, 30.f});
                sf::FloatRect deleteRect({boardContextPos.x, boardContextPos.y + 30.f}, {120.f, 30.f});

                if (renameRect.contains(mouse)) {
                    renamingBoard = true;
                    renameInput = boards[boardContextIndex].name;
                    boardContextVisible = false;
                }
                else if (deleteRect.contains(mouse)) {
                    boards.erase(boards.begin() + boardContextIndex);
                    boardContextVisible = false;
                }
                else {
                    boardContextVisible = false;
                }
            }
            // ===== Draw context menu =====
            if (boardContextVisible) {

                sf::RectangleShape menuBg({120.f, 60.f});
                menuBg.setPosition(boardContextPos);
                menuBg.setFillColor(sf::Color(40,40,40));
                menuBg.setOutlineColor(sf::Color::White);
                menuBg.setOutlineThickness(1.f);
                window.draw(menuBg);

                sf::Text renameText("Rename", font, 16);
                renameText.setFillColor(sf::Color::White);
                renameText.setPosition(boardContextPos.x + 10.f, boardContextPos.y + 5.f);
                window.draw(renameText);

                sf::Text deleteText("Delete", font, 16);
                deleteText.setFillColor(sf::Color::White);
                deleteText.setPosition(boardContextPos.x + 10.f, boardContextPos.y + 30.f);
                window.draw(deleteText);
            }
            // ===== Draw rename overlay =====
            if (renamingBoard) {

                sf::RectangleShape overlay({1000, 700});
                overlay.setFillColor(sf::Color(0,0,0,150));
                window.draw(overlay);

                sf::RectangleShape box({400, 120});
                box.setPosition(300, 250);
                box.setFillColor(sf::Color(30,30,30));
                box.setOutlineColor(sf::Color::White);
                box.setOutlineThickness(2.f);
                window.draw(box);

                sf::Text title("Rename Board", font, 20);
                title.setFillColor(sf::Color::White);
                title.setPosition(420, 260);
                window.draw(title);

                sf::RectangleShape inputBox({300, 40});
                inputBox.setPosition(350, 300);
                inputBox.setFillColor(sf::Color::White);
                window.draw(inputBox);

                sf::Text input(renameInput, font, 16);
                input.setFillColor(sf::Color::Black);
                input.setPosition(360, 310);
                window.draw(input);

                sf::Text hint("Press Enter to validate", font, 14);
                hint.setFillColor(sf::Color::White);
                hint.setPosition(380, 350);
                window.draw(hint);
            }

        }

        else if (state == AppState::BOARD_VIEW) {

            Board& board = boards[selectedBoard];

            sf::Text title(board.name, font, 36);
            title.setFillColor(sf::Color::White);
            title.setPosition(350.f, 20.f);
            window.draw(title);

            Button backBtn = createButton(font, "Retour", {20.f, 20.f});
            window.draw(backBtn.shape);
            window.draw(backBtn.text);

            Button addListBtn = createButton(font, "+ Ajouter une liste", {20.f, 90.f});
            window.draw(addListBtn.shape);
            window.draw(addListBtn.text);

            Button addCardBtn = createButton(font, "+ Ajouter une carte", {20.f, 140.f});
            window.draw(addCardBtn.shape);
            window.draw(addCardBtn.text);

            Button saveBtn = createButton(font, "Save Board", {20.f, 190.f});
            saveBtn.shape.setFillColor(sf::Color(100, 100, 220));
            if (saveBtn.isHovered(mouse))
                saveBtn.shape.setFillColor(sf::Color(150, 150, 255));
            window.draw(saveBtn.shape);
            window.draw(saveBtn.text);

            float x = 250.f;
            for (auto& list : board.lists) {

                sf::RectangleShape col({180.f, 400.f});
                col.setPosition(x, 150.f);
                col.setFillColor(sf::Color(70, 70, 100));
                window.draw(col);

                // Nom de la liste
                sf::Text name(list.name, font, 18);
                name.setFillColor(sf::Color::White);
                name.setPosition(x + 10.f, 160.f);
                window.draw(name);

                // Bouton rouge pour supprimer la liste
                sf::RectangleShape delBtn({30.f, 30.f});
                delBtn.setPosition(x + 140.f, 155.f);
                delBtn.setFillColor(sf::Color(180, 50, 50));
                window.draw(delBtn);

                sf::Text delText("X", font, 16);
                delText.setFillColor(sf::Color::White);
                delText.setPosition(x + 147.f, 160.f);
                window.draw(delText);

                // Changement couleur au survol
                if (delBtn.getGlobalBounds().contains(mouse))
                    delBtn.setFillColor(sf::Color(220, 80, 80));


                float cardY = 190.f;
                for (auto& card : list.cards) {

                    sf::RectangleShape cardShape({160.f, 30.f});
                    cardShape.setPosition(x + 10.f, cardY);
                    cardShape.setFillColor(sf::Color(220, 220, 220));
                    window.draw(cardShape);

                    sf::Text t(card.title, font, 14);
                    t.setFillColor(sf::Color::Black);
                    t.setPosition(x + 15.f, cardY + 5.f);
                    window.draw(t);

                    cardY += 40.f;
                }

                x += 200.f;
            }

            if (isDragging && draggedCard) {
                sf::RectangleShape cardShape({160.f, 30.f});
                cardShape.setPosition(mouse - dragOffset);
                cardShape.setFillColor(sf::Color(200, 200, 0));
                window.draw(cardShape);
            }
        }

        cardEditor.draw(window, font);
        window.display();
    }

    return 0;
}
