#pragma once

#include "LibUtils.hpp"
#include "Function.hpp"

class Schedule {
private:
    static constexpr const char* configPath = "/sdcard/Android/CTS/mode.txt";
    static constexpr const char* jsonPath = "/sdcard/Android/CTS/config.json";
    static constexpr const char* cpusetEventPath = "/dev/cpuset/top-app";
    static constexpr const char* onlinePath = "/sys/devices/system/cpu/cpu%d/online";
    static constexpr const char* SchedParamPath = "/sys/devices/system/cpu/cpufreq/policy%d/%s/%s";
    static constexpr const char* GovernorPath = "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor";
    static constexpr const char* MinFreqPath = "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq";
    static constexpr const char* MaxFreqPath = "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq";

    std::vector<thread> threads;

    Function function;
    JsonConfig conf;
    Logger logger;
    Utils utils;

    bool cpuBoost = false;
    
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
        FastSnprintf(temp, sizeof(temp), MinFreqPath, Policy);
        utils.FileWrite(temp, MinFreq);
        logger.Debug("CPU簇: %d 最小频率: %s", Policy, MinFreq.c_str());

        FastSnprintf(temp, sizeof(temp), MaxFreqPath, Policy);
        utils.FileWrite(temp, MaxFreq);
        logger.Debug("CPU簇: %d 最大频率: %s", Policy, MaxFreq.c_str());

        FastSnprintf(temp, sizeof(temp), GovernorPath, Policy);
        utils.FileWrite(temp, Governor);
        logger.Debug("CPU簇: %d 调速器: %s", Policy, Governor.c_str());
    }

    void release() {
        if (cpuBoost) 
            goto boost;
        else if (conf.mode == "fast" && OfficialMode::enable)
            goto reset;
        else 
            goto release;
boost:
        for (int i = 0; i <= 3; i++) {
            if (Policy::CpuPolicy[i] == -1) continue;
            FreqWriter(Policy::CpuPolicy[i], Performances::MinFreq[i], LaunchBoost::BoostFreq[i], Performances::CpuGovernor[i]);
            utils.sleep_ms(LaunchBoost::boost_rate_limit_ms);
            goto release;
        }  
        cpuBoost = false;
release: 
        for (int i = 0; i <= 3; i++) {
            if (Policy::CpuPolicy[i] == -1) continue;
            FreqWriter(Policy::CpuPolicy[i], Performances::MinFreq[i], Performances::MaxFreq[i], Performances::CpuGovernor[i]);
        } 
        function.FeasFunc(false);
reset: 
        for (int i = 0; i <= 3; i++) {
            if (Policy::CpuPolicy[i] == -1) continue;
            FreqWriter(Policy::CpuPolicy[i], "0", "2147483647", function.checkQcom() ? "walt" : "sugov_ext");
        } 
        function.FeasFunc(true);
    }

    void online() {
        for (int i = 0; i <= 7; i++) {
            FastSnprintf(temp, sizeof(temp), onlinePath, i);
            utils.WriteInt(temp, Performances::Online[i]);
            logger.Debug("核心: %d %s", i, Performances::Online[i] ? "开启" : "关闭");
        }
    }

    void SchedParam() {
        for (int i = 0; i <= 3; i++) {
            for (int j = 1; j <= 12; j++) {
                if (Policy::CpuPolicy[i] == -1 || conf.schedParam[i].Name[j].empty()) continue;
                FastSnprintf(temp, sizeof(temp), SchedParamPath, Policy::CpuPolicy[i], Performances::CpuGovernor[i].c_str(), conf.schedParam[i].Name[j].c_str());
                utils.FileWrite(temp, conf.schedParam[i].Value[j].c_str());
                logger.Debug("CPU簇: %d 调速器参数: %d 名称: %s 值: %d", Policy::CpuPolicy[i], j, conf.schedParam[i].Name[j].c_str(), conf.schedParam[i].Value[j].c_str());
            }
        }
    }

    void configTriggerTask() {
        sleep(2);
        while (true) {
            utils.InotifyMain(configPath, IN_MODIFY);
            conf.readConfig();
            release();
            SchedParam();
            online();
        }
    }

    void jsonTriggerTask() {
        sleep(2);
        while (true) {
            utils.InotifyMain(jsonPath, IN_MODIFY);
            conf.readConfig();
            function.AllFunC();
        }
    }

    void cpuSetTriggerTask() {
        if (!LaunchBoost::enable) return;
        constexpr int TRIGGER_BUF_SIZE = 8192;
        sleep(1);

        while (true) {
            const int fd = inotify_init();
            if (fd < 0) {
                fprintf(stderr, "同步事件: 0xB1 (1/3)失败: [%d]:[%s]", errno, strerror(errno));
                exit(-1);
            }

            const int wd = inotify_add_watch(fd, cpusetEventPath, IN_ALL_EVENTS);
            
            if (wd < 0) {
                fprintf(stderr, "同步事件: 0xB1 (2/3)失败: [%d]:[%s]", errno, strerror(errno));
                exit(-1);
            }

            logger.Info("监听顶层应用切换事件成功");

            char buf[TRIGGER_BUF_SIZE];
            while (read(fd, buf, TRIGGER_BUF_SIZE) > 0) {
                cpuBoost = true;
                release();
                logger.Debug("前台进程已切换 已触发LaunchBoost");
                utils.sleep_ms(500); // 防抖
            }

            inotify_rm_watch(fd, wd);
            close(fd);

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
            logger.Error("CTS调度已经在运行(pid: %s), 当前进程(pid:%d)即将退出", buf, getpid());
            printf("\n!!! \n!!! CTS调度已经在运行(pid: %s), 当前进程(pid:%d)即将退出 \n!!!\n\n", buf, getpid());
            exit(-1);
        }

        logger.clear_log();
        conf.readConfig();

        logger.setLogLevel(Meta::loglevel);
        logger.Info("名称: %s", Meta::name.c_str());
        logger.Info("版本: %d", Meta::version);
        logger.Info("作者: %s", Meta::author.c_str());
        logger.Info("日志等级: %s", Meta::loglevel.c_str());
        function.AllFunC();
        release();
        online();
        SchedParam();
    }
};