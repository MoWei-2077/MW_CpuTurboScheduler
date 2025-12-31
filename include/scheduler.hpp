#pragma once

#include "utils.hpp"

class Schedule {
private:
    static constexpr const char* MinFreqPath= "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq";
    static constexpr const char* MaxFreqPath= "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq";

    Utils utils;
public:
    void release() {

    }

    void boost() {
        
    }
};