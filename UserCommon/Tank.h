#pragma once

#include <memory>

#include "GameObject.h"
#include "MovingGameObject.h"
#include "Position.h"
#include "Direction.h"
#include "../common/TankAlgorithm.h"
#include "../common/ActionRequest.h"
#include "UserCommonNamespace.h"

USERCOMMON_BEGIN

class Tank : public MovingGameObject {
public:
    Tank(Position pos, Direction dir, int playerId, int Id);

    static constexpr int SHELLS_NUMBER = 16;
    static constexpr int AFTER_SHOOT_WAIT_TURNS = 4;
    static constexpr int MOVE_BACK_WAIT_TURNS = 2;



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
    TankAlgorithm* getAlgorithm() const;
    bool getIstRequestedBattleInfo() const; //not needed?

    // Setters
    //void setShellsLeft();
    //void setWaitAfterShootCounter();
    //void setWaitToMoveBackCounter();
    void setAlgorithm(std::unique_ptr<TankAlgorithm> algo);
    void setNextAction(ActionRequest action);
    void setLastAction(ActionRequest action);


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

    //for creating the output file
    bool getWasKilledThisStep() const {return wasKilledThisStep_;}
    bool getWasLastActionIgnored() const {return wasLastActionIgnored;}
    void setWasKilledThisStep(bool val) {wasKilledThisStep_ = val;}
    void setWasLastActionIgnored(bool val) {wasLastActionIgnored = val;}


private:
    int playerId_ = 0;
    int id_ = 0;

    std::unique_ptr<TankAlgorithm> algorithm_;

    bool requestedBattleInfo_ = false; //not sure if neede

    //shells and shoot state
    int shellsLeft_ = SHELLS_NUMBER;
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

USERCOMMON_END







