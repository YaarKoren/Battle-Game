#pragma once

#include "Position.h"

namespace UserCommon_207177197_301251571
{
    class GameObject {
    public:
        explicit GameObject(Position pos) //"explicit", to avoid implicit conversions
            : pos_(pos) {}

        virtual char getSymbol() const = 0;
        Position getPosition() const { return pos_; }
        void setPosition(const Position& pos) { pos_ = pos; }
        virtual void destroy() { destroyed_ = true; }
        virtual bool isDestroyed() const { return destroyed_; }
        virtual ~GameObject() = default;

    protected:
        Position pos_;
        bool destroyed_ = false;
    };
}