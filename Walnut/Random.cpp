//
//  Random.cpp
//  Walnut
//
//  Created by Stephen H. Gerstacker on 2022-06-16.
//

#include "Random.h"

namespace Walnut {
    std::mt19937 Random::randomEngine;
    std::uniform_int_distribution<std::mt19937::result_type> Random::distribution;
}

