#pragma once


namespace UserCommon_207177197_301251571
{
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
        void setX(size_t x) { x_ = x; }
        void setY(size_t y) { y_ = y; }

        // Set both
        void set(size_t x, size_t y) { x_ = x; y_ = y; }

        // Wrap around board dimensions
        // Wrap around board dimensions (safe for negatives and large values)
        void wrap(const int boardWidth, const int boardHeight) {
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
}
