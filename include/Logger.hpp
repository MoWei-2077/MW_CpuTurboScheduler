#pragma once 

#include "Utils.hpp"
#include "LibUtils.hpp"

// 配置编译选项 *****************
#define DEBUG_DURATION 0 
// *****************************

enum class LOG_LEVEL : uint8_t { DEBUG, INFO, WARN, ERROR, UNKNOWN };

class Logger {
private:
    static constexpr const char* logpath = "/sdcard/Android/CTS/log.txt";
    inline static LOG_LEVEL logLevel_;
    inline static mutex logPrintMutex;
public:
    static void Debug(const char* message) {
        Log(LOG_LEVEL::DEBUG, message);
    } 

    static void Info(const char* message) {
        Log(LOG_LEVEL::INFO, message);
    }
    static void Warn(const char* message) {
        Log(LOG_LEVEL::WARN, message);
    }

    static void Error(const char* message) {
        Log(LOG_LEVEL::ERROR, message);
    }

    static void Debug(string message) {
        Log(LOG_LEVEL::DEBUG, message);
    } 

    static void Info(string message) {
        Log(LOG_LEVEL::INFO, message);
    }
    static void Warn(string message) {
        Log(LOG_LEVEL::WARN, message);
    }

    static void Error(string message) {
        Log(LOG_LEVEL::ERROR, message);
    }

    static void setLogLevel(string level) {
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

    void clear_log() {
        auto temp = fopen(logpath, "wb");
        if (!temp) {
            fprintf(stderr, "ERROR:清理日志文件失败");
            return;
        }
    }

private:
    static void toFile(const char* logStr, const int len) {
        auto fp = fopen(logpath, "ab");
        if (!fp) {
            fprintf(stderr, "日志输出(追加模式)失败 [%d][%s]", errno, strerror(errno));
            return;
        }
        fwrite(logStr, 1, len, fp);
        fclose(fp);
    }

    static int getCurrentTimeStr(char* buf, size_t size) {
        time_t now = time(nullptr);
        struct tm* local_time = localtime(&now);
        return strftime(buf, size, "%Y-%m-%d %H:%M:%S", local_time);
    }

    static void Log(LOG_LEVEL level, const char* message) {
        lock_guard<mutex> lock(logPrintMutex);

        if (level >= logLevel_) {
            char buff[200];
            int len = getCurrentTimeStr(buff, sizeof(buff));
            len += FastSnprintf(buff + len, sizeof(buff) - len, " %s %s\n", levelStrings.at(logLevel_), message);
            #if DEBUG_DURATION
                printf("%s\n", buff);
            #endif
            toFile(buff, len);
        }
    }

    static void Log(LOG_LEVEL level, const string message) {
        lock_guard<mutex> lock(logPrintMutex);

        if (level >= logLevel_) {
            char buff[200];
            int len = getCurrentTimeStr(buff, sizeof(buff));
            len += FastSnprintf(buff + len, sizeof(buff) - len, " %s %s\n", levelStrings.at(logLevel_), message.c_str());
            #if DEBUG_DURATION
                printf("%s\n", buff);
            #endif
            toFile(buff, len);
        }
    }

    inline static const unordered_map<LOG_LEVEL, const char*> levelStrings = {
        {LOG_LEVEL::DEBUG, "调试 ->"},
        {LOG_LEVEL::INFO, "信息 ->"},
        {LOG_LEVEL::WARN, "警告 ->"},
        {LOG_LEVEL::ERROR, "错误 ->"},
    };
};