#pragma once

#include "GameObject.h"
#include "Position.h"
#include "Direction.h"


namespace UserCommon_207177197_301251571
{
    class MovingGameObject : public GameObject {
    public:
        MovingGameObject(Position pos, Direction dir)
            : GameObject(pos), dir_(dir) {}
        Direction getDirection() const { return dir_; }
        void setDirection(Direction dir) { dir_ = dir;  }
        virtual bool moveForward () = 0;

    protected:
        Direction dir_;
    };

}