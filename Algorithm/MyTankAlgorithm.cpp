#include "MyTankAlgorithm.h"
#include "../common/TankAlgorithmRegistration.h"

namespace Algorithm_207177197_301251571 {

REGISTER_TANK_ALGORITHM(MyTankAlgorithm);

// why this is not working? cuz we already have a constructor, in the registration process?
// but in assignment 2 we also had a factory, and it did work...
//MyTankAlgorithm::MyTankAlgorithm( int playerIndex, int tankIndex)
//        : playerIndex_(playerIndex),
//        tankIndex_(tankIndex) {}


//implement

}
