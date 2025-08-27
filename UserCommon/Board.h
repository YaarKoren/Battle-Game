#pragma once

#include <vector>
#include <memory>

#include "Position.h"
#include "GameObject.h"
#include "Wall.h"
#include "Tank.h"
#include "Mine.h"
#include "Shell.h"



namespace UserCommon_207177197_301251571 {

class Board {
private:
    size_t width_, height_;

    std::vector<std::vector<std::vector<GameObject*>>> grid; // each entry of the matrix, holds of a vector of GameObject*,
                                                            //to support multiple GameObjects in the entry (i.e. a mine and a shell)

public:
    Board(size_t w, size_t h);

    // No default-constructed boards:
    Board() = delete;

    ~Board() = default;

    // Allow shallow copy (copies the pointers):
    Board(const Board&)            = default;
    Board& operator=(const Board&) = default;

    // Enable real moves:
    Board(Board&&) noexcept            = default;
    Board& operator=(Board&&) noexcept = default;

    //Getters
    size_t getWidth() const { return width_; }
    size_t getHeight() const { return height_; }
    const std::vector<GameObject*>& getObjectsAt(Position pos) const;


    void addGameObject(GameObject* obj, Position pos);
    void removeAllAt(Position pos);

    void removeObject(GameObject* objToRemove, Position pos);

    //void cleanDestroyedWalls();
    void clear();

    void Board::boardToCharGrid(std::vector<std::vector<char>>& char_grid) const;

    void Board::resize(size_t w, size_t h);

};

}