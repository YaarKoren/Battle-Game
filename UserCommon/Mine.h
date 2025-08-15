#pragma once

#include "GameObject.h"
#include "UserCommonNamespace.h"

USERCOMMON_BEGIN

class Mine : public GameObject {
public:
    explicit Mine(Position pos) : GameObject(pos) {}
    char getSymbol() const override { return '@'; }
};

USERCOMMON_END