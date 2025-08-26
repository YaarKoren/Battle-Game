#include "MyGameManager.h"
#include <iostream>
#include <fstream>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <memory>
#include <cstdlib>
#include <exception>
#include <optional>

using namespace UserCommon_207177197_301251571;

namespace GameManager_207177197_301251571 {

REGISTER_GAME_MANAGER(MyGameManager);



MyGameManager::MyGameManager(bool verbose) :  verbose_(verbose),
    board_(1, 1) // meaningless initiazlization just to make it work
    {}

GameResult MyGameManager::run(
        size_t map_width, size_t map_height,
        const SatelliteView& map, // <= a snapshot, NOT updated
        string map_name,
        size_t max_steps, size_t num_shells,
        Player& player1, string name1, Player& player2, string name2,
        TankAlgorithmFactory player1_tank_algo_factory,
        TankAlgorithmFactory player2_tank_algo_factory) {

   map_width_ = map_width;
   map_height_ = map_height;
   num_shells_ = num_shells;
   max_steps_ = max_steps;
   map_name_ = map_name;
   if (setalliteViewToBoardAndVectores(map)) { //sets board_, p1Tanks_, p2Tanks_, walls_, mines_
     //error
   }

   player1_ = &player1;
   player2_ = &player2;
   player1_name_ = std::move(name1);
   player2_name_ = std::move(name2);

   //set tank algorithm for each tank
   for (auto& t: p1Tanks_) {
        t->setAlgorithm( player1_tank_algo_factory(t->getPlayerId(), t->getId()) );
   }

    for (auto& t: p2Tanks_) {
        t->setAlgorithm( player2_tank_algo_factory(t->getPlayerId(), t->getId()) );
   }

   allTanksSorted_ = sortAllTanks(p1Tanks_, p2Tanks_); //for output file //TODO maybe inside run()

   run(); //print results if verobose_ == true
   //TODO try catch/ check res

   ::std::unique_ptr<SatelliteView> final_s_view = std::make_unique<SatelliteViewImpl>();
   //TODO understand where this created
   boardAndVectoresToSatelliteView();


  GameResult result{};
   //TODO BUILD IT

   return result;

}

int MyGameManager::setalliteViewToBoardAndVectores(const SatelliteView& satelliteView){

	// ensure sizes match your stored dims
    // assert(map_width_  == satelliteView.width()); //no such method in the interface
    // assert(map_height_ == satelliteView.height()); //no such method in the interface

    // clear previous state
    board_.clear();
    walls_.clear(); mines_.clear(); p1Tanks_.clear(); p2Tanks_.clear();

    //go thru all entries, get the char
	// Birth order: TOP->BOTTOM (y outer), LEFT->RIGHT (x inner)
    int tankIdCounter_1 = 0;
    int tankIdCounter_2 = 0;

     for (size_t y = 0; y < map_height_; y++) {
       for (size_t x = 0; x < map_width_; x++) {
         Position pos(x, y);
        char ch = satelliteView.getObjectAt(x, y);

         switch (ch) {
            case '#': { // wall
                auto obj = std::make_unique<Wall>(pos);
                board_.addGameObject(obj.get(), pos);       // board is non-owning
                walls_.push_back(std::move(obj));           // GM owns
                break;
            }
            case '@': { // mine
                auto obj = std::make_unique<Mine>(pos);
                board_.addGameObject(obj.get(), pos);
                mines_.push_back(std::move(obj));
                break;
            }
            case '1': { // player 1 tank
                auto obj = std::make_unique<Tank>(pos, Direction::Right, 1, ++ tankIdCounter_1);
                board_.addGameObject(obj.get(), pos);
                p1Tanks_.push_back(std::move(obj));
                break;
            }
            case '2': { // player 2 tank
                auto obj = std::make_unique<Tank>(pos, Direction::Left, 2, ++ tankIdCounter_2);
                board_.addGameObject(obj.get(), pos);
                p2Tanks_.push_back(std::move(obj));
                break;
            }
            default: //any other char, includin empty space and invalid chars - A decision we made about handling invalid chars
                // empty cell; do nothing
                break;
            }
        }
    }
    return 0;
}



int boardAndVectoresToSatelliteView() {
    //TODO
    return 0;
}




int MyGameManager::run(const ::std::string& inputFile) {

    //TODO: checks for edge cases: empty board, no tanks etc

    //create and name output file / screen in case of fail to open
    const std::string outputFileName = "output_" + inputFile;
    std::ofstream file("../" + outputFileName);          // owns the file (if opened)

    if (!file) {
        std::cerr << "Failed to open output file: " << outputFileName << "\n"
                  << "Printing results to the screen instead.\n";
    }

    // one variable to pass to all printers:
    std::ostream& output_path = file ? static_cast<std::ostream&>(file)
                             : static_cast<std::ostream&>(std::cout);


    int stepCounter = 0;
    stepsLeftWhenShellsOver_ = STEPS_WHEN_SHELLS_OVER;
    int p1Alive = 0;
    int p2Alive = 0;

    //printToFile("=== Tank Game Start ==="); //this is for convenience, not for submitting

    while (stepCounter < maxSteps_ && stepsLeftWhenShellsOver_ > 0) {
        //printToFile("\n--- Step " + std::to_string(stepCounter) + " ---", output_path); //this is for convenience, not for submitting

        //cheking is first, to cover case of no tanks for one player or both, in the input setallite view
        if (checkIfPlayerLostAllTanks(p1Alive, p2Alive)) {break;}  //this returns true if a player, or both, lost all of his tanks.
                                                                        //it also counts the alive tanks of each player, and keep it in p1Alive, p2Alive
        //reset "setWasKilledThisStep" for all tanks
        for (auto& t : p1Tanks_) {
            t->setWasKilledThisStep(false);
            t->setWasLastActionIgnored(false);
        }
        for (auto& t : p2Tanks_) {
            t->setWasKilledThisStep(false);
            t->setWasLastActionIgnored(false);
        }

        //***counters handling***
        for (auto& t : p1Tanks_) if (!t->isDestroyed()) countersHandler(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed()) countersHandler(*t);

        //***deciding tanks actions***
        for (auto& t : p1Tanks_) {
            if (!t->isDestroyed()) {
                ActionRequest action = decideAction(*t, *t->getAlgorithm());
                t->setNextAction(action);
            }
        }
        for (auto& t : p2Tanks_) {
            if (!t->isDestroyed()) {
                ActionRequest action = decideAction(*t, *t->getAlgorithm());
                t->setNextAction(action);
            }
        }

        //***handle get battle info***
        //it's first cuz the view is of the board before the current step
        for (auto& t : p1Tanks_) if (!t->isDestroyed() && t->getNextAction() == ActionRequest::GetBattleInfo) handleRequestBattleInfo(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed() && t->getNextAction() == ActionRequest::GetBattleInfo) handleRequestBattleInfo(*t);

        //***handle shooting***
        //it's before other actions (except get battle info) cuz this is the choice we made in the game logic:
        //first, we move the shells. then, we move the tanks.
        for (auto& t : p1Tanks_) if (!t->isDestroyed() && t->getNextAction() == ActionRequest::Shoot) handleShoot(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed() && t->getNextAction() == ActionRequest::Shoot) handleShoot(*t);

        for (int i = 0; i < shellMovesPerStep_; ++i) {
            shellStep(); //collisions are handled inside this function
            cleanupDestroyedObjects(shells_);
            cleanupDestroyedObjects(walls_);
            //cleanupDestroyedObjects(p1Tanks_); //not cleaning, it's needed for creating output file
            //cleanupDestroyedObjects(p2Tanks_); //not cleaning, it's needed for creating output file
            //no need to clean mines at this point. shells do not hit mines.
        }

        //***handle other actions (not get battle info / shoot)***
        for (auto& t : p1Tanks_) if (!t->isDestroyed() && t->getNextAction() != ActionRequest::Shoot && t->getNextAction() != ActionRequest::GetBattleInfo) handleAction(*t, t->getNextAction());
        for (auto& t : p2Tanks_) if (!t->isDestroyed() && t->getNextAction() != ActionRequest::Shoot && t->getNextAction() != ActionRequest::GetBattleInfo) handleAction(*t, t->getNextAction());

        //***check if we need to move the tank back and move it, if yes***
        for (auto& t : p1Tanks_) if (!t->isDestroyed()) handleAutoMoveTankBack(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed()) handleAutoMoveTankBack(*t);

        //***resolve collisions***
        for (auto& t : p1Tanks_) if (!t->isDestroyed()) resolveTankCollisionsAtPosition(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed()) resolveTankCollisionsAtPosition(*t);

        cleanupDestroyedObjects(mines_);
        //cleanupDestroyedObjects(p1Tanks_); //not cleaning, it's needed for creating output file
        //cleanupDestroyedObjects(p2Tanks_); //not cleaning, it's needed for creating output file
        //no need to clean shells at this point. shells has been handled before.
        //no need to clean walls at this point, cuz tanks can not hit walls.

        if (getTotalShellsLeft() <= 0) {
            --stepsLeftWhenShellsOver_;
        }

        stepCounter++;
         printRoundToFile(output_path);
    }

    printGameResult(p1Alive, p2Alive, output_path);
}

ActionRequest MyGameManager::decideAction(Tank& tank, TankAlgorithm& algo){

    ::std::vector<Tank*> aliveTanks;
    for (const auto& t : p1Tanks_) {
        if (!t->isDestroyed()) {
            aliveTanks.push_back(t.get());
        }
    }
    for (const auto& t : p2Tanks_) {
        if (!t->isDestroyed()) {
            aliveTanks.push_back(t.get());
        }
    }
    //it's ok that it's all tank, the algorithm can tell between its own tank and the opponent's
    return tank.decideNextAction(aliveTanks);

}

int GameManager::getTotalShellsLeft() const {
    int total = 0;
    for (const auto& t : p1Tanks_) total += t->getShellsLeft();
    for (const auto& t : p2Tanks_) total += t->getShellsLeft();
    return total;
}

void GameManager::handleRequestBattleInfo(Tank& tank) {
    //pre-action reset and check
    tank.resetIsRightAfterMoveBack();
    if (tank.getIsWaitingToMoveBack()) {
        tank.setWasLastActionIgnored(true);
        return;
    }

    std::vector<std::vector<char>> view(boardHeight_, std::vector<char>(boardWidth_, ' '));
    //create a 2D char representation of the board
    for (size_t y = 0; y < boardHeight_; ++y) {
        for (size_t x = 0; x < boardWidth_; ++x) {
            const auto& objects = board_.getObjectsAt({(int)x, (int)y});
            if (!objects.empty()) {
                view[y][x] = objects.front()->getSymbol();
            }
        }
    }
    //create a SatelliteViewImpl from it
    SatelliteViewImpl satellite(view);

    //determine which player owns this tank
    int playerId = tank.getPlayerId();
    Player* player = (playerId == 1) ? player1_.get() : player2_.get();

    //let the player update the tank's algorithm with BattleInfo
    if (player) {
        if (!tank.getAlgorithm()) {
            throw ::std::runtime_error (::std::string("Tank has no algorithm! Player: ") + tank.getPlayerId()
                      +  ::std::string(", Tank ID: ") + tank.getId())

        }
        player->updateTankWithBattleInfo(*tank.getAlgorithm(), satellite);
    }

    tank.setLastAction(ActionRequest::GetBattleInfo);
    tank.setWasLastActionIgnored(false);
}

void GameManager::handleShoot(Tank& tank) {
    tank.resetIsRightAfterMoveBack();

    if (tank.getIsWaitingAfterShoot()) {
        tank.setWasLastActionIgnored(true);
        return;
    }

    if (!tank.shoot()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    shells_.push_back(std::make_unique<Shell>(tank.getPosition(), tank.getDirection(), tank.getId()));
    tank.setLastAction(ActionRequest::Shoot);
    tank.setWasLastActionIgnored(false);
}

//not including get battle info + shoot
void GameManager::handleAction(Tank& tank, ActionRequest action) {
    switch (action) {
        case ActionRequest::MoveForward: handleMoveTankForward(tank); break;
        case ActionRequest::MoveBackward: handleTankAskMoveBack(tank); break;
        case ActionRequest::RotateLeft45: handleRotateEighthLeft(tank); break;
        case ActionRequest::RotateLeft90: handleRotateFourthLeft(tank); break;
        case ActionRequest::RotateRight45: handleRotateEighthRight(tank); break;
        case ActionRequest::RotateRight90: handleRotateFourthRight(tank); break;
        case ActionRequest::DoNothing: handleDoNothing(tank); break;
        //default: std::cerr << "Error in handleAction: fall to default\n"; break; //should not happen!
    }
}

void MyGameManager::countersHandler(Tank& tank) {
    tank.updateWaitAfterShootCounter(); //decrease counter only if the tank is after shoot + counter>0
    tank.resetIsWaitingAfterShoot(); //change waiting_after_shoot to false, only if the tank is after shoot + counter==0

    tank.updateWaitToMoveBackCounter(); //decrease counter only if the tank is waiting to move back + counter>0
    tank.resetIsWaitingToMoveBack(); //change waiting_to_move_back to false, only if the tank was in waiting state + counter==0
}

void GameManager::handleAutoMoveTankBack(Tank& tank) {
    if (tank.getIsWaitingToMoveBack() && tank.getWaitToMoveBackCounter() == 0) {
        handleMoveTankBack(tank);
    }
}

void GameManager::moveForwardAndWrap(Shell& shell) {
    shell.moveForward();
    Position newPos = shell.getPosition();
    newPos.wrap(boardWidth_, boardHeight_);
    shell.setPosition(newPos);
}

void GameManager::shellStep() {
    for (auto& shell : shells_) {
        moveForwardAndWrap(*shell);
        board_.addGameObject(shell.get(), shell->getPosition());
    }

    for (auto& shell : shells_) {
        if (!shell->isDestroyed()) {
            resolveShellCollisionsAtPosition(*shell);
        }
    }
}

void GameManager::resolveShellCollisionsAtPosition(Shell& shell) {
    Position pos = shell.getPosition();
    const auto& objects = board_.getObjectsAt(pos);
    for (const auto& obj : objects) {
        if (auto* wall = dynamic_cast<Wall*>(obj)) {
            wall->decreaseLifeLeft();
            shell.destroy();
        } else if (auto* anotherShell = dynamic_cast<Shell*>(obj)) {
            anotherShell->destroy();
            shell.destroy();
        } else if (auto* tank = dynamic_cast<Tank*>(obj)) {
            tank->destroy();
            tank->setWasKilledThisStep(true);
            shell.destroy();
        }
    }
}

void GameManager::resolveTankCollisionsAtPosition(Tank& tank) {
    Position pos = tank.getPosition();
    const auto& objects = board_.getObjectsAt(pos);
    for (const auto& obj : objects) {
        if (auto* mine = dynamic_cast<Mine*>(obj)) {
            mine->destroy();
            tank.destroy();
            tank.setWasKilledThisStep(true);
        } else if (auto* anotherTank = dynamic_cast<Tank*>(obj)) {
            if (anotherTank != &tank) {
                anotherTank->destroy();
                anotherTank->setWasKilledThisStep(true);
                tank.destroy();
                tank.setWasKilledThisStep(true);
            }
        }
    }
}

std::vector<Shell*> GameManager::getShellPtrs() const {
    std::vector<Shell*> result;
    for (const auto& shell : shells_) {
        result.push_back(shell.get());
    }
    return result;
}

std::vector<Mine*> GameManager::getMinePtrs() const {
    std::vector<Mine*> result;
    for (const auto& mine : mines_) {
        result.push_back(mine.get());
    }
    return result;
}

void GameManager::handleMoveTankForward(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    Position oldPos = tank.getPosition();

    if (!tank.moveForward()) {
        tank.setWasLastActionIgnored(true);
        return;
    }

    Position newPos = tank.getPosition();
    newPos.wrap(boardWidth_, boardHeight_);

    const auto& objects = board_.getObjectsAt(newPos);
    for (const auto& obj : objects) {
        if (dynamic_cast<Wall*>(obj)) {
            tank.setPosition(oldPos);
            tank.setWasLastActionIgnored(true);
            return;
        }
    }

    tank.setPosition(newPos);
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::MoveForward);
}

void GameManager::handleMoveTankBack(Tank& tank) {
    Position oldPos = tank.getPosition();

    //in this case, we treat moving back as action request
    if (tank.getIsRightAfterMoveBack()) {
        if (!tank.moveBack()) {
            tank.setWasLastActionIgnored(true);
            return;
        }

        Position newPos = tank.getPosition();
        newPos.wrap(boardWidth_, boardHeight_);

        //check if there is wall in the new position.
        //if there is, set last action to ignored and do not move the tank, aka do not change the tank's position
        const auto& objects = board_.getObjectsAt(newPos);
        for (const auto& obj : objects) {
            if (dynamic_cast<Wall*>(obj)) {
                tank.setPosition(oldPos);
                tank.setWasLastActionIgnored(true);
                return;
            }
        }

        //if there is no wall
        tank.setWasLastActionIgnored(false);
        tank.setLastAction(ActionRequest::MoveBackward);
    }

    //in this case, we don't treat moving back as action request (it happens automatically after the waiting)
    //this is why we don't update last action and if it eas ignored
    if (tank.getIsWaitingToMoveBack() && tank.getWaitToMoveBackCounter() == 0) {
        if (!tank.moveBack()) {
            return;
        }
        Position newPos = tank.getPosition();
        newPos.wrap(boardWidth_, boardHeight_);

        //check if there is wall in the new position.
        //if there is, set last action to ignored and do not move the tank, aka do not change the tank's position
        const auto& objects = board_.getObjectsAt(newPos);
        for (const auto& obj : objects) {
            if (dynamic_cast<Wall*>(obj)) {
                tank.setPosition(oldPos);
                return;
            }
        }
    }
}

void GameManager::handleTankAskMoveBack(Tank& tank) {
    if (tank.getIsRightAfterMoveBack()) {
        handleMoveTankBack(tank); //updates last action + if it was ignored
        return;
    }
    if (!tank.askToMoveBack()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::MoveBackward);
}

void GameManager::handleRotateEighthLeft(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateEighthLeft()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateLeft45);
}

