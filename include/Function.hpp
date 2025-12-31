#pragma once

#include "Config.hpp"
#include "Utils.hpp" 
#include "Logger.hpp"

using namespace Config;

class Function {
private:
    static constexpr const char* qcomFeas = "/sys/module/perfmgr/parameters/perfmgr_enable";
    static constexpr const char* mtkFeas = "/sys/module/mtk_fpsgo/parameters/perfmgr_enable";
    static constexpr const char* cpusetPath = "/dev/cpuset/";
    static constexpr const char* cpuctlPath = "/dev/cpuctl/";
    static constexpr const char* qcomGpuPath = "/sys/class/kgsl/kgsl-3d0/";
    static constexpr const char* easSchedPath = "/proc/sys/kernel/sched_energy_aware";
    
    Utils utils;
    Logger logger;
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
        if (!Cpuset::enable) return;
        utils.FileWrite("/dev/cpuset/top-app/cpus", Cpuset::top_app.c_str());
        utils.FileWrite("/dev/cpuset/foreground/cpus", Cpuset::foreground.c_str());
        utils.FileWrite("/dev/cpuset/background/cpus", Cpuset::background.c_str());
        utils.FileWrite("/dev/cpuset/system-background/cpus", Cpuset::system_background.c_str());
        utils.FileWrite("/dev/cpuset/restricted/cpus", Cpuset::restricted.c_str());

        logger.Debug("top_app: " + std::string(Cpuset::top_app.c_str()));
        logger.Debug("foreground: " + std::string(Cpuset::foreground.c_str()));
        logger.Debug("background: " + std::string(Cpuset::background.c_str()));
        logger.Debug("system_background: " + std::string(Cpuset::system_background.c_str()));
        logger.Debug("restricted: " + std::string(Cpuset::restricted.c_str()));

