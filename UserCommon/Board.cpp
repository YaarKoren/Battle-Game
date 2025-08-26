#include <algorithm>

#include "Board.h"

namespace UserCommon_207177197_301251571 {


Board::Board(int w, int h) : width_(w), height_(h) {
    grid.resize(height_, std::vector<std::vector<GameObject*>>(width_));
}


//don’t need to clear nested vectors; they destruct themselves
/*
Board::~Board() {
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            for (GameObject* obj : grid[y][x]) {
                //delete obj;  // Only if Board owns the objects
            }
            grid[y][x].clear();
        }
    }
}
*/

// ADD GAME OBJECT TO VECTOR IN CELL


void Board::addGameObject(GameObject* obj, Position pos)
{
    pos.wrap(width_, height_); //maybe not needed. note: the function gets a copy of "pos", so this does not change the original Position
    // Board stores raw pointers — caller owns the unique_ptr
    grid[pos.getY()][pos.getX()].push_back(obj); // adds the object to the end of the vector at the specified cell; just store pointer, no ownership!
}


// GET  A VECTOR OF GAME 0BJECTS IN A CELL

// the meaning of the first "const": the function returns a reference to a vector of pointers to GameObject,
// and you are not allowed to change the vector itself (add/remove/etc).
// the meaning of the second "const": This method does not modify the Board object.
const std::vector<GameObject*>& Board::getObjectsAt(Position pos) const
{
    pos.wrap(width_, height_); //maybe not needed. note: the function gets a copy of "pos", so this does not change the original Position
    return grid[pos.getY()][pos.getX()]; // return the vector, empty or not (the caller handles it)

}

// REMOVE VECTOR IN CELL

void Board::removeAllAt(Position pos) {
    pos.wrap(width_, height_); //maybe not needed. note: the function gets a copy of "pos", so this does not change the original Position
    grid[pos.getY()][pos.getX()].clear();  // Just drop all pointers
}

// REMOVE A SPECIFIC OBJECT IN A CELL'S VECTOR

void Board::removeObject(GameObject* objToRemove, Position pos)
{
    pos.wrap(width_, height_); //maybe not needed. note: the function gets a copy of "pos", so this does not change the original Position
    auto& cell = grid[pos.getY()][pos.getX()];
    cell.erase(
            std::remove(cell.begin(), cell.end(), objToRemove),
            cell.end()
    );
}

void Board::clear()
{
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            grid[y][x].clear();
        }
    }
}

}

