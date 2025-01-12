#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <regex>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/system_properties.h>
#include <sys/inotify.h>
#include <sys/mount.h>

#include "INIreader.hpp"
#include "config.hpp"
std::mutex mtx;

class Utils {
private:
    const char* logpath = "/sdcard/Android/MW_CpuSpeedController/log.txt";
    const char* QcomFeas_path = "/sys/module/perfmgr/parameters/perfmgr_enable";
    const char* MTKFeas_path = "/sys/module/mtk_fpsgo/parameters/perfmgr_enable";
public:
    bool FileWrite(const char* filePath, const std::string content) noexcept {
        int fd = open(filePath, O_WRONLY | O_NONBLOCK);

        if (fd < 0) {
            chmod(filePath, 0666);
            fd = open(filePath, O_WRONLY | O_NONBLOCK); 
        }

        if (fd >= 0) {
            write(fd, content.data(), content.size());
            close(fd);
            chmod(filePath, 0444);
            return true;
        }
        return false;
    }
    void CpuSetWrite(const char* filePath, std::string content) noexcept {
        int fd = open(filePath, O_WRONLY | O_NONBLOCK);

        if (fd < 0) {
            chmod(filePath, 0666);
            fd = open(filePath, O_WRONLY | O_NONBLOCK); 
        }

        if (fd >= 0) {
            write(fd, content.data(), content.size());
            close(fd);
        }
    }
    void WriteFile(std::string filePath, std::string content) noexcept {
        int fd = open(filePath.c_str(), O_WRONLY | O_NONBLOCK);

        if (fd < 0) {
            chmod(filePath.c_str(), 0666);
            fd = open(filePath.c_str(), O_WRONLY | O_NONBLOCK); 
        }

        if (fd >= 0) {
            write(fd, content.data(), content.size());
            close(fd);
            chmod(filePath.c_str(), 0444);
        }
    }
    int InotifyMain(const char* dir_name) {
        int fd = inotify_init();
        if (fd < 0) {
            printf("Failed to initialize inotify.\n");
            return -1;
        }

        int wd = inotify_add_watch(fd, dir_name, IN_MODIFY);
        if (wd < 0) {
            printf("Failed to add watch for directory: %s\n", dir_name);
            close(fd);
            return -1;
        }

        const int buflen = sizeof(struct inotify_event) + NAME_MAX + 1;
        char buf[buflen];
        fd_set readfds;

        while (true) {
            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);

            int iRet = select(fd + 1, &readfds, nullptr, nullptr, nullptr);
            if (iRet < 0) {
                break;
            }

            int len = read(fd, buf, buflen);
            if (len < 0) {
                printf("Failed to read inotify events.\n");
                break;
            }

            const struct inotify_event* event = reinterpret_cast<const struct inotify_event*>(buf);
            if (event->mask & IN_MODIFY) {
                break;
            }
        }

        inotify_rm_watch(fd, wd);
        close(fd);

