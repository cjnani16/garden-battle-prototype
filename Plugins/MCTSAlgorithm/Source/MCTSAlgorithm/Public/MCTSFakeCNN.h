#pragma once

#include <random>
#include "./MCTSAgent.h"

using namespace MCTS;

class FakeCNN : public IModel {
    std::vector<float> Evaluate(const State& state) {
        std::vector<float> evaluations;
        //give random evaluations
        for (int i = 0; i < evaluations.size(); i++) {
            evaluations.emplace_back(static_cast<float>(std::rand()) / RAND_MAX);
        }
        return evaluations;
    }
};