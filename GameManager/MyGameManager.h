#pragma once

#include <algorithm>
#include <type_traits>
#include <fstream>
#include <memory>
#include <vector>
#include <xmmintrin.h>

#include "../common/AbstractGameManager.h"
#include "../common/ActionRequest.h"
#include "../common/BattleInfo.h"
#include "../common/GameManagerRegistration.h"
#include "../common/GameResult.h"
#include "../common/Player.h"
#include "../common/SatelliteView.h"
#include "../common/TankAlgorithm.h"

#include "../UserCommon/Board.h"
#include "../UserCommon/Direction.h"
#include "../UserCommon/Position.h"
#include "../UserCommon/Tank.h"
#include "../UserCommon/Mine.h"
#include "../UserCommon/Wall.h"
#include "../UserCommon/Shell.h"
#include "../UserCommon/MyBattleInfo.h" //TODO: or the abstruct?
#include "../UserCommon/SatelliteViewImpl.h"   //TODO: or the abstruct?cd ..



namespace GameManager_207177197_301251571 {

class MyGameManager : public AbstractGameManager {
public:
    MyGameManager(bool verbose);  // our constructor
    ~MyGameManager() = default;   // optional, if we need

    GameResult run(
    size_t map_width, size_t map_height,
    const SatelliteView& map, // <= a snapshot, NOT updated
    string map_name,
    size_t max_steps, size_t num_shells,
    Player& player1, string name1, Player& player2, string name2,
    TankAlgorithmFactory player1_tank_algo_factory,
    TankAlgorithmFactory player2_tank_algo_factory) override;

    bool getVerbose () const {return verbose_;}

    static constexpr int STEPS_WHEN_SHELLS_OVER = 40;
    static constexpr int SHELL_MOVES_PER_STEP = 2;

private:
    bool verbose_;

    size_t map_width_;
    size_t map_height_;
    size_t num_shells_;
    size_t max_steps_;
    string map_name;

    std::unique_ptr<Player> player1_;
    std::unique_ptr<Player> player2_;

    TankAlgorithmFactory player1_tank_algo_factory_;
    TankAlgorithmFactory player2_tank_algo_factory_;

    UserCommon_207177197_301251571::Board board_;
    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Wall>> walls_;
    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Mine>> mines_;
    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Shell>> shells_;

    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Tank>> p1Tanks_;
    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Tank>> p2Tanks_;
    std::vector<UserCommon_207177197_301251571::Tank*> allTanksSorted_;

    int stepsLeftWhenShellsOver_ = STEPS_WHEN_SHELLS_OVER;
    int shellMovesPerStep_ = SHELL_MOVES_PER_STEP;
    int stepCounter_ = 0;

    void initializeGameState(const SatelliteView& map);
    int pre_run();
    int run();



    //Handling tank's actions
    void handleMoveTankForward(UserCommon_207177197_301251571::Tank& tank);
    void handleMoveTankBack(UserCommon_207177197_301251571::Tank& tank);
    void handleTankAskMoveBack(UserCommon_207177197_301251571::Tank& tank);
    void handleShoot(UserCommon_207177197_301251571::Tank& tank);
    void handleRotateEighthLeft(UserCommon_207177197_301251571::Tank& tank);
    void handleRotateFourthLeft(UserCommon_207177197_301251571::Tank& tank);
    void handleRotateEighthRight(UserCommon_207177197_301251571::Tank& tank);
    void handleRotateFourthRight(UserCommon_207177197_301251571::Tank& tank);
    void handleDoNothing(UserCommon_207177197_301251571::Tank& tank);
    void handleRequestBattleInfo(UserCommon_207177197_301251571::Tank& tank);

    void handleAction(UserCommon_207177197_301251571::Tank& tank, ActionRequest action);

    //helper functions for the run() functions
    int getTotalShellsLeft() const;
    void countersHandler(UserCommon_207177197_301251571::Tank& tank);
    void handleAutoMoveTankBack(UserCommon_207177197_301251571::Tank& tank);

    void resolveShellCollisionsAtPosition(UserCommon_207177197_301251571::Shell& shell);

    template<typename T>
    void cleanupDestroyedObjects(std::vector<std::unique_ptr<T>>& vec) {
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](std::unique_ptr<T>& obj) {
                if (obj->isDestroyed()) {
                    board_.removeObject(obj.get(), obj->getPosition()); // remove from board
                    return true; // remove from vector
                }
                return false;
            }), vec.end());
    }

    void moveForwardAndWrap(UserCommon_207177197_301251571::Shell& shell);
    void resolveTankCollisionsAtPosition(UserCommon_207177197_301251571::Tank& tank);
    void shellStep();

    //get pointers
    std::vector<UserCommon_207177197_301251571::Shell*> getShellPtrs() const;
    std::vector<UserCommon_207177197_301251571::Mine*> getMinePtrs() const;

    //more helper functions
    static void printToFile(const std::string& message, std::ostream& output_path);


    //for creating the output file
    std::vector<UserCommon_207177197_301251571::Tank*> sortAllTanks(const std::vector<std::unique_ptr<UserCommon_207177197_301251571::Tank>>& p1Tanks,
                                    const std::vector<std::unique_ptr<UserCommon_207177197_301251571::Tank>>& p2Tanks);
    void printRoundToFile(std::ostream& output_path);
    void printGameResult(int p1Alive, int p2Alive, std::ostream& output_path);
    bool checkIfPlayerLostAllTanks(int& p1Alive, int& p2Alive);
    static std::string makeUniquePath(std::string folder_path, std::string mode_name); //time helper function

};

}