void GameManager::handleRotateFourthLeft(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateFourthLeft()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateLeft90);
}

void GameManager::handleRotateEighthRight(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateEighthRight()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateRight45);
}

void GameManager::handleRotateFourthRight(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateFourthRight()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateRight90);
}

void GameManager::handleDoNothing(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    tank.doNothing();
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::DoNothing);
}

void GameManager::printToFile(const std::string& message, std::ostream& output_path) {
    output_path << message << std::endl;
}



std::vector<Tank*> GameManager::sortAllTanks(const std::vector<std::unique_ptr<Tank>>& p1Tanks,
                                       const std::vector<std::unique_ptr<Tank>>& p2Tanks) {
    std::vector<Tank*> allTanks;

    for (const auto& t : p1Tanks)
       allTanks.push_back(t.get());

    for (const auto& t : p2Tanks)
        allTanks.push_back(t.get());

    //sort tanks by birth order (top to bottom (y axis), left to right (x axis))
    std::sort(allTanks.begin(), allTanks.end(), [](Tank* a, Tank* b) {
        const Position& pa = a->getPosition();
        const Position& pb = b->getPosition();
        return (pa.getY() != pb.getY()) ? pa.getY() < pb.getY() : pa.getX() < pb.getX();
    });

    return allTanks;
}

