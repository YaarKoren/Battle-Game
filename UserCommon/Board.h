#pragma once

#include <vector>
#include <memory>
#include "GameObject.h"
#include "Position.h"
#include "UserCommonNamespace.h"
USERCOMMON_BEGIN

class Board {
private:
    int width_, height_;
    std::vector<std::vector<std::vector<GameObject*>>> grid;

public:
    Board(int w, int h);
    ~Board();

    //Getters
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    void addGameObject(GameObject* obj, Position pos);
    const std::vector<GameObject*>& getObjectsAt(Position pos) const;
    void removeAllAt(Position pos);
    void removeObject(GameObject* objToRemove, Position pos);

    bool isWall(Position pos) const {
        // Returns true if the cell at (x,y) contains a wall
        return getObjectsAt(pos).front()->getSymbol() == '#';
    }

    //void cleanDestroyedWalls();
    //void clear();


};

USERCOMMON_END