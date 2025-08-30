#pragma once

#include <memory>
#include <cassert>
#include <string>
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"

//this is for both Player and Tank Algotitm
class AlgorithmRegistrar {
    class AlgorithmAndPlayerFactories {
        std::string so_name_;
        TankAlgorithmFactory tankAlgorithmFactory_;
        PlayerFactory playerFactory_;
    public:
        AlgorithmAndPlayerFactories(const std::string& so_name) : so_name_
                                                                (so_name) {}
        void setTankAlgorithmFactory(TankAlgorithmFactory&& factory) {
            assert(tankAlgorithmFactory_ == nullptr);
            tankAlgorithmFactory_ = std::move(factory);
        }
        void setPlayerFactory(PlayerFactory&& factory) {
            assert(playerFactory_ == nullptr);
            playerFactory_ = std::move(factory);
        }
        const std::string& name() const { return so_name_    ; }
        std::unique_ptr<Player> createPlayerFactory(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells) const {
            return playerFactory_(player_index, x, y, max_steps, num_shells);
        }
        std::unique_ptr<TankAlgorithm> createTankAlgorithmFactory(int player_index, int tank_index) const {
            return tankAlgorithmFactory_(player_index, tank_index);
        }
        bool hasPlayerFactory() const {
            return playerFactory_ != nullptr;
        }
        bool hasTankAlgorithmFactory() const {
            return tankAlgorithmFactory_ != nullptr;
        }
    };
    std::vector<AlgorithmAndPlayerFactories> algorithms;
    static AlgorithmRegistrar registrar;
public:
    static AlgorithmRegistrar& getAlgorithmRegistrar();
    void createAlgorithmFactoryEntry(const std::string& name) {
        algorithms.emplace_back(name);
    }
    void addPlayerFactoryToLastEntry(PlayerFactory&& factory) {
        algorithms.back().setPlayerFactory(std::move(factory));
    }
    void addTankAlgorithmFactoryToLastEntry(TankAlgorithmFactory&& factory) {
        algorithms.back().setTankAlgorithmFactory(std::move(factory));
    }
    struct BadRegistrationException {
        std::string name;
        bool hasName, hasPlayerFactory, hasTankAlgorithmFactory;
    };
    void validateLastRegistration() {
        const auto& last = algorithms.back();
        bool hasName = (last.name() != "");
        if(!hasName || !last.hasPlayerFactory() || !last.hasTankAlgorithmFactory() ) {
            throw BadRegistrationException{
                .name = last.name(),
                .hasName = hasName,
                .hasPlayerFactory = last.hasPlayerFactory(),
                .hasTankAlgorithmFactory = last.hasTankAlgorithmFactory()
            };
        }
    }
    void removeLast() {
        algorithms.pop_back();
    }
    auto begin() const {
        return algorithms.begin();
    }
    auto end() const {
        return algorithms.end();
    }
    std::size_t count() const { return algorithms.size(); }
    void clear() { algorithms.clear(); }
};
