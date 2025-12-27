#pragma once 

#include "Utils.hpp"
#include "LibUtils.hpp"

enum class LOG_LEVEL : uint8_t { DEBUG, INFO, WARN, ERROR, UNKNOWN };

namespace Logger {
    LOG_LEVEL logLevel_;
    mutex logPrintMutex;

    constexpr const char* logpath = "";


    void Debug(const char* message) {
        log(LOG_LEVEL::DEBUG, message);
    } 

    void Info(const char* message) {
        log(LOG_LEVEL::INFO, message);
    }

    void Warn(const char* message) {
        log(LOG_LEVEL::WARN, message);
    }

    void Error(const char* message) {
        log(LOG_LEVEL::ERROR, message);
    }

    void setLogLevel(string& level) {
        if (level == "DEBUG") 
            logLevel_ = LOG_LEVEL::DEBUG;
        else if (level == "INFO") 
            logLevel_ = LOG_LEVEL::INFO;
        else if (level == "WARN") 
            logLevel_ = LOG_LEVEL::WARN;
        else if (level == "ERROR") 
            logLevel_ = LOG_LEVEL::ERROR;
        else 
            logLevel_ = LOG_LEVEL::UNKNOWN;
    }

    template<typename... Args>
    void log(LOG_LEVEL level, const char* message, Args&&... args) {
        lock_guard<mutex> lock(logPrintMutex);
        static const unordered_map<LOG_LEVEL, const char*> levelStrings = {
            {LOG_LEVEL::ERROR, " [ERROR] "},
            {LOG_LEVEL::WARN, " [WARN] "},
            {LOG_LEVEL::INFO, " [INFO] "},
            {LOG_LEVEL::DEBUG, " [DEBUG] "},
        };

        if (level <= logLevel_) {
            time_t now = time(nullptr);
            struct tm* local_time = localtime(&now);
            char time_str[100];
            char buff[200];
            strftime(time_str, sizeof(time_str), "[%Y-%m-%d %H:%M:%S]", local_time);
            FastSnprintf(buff, sizeof(buff), "%s %s %s %d\n", time_str, levelStrings.at(level), message, std::forward<Args>(args)...);
            toFile(buff, Faststrlen(buff));
        }
    }

    void toFile(const char* logStr, const int len) {
        auto fp = fopen(logpath, "ab");
        if (!fp) {
            fprintf(stderr, "日志输出(追加模式)失败 [%d][%s]", errno, strerror(errno));
            return;
        }
        fwrite(logStr, 1, len, fp);
        fclose(fp);
    }

    void clear_log() {
        auto temp = fopen(logpath, "wb");
        if (!temp) {
            fprintf(stderr, "ERROR:清理日志文件失败");
            return;
        }
    }
};