void GameManager::printRoundToFile(std::ostream& output_path) {
    std::vector<std::string> actionStrs;
    actionStrs.reserve(allTanksSorted_.size());
    bool allDoNothing = true;

    for (size_t i = 0; i < allTanksSorted_.size(); ++i) {
        Tank* tank = allTanksSorted_[i];
        std::string actionStr;

        ActionRequest next = tank->getNextAction();
        if (next == ActionRequest::GetBattleInfo) {
            actionStr = ActionUtils::toString(ActionRequest::DoNothing);
        } else {
            actionStr = ActionUtils::toString(next);
        }

        if (tank->getWasLastActionIgnored())
            actionStr += " (ignored)";
        if (tank->getWasKilledThisStep())
            actionStr += " (killed)";

        if (next != ActionRequest::DoNothing && next != ActionRequest::GetBattleInfo)
            allDoNothing = false;
        if (tank->getWasLastActionIgnored() || tank->getWasKilledThisStep())
            allDoNothing = false;

        actionStrs.push_back(std::move(actionStr));
    }

    if (allDoNothing) {
        return;
    }

    for (size_t i = 0; i < actionStrs.size(); ++i) {
        output_path << actionStrs[i];
        if (i + 1 < actionStrs.size()) output_path << " ";
    }
   output_path << "\n";
}