        return 0;
    }
    std::vector<std::vector<long>> readCpuStats() {
         std::ifstream file("/proc/stat");
        std::string line;
        std::vector<std::vector<long>> cpuStats(8, std::vector<long>(4));

        int core = 0;
        while (std::getline(file, line)) {
            if (line.find("cpu") == 0 && line.find("cpu ") != 0) { 
                std::istringstream iss(line);
                std::string cpu;
                iss >> cpu;
                for (int i = 0; i < 4; ++i) {
                    iss >> cpuStats[core][i];
                }
                core++;
                if (core >= 8) break;
            }
        }
        return cpuStats;
    }
    int calculateTotalCpuLoad(const std::vector<std::vector<long>>& stats1, const std::vector<std::vector<long>>& stats2) {
        long total1 = 0, idle1 = 0;
        long total2 = 0, idle2 = 0;

        for (const auto& coreStats : stats1) {
            total1 += coreStats[0] + coreStats[1] + coreStats[2] + coreStats[3];
            idle1 += coreStats[3];
        }
  
        for (const auto& coreStats : stats2) {
            total2 += coreStats[0] + coreStats[1] + coreStats[2] + coreStats[3];
            idle2 += coreStats[3];
        }

        long totalDiff = total2 - total1;
        long idleDiff = idle2 - idle1;

        if (totalDiff == 0) {
            return 0;
        }

        return static_cast<int>((totalDiff - idleDiff) * 100 / totalDiff);
    }
    void Init(){
        INIReader reader("/sdcard/Android/MW_CpuSpeedController/config.ini");
        log("配置文件:" + reader.Get("meta", "name", "Unknown"));
        log("作者:" + reader.Get("meta", "author", "Unknown"));
        log("版本号:" + std::to_string(reader.GetInteger("meta", "configVersion", 0)));
    }
    void initialization() {
        char buf[256] = { 0 };
        if (popenRead("pidof MW_CpuSpeedController", buf, sizeof(buf)) == 0) {
            log("进程检测失败");
            exit(-1);
        }

        auto ptr = strchr(buf, ' ');
        if (ptr) { // "pidNum1 pidNum2 ..."  如果存在多个pid就退出
            char tips[256];
            auto len = snprintf(tips, sizeof(tips),
                "警告: CS调度已经在运行 (pid: %s), 当前进程(pid:%d)即将退出", buf, getpid());
            printf("\n!!! \n!!! %s\n!!!\n\n", tips);
            printf(tips, len);
            exit(-2);
        }
    }
    void clear_log() {
        std::ofstream ofs;
        ofs.open(logpath, std::ofstream::out | std::ofstream::trunc);
        ofs.close();
    }
    void log(const std::string& message) {
        std::time_t now = std::time(nullptr);
        std::tm* local_time = std::localtime(&now);
        char time_str[100];
        std::strftime(time_str, sizeof(time_str), "[%Y-%m-%d %H:%M:%S]", local_time);

        std::ofstream logfile(logpath, std::ios_base::app);
        if (logfile.is_open()) {
            logfile << time_str << " " << message << std::endl;
            logfile.close();
        }
    }
    int getScreenProperty() {
        static const prop_info* pi = nullptr;

        if (pi == nullptr) {
            pi = __system_property_find("debug.tracing.screen_state");
            if (pi == nullptr) {
                return -1;
            }
        }

        char res[PROP_VALUE_MAX] = { 0 };
        __system_property_read_callback(pi,
            [](void* cookie, const char*, const char* value, unsigned) {
                if (value[0])
                    strncpy((char*)cookie, value, PROP_VALUE_MAX);
                else  ((char*)cookie)[0] = 0;
            },
            res);

        return res[0] ? res[0] - '0' : -1;
    }
    std::string exec(const std::string& command) {
        char buffer[128];
        std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) {
            std::cerr << "popen() failed!" << std::endl;
            return "";
        }
        
        if (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
            result = buffer; 
        }

        return result;  
    }
    size_t popenRead(const char* cmd, char* buf, const size_t maxLen) {
        auto fp = popen(cmd, "r");
        if (!fp) return 0;
        auto readLen = fread(buf, 1, maxLen, fp);
        pclose(fp);
        return readLen;
    }
    bool checkqcomFeas() {
        return (!access(QcomFeas_path, F_OK));
    }
    bool checkMTKFeas() {
        return (!access(MTKFeas_path, F_OK));
    }
    std::string getPids(std::vector<std::string>& processNames) {
        DIR* dir = opendir("/proc");
        if (dir == nullptr) {
            log("错误:无法打开/proc 目录");
            return "";
        }

        std::ostringstream Pids;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR && std::all_of(entry->d_name, entry->d_name + strlen(entry->d_name), ::isdigit)) {
                pid_t pid = static_cast<pid_t>(std::stoi(entry->d_name));
                std::string cmdlinePath = "/proc/" + std::string(entry->d_name) + "/cmdline";

                std::ifstream cmdlineFile(cmdlinePath);
                if (cmdlineFile) {
                    std::string cmdline;
                    std::getline(cmdlineFile, cmdline, '\0'); 
                    for (const auto& processName : processNames) {
                        if (cmdline.find(processName) != std::string::npos) {
                            Pids << pid << '\n';
                            break; 
                        }
                    }
                }
            }
        }
        closedir(dir);
        return Pids.str();
    }
    std::string getTids(const std::string& pids) {
        std::ostringstream Tids;
        std::istringstream iss(pids);
        std::string pid;

        while (std::getline(iss, pid, '\n')) {
            std::string taskDir = "/proc/" + pid + "/task";
            DIR* dir = opendir(taskDir.c_str());
            if (dir == nullptr) {
                log("错误:无法打开/proc/" + pid + "/task 目录");
                return "";
            }

            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_type == DT_DIR && std::all_of(entry->d_name, entry->d_name + strlen(entry->d_name), ::isdigit)) {
                    Tids << entry->d_name << '\n';
                }
            }
            closedir(dir);
        }
        return Tids.str();
    }
    std::string getCommAndTids(const std::string& pids, std::vector<std::string>& ThreadName) {
        std::ostringstream Tids;
        std::istringstream iss(pids);
        std::string pid;

        while (std::getline(iss, pid, '\n')) {
           std::string taskDir = "/proc/" + pid + "/task";
           DIR* dir = opendir(taskDir.c_str());
            if (dir == nullptr) {
                log("错误:无法打开 " + taskDir);
                continue;
           }

            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_type == DT_DIR && std::all_of(entry->d_name, entry->d_name + strlen(entry->d_name), ::isdigit)) {
                    std::string tid = entry->d_name;
                    std::string commPath = taskDir + "/" + tid + "/comm";
                    std::ifstream commFile(commPath);
                    if (!commFile) {
                        log("错误:无法打开 " + commPath);
                        continue;
                   }

                    std::string comm;
                    std::getline(commFile, comm);
                    if (std::find(ThreadName.begin(), ThreadName.end(), comm) != ThreadName.end()) {
                        Tids << tid << '\n';
                    }
                    
                }
            }
            closedir(dir);
        }
        return Tids.str();
    }
    std::string GetTopApp() {
        char buffer[128]; 
        const char* cmd = "am stack list | awk '/taskId/&&!/unknown/{print $2}' | awk 'NR==1 || NR==2' | awk -F '/' '{print $1}'";
    
        size_t maxLen = sizeof(buffer);
        popenRead(cmd, buffer, maxLen);

    
        std::string result(buffer);
        size_t pos = result.find('\n');
        if (pos != std::string::npos) {
            result = result.substr(0, pos);
        }
    
        return result;
    }
};