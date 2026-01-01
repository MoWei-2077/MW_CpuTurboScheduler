#pragma once
 
#include "Json/json.h"
#include "Config.hpp"
#include "Logger.hpp"
#include <stdexcept>

using namespace Config;
using namespace qlib;

class JsonConfig {
private:
    static constexpr const char* configPath = "/sdcard/Android/CTS/config.json";

    Logger logger;
    json_view_t json;
    SwitchConfig config;

    mutex jsonReadMutex;

    std::string lastSwitchMode;

    char buff[256];
    char cluster [64];
public:
    SchedParam schedParam[4];
    
    inline bool switchConfig() const {
        if (config.mode == "powersave" || "balance" || "performance" || "fast") return true;
        return false;
    }

    bool readConfig() {

        lock_guard<mutex> lock(jsonReadMutex);

        config.LoadConfig();
        ifstream ifs(configPath, std::ios::binary);
        if (!ifs) {
            logger.Error("无法打开json配置文件");
            return false;
        }

        std::string text((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        int result = json::parse(&json, text.data(), text.data() + text.size());
        if (result != 0) {
            logger.Error("解析json配置文件失败 错误: " + std::to_string(result));
            return false; 
        }

        Meta::name = json["meta"]["name"].get<string_t>();
        Meta::version = json["meta"]["version"].get<int>();
        Meta::author = json["meta"]["author"].get<string_t>();
        Meta::loglevel = json["meta"]["loglevel"].get<string_t>();
        
        #if DEBUG_DURATION
            logger.setLogLevel(std::string(Meta::loglevel.c_str()));
            logger.Debug("---------源信息---------");
            logger.Info("名称: " + std::string(Meta::name.c_str()));
            logger.Info("版本: " + std::to_string(Meta::version));
            logger.Info("作者: " + std::string(Meta::author.c_str()));
            logger.Info("日志等级: " + std::string(Meta::loglevel.c_str()));
            logger.Info("---------CPU簇---------");
        #endif

        for (int i = 0; i <= 3; i++) {
            FastSnprintf(buff, sizeof(buff), "c%d", i);
            if (json["Policy"][buff].get<int>() == 255) continue;
            Policy::CpuPolicy[i] = json["Policy"][buff].get<int>();
            #if DEBUG_DURATION
                logger.Debug("CPU簇 " + std::to_string(i) + " 的值为: " + std::to_string(Policy::CpuPolicy[i]));
            #endif
        }

        #if DEBUG_DURATION
            logger.Debug("---------附加功能---------");
        #endif 
        auto& Cpuset = json["Function"]["Cpuset"];
        Cpuset::enable = Cpuset["enable"].get<bool>();
        Cpuset::top_app = Cpuset["top_app"].get<string_t>();
        Cpuset::foreground = Cpuset["foreground"].get<string_t>();
        Cpuset::background = Cpuset["background"].get<string_t>();
        Cpuset::system_background = Cpuset["system_background"].get<string_t>();
        Cpuset::restricted = Cpuset["restricted"].get<string_t>();

        #if DEBUG_DURATION
            logger.Debug("Cpuset 开关: " + std::string(Cpuset::enable ? "开启" : "关闭"));
            logger.Debug("top_app: " + std::string(Cpuset::top_app.c_str()));
            logger.Debug("foreground: " + std::string(Cpuset::foreground.c_str()));
            logger.Debug("background: " + std::string(Cpuset::background.c_str()));
            logger.Debug("system_background: " + std::string(Cpuset::system_background.c_str()));
            logger.Debug("restricted: " + std::string(Cpuset::restricted.c_str()));
            logger.Debug("---------均衡负载---------");
        #endif

        auto& LoadBalancing = json["Function"]["LoadBalancing"];
        LoadBanlace::enable = LoadBalancing["enable"].get<bool>();

        #if DEBUG_DURATION
            logger.Debug("LoadBalancing 开关: " + std::string(LoadBanlace::enable ? "开启" : "关闭")); 

            logger.Debug("---------LaunchBoost---------");
        #endif
        auto& LaunchBoost = json["Function"]["LaunchBoost"];
        LaunchBoost::enable = LaunchBoost["enable"].get<bool>();

        #if DEBUG_DURATION
            logger.Debug("LaunchBoost 开关: " + std::string(LaunchBoost::enable ? "开启" : "关闭"));

            logger.Debug("---------ActivityBoost---------");
        #endif
        auto& ActivityBoost = json["Function"]["ActivityBoost"];
        ActivityBoost::enable = ActivityBoost["enable"].get<bool>();

        #if DEBUG_DURATION
            logger.Debug("ActivityBoost 开关: " + std::string(ActivityBoost::enable ? "开启" : "关闭"));

            logger.Debug("---------DisableGpuBoost---------");
        #endif
        auto& DisableGpuBoost = json["Function"]["DisableGpuBoost"];
        DisableGpuBoost::enable = DisableGpuBoost["enable"].get<bool>();

        #if DEBUG_DURATION
            logger.Debug("DisableGpuBoost 开关: " + std::string(DisableGpuBoost::enable ? "开启" : "关闭"));

            logger.Debug("---------I/O优化---------");
        #endif

        auto& IOOptimization = json["Function"]["IO_Optimization"];
        IO_Optimization::enable = IOOptimization["enable"].get<bool>();
        IO_Optimization::scheduler = IOOptimization["scheduler"].get<string_t>();
        IO_Optimization::iostats = IOOptimization["iostats"].get<string_t>();
        IO_Optimization::nomerges = IOOptimization["nomerges"].get<string_t>();
        IO_Optimization::read_ahead_kb = IOOptimization["read_ahead_kb"].get<string_t>();

        #if DEBUG_DURATION
            logger.Debug("IO_Optimization 开关: " + std::string(IO_Optimization::enable ? "开启" : "关闭"));
            logger.Debug("scheduler: " + std::string(IO_Optimization::scheduler.c_str()));
            logger.Debug("iostats: " + std::string(IO_Optimization::iostats.c_str()));
            logger.Debug("nomerges: " + std::string(IO_Optimization::nomerges.c_str()));
            logger.Debug("read_ahead_kb: " + std::string(IO_Optimization::read_ahead_kb.c_str()));

            logger.Debug("---------调度器优化---------");
        #endif 
        auto& Scheduler = json["Function"]["Scheduler"];
        Scheduler::enable = Scheduler["enable"].get<bool>();
        Scheduler::Sched_energy_aware = Scheduler["sched_energy_aware"].get<bool>();
        Scheduler::Sched_schedstats = Scheduler["sched_schedstats"].get<bool>();
        Scheduler::Sched_latency_ns = Scheduler["sched_latency_ns"].get<string_t>();
        Scheduler::Sched_migration_cost_ns = Scheduler["sched_migration_cost_ns"].get<string_t>();
        Scheduler::Sched_min_granularity_ns = Scheduler["sched_min_granularity_ns"].get<string_t>();
        Scheduler::Sched_wakeup_granularity_ns = Scheduler["sched_wakeup_granularity_ns"].get<string_t>();
        Scheduler::Sched_nr_migrate = Scheduler["sched_nr_migrate"].get<string_t>();
        Scheduler::Sched_util_clamp_min = Scheduler["sched_util_clamp_min"].get<string_t>();
        Scheduler::Sched_util_clamp_max = Scheduler["sched_util_clamp_max"].get<string_t>();

        #if DEBUG_DURATION
            logger.Debug("Scheduler 开关: " + std::string(Scheduler::enable ? "开启" : "关闭"));
            logger.Debug("Sched_energy_aware: " + std::string(Scheduler::Sched_energy_aware ? "true" : "false"));
            logger.Debug("Sched_schedstats: " + std::string(Scheduler::Sched_schedstats ? "true" : "false"));
            logger.Debug("Sched_latency_ns: " + std::string(Scheduler::Sched_latency_ns.c_str()));
            logger.Debug("Sched_migration_cost_ns: " + std::string(Scheduler::Sched_migration_cost_ns.c_str()));
            logger.Debug("Sched_min_granularity_ns: " + std::string(Scheduler::Sched_min_granularity_ns.c_str()));
            logger.Debug("Sched_wakeup_granularity_ns: " + std::string(Scheduler::Sched_wakeup_granularity_ns.c_str()));
            logger.Debug("Sched_nr_migrate: " + std::string(Scheduler::Sched_nr_migrate.c_str()));
            logger.Debug("Sched_util_clamp_min: " + std::string(Scheduler::Sched_util_clamp_min.c_str()));
            logger.Debug("Sched_util_clamp_max: " + std::string(Scheduler::Sched_util_clamp_max.c_str()));

            logger.Debug("---------性能模式---------");
        #endif
        
        if (config.mode.empty()) {
            logger.Error("情景模式为空 无法读取数据");
            return false;
        }

        if (!switchConfig()) {
            logger.Error("情景模式异常 当前情景模式: " + config.mode);
            return false;
        }

        if (lastSwitchMode != config.mode && !lastSwitchMode.empty()) {
            logger.Info(std::string(lastSwitchMode) + " -> " + std::string(lastSwitchMode));
        }

        #if DEBUG_DURATION
            logger.Debug("当前性能模式: " + config.mode);
        #endif

        auto& Switch = json["Switch"][config.mode.c_str()];
        for (int i = 0; i <= 3; i++) {
            FastSnprintf(buff, sizeof(buff), "c%d", i);
            auto& MinFreq = Performances::MinFreq[i] = Switch["MinFreq"][buff].get<string_t>();
            auto& MaxFreq = Performances::MaxFreq[i] = Switch["MaxFreq"][buff].get<string_t>();
            auto& CpuGovernor = Performances::CpuGovernor[i] = Switch["governor"][buff].get<string_t>();
            if (MinFreq.empty() || MaxFreq.empty() || CpuGovernor.empty()) continue;
        #if DEBUG_DURATION
            logger.Debug("CPU簇 " + std::to_string(Policy::CpuPolicy[i]) + " MinFreq: " + std::string(Performances::MinFreq[i].c_str()));
            logger.Debug("CPU簇 " + std::to_string(Policy::CpuPolicy[i]) + " MaxFreq: " + std::string(Performances::MaxFreq[i].c_str()));
            logger.Debug("CPU簇 " + std::to_string(Policy::CpuPolicy[i]) + " CpuGovernor: " + std::string(Performances::CpuGovernor[i].c_str()));
        #endif
        }

        #if DEBUG_DURATION
            logger.Debug("---------核心管理---------");
        #endif

        for (int i = 0; i <= 7; i++) {
            FastSnprintf(buff, sizeof(buff), "Core%d", i);
            Performances::Online[i] = Switch["CoreOnline"][buff].get<int>();

            #if DEBUG_DURATION
                logger.Debug("CPU " + std::to_string(i) + " Online: " + std::to_string(Performances::Online[i]));
            #endif
        }

        #if DEBUG_DURATION
            logger.Debug("---------调速器参数---------");
        #endif 
        
        for (int i = 0; i <= 3; i++) {
            FastSnprintf(cluster, sizeof(cluster), "c%d", i);
            for (int j = 1; j <= 12; j++) {
                FastSnprintf(buff, sizeof(buff), "Path%d", j);

                auto& name = schedParam[i].Name[j] = Switch["SchedParam"][cluster][buff].get<string_t>();
                if (name.empty()) continue;

                FastSnprintf(buff, sizeof(buff), "value%d", j);
                schedParam[i].Value[j] = Switch["SchedParam"][cluster][buff].get<string_t>();

                #if DEBUG_DURATION
                    logger.Debug("CPU簇 " + std::to_string(Policy::CpuPolicy[i]) + " 调速器参数 " + std::to_string(j) + " 值: " + std::string(schedParam[i].Value[j].c_str()));
                    logger.Debug("CPU簇 " + std::to_string(Policy::CpuPolicy[i]) + " 调速器参数 " + std::to_string(j) + " 名称: " + std::string(schedParam[i].Name[j].c_str()));
                #endif
            }
        }
        
        return true;
    }
};