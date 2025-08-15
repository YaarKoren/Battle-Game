#pragma once

#include "UserCommonNamespace.h"

USERCOMMON_BEGIN

class Position {
public:
    // Default constructor
    Position() : x_(0), y_(0) {}

    // Param constructor (optional)
    Position(int x, int y) : x_(x), y_(y) {}

    // Getters
    int getX() const { return x_; }
    int getY() const { return y_; }

    // Setters
    void setX(int x) { x_ = x; }
    void setY(int y) { y_ = y; }

    // Set both
    void set(int x, int y) { x_ = x; y_ = y; }

    // Wrap around board dimensions
    // Wrap around board dimensions (safe for negatives and large values)
    void wrap(int boardWidth, int boardHeight) {
        x_ = ((x_ % boardWidth) + boardWidth) % boardWidth;
        y_ = ((y_ % boardHeight) + boardHeight) % boardHeight;
    }


    //Moving
    void moveUp() {y_ -= 1;}
    void moveDown() {y_ += 1;}
    void moveLeft() {x_ -= 1;}
    void moveRight() {x_ += 1;}

    // Equality check
    bool operator==(const Position& other) const {
        return x_ == other.x_ && y_ == other.y_;
    }

private:
    int x_;
    int y_;
};



USERCOMMON_END
