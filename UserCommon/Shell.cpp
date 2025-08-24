#include "Shell.h"

namespace UserCommon_207177197_301251571
{
    Shell::Shell(Position pos, Direction dir, int tankId)
        : MovingGameObject(pos, dir), tankId_(tankId) {}

    bool Shell::moveForward() {
        switch (dir_) {
        case Direction::Up:
            pos_.moveUp(); break;
        case Direction::Down:
            pos_.moveDown(); break;
        case Direction::Left:
            pos_.moveLeft(); break;
        case Direction::Right:
            pos_.moveRight(); break;
        case Direction::UpLeft:
            pos_.moveUp();
            pos_.moveLeft(); break;
        case Direction::UpRight:
            pos_.moveUp();
            pos_.moveRight(); break;
        case Direction::DownLeft:
            pos_.moveDown();
            pos_.moveLeft(); break;
        case Direction::DownRight:
            pos_.moveDown();
            pos_.moveRight(); break;
        default:
            return false;  // Unknown direction — fail safely. Should not happen tho.
        }

        return true;  // Move completed successfully
    }
}