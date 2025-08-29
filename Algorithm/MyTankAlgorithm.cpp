#include "MyTankAlgorithm.h"
#include "../common/TankAlgorithmRegistration.h"

namespace Algorithm_207177197_301251571 {

REGISTER_TANK_ALGORITHM(MyTankAlgorithm);

MyTankAlgorithm::MyTankAlgorithm(int playerIndex, int tankIndex)
         : playerIndex_(playerIndex),
           tankIndex_(tankIndex) {}

MyTankAlgorithm::MyTankAlgorithm(std::unique_ptr<TankAlgorithm> actualAlgo,
                                 int playerIndex, int tankIndex)
        : actualAlgo_(std::move(actualAlgo)),
          playerIndex_(playerIndex),
          tankIndex_(tankIndex) {}

ActionRequest MyTankAlgorithm::getAction() {
    return actualAlgo_->getAction();
}

void MyTankAlgorithm::updateBattleInfo(BattleInfo& info) {
    actualAlgo_->updateBattleInfo(info);
}


}
