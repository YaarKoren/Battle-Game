#pragma once

#include "Position.h"

namespace UserCommon_207177197_301251571
{

enum class ObjectKind { Wall, Mine, Tank, Shell };

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

    virtual ObjectKind kind() const noexcept = 0;

    virtual void decreaseLifeLeft() {} //inline no-op; only for Wall use; other objects inherits default no-op
    virtual void setWasKilledThisStep([[maybe_unused]] bool val) {} //inline no-op; only for Tank use; other objects inherits default no-op


protected:
    Position pos_;
    bool destroyed_ = false;
};

}