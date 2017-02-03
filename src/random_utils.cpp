//
// Created by jimx on 16-12-19.
//

#include "random_utils.h"
#include <random>

std::random_device RandomUtils::rd;
std::mt19937 RandomUtils::mt(RandomUtils::rd());

int RandomUtils::random_int(int exclusiveMax)
{
    std::uniform_int_distribution<> dist(0, exclusiveMax - 1);
    return dist(mt);
}

int RandomUtils::random_int(int min, int max) // inclusive min/max
{
    std::uniform_int_distribution<> dist(0, max - min);
    return dist(mt) + min;
}

bool RandomUtils::random_bool(double probability)
{
    std::bernoulli_distribution dist(probability);
    return dist(mt);
}
