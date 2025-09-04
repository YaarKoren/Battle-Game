#include <iostream>
#include <fstream>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <memory>
#include <cstdlib>
#include <exception>
#include <optional>
#include <fstream>
#include <vector>
#include  <climits>


#include "../UserCommon/SatelliteViewImpl.h"
#include "MyGameManager.h"

namespace GameManager_207177197_301251571 {

using namespace UserCommon_207177197_301251571;

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

   satelliteViewToBoardAndVectores(map);  //sets board_, p1Tanks_, p2Tanks_, walls_, mines_
                                           // throws an error in case of out of bound access - that means there is a mismathc between the Satellite view and the dims
   									       //Simulator (aka caller) should catch it

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

   allTanksSorted_ = sortAllTanks(p1Tanks_, p2Tanks_); //for output file

   run();   //upadates final_result_ when Game is done
   			//prints results if verobose_ == true
   			//throws error
   			//Simulator (aka caller) should catch it

  GameResult result = std::move(final_result_);

   return result;

}

void MyGameManager::satelliteViewToBoardAndVectores(const SatelliteView& satelliteView){

	// ensure sizes match your stored dims
    // assert(map_width_  == satelliteView.width()); //no such method in the interface
    // assert(map_height_ == satelliteView.height()); //no such method in the interface

    // clear previous state, for saftey
    board_.clear();
    board_.resize(map_width_, map_height_);
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
                auto obj = std::make_unique<Tank>(pos, Direction::Right, 1, ++ tankIdCounter_1, num_shells_);
                board_.addGameObject(obj.get(), pos);
                p1Tanks_.push_back(std::move(obj));
                break;
            }
            case '2': { // player 2 tank
                auto obj = std::make_unique<Tank>(pos, Direction::Left, 2, ++ tankIdCounter_2, num_shells_);
                board_.addGameObject(obj.get(), pos);
                p2Tanks_.push_back(std::move(obj));
                break;
            }
            case '&': { //out of bound
             	std::ostringstream oss;
    			oss << "[Game Manager] Error in reading Satellite View: out of bounds at ("
      		 		 << x << ", " << y << ")";
    			throw std::runtime_error(oss.str());
            }

            default: //any other char, including empty spaces and invalid chars - A decision we made about handling invalid chars
                // empty cell; do nothing
                break;
            }
        }
    }
}


