#include <fstream>
#include <memory>
#include <vector>

#include "Tank.h"
#include "Board.h"

namespace UserCommon_207177197_301251571 {


Tank::Tank(const Position pos, const Direction dir, const int playerId, const int Id, const int numShells)
    : MovingGameObject(pos, dir), playerId_(playerId), id_(Id), shellsLeft_(numShells) {}

void Tank::setBoard(Board* board) {
board_ = board;
}

// Intelligent decision-making function
ActionRequest Tank::decideNextAction(const std::vector<Tank*>& allTanks) {
    if (!board_) {
        return ActionRequest::DoNothing;
    }

    // 1. First priority: Shoot if enemy is in sight and not in cooldown
    if (canSeeEnemy(allTanks) && !isWaitingAfterShoot_ && shellsLeft_ > 0) {
        return ActionRequest::Shoot;
    }

    // 2. Try to move toward enemy if path is clear
    if (canMoveForward() && !isWaitingToMoveBack_) {
        return ActionRequest::MoveForward;
    }

    // 3. Rotate toward enemy if can't move
    Direction targetDir = getDirectionTowardEnemy(allTanks);
    if (targetDir != dir_ && !isWaitingToMoveBack_) {
        // Choose the shortest rotation direction
        int currentDirVal = static_cast<int>(dir_);
        int targetDirVal = static_cast<int>(targetDir);
        int diff = (targetDirVal - currentDirVal + 8) % 8;

                if (diff <= 4) {
                return ActionRequest::RotateRight90;
            } else {
                return ActionRequest::RotateLeft90;
            }
    }

    // 4. Default action
    return ActionRequest::DoNothing;
}

bool Tank::canSeeEnemy(const std::vector<Tank*>& allTanks) const {
    if (!board_) return false;

    // Compute the forward delta as signed ints
    int dx = 0, dy = 0;
    switch (dir_) {
        case Direction::Up:        dy = -1; break;
        case Direction::Down:      dy =  1; break;
        case Direction::Left:      dx = -1; break;
        case Direction::Right:     dx =  1; break;
        case Direction::UpLeft:    dx = -1; dy = -1; break;
        case Direction::UpRight:   dx =  1; dy = -1; break;
        case Direction::DownLeft:  dx = -1; dy =  1; break;
        case Direction::DownRight: dx =  1; dy =  1; break;
    }

    // Work in signed ints to avoid underflow & sign-compare issues
    int x = (pos_.getX());
    int y = (pos_.getY());

    const std::size_t W = board_->getWidth();
    const std::size_t H = board_->getHeight();

    // Helper: signed check first, then compare as size_t
    auto in_bounds = [&](int xx, int yy) -> bool {
        return xx >= 0 && yy >= 0 &&
               static_cast<std::size_t>(xx) < W &&
               static_cast<std::size_t>(yy) < H;
    };

    using Coord = decltype(pos_.getX()); // underlying coord type (int or size_t)

    // Check along the line of sight
    for (;;) {
        x += dx; y += dy;

        // Check if we're out of bounds
        if (!in_bounds(x, y)) return false;

        // Check if position has a wall (blocks line of sight)
        // Build Position p from the signed temps (cast once we know it's valid)
        Position p = pos_;
        p.setX(static_cast<Coord>(x));
        p.setY(static_cast<Coord>(y));

        // Walls block line of sight
        const auto& objects = board_->getObjectsAt(p);
        for (GameObject* obj : objects) {
            if (obj && obj->getSymbol() == '#') return false;
        }

        // Check if there's an enemy tank at this position
        for (const Tank* t : allTanks) {
            if (!t || t->isDestroyed() || t->getPlayerId() == playerId_) continue;
            int tx = static_cast<int>(t->getPosition().getX());
            int ty = static_cast<int>(t->getPosition().getY());
            if (tx == x && ty == y) return true;
        }

        // For diagonal directions, we need to check if the path is blocked by walls
        // in the horizontal or vertical directions (Bresenham's line algorithm would be better)
        if (dx != 0 && dy != 0) {
            // Check horizontal neighbor
            int hx = x - dx, hy = y;
            if (in_bounds(hx, hy)) {
                Position hp = p;
                hp.setX(static_cast<Coord>(hx));
                hp.setY(static_cast<Coord>(hy));
                for (GameObject* obj : board_->getObjectsAt(hp))
                    if (obj && obj->getSymbol() == '#') return false;
            }

            // Check vertical neighbor
            int vx = x, vy = y - dy;
            if (in_bounds(vx, vy)) {
                Position vp = p;
                vp.setX(static_cast<Coord>(vx));
                vp.setY(static_cast<Coord>(vy));
                for (GameObject* obj : board_->getObjectsAt(vp))
                    if (obj && obj->getSymbol() == '#') return false;
            }
        }
    }
}

bool Tank::canMoveForward() const {
    if (!board_) return false;

    // Compute the forward delta as signed ints
    int dx = 0, dy = 0;
    switch (dir_) {
        case Direction::Up:        dy = -1; break;
        case Direction::Down:      dy =  1; break;
        case Direction::Left:      dx = -1; break;
        case Direction::Right:     dx =  1; break;
        case Direction::UpLeft:    dx = -1; dy = -1; break;
        case Direction::UpRight:   dx =  1; dy = -1; break;
        case Direction::DownLeft:  dx = -1; dy =  1; break;
        case Direction::DownRight: dx =  1; dy =  1; break;
        default: return false; // e.g., Stay
    }

    // Work in signed temps to avoid underflow & sign-compare issues
    int nx = (pos_.getX()) + dx;
    int ny = (pos_.getY()) + dy;

    const std::size_t W = board_->getWidth();
    const std::size_t H = board_->getHeight();

    // Check if the position is valid (within bounds)
    // Signed check for negatives, then compare as size_t for upper bounds
    if (nx < 0 || ny < 0) return false;
    if (static_cast<std::size_t>(nx) >= W ||
        static_cast<std::size_t>(ny) >= H) return false;

    // Build the next cell Position using the underlying coord type
    using Coord = decltype(pos_.getX());  // int or size_t, whatever Position uses
    Position nextPos = pos_;
    nextPos.setX(static_cast<Coord>(nx));
    nextPos.setY(static_cast<Coord>(ny));

    // Check if the position has a wall
    const auto& objects = board_->getObjectsAt(nextPos);
    for (GameObject* obj : objects) {
        if (!obj) continue;
        if (obj->getSymbol() == '#') {
            return false; // wall blocks movement
        }
    }

    return true; // in bounds, no wall, no other tank
}




Direction Tank::getDirectionTowardEnemy(const std::vector<Tank*>& allTanks) const {
    // Find the closest enemy tank
    const Tank* closestEnemy = nullptr;
    int minDistance = INT_MAX;

    for (Tank* tank : allTanks) {
        if (tank && tank->getPlayerId() != playerId_) {
            int dist = std::abs(tank->getPosition().getX() - pos_.getX()) +
                      std::abs(tank->getPosition().getY() - pos_.getY());
            if (dist < minDistance) {
                minDistance = dist;
                closestEnemy = tank;
            }
        }
    }

    if (!closestEnemy) return dir_; // No enemy found

    // Determine best direction to move toward enemy
    int enemyX = closestEnemy->getPosition().getX();
    int enemyY = closestEnemy->getPosition().getY();
    int myX = pos_.getX();
    int myY = pos_.getY();

    if (std::abs(enemyX - myX) > std::abs(enemyY - myY)) {
        // Move horizontally toward enemy
        return (enemyX > myX) ? Direction::Right : Direction::Left;
    } else {
        // Move vertically toward enemy
        return (enemyY > myY) ? Direction::Down : Direction::Up;
    }
}



// MOVE FORWARD

// moveForward() returns true if the action succeeds in changing state, even
// if no physical movement occurs (e.g., canceling a backward wait)
bool Tank::moveForward() {

    if (isWaitingToMoveBack_) {
        isWaitingToMoveBack_ = false;  // Cancel the backward waiting
        return true;  // Action succeeded: canceled waiting
    }

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
        return false;  // Unknown direction — fails safely. Should not happen tho.
    }

