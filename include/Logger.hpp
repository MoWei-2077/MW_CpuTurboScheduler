#pragma once 

#include "Utils.hpp"
#include "LibUtils.hpp"

class Logger {
private:
    static constexpr const char* logpath = "/sdcard/Android/CTS/log.txt";

    constexpr static int LINE_SIZE = 1024 * 32;   //  32 KiB
    char lineCache[LINE_SIZE];

    LOG_LEVEL logLevel_ = LOG_LEVEL::INFO;
    mutex logPrintMutex;
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

    void setLogLevel(string_t& level) {
        if (level == "DEBUG") 
            logLevel_ = LOG_LEVEL::DEBUG;
        else if (level == "INFO") 
            logLevel_ = LOG_LEVEL::INFO;
        else if (level == "WARN") 
            logLevel_ = LOG_LEVEL::WARN;
        else if (level == "ERROR") 
            logLevel_ = LOG_LEVEL::ERROR;
    }

    void clear_log() {
        auto temp = fopen(logpath, "wb");
        if (!temp) {
            fprintf(stderr, "ERROR:清理日志文件失败");
            return;
        }
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

    int getCurrentTimeStr(char* buf, size_t size) {
        time_t now = time(nullptr);
        struct tm* local_time = localtime(&now);
        return strftime(buf, size, "%Y-%m-%d %H:%M:%S ", local_time);
    }

    void Log(LOG_LEVEL level, const char* message) {
        lock_guard<mutex> lock(logPrintMutex);

        if (level >= logLevel_) {
            int len = getCurrentTimeStr(lineCache, sizeof(lineCache));
            len += FastSnprintf(lineCache + len, sizeof(lineCache) - len, "%s %s\n", levelStrings.at(level), message);

            #if DEBUG_DURATION
                printf("%s\n", lineCache);
            #endif
            toFile(lineCache, len);
        }
    }

    template<typename... Args>
    void Log(LOG_LEVEL level, const char* message, Args&&... args) {
        lock_guard<mutex> lock(logPrintMutex);

        if (level >= logLevel_) {    
            int len = getCurrentTimeStr(lineCache, sizeof(lineCache));

            len += snprintf(lineCache + len, sizeof(lineCache) - len, message, std::forward<Args>(args)...) + len;
            lineCache[len++] = '\n';

            #if DEBUG_DURATION
                printf("%s\n", lineCache);
            #endif
            toFile(lineCache, len);
        }
    }


    inline static const unordered_map<LOG_LEVEL, const char*> levelStrings = {
        {LOG_LEVEL::DEBUG, "调试 ->"},
        {LOG_LEVEL::INFO, "信息 ->"},
        {LOG_LEVEL::WARN, "警告 ->"},
        {LOG_LEVEL::ERROR, "错误 ->"},
    };
};