void GameManager::printGameResult(int p1Alive, int p2Alive, std::ostream& output_path) {
    if (p1Alive > 0 && p2Alive == 0) {
        output_path << "Player 1 won with " << p1Alive << " tanks still alive\n";
    } else if (p2Alive > 0 && p1Alive == 0) {
       output_path << "Player 2 won with " << p2Alive << " tanks still alive\n";
    } else if (p1Alive == 0 && p2Alive == 0) {
        output_path << "Tie, both players have zero tanks\n";
    } else if (stepsLeftWhenShellsOver_ >= STEPS_WHEN_SHELLS_OVER) {
        output_path << "Tie, both players have zero shells for <"
                    << STEPS_WHEN_SHELLS_OVER << "> steps\n";
    } else {
        output_path << "Tie, reached max steps = " << maxSteps_
                    << ", player 1 has " << p1Alive
                    << " tanks, player 2 has " << p2Alive << " tanks\n";
    }
}

    //returns true if a player, or both, lost all of his tanks.
    //else, returns false
    //also count and update the number of tanks for each player
bool GameManager::checkIfPlayerLostAllTanks(int& p1Alive, int& p2Alive) {
    p1Alive = std::count_if(p1Tanks_.begin(), p1Tanks_.end(),
                                  [](const std::unique_ptr<Tank>& t) { return !t->isDestroyed(); });

    p2Alive = std::count_if(p2Tanks_.begin(), p2Tanks_.end(),
                                  [](const std::unique_ptr<Tank>& t) { return !t->isDestroyed(); });

    if (p1Alive == 0 || p2Alive == 0) {
       return true;
    }

    return false;
}

/*not sure if needed
template void GameManager::cleanupDestroyedObjects<Shell>(std::vector<std::unique_ptr<Shell>>&);
template void GameManager::cleanupDestroyedObjects<Wall>(std::vector<std::unique_ptr<Wall>>&);
template void GameManager::cleanupDestroyedObjects<Tank>(std::vector<std::unique_ptr<Tank>>&);
template void GameManager::cleanupDestroyedObjects<Mine>(std::vector<std::unique_ptr<Mine>>&);
 */


}