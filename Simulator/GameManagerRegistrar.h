#pragma once

#include <memory>
#include <cassert>
#include <string>
#include "../common/AbstractGameManager.h"

class GameManagerRegistrar {
   class GMFactoryNamePair {
        std::string so_name_;
        GameManagerFactory GMFactory_;
    public:
       GMFactoryNamePair(const std::string& so_name) : so_name_
                                                                (so_name) {}
        void setGMFactory(GameManagerFactory&& factory) {
            assert(GMFactory_ == nullptr);
            GMFactory_ = std::move(factory);
        }

        const std::string& name() const { return so_name_    ; }

        std::unique_ptr<AbstractGameManager> createGameManager(bool verbose) const {
            return GMFactory_(verbose);
        }

        bool hasGMFactory() const {
            return GMFactory_ != nullptr;
        }

    };
    std::vector<GMFactoryNamePair> GMFactories_;
    static GameManagerRegistrar registrar_;
public:
    static GameManagerRegistrar& getGameManagerRegistrar();
    void createGMFactoryEntry(const std::string& name) {
        GMFactories_.emplace_back(name);
    };
    void addGMFactoryToLastEntry(GameManagerFactory&& factory) {
        GMFactories_.back().setGMFactory(std::move(factory));
    }
    struct BadRegistrationException {
        std::string name;
        bool hasName, hasGMFactory;
    };
    void validateLastRegistration() {
        const auto& last = GMFactories_.back();
        bool hasName = (last.name() != "");
        if(!hasName || !last.hasGMFactory() ) {
            throw BadRegistrationException{
                .name = last.name(),
                .hasName = hasName,
                .hasGMFactory = last.hasGMFactory(),

            };
        }
    }
    void removeLast() {
        GMFactories_.pop_back();
    }
    auto begin() const {
        return GMFactories_.begin();
    }
    auto end() const {
        return GMFactories_.end();
    }
    std::size_t count() const { return GMFactories_.size(); }
    void clear() { GMFactories_.clear(); }

};
