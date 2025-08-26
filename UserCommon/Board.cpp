#include <algorithm>

#include "Board.h"

namespace UserCommon_207177197_301251571 {


Board::Board(size_t w, size_t h) : width_(w), height_(h) {
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
    grid.clear(); //deletes also the rows and cols
}

//this function should help create a SetelliteView, thus according to assignment specs, the char chose in//case of multipile game objects in the enrty, is the most top one (i.e. shell is on top of a mine etc)
//the caller is responsible to pass the char grid in the right size
    //TODO it makes board knows the specifc game objects but maybe it's ok
void Board::boardToCharGrid(std::vector<std::vector<char>> char_grid) const
{
    for (size_t y = 0; y < height_; ++y)
    {
        for (size_t x = 0; x < width_; ++x)
        {
            //TODO
            const auto& objects = grid[y][x];

            for (const auto& obj : objects) {

                if (dynamic_cast<Wall*>(obj)) char_grid[y][x] = '#';
                else if (dynamic_cast<Wall*>(obj)) char_grid[y][x] = '*';
                }
            }
        }
    }

void Board::resize(size_t w, size_t h) {
    width_ = w;
    height_ = h;
    grid.assign(height_, std::vector<std::vector<GameObject*>>(width_));
}
}




