#pragma once 
#include "utils.hpp"

class CPUTurboScheduler {
private:
    Utils utils;
    Config config;
    bool DisableQcomGpu, ufsClkGate, LoadBalancing, cpuset, cpuctl, CpuIdleScaling_Governor, EasScheduler, AffintySetter, EnableFeas, CoreAllocation;
    std::string top_app_cpuset, foreground_cpuset, background_cpuset, system_background_cpuset, restricted_cpuset;
    std::string sched_min_granularity_ns, sched_nr_migrate, sched_wakeup_granularity_ns, sched_schedstats;
    std::string SwitchMode, loglevel, cpuidle_governor, scaling_governor, cpusetCore, cpuctlUclampBoostMin, cpuctlUclampBoostMax;
    bool LoadBoost, RefreshTopAppBoost;
    int LoadCritical, ScreenState;
    std::string modelType0, BoostFreq0, MinFreq0, MaxFreq0, modelType1, BoostFreq1, MinFreq1, MaxFreq1, modelType2, BoostFreq2, MinFreq2, MaxFreq2, Boost_Duration_ms, UclampTopAppMin, UclampTopAppMax, UclampForeGroundMin,UclampForeGroundMax,TopApplatency_sensitive;
public:
    void ReadPerformancesConfig() {
        std::string Mode;
        std::ifstream file = config.Getconfig();
        while (std::getline(file, Mode)) {
            SwitchMode = Mode;
        }
    }

