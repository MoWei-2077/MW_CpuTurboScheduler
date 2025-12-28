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

    char buff[256];
public:

    bool readConfig() {
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
        
        logger.setLogLevel(std::string(Meta::loglevel.c_str()));
        logger.Debug("---------源信息---------");
        logger.Info("名称: " + std::string(Meta::name.c_str()));
        logger.Info("版本: " + std::to_string(Meta::version));
        logger.Info("作者: " + std::string(Meta::author.c_str()));
        logger.Info("日志等级: " + std::string(Meta::loglevel.c_str()));
        logger.Info("---------CPU簇---------");

        for (int i = 0; i <= 3; i++) {
            FastSnprintf(buff, sizeof(buff), "c%d", i);
            if (json["Policy"][buff].get<int>() == 255) continue;
            Policy::CpuPolicy[i] = json["Policy"][buff].get<int>();
            logger.Debug("CPU簇 " + std::to_string(i) + " 的值为: " + std::to_string(Policy::CpuPolicy[i]));
        }

        logger.Debug("---------附加功能---------");
        auto& Cpuset = json["Function"]["Cpuset"];
        Function::Cpuset = Cpuset["enable"].get<bool>();
        Function::top_app = Cpuset["top_app"].get<string_t>();
        Function::foreground = Cpuset["foreground"].get<string_t>();
        Function::background = Cpuset["background"].get<string_t>();
        Function::system_background = Cpuset["system_background"].get<string_t>();
        Function::restricted = Cpuset["restricted"].get<string_t>();

        logger.Debug("Cpuset 开关: " + std::string(Function::Cpuset ? "开启" : "关闭"));
        logger.Debug("top_app: " + std::string(Function::top_app.c_str()));
        logger.Debug("foreground: " + std::string(Function::foreground.c_str()));
        logger.Debug("background: " + std::string(Function::background.c_str()));
        logger.Debug("system_background: " + std::string(Function::system_background.c_str()));
        logger.Debug("restricted: " + std::string(Function::restricted.c_str()));




        return true;
    }
};