        logger.Info("CpuSet调整完毕");
    }

    void disableGpuBoost() {
        if (!checkQcom() || !DisableGpuBoost::enable) return;
        const int buff = utils.readInt("/sys/class/kgsl/kgsl-3d0/num_pwrlevels");
        const int minPwrlvl = buff - 1;
        utils.WriteInt("/sys/class/kgsl/kgsl-3d0/default_pwrlevel", minPwrlvl);
        utils.WriteInt("/sys/class/kgsl/kgsl-3d0/min_pwrlevel", minPwrlvl);
    
        utils.FileWrite("/sys/class/kgsl/kgsl-3d0/max_pwrlevel", "0");
        utils.FileWrite("/sys/class/kgsl/kgsl-3d0/thermal_pwrlevel", "0");   
        utils.FileWrite("/sys/class/kgsl/kgsl-3d0/throttling", "0");
        utils.FileWrite("/sys/class/kgsl/kgsl-3d0/force_bus_on", "0");
        utils.FileWrite("/sys/class/kgsl/kgsl-3d0/force_clk_on", "0");
        utils.FileWrite("/sys/class/kgsl/kgsl-3d0/force_no_nap", "0");
        utils.FileWrite("/sys/class/kgsl/kgsl-3d0/force_rail_on", "0");     
        utils.FileWrite("/sys/class/kgsl/kgsl-3d0/bus_split", "1"); 

        logger.Debug("default_pwrlevel调整为: " + std::to_string(minPwrlvl));
        logger.Debug("min_pwrlevel调整为: " + std::to_string(minPwrlvl));
        logger.Debug("max_pwrlevel调整为: 0");
        logger.Debug("thermal_pwrlevel调整为: 0");
        logger.Debug("throttling调整为: 0");
        logger.Debug("force_bus_on调整为: 0");
        logger.Debug("force_clk_on调整为: 0");
        logger.Debug("force_no_nap调整为: 0");
        logger.Debug("force_rail_on调整为: 0");
        logger.Debug("bus_split调整为: 1");

        logger.Info("高通GPU已优化完毕");
    }

    void LoadBanlance() {
        if (!LoadBanlace::enable) return;
        if (checkCpuset()) { logger.Warn("您的设备并不支持均衡负载"); return; }
        utils.FileWrite("/dev/cpuset/sched_load_balance", "1");
        utils.FileWrite("/dev/cpuset/*/sched_load_balance", "1");
        utils.FileWrite("/dev/cpuset/sched_relax_domain_level", "0"); 
        utils.FileWrite("/dev/cpuset/*/sched_relax_domain_level", "0"); 
        utils.FileWrite("/dev/cpuset/memory_migrate", "0"); 
        utils.FileWrite("/dev/cpuset/*/memory_migrate", "0"); 

        logger.Debug("sched_load_balance调整为: 1");
        logger.Debug("sched_relax_domain_level调整为: 0");
        logger.Debug("memory_migrate调整为: 0");

        logger.Info("负载均衡已优化完毕");
    }

    void IoSchedOpt() {
        if (!DisableGpuBoost::enable) return;
        utils.FileWrite("/sys/block/*/queue/scheduler", IO_Optimization::scheduler.c_str());
        utils.FileWrite("/sys/block/*/queue/nomerges", IO_Optimization::nomerges.c_str());
        utils.FileWrite("/sys/block/*/queue/iostats", IO_Optimization::iostats.c_str());
        utils.FileWrite("/sys/block/*/queue/read_ahead_kb", IO_Optimization::read_ahead_kb.c_str());
        utils.FileWrite("/sys/block/*/bdi/read_ahead_kb", IO_Optimization::read_ahead_kb.c_str());

        // 后续可以考虑区分"emmc"和"ufs"设备进行自动优化 只需要 IO_Optimization::scheduler = "auto"即可
        // mq-deadline kyber [bfq] none 
        logger.Debug("IO调度器调整为: " + std::string(IO_Optimization::scheduler.c_str()));
        logger.Debug("nomerges调整为: " + std::string(IO_Optimization::nomerges.c_str()));
        logger.Debug("iostats调整为: " + std::string(IO_Optimization::iostats.c_str()));
        logger.Debug("read_ahead_kb调整为: " + std::string(IO_Optimization::read_ahead_kb.c_str()));
        
        logger.Info("IO优化完毕");
    }

    void CfsSchedOpt() {
        if (!Scheduler::enable) return;
        utils.FileWrite("/proc/sys/kernel/sched_schedstats", Scheduler::Sched_schedstats ? "1" : "0");
        utils.FileWrite("/proc/sys/kernel/sched_latency_ns", Scheduler::Sched_latency_ns.c_str());
        utils.FileWrite("/proc/sys/kernel/sched_migration_cost_ns", Scheduler::Sched_migration_cost_ns.c_str());
        utils.FileWrite("/proc/sys/kernel/sched_min_granularity_ns", Scheduler::Sched_min_granularity_ns.c_str());
        utils.FileWrite("/proc/sys/kernel/sched_wakeup_granularity_ns", Scheduler::Sched_wakeup_granularity_ns.c_str());
        utils.FileWrite("/proc/sys/kernel/sched_nr_migrate", Scheduler::Sched_nr_migrate.c_str());
        utils.FileWrite("/proc/sys/kernel/sched_util_clamp_min", Scheduler::Sched_util_clamp_min.c_str());
        utils.FileWrite("/proc/sys/kernel/sched_util_clamp_max", Scheduler::Sched_util_clamp_max.c_str());
        
        if (checkEasSched()) {
            utils.FileWrite("/proc/sys/kernel/sched_energy_aware", Scheduler::Sched_energy_aware ? "1" : "0");
            logger.Info("已开启EAS调度器");
        } else 
            logger.Warn("您的设备并不支持EAS调度器");

        logger.Debug("Sched_energy_aware调整为:" + std::string(Scheduler::Sched_energy_aware ? "1" : "0"));
        logger.Debug("Sched_schedstats调整为:" + std::string(Scheduler::Sched_schedstats ? "1" : "0"));
        logger.Debug("Sched_latency_ns调整为:" + std::string(Scheduler::Sched_latency_ns.c_str()));
        logger.Debug("Sched_migration_cost_ns调整为:" + std::string(Scheduler::Sched_migration_cost_ns.c_str()));
        logger.Debug("Sched_min_granularity_ns调整为:" + std::string(Scheduler::Sched_min_granularity_ns.c_str()));
        logger.Debug("Sched_wakeup_granularity_ns调整为:" + std::string(Scheduler::Sched_wakeup_granularity_ns.c_str()));
        logger.Debug("Sched_nr_migrate调整为:" + std::string(Scheduler::Sched_nr_migrate.c_str()));
        logger.Debug("Sched_util_clamp_min调整为:" + std::string(Scheduler::Sched_util_clamp_min.c_str()));
        logger.Debug("Sched_util_clamp_max调整为:" + std::string(Scheduler::Sched_util_clamp_max.c_str()));

        logger.Info("CFS调度器已优化完毕");
    }

    bool FeasFunc(bool Enable) {
        // 后续可直接获取前台 检测是否是游戏 来确定是否需要开启"Feas"
        if (checkQcom()) {
            utils.FileWrite(qcomFeas, Enable ? "1" : "0");
            logger.Debug("QCOM Feas 已" + (Enable ? "开启" : "关闭"));
            return true;
        } 

        if (checkMtkFeas()) {
            utils.FileWrite(mtkFeas, Enable ? "1" : "0");
            logger.Debug("MTK Feas 已" + (Enable ? "开启" : "关闭"));
            return true;
        }
        return false;
    }
private:
    bool checkQcom() const {
        return (!access(qcomFeas, F_OK));
    }
    
    bool checkMtkFeas() const {
        return (!access(mtkFeas, F_OK))
    }
    bool checkCpuset() const {
        return (!access(cpusetPath, F_OK));
    }

    bool checkCpuctl() const {
        return (!access(cpuctlPath, F_OK))
    }

    bool checkEasSched() const {
        return (!access(easSchedPath, F_OK));
    }

    bool checkQcom() const {
        return (!access(qcomGpuPath, F_OK));
    }
};