#include "utils.hpp"
#include "CpuTurboScheduler.hpp"
int main() {
    Utils utils;
    CPUTurboScheduler cpuTurboScheduler;
    utils.clear_log();
    utils.initialization();
    utils.Init();
    cpuTurboScheduler.GetConfig();
    cpuTurboScheduler.UpdateFunction();
    utils.exec("rm -rf /dev/cpuset/foreground/boost");
    cpuTurboScheduler.ReadPerformancesConfig();
    cpuTurboScheduler.GetPerformanceConfig();
    cpuTurboScheduler.DetectedModes();
    usleep(1000);

    std::thread([&cpuTurboScheduler] { cpuTurboScheduler.ScreenProperty(); }).detach();
    std::thread([&cpuTurboScheduler] { cpuTurboScheduler.GetCpuLoad(); }).detach();
    std::thread([&cpuTurboScheduler] { cpuTurboScheduler.TopAppMonitor(); }).detach();
    std::thread([&cpuTurboScheduler] { cpuTurboScheduler.UpdateConfig(); }).detach();
    while (true) {
        utils.InotifyMain("/sdcard/Android/MW_CpuSpeedController/config.txt");
        cpuTurboScheduler.ReadPerformancesConfig();
        cpuTurboScheduler.GetPerformanceConfig();
        cpuTurboScheduler.DetectedModes();
    }
    return 0;
}
