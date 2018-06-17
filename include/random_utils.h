//
// Created by jimx on 16-12-19.
//

#ifndef DSPROJECT_RANDOM_UTILS_H_H
#define DSPROJECT_RANDOM_UTILS_H_H

#include <random>

class RandomUtils {
private:
    static std::random_device rd;
    static std::mt19937 mt;

public:
    static int random_int(int exclusiveMax);
    static int random_int(int min, int max); // inclusive min/max
    static bool random_bool(double probability = 0.5);
};

#endif //DSPROJECT_RANDOM_UTILS_H_H
