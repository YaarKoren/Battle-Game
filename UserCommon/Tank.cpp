#include <fstream>
#include <memory>
#include <vector>

#include "Tank.h"

USERCOMMON_BEGIN

Tank::Tank(Position pos, Direction dir, int playerId, int id)
    : MovingGameObject(pos, dir), playerId_(playerId), id_(id) {}


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

// moveBack() returns true if the action succeeds in changing state (e.g., entering to a backward wait state)
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
    if ( (isRightAfterMoveBack_) || ( (isWaitingToMoveBack_ == true) && (waitToMoveBackCounter_ == 0) ))
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
    if (playerId_ < 1 || playerId_ > 2)
    {
        return '?';
    }
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

USERCOMMON_END