#pragma once

#include <random>

using namespace std;

random_device device;
mt19937 rng(device());