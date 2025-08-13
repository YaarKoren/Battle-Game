#pragma once

#include "GameObject.h"

class Mine : public GameObject {
public:
    explicit Mine(Position pos) : GameObject(pos) {}
    char getSymbol() const override { return '@'; }
};
