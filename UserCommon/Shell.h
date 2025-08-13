#pragma once

#include "MovingGameObject.h"
#include "Position.h"
#include "Direction.h"

class Shell : public MovingGameObject {
public:
    Shell(Position pos, Direction dir, int tankId);

    char getSymbol() const override { return '*'; }
    int getTankId() const { return tankId_; }

    bool moveForward() override;

private:
    int tankId_ = 0;
};

