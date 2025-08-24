#pragma once

#include "GameObject.h"

namespace UserCommon_207177197_301251571
{
    class Mine : public GameObject {
    public:
        explicit Mine(Position pos) : GameObject(pos) {}
        char getSymbol() const override { return '@'; }
    };

}