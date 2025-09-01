#pragma once

#include <memory>
#include <vector>
#include  <climits>  // for INT_MAX

#include "../common/TankAlgorithm.h"
#include "../common/ActionRequest.h"

#include "MovingGameObject.h"
#include "Position.h"
#include "Direction.h"



namespace UserCommon_207177197_301251571

{
    class Board;   // forward declare
    class Tank : public MovingGameObject {
    public:
        Tank(Position pos, Direction dir, int playerId, int Id, int numShells);

        static constexpr int AFTER_SHOOT_WAIT_TURNS = 4;
        static constexpr int MOVE_BACK_WAIT_TURNS = 2;

        ObjectKind kind() const noexcept override { return ObjectKind::Tank; }

        // Getters
        char getSymbol() const override; //each Tank Object will have a different symbol - "1" or "2", by the player it belongs to
        int getPlayerId() const;
        int getId() const;
        int getShellsLeft() const;
        bool getIsWaitingToMoveBack() const; //the game manager needs this info, cuz it manages the turns. and the turns loop is what changing it.
        bool getIsWaitingAfterShoot() const; //dido
        bool getIsRightAfterMoveBack() const;
        int getWaitToMoveBackCounter() const;
        ActionRequest getNextAction() const;
        ActionRequest getLastAction() const;
        ActionRequest decideNextAction(const std::vector<Tank*>& allTanks); // Keep only this one
        TankAlgorithm* getAlgorithm() const;
        bool getIstRequestedBattleInfo() const; //not needed?

        // Setters
        //void setShellsLeft();
        //void setWaitAfterShootCounter();
        //void setWaitToMoveBackCounter();
        void setAlgorithm(std::unique_ptr<TankAlgorithm> algo);
        void setNextAction(ActionRequest action);
        void setLastAction(ActionRequest action);

        void setBoard(Board* board);

        void setShellsLeft(const int numShells) { shellsLeft_ = numShells; };

        // Actions
        bool moveForward() override;
        bool askToMoveBack();
        bool moveBack();
        bool shoot();
        bool rotateEighthLeft();
        bool rotateFourthLeft();
        bool rotateEighthRight();
        bool rotateFourthRight();
        void doNothing();

        // Actions utility functions
        void updateWaitToMoveBackCounter();
        void updateWaitAfterShootCounter();
        void resetIsWaitingAfterShoot();
        void resetIsWaitingToMoveBack();
        void resetIsRightAfterMoveBack();

        // Decision making functions - CHANGE THESE TO USE POINTERS
        bool canSeeEnemy(const std::vector<Tank*>& allTanks) const;
        bool canMoveForward() const;
        Direction getDirectionTowardEnemy(const std::vector<Tank*>& allTanks) const;

        // Remove the duplicate declaration
        // ActionRequest decideNextAction(const std::vector<Tank>& allTanks);


        //for creating the output file
        bool getWasKilledThisStep() const {return wasKilledThisStep_;}
        bool getWasLastActionIgnored() const {return wasLastActionIgnored;}
        void setWasKilledThisStep(bool val) override {wasKilledThisStep_ = val;}
        void setWasLastActionIgnored(bool val) {wasLastActionIgnored = val;}


    private:
        int playerId_ = 0;
        int id_ = 0;

        Board* board_ = nullptr;

        std::unique_ptr<TankAlgorithm> algorithm_;

        bool requestedBattleInfo_ = false; //not sure if needed

        //shells and shoot state
        int shellsLeft_;
        bool isWaitingAfterShoot_ = false; // cool down after shoot
        int waitAfterShootCounter_ = 0;

        //moving-back state
        bool isWaitingToMoveBack_ = false;
        int waitToMoveBackCounter_ = 0;
        bool isRightAfterMoveBack_ = false;

        ActionRequest nextAction_;
        ActionRequest lastAction_;

        // private methods - actions utility functions
        void actualRotateEighthLeft();
        void actualRotateEighthRight();

        //for creating the output file
        bool wasKilledThisStep_ = false;
        bool wasLastActionIgnored = false;

    };
}







