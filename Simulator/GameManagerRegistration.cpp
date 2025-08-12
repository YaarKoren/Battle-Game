#pragma once

#include "../common/GameManagerRegistration.h"
#include "GameManagerRegistrar.h"

GameManagerRegistration::GameManagerRegistration(std::function<std::unique_ptr<AbstractGameManager>()> factory) {
    auto& regsitrar = GameManagerRegistrar::getAlgorithmRegistrar();
    //regsitrar.addTankAlgorithmFactoryToLastEntry(std::move(factory));
}
