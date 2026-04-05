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
    static constexpr const char* modePath = "/sdcard/Android/CTS/mode.txt";

    Logger logger;
    json_view_t json;

    char buff[256];
    char cluster [64];
public:
    SchedParam schedParam[4];
    std::string mode;

    void LoadConfig() {
        ifstream file;
        std::string temp; 
        file.open(modePath);
        if (!file.is_open()) { 
            fprintf(stderr, "无法打开配置文件: %s\n", modePath);
            return;
        }

        getline(file, temp);
        mode = std::move(temp);

        file.close();
    }

    bool switchConfig() const {
        if (mode == "powersave" || mode == "balance" || mode == "performance" || mode == "fast") return true;
        
        return false;
    }

    bool readConfig() {
        ifstream ifs(configPath, std::ios::binary);
        if (!ifs) {
            logger.Error("无法打开json配置文件");
            return false;
        }

        std::string text((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        int result = json::parse(&json, text.data(), text.data() + text.size());
        if (result != 0) {
            logger.Error("解析json配置文件失败 错误: %d", result);
            return false; 
        }

        try {
            Meta::name = json["meta"]["name"].get<string_t>();
            Meta::version = json["meta"]["version"].get<int>();
            Meta::author = json["meta"]["author"].get<string_t>();
            Meta::loglevel = json["meta"]["loglevel"].get<string_t>();
            
            #if DEBUG_DURATION
                logger.Debug("---------源信息---------");
                logger.Info("名称: %s", Meta::name.c_str());
                logger.Info("版本: %d", Meta::version);
                logger.Info("作者: %s", Meta::author.c_str());
                logger.Info("日志等级: %s", Meta::loglevel.c_str());
                logger.Info("---------CPU簇---------");
            #endif

            for (int i = 0; i <= 3; i++) {
                FastSnprintf(buff, sizeof(buff), "c%d", i);
                if (json["Policy"][buff].get<int>() == 255) continue;
                Policy::CpuPolicy[i] = json["Policy"][buff].get<int>();
                #if DEBUG_DURATION
                    logger.Debug("CPU簇 %d 的值为: %d", i, Policy::CpuPolicy[i]);
                #endif
            }
        } catch (const qlib::exception& e) {
            logger.Error("Meta节点异常 错误消息: %d", e.what());
        }

        try {
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
                logger.Debug("Cpuset 开关: %s", Cpuset::enable ? "开启" : "关闭");
                logger.Debug("top_app: %s", Cpuset::top_app.c_str());
                logger.Debug("foreground: %s", Cpuset::foreground.c_str());
                logger.Debug("background: %s", Cpuset::background.c_str());
                logger.Debug("system_background: %s", Cpuset::system_background.c_str());
                logger.Debug("restricted: %s", Cpuset::restricted.c_str());
            #endif

            auto& LaunchBoost = json["Function"]["LaunchBoost"];
            LaunchBoost::enable = LaunchBoost["enable"].get<bool>();
            LaunchBoost::boost_rate_limit_ms = LaunchBoost["boost_rate_limit_ms"].get<int>();
            for (int i = 0; i <= 3; i++) {
                FastSnprintf(buff, sizeof(buff), "c%d", i);
                auto& BoostFreq = LaunchBoost::BoostFreq[i] = LaunchBoost["BoostFreq"][buff].get<string_t>();
                if (BoostFreq.empty()) continue;
        
                #if DEBUG_DURATION
                    logger.Debug("LaunchBoost开关: %s", LaunchBoost::enable ? "开启" : "关闭");
                    logger.Debug("LaunchBoost升频持续时间: %d", LaunchBoost::boost_rate_limit_ms);
                    logger.Debug("LaunchBoost频率: %s", LaunchBoost::BoostFreq[i].c_str());
                #endif
            }

            auto& officialMode = json["Function"]["OfficialMode"];
            OfficialMode::enable = officialMode["enable"].get<bool>();
            #if DEBUG_DURATION
                logger.Debug("OfficialMode 开关: %s", OfficialMode::enable ? "开启" : "关闭"); 
            #endif
            
            auto& LoadBalancing = json["Function"]["LoadBalancing"];
            LoadBanlace::enable = LoadBalancing["enable"].get<bool>();

            #if DEBUG_DURATION
                logger.Debug("LoadBalancing 开关: %s", LoadBanlace::enable ? "开启" : "关闭"); 
            #endif

            auto& DisableGpuBoost = json["Function"]["DisableGpuBoost"];
            DisableGpuBoost::enable = DisableGpuBoost["enable"].get<bool>();

            #if DEBUG_DURATION
                logger.Debug("DisableGpuBoost 开关: %s", DisableGpuBoost::enable ? "开启" : "关闭");
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
                logger.Debug("Scheduler 总开关: %s", Scheduler::enable ? "开启" : "关闭");
                logger.Debug("Sched_energy_aware: %s", Scheduler::Sched_energy_aware ? "true" : "false");
                logger.Debug("Sched_schedstats: %s", Scheduler::Sched_schedstats ? "true" : "false");
                logger.Debug("Sched_latency_ns: %s", Scheduler::Sched_latency_ns.c_str());
                logger.Debug("Sched_migration_cost_ns: %s", Scheduler::Sched_migration_cost_ns.c_str());
                logger.Debug("Sched_min_granularity_ns: %s", Scheduler::Sched_min_granularity_ns.c_str());
                logger.Debug("Sched_wakeup_granularity_ns: %s", Scheduler::Sched_wakeup_granularity_ns.c_str());
                logger.Debug("Sched_nr_migrate: %s", Scheduler::Sched_nr_migrate.c_str());
                logger.Debug("Sched_util_clamp_min: %s", Scheduler::Sched_util_clamp_min.c_str());
                logger.Debug("Sched_util_clamp_max: %s", Scheduler::Sched_util_clamp_max.c_str());
            #endif
        } catch (const qlib::exception& e) {
            logger.Error("Function节点异常 错误消息: %d", e.what());
        }

        LoadConfig();

        if (mode.empty()) {
            logger.Error("情景模式为空 无法读取数据");
            return false;
        }
        
        if (!switchConfig()) {
            logger.Error("情景模式异常 当前情景模式: %s", mode.c_str());
            return false;
        }
        
        try {
            #if DEBUG_DURATION
                logger.Debug("当前性能模式: %s", mode.c_str());
            #endif

            auto& Switch = json["Switch"][mode.c_str()];
            for (int i = 0; i <= 3; i++) {
                FastSnprintf(buff, sizeof(buff), "c%d", i);
                auto& MinFreq = Performances::MinFreq[i] = Switch["MinFreq"][buff].get<string_t>();
                auto& MaxFreq = Performances::MaxFreq[i] = Switch["MaxFreq"][buff].get<string_t>();
                auto& CpuGovernor = Performances::CpuGovernor[i] = Switch["governor"][buff].get<string_t>();
                if (MinFreq.empty() || MaxFreq.empty() || CpuGovernor.empty()) continue;
                #if DEBUG_DURATION
                    logger.Debug("CPU簇: %d 最小频率: %s 最大频率: %s 调速器: %s", 
                        Policy::CpuPolicy[i], Performances::MinFreq[i].c_str(), 
                            Performances::MaxFreq[i].c_str(), Performances::CpuGovernor[i].c_str());
                #endif
            }

            #if DEBUG_DURATION
                logger.Debug("---------核心管理---------");
            #endif

            for (int i = 0; i <= 7; i++) {
                FastSnprintf(buff, sizeof(buff), "Core%d", i);
                Performances::Online[i] = Switch["CoreOnline"][buff].get<int>();

                #if DEBUG_DURATION
                    logger.Debug("核心: %d %s", i, Performances::Online[i] ? "开启" : "关闭");
                #endif
            }

            #if DEBUG_DURATION
                logger.Debug("---------调速器参数---------");
            #endif 
            
            for (int i = 0; i <= 3; i++) {
                FastSnprintf(cluster, sizeof(cluster), "c%d", i);
                for (int j = 1; j <= 12; j++) {
                    FastSnprintf(buff, sizeof(buff), "Path%d", j);
                    auto name = Switch["SchedParam"][cluster][buff].get<string_t>();
                    if(name.empty()) continue;

                    FastSnprintf(buff, sizeof(buff), "value%d", j);
                    auto value = Switch["SchedParam"][cluster][buff].get<string_t>();
                    if (value.empty()) continue;

                    schedParam[i].Name[j] = name;
                    schedParam[i].Value[j] = value;

                    #if DEBUG_DURATION
                        logger.Debug("CPU簇: %d 调速器参数: %d 名称: %s 值: %s", 
                            Policy::CpuPolicy[i], j, schedParam[i].Name[j].c_str(), 
                                schedParam[i].Value[j].c_str());
                    #endif
                }
            }
        } catch (const qlib::exception& e) {
            logger.Error("情景模式异常 错误消息: %d", e.what());
        }
        
        return true;
    }
};