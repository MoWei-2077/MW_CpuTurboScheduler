#pragma once
 
#include "Json/json.h"
#include "Config.hpp"
#include "Logger.hpp"

Logger logger;
using namespace Config;
using namespace qlib;

class JsonConfig {
private:
    json_view_t json;
    static constexpr const char* configPath = "/sdcard/Android/CTS/config.json";
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
            logger.Error("解析json配置文件失败 错误: %d", result);
            return false; 
        }
        name = json["meta"]["name"].get<string_t>();
        printf("名称: %s\n" , name.c_str());
        return true;
    }
};