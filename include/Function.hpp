#pragma once

#include "Utils.hpp" 

class Function {
private:
    Utils utils;
    static constexpr const char* cpuctlPath = "/dev/cpuctl";
public:
    void cpuctlFunction() {
        if (!checkCpuctl()) return;
        utils.FileWrite("/dev/cpuctl/top-app/cpu.uclamp.max", "max");
        utils.FileWrite("/dev/cpuctl/top-app/cpu.uclamp.min", "0");
        utils.FileWrite("/dev/cpuctl/foreground/cpu.uclamp.max", "80");
        utils.FileWrite("/dev/cpuctl/foreground/cpu.uclamp.min", "0");
        utils.FileWrite("/dev/cpuctl/background/cpu.uclamp.max", "20");
        utils.FileWrite("/dev/cpuctl/background/cpu.uclamp.min", "0");
    }
    
    void cpusetFunction() {
        utils.FileWrite("/dev/cpuset/top-app/cpus", "0-7");
        utils.FileWrite("/dev/cpuset/foreground/cpus", "0-6");
        utils.FileWrite("/dev/cpuset/background/cpus", "0-3");
        utils.FileWrite("/dev/cpuset/system-background/cpus", "0-4");
        utils.FileWrite("/dev/cpuset/restricted/cpus", "0-6");
    }

    bool checkCpuctl() {
        return(!access(cpuctlPath, F_OK))
    }
};