#pragma once

#include <vector>
#include <memory>

#include "Position.h"
#include "GameObject.h"

//those are not needed, cuz we use the abstract class
/*
#include "Shell.h"
#include "Mine.h"
#include "Wall.h"
*/



namespace UserCommon_207177197_301251571 {

class Board {
private:
    int width_, height_;
    std::vector<std::vector<std::vector<GameObject*>>> grid;

public:
    Board(int w, int h);

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
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    void addGameObject(GameObject* obj, Position pos);
    const std::vector<GameObject*>& getObjectsAt(Position pos) const;
    void removeAllAt(Position pos);
    void removeObject(GameObject* objToRemove, Position pos);

    bool isWall(Position pos) const {
        // Returns true if the cell at (x,y) contains a wall
        return getObjectsAt(pos).front()->getSymbol() == '#'; //TODO: check this, cuz the board should have pointer to Game Objects and not chars
    }

    //void cleanDestroyedWalls();
    //void clear();


};

}