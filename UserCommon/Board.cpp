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

//this function should help create a SatelliteView
//specs: "For artillery shell, in case an artillery shell is in the air above a mine, the satellite sees the artillery shell and misses the mine"
    //(this function handle only case of shells + some other object/objects.
    //for other cases of multiple objects: those cases happens only fot a short time, before the two objects gets destroyed.
    //creating the SatelliteView from the board does not happen during this short times. So, we don't handle those cases).
//the caller is responsible to pass the char grid in the right size
//(notice that this function is the reason Board has to know the specific game objects; can move this to Game Manager if it's better)
void Board::boardToCharGrid(std::vector<std::vector<char>>& char_grid) const
{
    for (size_t y = 0; y < height_; ++y) {
        for (size_t x = 0; x < width_; ++x)
        {
            const auto& cell = grid[y][x];

            if (cell.empty()) {
                char_grid[y][x] = ' ';
                continue;
            }

            // Only overlap we handle: shell + mine.
            const GameObject* any = nullptr;
            const GameObject* shell = nullptr;

            for (const GameObject* obj : cell) {
                if (!obj) continue;
                any = obj; // remember something in case there's no shell
                if (dynamic_cast<const Shell*>(obj)) {
                    shell = obj;      // shell wins if present
                    break;            // no need to keep scanning
                }
            }

            if (!any) {
                char_grid[y][x] = ' '; // Empty cell → space
            } else {
                char_grid[y][x] = (shell ? shell : any)->getSymbol(); //shell wins; if there is no shell, other object gets to be shown
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




