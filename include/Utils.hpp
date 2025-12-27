#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include <thread>
#include <string>
#include <vector>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdint>
#include <stdarg.h>
#include <stddef.h>
#include <chrono>
#include <cstdio>
#include <sys/inotify.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <string_view>
#include <unordered_map>
#include <functional>
#include <sys/system_properties.h>
#include <mutex>
#include <atomic>
#include <memory>
#include <format>
#include "LibUtils.hpp"
#include "Json/string.hpp"

#define MAX_PKG_LEN 128
#define MAX_THREAD_LEN 128
#define CPU_POLICY 8 

using namespace LibUtils;

using string_t = qlib::string_t;

using std::atomic;
using std::unordered_map;
using std::lock_guard;
using std::unique_ptr;
using std::ifstream;
using std::vector;
using std::string;
using std::string_view;
using std::thread;
using std::mutex;
using std::exception;
using std::make_unique;
using std::to_string;
using std::move;


class Utils {
private:
    static constexpr const char* thermalPath = "/sys/class/thermal";
    static constexpr int maxBucketSize = 32;
public:
    void FileWrite(const char* filePath, const char* content) noexcept {
        int fd = open(filePath, O_WRONLY | O_NONBLOCK, 0666);

        if (fd < 0) {
            chmod(filePath, 0666);
            fd = open(filePath, O_WRONLY | O_CREAT | O_NONBLOCK); 
        }

        if (fd >= 0) {
            write(fd, content, Faststrlen(content));
            close(fd);
            chmod(filePath, 0444);
        }
    }
    
    void FileWrite(const string& filePath, const string& content) noexcept {
        int fd = open(filePath.c_str(), O_WRONLY | O_NONBLOCK);

        if (fd < 0) {
            chmod(filePath.c_str(), 0666);
            fd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_NONBLOCK); 
        }

