#include "InputParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

////need to update to assigmnment 3

InputParser::InputParser(const std::string& filename)
        : board_(1, 1), maxSteps_(0), numShells_(0) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Failed to open input file");

    std::string line;
    std::vector<std::string> lines;

    while (std::getline(file, line)) {
        if (!line.empty()) lines.push_back(line);
    }

    if (lines.size() < 5) throw std::runtime_error("Input file missing metadata lines");

    // Parse metadata
    int rows = 0, cols = 0;
    for (int i = 1; i <= 4; ++i) {
        std::string line = lines[i];
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end()); // remove all spaces

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            throw std::runtime_error("Malformed metadata line");
        }

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);
        int val = std::stoi(value);

        if (key == "MaxSteps")        maxSteps_ = val;
        else if (key == "NumShells")  numShells_ = val;
        else if (key == "Rows")       rows = val;
        else if (key == "Cols")       cols = val;
    }


    board_ = Board(cols, rows);
    int tankIdCounter = 0;

    for (int y = 0; y < rows; ++y) {
        std::string rowLine = (5 + y < (int)lines.size()) ? lines[5 + y] : "";
        if ((int)rowLine.length() < cols) rowLine += std::string(cols - rowLine.length(), ' ');
        else if ((int)rowLine.length() > cols) rowLine = rowLine.substr(0, cols);

        for (int x = 0; x < cols; ++x) {
            char ch = rowLine[x];
            Position pos(x, y);

            if (ch == '#') {
                auto wall = std::make_unique<Wall>(pos);
                board_.addGameObject(wall.get(), pos);
                activeWalls_.push_back(std::move(wall));
            } else if (ch == '@') {
                auto mine = std::make_unique<Mine>(pos);
                board_.addGameObject(mine.get(), pos);
                activeMines_.push_back(std::move(mine));
            } else if (ch == '1') {
                auto tank = std::make_unique<Tank>(pos, Direction::Right, 1, ++tankIdCounter);
                board_.addGameObject(tank.get(), pos);
                player1Tanks_.push_back(std::move(tank));
            } else if (ch == '2') {
                auto tank = std::make_unique<Tank>(pos, Direction::Left, 2, ++tankIdCounter);
                board_.addGameObject(tank.get(), pos);
                player2Tanks_.push_back(std::move(tank));
            }
        }
    }
}

Board InputParser::getBoard() const {
    return board_;
}

std::vector<std::unique_ptr<Tank>> InputParser::getPlayer1Tanks() {
    return std::move(player1Tanks_);
}

std::vector<std::unique_ptr<Tank>> InputParser::getPlayer2Tanks() {
    return std::move(player2Tanks_);
}


std::vector<std::unique_ptr<Mine>>& InputParser::getActiveMines() {
    return activeMines_;
}

std::vector<std::unique_ptr<Wall>>& InputParser::getActiveWalls() {
    return activeWalls_;
}

int InputParser::getMaxSteps() const {
    return maxSteps_;
}

int InputParser::getNumShells() const {
    return numShells_;
}

