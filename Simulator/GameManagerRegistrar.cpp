#pragma once

#include "GameManagerRegistrar.h"

GameManagerRegistrar GameManagerRegistrar::registrar_;

GameManagerRegistrar& GameManagerRegistrar::getGameManagerRegistrar() {
    return registrar_;
}

