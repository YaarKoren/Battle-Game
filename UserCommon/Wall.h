#pragma once

#include "GameObject.h"

namespace UserCommon_207177197_301251571
{
    class Wall : public GameObject {
    public:
        explicit Wall(Position pos) : GameObject(pos) {}
        static constexpr int TIMES_TO_HIT_BEFORE_GONE = 2;

        char getSymbol() const override { return '#'; }

        int getLifeLeft() const { return lifeLeft_; }
        void decreaseLifeLeft() override { lifeLeft_--; }

        bool isDestroyed() const override { return lifeLeft_ <= 0; }

        ObjectKind kind() const noexcept override { return ObjectKind::Wall; }

    private:
        int lifeLeft_ = TIMES_TO_HIT_BEFORE_GONE;
    };
}