    return true;  // Move completed successfully
}

// MOVE BACKWARDS

// askToMoveBack() returns true if the action succeeds in changing state (e.g., entering to a backward wait state)
// no physical movement occurs.
// return false if did not succeed, cuz the tank is already in waiting state.
// make sure game manager call this function only if isRightAfterMoveBack_ == false. If it's true, the tank can move
// back immediately, no need to ask.
bool Tank::askToMoveBack() {

    // NOT in waiting state → start waiting
    if (!isWaitingToMoveBack_)
    {
        isWaitingToMoveBack_ = true;
        waitToMoveBackCounter_ = MOVE_BACK_WAIT_TURNS;
        return true; // Action succeeded: entered waiting state
    }

    // STILL waiting, can't act
    // including when still waiting but waitToMoveBackCounter == 0
    return false;

}


// Returns true if the tank did move.
bool Tank::moveBack()
{
    // extra check (cuz the game manager should check it)
    // those are the only 2 situations the tank can move back
    if ( (isRightAfterMoveBack_) || ( isWaitingToMoveBack_ && (waitToMoveBackCounter_ == 0) ))
    {
        switch (dir_) {
            case Direction::Up:
                pos_.moveDown(); break;
            case Direction::Down:
                pos_.moveUp(); break;
            case Direction::Left:
                pos_.moveRight(); break;
            case Direction::Right:
                pos_.moveLeft(); break;
            case Direction::UpLeft:
                pos_.moveDown();
                pos_.moveRight(); break;
            case Direction::UpRight:
                pos_.moveDown();
                pos_.moveLeft(); break;
            case Direction::DownLeft:
                pos_.moveUp();
                pos_.moveRight(); break;
            case Direction::DownRight:
                pos_.moveUp();
                pos_.moveLeft(); break;
            default:
                return false;  // Unknown direction — fails safely. Should not happen tho.
            }

        isRightAfterMoveBack_ = true;
        isWaitingToMoveBack_= false;
        return true;

    }

    return false;

}

