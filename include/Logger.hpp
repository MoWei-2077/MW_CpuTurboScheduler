#pragma once 

#include "Utils.hpp"
#include "LibUtils.hpp"

enum class LOG_LEVEL : uint8_t { DEBUG, INFO, WARN, ERROR, UNKNOWN };

class Logger {
private:
    LOG_LEVEL logLevel_;
    mutex logPrintMutex;
    static constexpr const char* logpath = "/sdcard/Android/CTS/log.txt";
public:
    void Debug(const char* message) {
        Log(LOG_LEVEL::DEBUG, message);
    } 

    void Info(const char* message) {
        Log(LOG_LEVEL::INFO, message);
    }
    void Warn(const char* message) {
        Log(LOG_LEVEL::WARN, message);
    }

    void Error(const char* message) {
        Log(LOG_LEVEL::ERROR, message);
    }

    template<typename... Args>
    void Debug(const char* message, Args&&... args) {
        Log(LOG_LEVEL::DEBUG, message, std::forward<Args>(args)...);
    } 

    template<typename... Args>
    void Info(const char* message, Args&&... args) {
        Log(LOG_LEVEL::INFO, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Warn(const char* message, Args&&... args) {
        Log(LOG_LEVEL::WARN, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Error(const char* message, Args&&... args) {
        Log(LOG_LEVEL::ERROR, message, std::forward<Args>(args)...);
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

private:
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

    
    void Log(LOG_LEVEL level, const char* message) {
        lock_guard<mutex> lock(logPrintMutex);
        static const unordered_map<LOG_LEVEL, const char*> levelStrings = {
            {LOG_LEVEL::ERROR, " [ERROR]:"},
            {LOG_LEVEL::WARN, " [WARN]:"},
            {LOG_LEVEL::INFO, " [INFO]:"},
            {LOG_LEVEL::DEBUG, " [DEBUG]:"},
        };

        if (level >= logLevel_) {
            time_t now = time(nullptr);
            struct tm* local_time = localtime(&now);
            char time_str[100];
            char buff[200];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
            FastSnprintf(buff, sizeof(buff), "%s %s %s \n", time_str, levelStrings.at(level), message);
            toFile(buff, Faststrlen(buff));
        }
    }

    template<typename... Args>
    void Log(LOG_LEVEL level, const char* message, Args&&... args) {
        lock_guard<mutex> lock(logPrintMutex);
        static const unordered_map<LOG_LEVEL, const char*> levelStrings = {
            {LOG_LEVEL::ERROR, " [ERROR]:"},
            {LOG_LEVEL::WARN, " [WARN]:"},
            {LOG_LEVEL::INFO, " [INFO]:"},
            {LOG_LEVEL::DEBUG, " [DEBUG]:"},
        };

        if (level >= logLevel_) {
            time_t now = time(nullptr);
            struct tm* local_time = localtime(&now);
            char time_str[100];
            char buff[200];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
            FastSnprintf(buff, sizeof(buff), "%s %s %s %s\n", time_str, levelStrings.at(level), message, std::forward<Args>(args)...);
            toFile(buff, Faststrlen(buff));
        }
    }
};