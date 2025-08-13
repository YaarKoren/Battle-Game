#pragma once

#include "../common/GameManagerRegistration.h"
#include "GameManagerRegistrar.h"

GameManagerRegistration::GameManagerRegistration(GameManagerFactory factory) {
    auto& regsitrar = GameManagerRegistrar::getGameManagerRegistrar();
    regsitrar.addGMFactoryToLastEntry(std::move(factory));
}