void MyGameManager::run() {

	//------------------------------------------------------------------------------------------------
	//create and name output file / screen in case of fail to open
    // Fourm specs: create the file in the working directory
    const std::string output_file_name = makeUniquePath();
   	std::ofstream file(output_file_name);          // owns the file (if opened)
   	if (!file) {
              if (verbose_) {
                std::cerr << "[Game Manager] Failed to open output file: " << output_file_name << "\n"
       	       		<< "Printing results to the screen instead.\n";
              }

    }

   	// one variable to pass to all printers:
   	std::ostream& output_path = file ? static_cast<std::ostream&>(file)
                      : static_cast<std::ostream&>(std::cout);

	//------------------------------------------------------------------------------------------------

	size_t num_of_alive_p1_tanks = 0;
    size_t num_of_alive_p2_tanks = 0;
    //std::vector<std::vector<char>> char_grid;        // default-constructed, empty
    std::vector<std::vector<char>> char_grid(map_height_, std::vector<char>(map_width_, ' '));

	//check for edge cases
   	if (map_width_ == 0 || map_height_ == 0) {
          //technically there are no tanks for each player, so it's a tie
          final_result_.winner = 0 ;//tie
          final_result_.reason = GameResult::Reason::ALL_TANKS_DEAD;
          final_result_.remaining_tanks = {num_of_alive_p1_tanks, num_of_alive_p1_tanks};
          final_result_.gameState = std::make_unique<SatelliteViewImpl>(char_grid); //empty
          final_result_.rounds = 0;

          //print to file if verbose = true
          if(verbose_) {printTieZeroTanks(output_path);}

          return;
   	}
    if (max_steps_ == 0) { //as long as both players still have tanks, we treat this as tie
 		  final_result_.winner = 0;
          final_result_.reason = GameResult::Reason::MAX_STEPS;
          checkIfPlayerLostAllTanks(num_of_alive_p1_tanks, num_of_alive_p2_tanks); //updates values ; we know both playres still have tanks, cuz otherwise we would not get here
          final_result_.remaining_tanks = {num_of_alive_p1_tanks, num_of_alive_p1_tanks};
          board_.boardToCharGrid(char_grid); //updates char_grid
          final_result_.gameState = std::make_unique<SatelliteViewImpl>(char_grid);
          final_result_.rounds = 0;

          //print to file if verbose = true
          if(verbose_) {printTieReacehedMaxSteps(num_of_alive_p1_tanks, num_of_alive_p2_tanks, output_path);}

          return;
    }

    //----------------------------------------------------------------------------------------------------

    if (num_shells_ == 0) max_steps_ = STEPS_WHEN_SHELLS_OVER; //cuz we check if shlles are over only insdie the loop, after the first round

    size_t stepCounter = 0;

    //printToFile("=== Tank Game Start ==="); //this is for convenience, not for submitting

    while (stepCounter < max_steps_ && stepsLeftWhenShellsOver_ > 0) {
        //printToFile("\n--- Step " + std::to_string(stepCounter) + " ---", output_path); //this is for convenience, not for submitting

        //cheking is first, to cover case of no tanks for one player or both, in the input setallite view
        if (checkIfPlayerLostAllTanks(num_of_alive_p1_tanks, num_of_alive_p2_tanks)) {break;}  //this returns true if a player, or both, lost all of his tanks.
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
                ActionRequest action = decideAction(*t);
                t->setNextAction(action);
            }
        }
        for (auto& t : p2Tanks_) {
            if (!t->isDestroyed()) {
                ActionRequest action = decideAction(*t);
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

        for (size_t i = 0; i < shellMovesPerStep_; ++i) {
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

        if (verbose_) printRoundToFile(output_path);
    }

    //--------------------------------------------------------------------------------------------------------------

    //set final result + print if verbose_
    setGameResultBesideSatellite(num_of_alive_p1_tanks, num_of_alive_p2_tanks, stepCounter);
    //set Satellite View for result
    board_.boardToCharGrid(char_grid); //does not change board_ ; changes only the char grid
    final_result_.gameState = std::make_unique<SatelliteViewImpl>(char_grid);

    if (verbose_) printGameResult(num_of_alive_p1_tanks, num_of_alive_p2_tanks, output_path);
}

//-----------------------------------------------------------------------------------------------------


ActionRequest MyGameManager::decideAction(Tank& tank){

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
    //it's ok that it's all tanks, the algorithm can tell between its own tank and the opponent's
    return tank.decideNextAction(aliveTanks);

}

int MyGameManager::getTotalShellsLeft() const {
    int total = 0;
    for (const auto& t : p1Tanks_) total += t->getShellsLeft();
    for (const auto& t : p2Tanks_) total += t->getShellsLeft();
    return total;
}

void MyGameManager::handleRequestBattleInfo(Tank& tank) {
    //pre-action reset and check
    tank.resetIsRightAfterMoveBack();
    if (tank.getIsWaitingToMoveBack()) {
        tank.setWasLastActionIgnored(true);
        return;
    }

    std::vector<std::vector<char>> char_grid(map_height_, std::vector<char>(map_width_, ' '));
    //create a 2D char representation of the board
    board_.boardToCharGrid(char_grid); //does not change board_ ; changes only the char grid
    //create a SatelliteViewImpl from it
    auto satellite = std::make_unique<SatelliteViewImpl>(char_grid);

    //determine which player owns this tank
    int playerId = tank.getPlayerId();
    Player* player = (playerId == 1) ? player1_ : player2_;

    //let the player update the tank's algorithm with BattleInfo
    if (player) {
        if (!tank.getAlgorithm()) {
           throw std::runtime_error (
                  std::string("[Game Manager] Error in handle_Request_Battle_Info: Tank has no algorithm! Player: ")
                  + std::to_string(tank.getPlayerId())
                  + std::string(", Tank ID: ")
                  + std::to_string(tank.getId())
           );
        }
       	player->updateTankWithBattleInfo(*tank.getAlgorithm(), *satellite);
    }

    tank.setLastAction(ActionRequest::GetBattleInfo);
    tank.setWasLastActionIgnored(false);
}

void MyGameManager::handleShoot(Tank& tank) {
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
void MyGameManager::handleAction(Tank& tank, ActionRequest action) {
    switch (action) {
        case ActionRequest::MoveForward: handleMoveTankForward(tank); break;
        case ActionRequest::MoveBackward: handleTankAskMoveBack(tank); break;
        case ActionRequest::RotateLeft45: handleRotateEighthLeft(tank); break;
        case ActionRequest::RotateLeft90: handleRotateFourthLeft(tank); break;
        case ActionRequest::RotateRight45: handleRotateEighthRight(tank); break;
        case ActionRequest::RotateRight90: handleRotateFourthRight(tank); break;
        case ActionRequest::DoNothing: handleDoNothing(tank); break;
        default: throw std::runtime_error(std::string("[Game Manager] Error in handleAction: fall to default\n")); break; //should not happen!
    }
}

void MyGameManager::countersHandler(Tank& tank) {
    tank.updateWaitAfterShootCounter(); //decrease counter only if the tank is after shoot + counter>0
    tank.resetIsWaitingAfterShoot(); //change waiting_after_shoot to false, only if the tank is after shoot + counter==0

    tank.updateWaitToMoveBackCounter(); //decrease counter only if the tank is waiting to move back + counter>0
    tank.resetIsWaitingToMoveBack(); //change waiting_to_move_back to false, only if the tank was in waiting state + counter==0
}

void MyGameManager::handleAutoMoveTankBack(Tank& tank) {
    if (tank.getIsWaitingToMoveBack() && tank.getWaitToMoveBackCounter() == 0) {
        handleMoveTankBack(tank);
    }
}


void MyGameManager::moveForwardAndWrap(Shell& shell) {
    shell.moveForward();
    Position newPos = shell.getPosition();
    newPos.wrap(map_width_, map_height_);
    shell.setPosition(newPos);
}

void MyGameManager::shellStep() {
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

void MyGameManager::resolveShellCollisionsAtPosition(Shell& shell) {
    Position pos = shell.getPosition();
    const auto& objects = board_.getObjectsAt(pos);
    for (const auto& obj : objects) {
        if (obj->kind() == ObjectKind::Wall) {
            obj->decreaseLifeLeft();
            shell.destroy();
        } else if (obj->kind() == ObjectKind::Shell) {
            obj->destroy();
            shell.destroy();
        } else if (obj->kind() == ObjectKind::Tank) {
            obj->destroy();
            obj->setWasKilledThisStep(true);
            shell.destroy();
        }
    }
}

void MyGameManager::resolveTankCollisionsAtPosition(Tank& tank) {
    Position pos = tank.getPosition();
    const auto& objects = board_.getObjectsAt(pos);
    for (const auto& obj : objects) {
        if (obj->kind() == ObjectKind::Mine) {
            obj->destroy();
            tank.destroy();
            tank.setWasKilledThisStep(true);
        } else if (obj->kind() == ObjectKind::Tank) {
            if (obj != &tank) {
                obj->destroy();
                obj->setWasKilledThisStep(true);
                tank.destroy();
                tank.setWasKilledThisStep(true);
            }
        }
    }
}

std::vector<Shell*> MyGameManager::getShellPtrs() const {
    std::vector<Shell*> result;
    for (const auto& shell : shells_) {
        result.push_back(shell.get());
    }
    return result;
}

std::vector<Mine*> MyGameManager::getMinePtrs() const {
    std::vector<Mine*> result;
    for (const auto& mine : mines_) {
        result.push_back(mine.get());
    }
    return result;
}

void MyGameManager::handleMoveTankForward(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    Position oldPos = tank.getPosition();

    if (!tank.moveForward()) {
        tank.setWasLastActionIgnored(true);
        return;
    }

    Position newPos = tank.getPosition();
    newPos.wrap(map_width_, map_height_);

    const auto& objects = board_.getObjectsAt(newPos);
    for (const auto& obj : objects) {
        if (obj->kind() == ObjectKind::Wall) {
            tank.setPosition(oldPos);
            tank.setWasLastActionIgnored(true);
            return;
        }
    }

    tank.setPosition(newPos);
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::MoveForward);
}

void MyGameManager::handleMoveTankBack(Tank& tank) {
    Position oldPos = tank.getPosition();

    //in this case, we treat moving back as action request
    if (tank.getIsRightAfterMoveBack()) {
        if (!tank.moveBack()) {
            tank.setWasLastActionIgnored(true);
            return;
        }

        Position newPos = tank.getPosition();
        newPos.wrap(map_width_, map_height_);

        //check if there is wall in the new position.
        //if there is, set last action to ignored and do not move the tank, aka do not change the tank's position
        const auto& objects = board_.getObjectsAt(newPos);
        for (const auto& obj : objects) {
            if (obj->kind() == ObjectKind::Wall) {
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
        newPos.wrap(map_width_, map_height_);

        //check if there is wall in the new position.
        //if there is, set last action to ignored and do not move the tank, aka do not change the tank's position
        const auto& objects = board_.getObjectsAt(newPos);
        for (const auto& obj : objects) {
            if (obj->kind() == ObjectKind::Wall) {
                tank.setPosition(oldPos);
                return;
            }
        }
    }
}

void MyGameManager::handleTankAskMoveBack(Tank& tank) {
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

void MyGameManager::handleRotateEighthLeft(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateEighthLeft()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateLeft45);
}

void MyGameManager::handleRotateFourthLeft(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateFourthLeft()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateLeft90);
}

void MyGameManager::handleRotateEighthRight(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateEighthRight()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateRight45);
}

void MyGameManager::handleRotateFourthRight(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateFourthRight()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateRight90);
}

void MyGameManager::handleDoNothing(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    tank.doNothing();
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::DoNothing);
}

