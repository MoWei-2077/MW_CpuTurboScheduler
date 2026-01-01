#pragma once

#include "Function.hpp"

class Schedule {
private:
    static constexpr const char* configPath = "/sdcard/Android/CTS/mode.txt";
    static constexpr const char* jsonPath = "/sdcard/Android/CTS/config.json";
    static constexpr const char* cpusetEventPath = "/dev/cpuset/top-app/cgroup.procs";
    static constexpr const char* onlinePath = "/sys/devices/system/cpu/cpu%d/online";
    static constexpr const char* SchedParamPath = "/sys/devices/system/cpu/cpufreq/policy%d/%s/%s";
    static constexpr const char* GovernorPath = "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor";
    static constexpr const char* MinFreqPath = "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq";
    static constexpr const char* MaxFreqPath = "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq";

    std::vector<thread> threads;

    Function function;
    JsonConfig config;
    Logger logger;
    Utils utils;

    mutex FreqWriterMutex;

    char temp[256];
public:
    Schedule& operator=(Schedule&&) = delete;

    Schedule() {
        Init();
        threads.emplace_back(thread(&Schedule::configTriggerTask, this));
        threads.emplace_back(thread(&Schedule::jsonTriggerTask, this));
        threads.emplace_back(thread(&Schedule::cpuSetTriggerTask, this));
    }

    void FreqWriter(const int Policy, const string_t MinFreq, const string_t MaxFreq, const string_t Governor) {
        lock_guard<mutex> lock(FreqWriterMutex);
        
        FastSnprintf(temp, sizeof(temp), MinFreqPath, Policy);
        utils.FileWrite(temp, MinFreq);

        logger.Debug("CPU簇" + std::to_string(Policy) + "最小频率: " + std::string(MinFreq.c_str()));

        FastSnprintf(temp, sizeof(temp), MaxFreqPath, Policy);
        utils.FileWrite(temp, MaxFreq);
        logger.Debug("CPU簇" + std::to_string(Policy) + "最大频率: " + std::string(MaxFreq.c_str()));

        FastSnprintf(temp, sizeof(temp), GovernorPath, Policy);
        utils.FileWrite(temp, Governor);
        logger.Debug("CPU簇" + std::to_string(Policy) + "调速器: " + std::string(Governor.c_str()));
    }

    void release() {
        for (int i = 0; i <= 3; i++) {
            if (Policy::CpuPolicy[i] == -1) continue;
            FreqWriter(Policy::CpuPolicy[i], Performances::MinFreq[i], Performances::MaxFreq[i], Performances::CpuGovernor[i]);
        }
    }

    void boost() {
        for (int i = 0; i <= 3; i++) {
            if (Policy::CpuPolicy[i] == -1) continue;
            FreqWriter(Policy::CpuPolicy[i], Performances::MinFreq[i], Performances::MaxFreq[i], Performances::CpuGovernor[i]);
        }
    }

    void online() {
        for (int i = 0; i <= 7; i++) {
            FastSnprintf(temp, sizeof(temp), onlinePath, i);
            utils.WriteInt(temp, Performances::Online[i]);
            logger.Debug("核心" + std::to_string(i) + ": " + (Performances::Online[i] ? "开启" : "关闭"));
        }
    }

    void SchedParam() {
        for (int i = 0; i <= 3; i++) {
            for (int j = 1; j <= 12; j++) {
                if (Policy::CpuPolicy[i] == -1 || config.schedParam[i].Name[j].empty()) continue;
                FastSnprintf(temp, sizeof(temp), SchedParamPath, Policy::CpuPolicy[i], Performances::CpuGovernor[i].c_str(), config.schedParam[i].Value[j].c_str());
                utils.FileWrite(temp, config.schedParam[i].Name[j].c_str());
                logger.Debug("CPU簇 " + std::to_string(Policy::CpuPolicy[i]) + " 调速器参数 " + std::to_string(j) + " 值: " + std::string(config.schedParam[i].Value[j].c_str()));
                logger.Debug("CPU簇 " + std::to_string(Policy::CpuPolicy[i]) + " 调速器参数 " + std::to_string(j) + " 名称: " + std::string(config.schedParam[i].Name[j].c_str()));
            }
        }
    }

    void configTriggerTask() {
        while (true) {
            utils.InotifyMain(configPath, IN_MODIFY);
            config.readConfig();
            release();
            SchedParam();
            online();
        }
    }

    void jsonTriggerTask() {
        while (true) {
            utils.InotifyMain(jsonPath, IN_MODIFY);
            config.readConfig();
            function.AllFunC();
        }
    }

    void cpuSetTriggerTask() {
        if (!LaunchBoost::enable) return;

        while (true) {
            constexpr int TRIGGER_BUF_SIZE = 8192;

            sleep(1);

            int inotifyFd = inotify_init();
            if (inotifyFd < 0) {
                fprintf(stderr, "同步事件: 0xB1 (1/3)失败: [%d]:[%s]", errno, strerror(errno));
                exit(-1);
            }

            
            int watch_d = inotify_add_watch(inotifyFd, cpusetEventPath, IN_MODIFY);
            
            if (watch_d < 0) {
                fprintf(stderr, "同步事件: 0xB1 (2/3)失败: [%d]:[%s]", errno, strerror(errno));
                exit(-1);
            }

            logger.Info("监听顶层应用切换事件成功");

            char buf[TRIGGER_BUF_SIZE];
            while (read(inotifyFd, buf, TRIGGER_BUF_SIZE) > 0) {
                // Boost
                break;
            }

            inotify_rm_watch(inotifyFd, watch_d);
            close(inotifyFd);

            fprintf(stderr, "已退出监控同步事件: 0xB0");
        }
    }

    void Init() {
        char buf[256] = { 0 };
        if (!utils.popenRead("pidof CpuTurboScheduler", buf, sizeof(buf))) {
            logger.Error("进程检测失败");
            exit(-1);
        }

        auto ptr = strchr(buf, ' ');

        if (ptr) {
            char tips[256];
            FastSnprintf(tips, sizeof(tips), 
            "CTS调度已经在运行(pid: %s), 当前进程(pid:%d)即将退出", buf, getpid());
            logger.Error(tips);
            printf("\n!!! \n!!! %s\n!!!\n\n", tips);
            exit(-1);
        }

        logger.clear_log();
        config.readConfig();

        logger.setLogLevel(std::string(Meta::loglevel.c_str()));
        logger.Info("名称: " + std::string(Meta::name.c_str()));
        logger.Info("版本: " + std::to_string(Meta::version));
        logger.Info("作者: " + std::string(Meta::author.c_str()));
        logger.Info("日志等级: " + std::string(Meta::loglevel.c_str()));
        function.AllFunC();
        release();
        SchedParam();
        online();
    }
};