    void DetectedModes(){
        Adjscaling_governor();
        if (SwitchMode == "powersave"){
            AdjPerformanceModes();
            AdjEasScheduler();
            utils.log("INFO:省电模式已启用");
        } else if (SwitchMode == "balance"){
            AdjPerformanceModes();
            AdjEasScheduler();
            utils.log("INFO:均衡模式已启用");
        } else if (SwitchMode == "performance"){
            AdjPerformanceModes();
            AdjEasScheduler();
            utils.log("INFO:性能模式已启用");
        } else if (SwitchMode == "fast"){
            AdjPerformanceModes();
            AdjEasScheduler();
            utils.log("INFO:极速模式已启用");
            if (!EnableFeas) return;
                EnableMiFeas();
        }
    }
    void GetConfig() {
        INIReader reader("/sdcard/Android/MW_CpuSpeedController/config.ini");
        
        if (reader.ParseError() < 0) {
            if (loglevel != "close") return;
            utils.log("Error:请检查您的配置文件是否存在");
            exit(0);
        }

        DisableQcomGpu = reader.GetBoolean("function", "DisableQcomGpu", false);
        ufsClkGate = reader.GetBoolean("function", "ufsClkGate", false);
        LoadBalancing = reader.GetBoolean("function", "LoadBalancing", false);
        cpuset = reader.GetBoolean("function", "cpuset", false);
        cpuctl = reader.GetBoolean("function", "cpuctl", false);
        CpuIdleScaling_Governor = reader.GetBoolean("function", "CpuIdleScaling_Governor", false);
        AffintySetter = reader.GetBoolean("function", "AffintySetter", false);
        EnableFeas = reader.GetBoolean("function", "EnableFeas", false);
        CoreAllocation = reader.GetBoolean("function", "CoreAllocation", false);
        cpusetCore = reader.Get("CoreAllocation", "cpusetCore", "4-7");
        cpuctlUclampBoostMin = reader.Get("CoreAllocation", "cpuctlUclampBoostMin", "10");
        cpuctlUclampBoostMax = reader.Get("CoreAllocation", "cpuctlUclampBoostMax", "80");
        cpuidle_governor = reader.Get("CpuIdle", "cpuidle_governor", "0");
        top_app_cpuset = reader.Get("Cpuset", "top_app", "0");
        foreground_cpuset = reader.Get("Cpuset", "foreground", "0");
        background_cpuset = reader.Get("Cpuset", "background", "0");
        system_background_cpuset = reader.Get("Cpuset", "system_background", "0");
        restricted_cpuset = reader.Get("Cpuset", "restricted", "0");
        sched_min_granularity_ns = reader.Get("EasSchedulerVaule", "sched_min_granularity_ns", "0");
        sched_nr_migrate = reader.Get("EasSchedulerVaule", "sched_nr_migrate", "0");
        sched_wakeup_granularity_ns = reader.Get("EasSchedulerVaule", "sched_wakeup_granularity_ns", "0");
        sched_schedstats = reader.Get("EasSchedulerVaule", "sched_schedstats", "0");
        loglevel = reader.Get("meta", "loglevel", "0");
    }
    void GetPerformanceConfig() {
        INIReader reader("/sdcard/Android/MW_CpuSpeedController/config.ini");
        
        if (reader.ParseError() < 0) {
            if (loglevel != "close") return;
            utils.log("Error:请检查您的配置文件是否存在");
            exit(0);
        }

        LoadBoost = reader.GetBoolean(SwitchMode, "LoadBoost", false);
        RefreshTopAppBoost = reader.GetBoolean(SwitchMode, "RefreshTopAppBoost", false);
        EasScheduler = reader.GetBoolean(SwitchMode, "EasScheduler", false);
        LoadCritical = std::stoi(reader.Get(SwitchMode, "LoadCritical", "0"));
        modelType0 = reader.Get(SwitchMode, "modelType0", "0");
        BoostFreq0 = reader.Get(SwitchMode, "BoostFreq0", "0");
        MinFreq0 = reader.Get(SwitchMode, "MinFreq0", "0");
        MaxFreq0 = reader.Get(SwitchMode, "MaxFreq0", "0");
        modelType1 = reader.Get(SwitchMode, "modelType1", "0");
        BoostFreq1 = reader.Get(SwitchMode, "BoostFreq1", "0");
        MinFreq1 = reader.Get(SwitchMode, "MinFreq1", "0");
        MaxFreq1 = reader.Get(SwitchMode, "MaxFreq1", "0");
        modelType2 = reader.Get(SwitchMode, "modelType2", "0");
        BoostFreq2 = reader.Get(SwitchMode, "BoostFreq2", "0");
        MinFreq2 = reader.Get(SwitchMode, "MinFreq2", "0");
        MaxFreq2 = reader.Get(SwitchMode, "MaxFreq2", "0");
        scaling_governor = reader.Get(SwitchMode, "scaling_governor", "powersave");
        TopApplatency_sensitive = reader.Get(SwitchMode, "TopApplatency_sensitive", "0");
        Boost_Duration_ms = reader.Get(SwitchMode, "Boost_Duration_ms", "0");
        UclampTopAppMin = reader.Get(SwitchMode, "UclampTopAppMin", "0");
        UclampTopAppMax = reader.Get(SwitchMode, "UclampTopAppMax", "0");
        UclampForeGroundMin = reader.Get(SwitchMode, "UclampForeGroundMin", "0");
        UclampForeGroundMax = reader.Get(SwitchMode, "UclampForeGroundMax", "0");
    }
    void Adjscaling_governor() {
        for (int i = 0; i <= 7; ++i) {
            const std::string cpuDir = "/sys/devices/system/cpu/cpufreq/policy" + std::to_string(i) + "/scaling_governor";
            utils.WriteFile(cpuDir, scaling_governor);
        }
        if (loglevel == "Debug") {
            utils.log("INFO:设定CPU调速器为:"+ scaling_governor);
        }
    }
    void TemporaryBoost() {
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType0 + "/scaling_max_freq", BoostFreq0);
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType1 + "/scaling_max_freq", BoostFreq1);
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType2 + "/scaling_max_freq", BoostFreq2);
    }
    void ResetBoost() {
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType0 + "/scaling_max_freq", MaxFreq0);
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType1 + "/scaling_max_freq", MaxFreq1);
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType2 + "/scaling_max_freq", MaxFreq2);
    }
    void MaxBoost(){
        utils.log("Debug:检测到多次Boost,将采用全速Boost");
        for(int i = 0; i <= 7; ++i){
            utils.WriteFile("/sys/devices/system/cpu/cpufreq/policy" + std::to_string(i)+ "/scaling_max_freq", "2147483647");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(std::stoi(Boost_Duration_ms)));
    }
    void UpdateConfig() {
        while (true) {
            utils.InotifyMain("/sdcard/Android/MW_CpuSpeedController/config.ini");
            GetConfig();
            UpdateFunction();
            if (loglevel == "Debug") {
                utils.log("Debug:附加功能参数已更新");
            }
        }
    }
    void ScreenProperty(){
        if (LoadBoost || RefreshTopAppBoost){
            utils.log("INFO:监听屏幕成功");
            while (true) {
                ScreenState = utils.getScreenProperty();// 1是息屏状态 2是亮屏状态
                std::this_thread::sleep_for(std::chrono::seconds(20));
            }
        }
    }
    void GetCpuLoad() {
        utils.log("INFO:监听负载变化成功");
        int LoadBoostCount = 0;
        while(true) {
            if (ScreenState == 1) {
                std::this_thread::sleep_for(std::chrono::seconds(20));
                continue;
            }
        
            if (LoadBoost){
                auto initialStats = utils.readCpuStats();
                sleep(1);
                auto finalStats = utils.readCpuStats();
                int totalLoad = utils.calculateTotalCpuLoad(initialStats, finalStats);
                if (LoadBoostCount >= 3) {
                    MaxBoost();
                    LoadBoostCount = 0;
                    std::this_thread::sleep_for(std::chrono::seconds(3)); 
                }
                if (totalLoad >= LoadCritical){
                    mtx.lock();
                    TemporaryBoost();
                    LoadBoostCount++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(std::stoi(Boost_Duration_ms)));
                    ResetBoost();
                    mtx.unlock();
                    if (loglevel == "Debug") {
                        utils.log("Debug: 检测到CPU负载大于" + std::to_string(LoadCritical) + "，已进行临时升频");
                    }
                } 
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    void AdjPerformanceModes() {
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType0 + "/scaling_mix_freq", MinFreq0);
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType1 + "/scaling_mix_freq", MinFreq1);
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType2 + "/scaling_mix_freq", MinFreq2);
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType0 + "/scaling_max_freq", MaxFreq0);
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType1 + "/scaling_max_freq", MaxFreq1);
        utils.WriteFile("/sys/devices/system/cpu/cpufreq/" + modelType2 + "/scaling_max_freq", MaxFreq2);
        utils.WriteFile("/dev/cpuctl/top-app/cpu.uclamp.latency_sensitive", TopApplatency_sensitive);
        utils.WriteFile("/dev/cpuctl/top-app/cpu.uclamp.min", UclampTopAppMin);
        utils.WriteFile("/dev/cpuctl/top-app/cpu.uclamp.max", UclampTopAppMax);
        utils.WriteFile("/dev/cpuctl/foreground/cpu.uclamp.min", UclampForeGroundMin);
        utils.WriteFile("/dev/cpuctl/foreground/cpu.uclamp.max", UclampForeGroundMax);
    }

    void TopAppMonitor() {
        utils.log("INFO:前台APP切换监听成功");
        while (true) {
            if (ScreenState == 1) {
                std::this_thread::sleep_for(std::chrono::seconds(20));
                continue;
            }
            if (RefreshTopAppBoost) {
                utils.InotifyMain("/dev/cpuset/top-app/cgroup.procs"); 
                mtx.lock();
                TemporaryBoost();
                std::this_thread::sleep_for(std::chrono::milliseconds(std::stoi(Boost_Duration_ms)));
                ResetBoost();
                mtx.unlock();  
                if (loglevel == "Debug") {
                    utils.log("Debug: 检测到前台APP切换，已进行临时升频");
                }
            } 
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    void DisableQcomGpuBoost(){
        if (!DisableQcomGpu) return;
            std::ifstream file("/sys/class/kgsl/kgsl-3d0/num_pwrlevels");
            int num_pwrlevels;
        if (file >> num_pwrlevels) {
            int MIN_PWRLVL = num_pwrlevels - 1;
            std::string minPwrlvlStr = std::to_string(MIN_PWRLVL);
            utils.FileWrite("/sys/class/kgsl/kgsl-3d0/default_pwrlevel", minPwrlvlStr);
            utils.FileWrite("/sys/class/kgsl/kgsl-3d0/min_pwrlevel", minPwrlvlStr);
            utils.FileWrite("/sys/class/kgsl/kgsl-3d0/max_pwrlevel", "0");
            utils.FileWrite("/sys/class/kgsl/kgsl-3d0/thermal_pwrlevel", "0");   
            utils.FileWrite("/sys/class/kgsl/kgsl-3d0/throttling", "0");
            utils.FileWrite("/sys/class/kgsl/kgsl-3d0/force_bus_on", "0");
            utils.FileWrite("/sys/class/kgsl/kgsl-3d0/force_clk_on", "0");
            utils.FileWrite("/sys/class/kgsl/kgsl-3d0/force_no_nap", "0");
            utils.FileWrite("/sys/class/kgsl/kgsl-3d0/force_rail_on", "0");
            utils.FileWrite("/sys/class/kgsl/kgsl-3d0/force_bus_on", "0");              
        }
        if (loglevel == "INFO") {
            utils.log("INFO:高通GpuBoost已调整完毕");
        }
    }
    void DisableUfsClkGate() { 
        if (!ufsClkGate) return;
            utils.FileWrite("/sys/bus/platform/devices/hi6220-ufs/ufs_clk_gate_disable", "1");
        
        if (loglevel == "INFO") {
            utils.log("INFO:UFS时钟门已调整完毕");
        }
    }
    void LoadBanlancing() {
        if (!LoadBalancing) return;
            utils.FileWrite("/dev/cpuset/top-app/sched_relax_domain_level", "1"); 
            utils.FileWrite("/dev/cpuset/foreground/sched_relax_domain_level", "1");
            utils.FileWrite("/dev/cpuset/sched_relax_domain_level", "1");
            utils.FileWrite("/dev/cpuset/background/sched_relax_domain_level", "1");
            utils.FileWrite("/dev/cpuset/system-background/sched_relax_domain_level", "1");

        if (loglevel == "INFO") {
            utils.log("INFO:负载均衡已调整完毕");
        }
    }
    void AdjCpuset() {
        if (!cpuset) return;
            utils.CpuSetWrite("/dev/cpuset/top-app/cpus", top_app_cpuset);
            utils.CpuSetWrite("/dev/cpuset/foreground/cpus", foreground_cpuset);
            utils.CpuSetWrite("/dev/cpuset/background/cpus", background_cpuset);
            utils.CpuSetWrite("/dev/cpuset/system-background/cpus", system_background_cpuset);
            utils.CpuSetWrite("/dev/cpuset/restricted/cpus", restricted_cpuset);

        if (loglevel == "Debug") {
            utils.log("Debug:top-app已调整完毕:" + top_app_cpuset);
            utils.log("Debug:foreground已调整完毕:" + foreground_cpuset);
            utils.log("Debug:background已调整完毕:" + background_cpuset);
            utils.log("Debug:system-background已调整完毕:" + system_background_cpuset);
            utils.log("Debug:restricted已调整完毕:" + restricted_cpuset);
        } else if (loglevel == "INFO") {
            utils.log("INFO:CPUSET已调整完毕");
        }
    }
    void AdjCpuIdleScaling_Governor() {
        if (!CpuIdleScaling_Governor) return;
            utils.FileWrite("/sys/devices/system/cpu/cpuidle/current_governor", cpuidle_governor);
        
        if (loglevel == "INFO") {
            utils.log("INFO:CPUIDLE已调整完毕:" + cpuidle_governor);
        }
    }
    void AdjEasScheduler() {
        if (EasScheduler) {
            utils.FileWrite("/proc/sys/kernel/sched_min_granularity_ns", sched_min_granularity_ns);
            utils.FileWrite("/proc/sys/kernel/sched_nr_migrate", sched_nr_migrate);
            utils.FileWrite("/proc/sys/kernel/sched_wakeup_granularity_ns", sched_wakeup_granularity_ns);
            utils.FileWrite("/proc/sys/kernel/sched_schedstats", sched_schedstats);
            utils.FileWrite("/proc/sys/kernel/sched_energy_aware", "1");
        } else {
            utils.FileWrite("/proc/sys/kernel/sched_energy_aware", "0");
        }
        
        if (loglevel == "Debug") {
            utils.log("Debug:sched_min_granularity_ns参数调整完毕:" + sched_min_granularity_ns);
            utils.log("Debug:sched_nr_migrate参数调整完毕:" + sched_nr_migrate);
            utils.log("Debug:sched_wakeup_granularity_ns参数调整完毕:" + sched_wakeup_granularity_ns);
            utils.log("Debug:sched_schedstats参数调整完毕:" + sched_schedstats);
        } else if (loglevel == "INFO") {
            utils.log("Debug:EAS调度器参数调整完毕");
        }
    }
    void EnableMiFeas() {
        if (utils.checkqcomFeas()) {
            utils.FileWrite("/sys/module/perfmgr/parameters/perfmgr_enable", "1");
        } else if(utils.checkMTKFeas()) {
            utils.FileWrite("/sys/module/mtk_fpsgo/parameters/perfmgr_enable", "1");
        }
    }
    void UpdateFunction() {
        DisableQcomGpuBoost();
        DisableUfsClkGate();
        LoadBanlancing();
        AdjCpuset();
        AdjCpuIdleScaling_Governor();
        ThreadCoreAllocation();
    }
    void mount_cpusetAndcpuctl() {
        utils.log("INFO:创建cpuset成功");
        mkdir("/dev/cpuset/top-app/MoWei", 0666);
        utils.FileWrite("/dev/cpuset/top-app/MoWei/cpus", cpusetCore); 
        utils.FileWrite("/dev/cpuset/top-app/MoWei/mems", "0");
        utils.log("INFO:创建cpuctl成功");
        mkdir("/dev/cpuctl/top-app/MoWei", 0666);
        utils.FileWrite("/dev/cpuctl/top-app/MoWei/cpu.uclamp.min", cpuctlUclampBoostMin);
        utils.FileWrite("/dev/cpuctl/top-app/MoWei/cpu.uclamp.max", cpuctlUclampBoostMax);
    }
    void AdjSystemProcessCpuctl() {
        std::vector<std::string> processNames = {"com.android.systemui", "vendor.qti.hardware.display.composer-service", "camerahalserver", "surfaceflinger", " system_server", "android.hardware.audio.service_64" , "audioserver", "media.audio.qc.codec.qti.media.c2audio@1.0-service", "vendor.xiaomi.hw.touchfeature@1.0-service", "'android:ui"};
        std::string pids = utils.getPids(processNames);
        std::string tids = utils.getTids(pids);
        utils.WriteFile("/dev/cpuctl/top-app/MoWei/cgroup.procs", pids);
        utils.WriteFile("/dev/cpuctl/top-app/MoWei/tasks", tids);
        utils.WriteFile("/dev/cpuset/top-app/MoWei/cgroup.procs", pids);
        utils.WriteFile("/dev/cpuset/top-app/MoWei/tasks", tids);
    }
    void ThreadCoreAllocation() {
        if (!CoreAllocation) return;
        mount_cpusetAndcpuctl();
        AdjSystemProcessCpuctl();
    }
};  