void MyGameManager::printToFile(const std::string& message, std::ostream& output_path) {
    output_path << message << std::endl;
}



std::vector<Tank*> MyGameManager::sortAllTanks(const std::vector<std::unique_ptr<Tank>>& p1Tanks,
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

void MyGameManager::printRoundToFile(std::ostream& output_path)
{
    for (size_t i = 0; i < allTanksSorted_.size(); ++i) {
        Tank* tank = allTanksSorted_[i];

        if (tank->isDestroyed()) {
            output_path << "killed";
        } else {
            std::string actionStr = actionRequestToString( (tank->getNextAction()) );

            if (tank->getWasLastActionIgnored())
                actionStr += " (ignored)";
            if (tank->getWasKilledThisStep())
                actionStr += " (killed)";

            output_path << actionStr;
        }

        if (i < allTanksSorted_.size() - 1) //to print ", " in between tanks and not after the last one
            output_path << ", ";
    }

    output_path << "\n";
}

//this funtion does not set final_result_.gameStat!!! (the Satellite View)
//final_result_.gameStat is built directly inside the run() function
 void MyGameManager::setGameResultBesideSatellite(const size_t& p1Alive, const size_t& p2Alive, const int& stepCounter){
   if (p1Alive > 0 && p2Alive == 0) {
     final_result_.winner = 1;
     final_result_.reason = GameResult::Reason::ALL_TANKS_DEAD;
    } else if (p2Alive > 0 && p1Alive == 0) {
     final_result_.winner = 2;
     final_result_.reason = GameResult::Reason::ALL_TANKS_DEAD;
    } else if (p1Alive == 0 && p2Alive == 0) {
     final_result_.winner = 0; //tie
     final_result_.reason = GameResult::Reason::ALL_TANKS_DEAD;
    } else if (stepsLeftWhenShellsOver_ >= STEPS_WHEN_SHELLS_OVER) {
     final_result_.winner = 0; //tie
     final_result_.reason = GameResult::Reason::ZERO_SHELLS;
    } else {
     final_result_.winner = 0; //tie
     final_result_.reason = GameResult::Reason::MAX_STEPS;
   }

    final_result_.remaining_tanks = {p1Alive, p2Alive}; // index 0 = player 1, etc.
	//final_result_.gameStat - it's the Satellite View. It's built directly inside the run()
    final_result_.rounds = stepCounter;

 }

//call this after setting the game result
void MyGameManager::printGameResult(const size_t p1Alive, const size_t p2Alive, std::ostream& output_path) const {
    if (final_result_.winner == 1) {
       printPlayer1Won(p1Alive, output_path);
    } else if (final_result_.winner == 2) {
      printPlayer2Won(p2Alive,output_path);
    } else if (final_result_.winner == 0) { //tie
      if (final_result_.reason == GameResult::Reason::ALL_TANKS_DEAD) {printTieZeroTanks(output_path);}
      else if (final_result_.reason == GameResult::Reason::ZERO_SHELLS) {printTieZeroShells(output_path);}
      else if (final_result_.reason == GameResult::Reason::MAX_STEPS) {printTieReacehedMaxSteps(p1Alive, p2Alive, output_path);}
    }
}

void MyGameManager::printPlayer1Won(const size_t p1Alive, std::ostream& output_path) const {
  output_path << "Player 1 won with " << p1Alive << " tanks still alive\n";
}

void MyGameManager::printPlayer2Won(const size_t p2Alive, std::ostream& output_path) const {
  output_path << "Player 2 won with " << p2Alive << " tanks still alive\n";
}

void MyGameManager::printTieZeroTanks(std::ostream& output_path) const {
    output_path << "Tie, both players have zero tanks\n";
}

void MyGameManager::printTieZeroShells(std::ostream& output_path) const {
	   output_path << "Tie, both players have zero shells for <"
                    << STEPS_WHEN_SHELLS_OVER << "> steps\n";
}

void MyGameManager::printTieReacehedMaxSteps(const size_t p1Alive, const size_t p2Alive, std::ostream& output_path) const{
   output_path << "Tie, reached max steps = " << max_steps_
                    << ", player 1 has " << p1Alive
                    << " tanks, player 2 has " << p2Alive << " tanks\n";
}


//returns true if a player, or both, lost all of his tanks.
//else, returns false
//also counts and updates the number of tanks for each player
bool MyGameManager::checkIfPlayerLostAllTanks(size_t& p1Alive, size_t& p2Alive) {
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

//make unique output name with a unique time stamp
//Fourm specs: create a name with names of players + map name + unique time stamp
std::string MyGameManager::makeUniquePath() {
    using namespace std::chrono;
    namespace fs = std::filesystem;

    constexpr std::size_t NUM_DIGITS   = 9;                  // width to print
    constexpr std::uint64_t NUM_DIGITS_P = 1'000'000'000ULL; // 10^9

    // Try a few variants (time + i) to dodge a same-tick collision.
    for (int i = 0; i < 100; ++i) {
        // microsecond resolution is plenty; cast for a stable integer
        auto now_us = time_point_cast<microseconds>(system_clock::now());
        std::uint64_t ticks = static_cast<std::uint64_t>(now_us.time_since_epoch().count());

        // Bounded, fixed-width numeric token (padded with leading zeros)
        std::uint64_t token = (ticks + static_cast<std::uint64_t>(i)) % NUM_DIGITS_P;

        std::ostringstream num;
        num << std::setw(NUM_DIGITS) << std::setfill('0') << token;

        std::string path = map_name_ + "_" + player1_name_ + "_" + player1_name_ + "_" + num.str() + ".txt";

        // If it doesn't exist, we're good.
        if (!fs::exists(path)) return path;
    }

    // Fallback: unbounded tick value (unique enough even if files exist)
    auto now_us = time_point_cast<microseconds>(system_clock::now());
    return map_name_ + "_" + player1_name_ + "_" + player1_name_ +  "_" + std::to_string(now_us.time_since_epoch().count()) + ".txt";
}

std::string MyGameManager::actionRequestToString(ActionRequest action){
	std::string action_str;
        switch (action) {
          case ActionRequest::MoveForward: action_str = "MoveForward"; break;
          case ActionRequest::MoveBackward: action_str = "MoveBackward"; break;
          case ActionRequest::RotateLeft90: action_str = "RotateLeft90"; break;
          case ActionRequest::RotateRight90: action_str = "RotateRight90"; break;
          case ActionRequest::RotateLeft45: action_str = "RotateLeft45"; break;
          case ActionRequest::RotateRight45: action_str = "RotateRight45"; break;
          case ActionRequest::Shoot: action_str = "Shoot"; break;
          case ActionRequest::GetBattleInfo: action_str = "GetBattleInfo"; break;
          case ActionRequest::DoNothing: action_str = "DoNothing"; break;
          default: action_str = "Unknown"; break;
        }
        return action_str;
}

REGISTER_GAME_MANAGER(MyGameManager);

}