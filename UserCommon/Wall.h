#pragma once

#include "GameObject.h"
#include "UserCommonNamespace.h"

USERCOMMON_BEGIN

class Wall : public GameObject {
public:
    explicit Wall(Position pos) : GameObject(pos) {}
    static constexpr int TIMES_TO_HIT_BEFORE_GONE = 2;

    char getSymbol() const override { return '#'; }

    int getLifeLeft() const { return lifeLeft_; }
    void decreaseLifeLeft() { lifeLeft_--; }

    bool isDestroyed() const override { return lifeLeft_ <= 0; }


private:
    int lifeLeft_ = TIMES_TO_HIT_BEFORE_GONE;
};

USERCOMMON_END