        if (fd >= 0) {
            write(fd, content.data(), content.size());
            close(fd);
            chmod(filePath.c_str(), 0444);
        }
    }

    void WriteFile(const char* filePath, const char* content) noexcept {
        int fd = open(filePath, O_WRONLY | O_TRUNC | O_CREAT, 0666); 

        if (fd >= 0) {
            write(fd, content, Faststrlen(content));
            close(fd);
        }
    }

    void InotifyMain(const char* dir_name, uint32_t mask) {
        int fd = inotify_init();
        if (fd < 0) {
            printf("Failed to initialize inotify.\n");
            exit(-1);
        }

        int wd = inotify_add_watch(fd, dir_name, mask);
        if (wd < 0) {
            printf("Failed to add watch for directory: %s",dir_name);
            close(fd);
            exit(-1);
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

            const struct inotify_event *event = reinterpret_cast<const struct inotify_event *>(buf);
            if (event->mask & mask) {
                break;
            }
        }

        inotify_rm_watch(fd, wd);
        close(fd);
    }

    void sleep_ms(const int ms) {
        usleep(1000 * ms);
    }

    bool exec(const char* cmd) {
        auto fp = popen(cmd, "r");
        if (fp == nullptr) return false;
        pclose(fp);
        return true;
    }

    int readInt(const char* path) noexcept {
        auto fd = open(path, O_RDONLY);
        if (fd < 0) return 0;

        char buff[16] = { 0 };
        auto len = read(fd, buff, sizeof(buff));
        close(fd);

        if (len <= 0) return 0;
    
        buff[15] = 0;
        return atoi(buff);
    }

    void WriteInt(const char* path, int value) noexcept {
        auto fd = open(path, O_WRONLY);
        if (fd < 0) {
            chmod(path,0666);
            fd = open(path, O_WRONLY);
        }

        char tmp[16];
        auto len = FastSnprintf(tmp, sizeof(tmp), "%d", value);
        write(fd, tmp, len);
        close(fd);
        chmod(path, 0444);
    }

    int getPid(const char* processName) {
        DIR *dir = opendir("/proc");
        if (dir == nullptr) {
            printf("ERROR:无法打开 /proc 目录\n");
            return -1; 
        }

        struct dirent* file;
        int pid = -1;

        while ((file = readdir(dir)) != nullptr) {
            if (file->d_name[0] < '0' || file->d_name[0] > '9') continue;
            char cmdlinePath[32] = "/proc/";
            char readBuff[256];
            size_t len = Faststrlen(file->d_name);

            memcpy(cmdlinePath + 6, file->d_name, len);
            memcpy(cmdlinePath + len + 6, "/cmdline", 9);


            if (readString(cmdlinePath, readBuff, sizeof(readBuff) - 1) <= 0) continue;
            const char *slash = strrchr(readBuff, '/');
            const char *proc_name = slash ? slash + 1 : readBuff;
            long base_len = Faststrlen(proc_name);

            if (base_len == Faststrlen(processName) && !memcmp(proc_name, processName, Faststrlen(processName))) {
                pid = Fastatoi(file->d_name);
                break;
            }
        }
        closedir(dir);
        return pid;
    }

    int getTid(const char* processName, const char* comm) {
        DIR *dir = opendir("/proc");
        if (dir == nullptr) {
            printf("ERROR: 无法打开 /proc 目录\n");
            return -1;
        }
    
        struct dirent* file;
        int tid = -1;
    
        while ((file = readdir(dir)) != nullptr) {
            if (file->d_name[0] < '0' || file->d_name[0] > '9') continue;
            char cmdlinePath[256] = "/proc/";
            size_t pid_len = strlen(file->d_name);
            memcpy(cmdlinePath + 6, file->d_name, pid_len);
            memcpy(cmdlinePath + 6 + pid_len, "/cmdline", 8); 
    
            cmdlinePath[14 + pid_len] = '\0';

            char readBuff[256];
            if (readString(cmdlinePath, readBuff, sizeof(readBuff)) <= 0) continue;
    
            const char *slash = strrchr(readBuff, '/');
            const char *proc_name = slash ? slash + 1 : readBuff;
    
            if (strcmp(proc_name, processName) != 0) continue;
    
            //int pid = atoi(file->d_name);
    
            char taskPath[256] = "/proc/";
            memcpy(taskPath + 6, file->d_name, pid_len);
            memcpy(taskPath + 6 + pid_len, "/task", 5);
            taskPath[12 + pid_len] = '\0';
    
            DIR *taskDir = opendir(taskPath);
            if (taskDir == nullptr) continue;
    
            struct dirent *taskFile;
            while ((taskFile = readdir(taskDir)) != nullptr) {
                if (taskFile->d_name[0] < '0' || taskFile->d_name[0] > '9') continue;

                char commPath[256] = "/proc/";
                memcpy(commPath + 6, file->d_name, pid_len);
                memcpy(commPath + 6 + pid_len, "/task/", 7);

                size_t tid_len = strlen(taskFile->d_name);
                memcpy(commPath + 12 + pid_len, taskFile->d_name, tid_len);
                memcpy(commPath + 12 + pid_len + tid_len, "/comm", 5); 

                commPath[18 + pid_len + tid_len] = '\0';

                char Comm[256];
                if (readString(commPath, Comm, sizeof(Comm)) <= 0) continue;
    
                Comm[strcspn(Comm, "\n")] = '\0';
    
                if (!strcmp(Comm, comm)) {
                    tid = atoi(taskFile->d_name);
                    printf("进程: %s 线程: %s PID: %s TID: %d)\n", processName, comm, file->d_name, tid);
                    closedir(taskDir);
                    closedir(dir);
                    return tid;
                }
            }
            closedir(taskDir);
        }
        closedir(dir);
        return tid;
    }

    int GetProperty(const char* key, char* res) {
        const prop_info* pi = __system_property_find(key); //如果频繁使用，建议缓存 对应Key的 prop_info
        if (pi == nullptr) {
            res[0] = 0;
            return -1;
        }

        __system_property_read_callback(pi,
            [](void* cookie, const char*, const char* value, unsigned) {
                if (value[0])
                    strncpy((char*)cookie, value, PROP_VALUE_MAX);
                else  ((char*)cookie)[0] = 0;
            },
            res);

        return res[0] ? 1 : -1;
    }


    int readFrequencies(const char* path, int frequencies[], int maxCount) noexcept {
        auto fd = open(path, O_RDONLY);
        if (fd < 0) return 0;

        char buff[1024];
        auto len = read(fd, buff, sizeof(buff) - 1);
        close(fd);
        if (len <= 0) return 0;

        buff[len] = '\0';
        
        int count = 0;
        for (char* token = strtok(buff, " \n\t"); token && count < maxCount; token = strtok(nullptr, " \n\t")) {
            int freq = atoi(token);
            if (freq > 0) frequencies[count++] = freq;
        }
        
        return count;
    }
    
    int openZonePath(const char* zoneName) {
        char path[256] = "/sys/class/thermal/";

        size_t zoneLen = strlen(zoneName);
        memcpy(path + 19, zoneName, zoneLen);
        memcpy(path + 19 + zoneLen, "/type", 6);

        auto fd = open(path, O_RDONLY);
        if (fd < 0) return -1;

        char buf[64];
        auto n = read(fd, buf, sizeof(buf) - 1);
        close(fd);
        
        if (n <= 0) return -1;
        
        buf[n - 1] = '\0';
        
        if (!checkSensorPath(buf)) return -1;

        memcpy(path + 19 + zoneLen, "/temp", 6);
        return open(path, O_RDONLY);
    }
    
    int readTemp(int fd) {
        char buf[32] = {0};

        lseek(fd, 0, SEEK_SET);
        auto n = read(fd, buf, sizeof(buf) - 1);
        close(fd);

        if (n <= 0) return 0;
        buf[31] = '\0';
        return atoi(buf);
    }


    int getMaxCpuTemp() {
        int maxTemp = -1;
        auto dir = opendir(thermalPath);

        if (dir == nullptr) {
            printf("无法打开文件夹:%s\n", thermalPath);
            return -1;
        }

        struct dirent* entry;
        while((entry = readdir(dir)) != nullptr) {
            if (!strncmp(entry->d_name, "thermal_zone", 12)) continue;    
            auto fd = openZonePath(entry->d_name);
            if (fd < 0) continue;
            int currentTemp = readTemp(fd);
            if (currentTemp > 0 && (maxTemp < 0 || currentTemp > maxTemp)) {
                maxTemp = currentTemp;
            }
        }

        closedir(dir);
        return (maxTemp != -1) ? maxTemp / 1000 : -1; // 舍去部分小数
    }

    void popenShell(const char* cmd, char* buf, size_t buf_size) {
        auto fp = popen(cmd, "r");
        if (!fp) return;
        while (fgets(buf, buf_size, fp) != nullptr)
        pclose(fp);
    }
    
    string getActivity() {
        char str[256];
        popenShell("dumpsys window | grep mCurrentFocus", str, sizeof(str));
        if (strstr(str, "mCurrentFocus=null")) return "null";
        const char* ptr = strstr(str, "/") + 1;
        const char* end_pos = strchr(ptr, '}');

        char activity[256];
        memcpy(activity, ptr, end_pos - ptr);
        activity[end_pos - ptr] = '\0';
        return string(activity);
    }

    string getTopApp() {
        char data[256];
        popenShell("dumpsys window | grep mCurrentFocus", data, sizeof(data));
        if (strstr(data, "mCurrentFocus=null")) return "null";
        const char* ptr = strstr(data, "u0") + 3;
        const char* end_pos = strchr(ptr, '/');

        char temp[256];
        memcpy(temp, ptr, end_pos - ptr);
        temp[end_pos - ptr] = '\0';
        return string(temp);
    }

    size_t popenRead(const char* cmd, char* buf) {
        auto fp = popen(cmd, "r");
        if (!fp) return 0;
        auto readLen = fread(buf, 1, sizeof(buf), fp);
        pclose(fp);
        return readLen;
    }
    
    size_t readString(const char* path, char* buff, const size_t maxLen) {
        auto fd = open(path, O_RDONLY);
        if (fd <= 0) {
            buff[0] = 0;
            return 0;
        }
        ssize_t len = read(fd, buff, maxLen);
        close(fd);
        if (len <= 0) {
            buff[0] = 0;
            return 0;
        }
        buff[len] = 0; 
        return (size_t)(len);
    }
private: 
    bool checkSensorPath(const char* str) {
        return strstr(str, "soc_max") != nullptr || strstr(str, "mtktscpu") != nullptr || strstr(str, "cpu-1-") != nullptr;
    }
};