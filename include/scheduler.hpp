#pragma once

#include "utils.hpp"

namespace CPU {
    static constexpr const char* MinFreqPath= "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq";
    static constexpr const char* MaxFreqPath= "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq";
    
    void release() {

    }

    void boost() {
        
    }
};

class Schedule {
private:
    using namespace CPU;
    Utils utils;
public:

};