// MOVE BACKWARDS UTILITY FUNCTIONS

// For game manager use only (important, to avoid double decreasing)
void Tank::updateWaitToMoveBackCounter()
{
    if (isWaitingToMoveBack_ && waitToMoveBackCounter_ > 0) {
        waitToMoveBackCounter_--;
    }
}

//For game manager use; reset after each action which is not a move back, including a failed action
void Tank::resetIsRightAfterMoveBack()
{
    isRightAfterMoveBack_ = false;
}

//exit the state of waiting to move back
//for game manager use
void Tank::resetIsWaitingToMoveBack()
{
    if (isWaitingToMoveBack_ &&  waitToMoveBackCounter_== 0)
    {
        isWaitingToMoveBack_ = false;
    }
}


int Tank::getWaitToMoveBackCounter() const
{
    return waitToMoveBackCounter_;
}

// CHANGE DIRECTION

// utility functions, for tank use only
void Tank::actualRotateEighthLeft() {
    dir_ = static_cast<Direction>((static_cast<int>(dir_) + 7) % 8);
}

void Tank::actualRotateEighthRight() {
    dir_ = static_cast<Direction>((static_cast<int>(dir_) + 1) % 8);
}


// for game manager use
// return false if action did not succeed, cuz tank is waiting to move back,
// including when it's the turn it should stop waiting and move back (i.e waitToMoveBackCounter_ ==0)
// else, it succeeds and return true
bool Tank::rotateEighthLeft()
{
    if (isWaitingToMoveBack_)
    {
        return false;
    }
    actualRotateEighthLeft();
    return true;
}

bool Tank::rotateFourthLeft()
{
    if (isWaitingToMoveBack_)
    {
        return false;
    }
    // do rotate eight, twice
    actualRotateEighthLeft();
    actualRotateEighthLeft();
    return true;
}

bool Tank::rotateEighthRight()
{
    if (isWaitingToMoveBack_)
    {
        return false;
    }
    actualRotateEighthRight();
    return true;
}

bool Tank::rotateFourthRight()
{
    if (isWaitingToMoveBack_)
    {
        return false;
    }
    // do rotate eight, twice
    actualRotateEighthRight();
    actualRotateEighthRight();
    return true;
}

// SHOOT
bool Tank::shoot()
{
    if (isWaitingToMoveBack_)
    {
        return false;
    }

    if (isWaitingAfterShoot_ && waitAfterShootCounter_ > 0)
    {
        return false;
    }

    if (shellsLeft_ <= 0)
    {
        return false;
    }

    //if the tank is not waiting after shoot, or waiting and waitAfterShootCounter_ == 0
    //shoot
    shellsLeft_--;
    isWaitingAfterShoot_ = true;
    waitAfterShootCounter_ = AFTER_SHOOT_WAIT_TURNS;
    return true;
}



// SHOOT utility functions

// For game manager use only (important, to avoid double decreasing)
void Tank::updateWaitAfterShootCounter()
{
    if (isWaitingAfterShoot_ && waitAfterShootCounter_ > 0)
    {
        waitAfterShootCounter_--;
    }
}

//exit the state of waiting after shoot
// for game manager use
void Tank::resetIsWaitingAfterShoot()
{
    if (isWaitingAfterShoot_ &&  waitAfterShootCounter_== 0)
    {
        isWaitingAfterShoot_ = false;
    }
}

void Tank::doNothing()
{
    return;
}


// GETTERS

//Each Tank object will have a different symbol - '1' or '2', depending on the player it belongs to
char Tank::getSymbol() const
{
    if (playerId_ < 1 || playerId_ > 2) return '?';
    return static_cast<char>('0' + playerId_);
}

int Tank::getPlayerId() const
{
    return playerId_;
}

int Tank::getId() const
{
    return id_;
}

int Tank::getShellsLeft() const
{
    return shellsLeft_;
}
bool Tank::getIsWaitingToMoveBack() const
{
    return isWaitingToMoveBack_;
}

bool Tank::getIsWaitingAfterShoot() const
{
    return isWaitingAfterShoot_;
}

bool Tank::getIsRightAfterMoveBack() const
{
    return isRightAfterMoveBack_;
}

ActionRequest Tank::getNextAction() const
{
    return nextAction_;
}

ActionRequest Tank::getLastAction() const
{
    return lastAction_;
}

TankAlgorithm* Tank::getAlgorithm() const {
    return algorithm_.get();
}

void Tank::setAlgorithm(std::unique_ptr<TankAlgorithm> algo) {
    algorithm_ = std::move(algo);
}


bool Tank::getIstRequestedBattleInfo() const {
    return requestedBattleInfo_;
}




//SETTERS
void Tank::setNextAction(ActionRequest action)
{
    nextAction_ = action;
}

void Tank::setLastAction(ActionRequest action) {
    lastAction_ = action;
}

}