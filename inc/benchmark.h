#pragma once

#include "logging.h"
#include <chrono>

using std::chrono::time_point;
using std::chrono::duration;
using std::chrono::nanoseconds;
using std::chrono::high_resolution_clock;

class Benchmark {
private:
    time_point<high_resolution_clock> t1;

    int64_t average = 0;
    int64_t count = 0;

public:
    void start() {
        this->t1 = std::chrono::high_resolution_clock::now();
    }

    void end() {
        this->average += std::chrono::duration_cast<nanoseconds>(
            std::chrono::high_resolution_clock::now() - this->t1).count();
        this->count++;
    }

    void print() {
        info("average: {} ns", this->average / this->count);
    }
};