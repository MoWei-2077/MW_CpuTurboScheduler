#pragma once 

#include <iostream>
#include <fstream>
#include <string>
#include "Json/string.hpp"

using string_t = qlib::string_t;
using bool_t = qlib::bool_t;

using std::ifstream;

namespace Config {
    namespace Meta {
        string_t name;
        int version = -1;
        string_t author;
        string_t loglevel;
    }
    
    namespace Policy {
        int CpuPolicy [4] = { -1, -1, -1, -1 }; 
    }

    namespace Function {
        bool Cpuset = false;
        string_t top_app;
        string_t foreground;
        string_t background;
        string_t system_background;
        string_t restricted;
    }
}

class SwitchConfig {
private:
    static constexpr const char* configPath = "";
public:
    std::string mode;

    void LoadConfig() {
        ifstream file;
        std::string temp; 
        file.open(configPath);
        if (!file.is_open()) {  
            printf("无法打开配置文件: %s\n", configPath);
            return;
        }
        while (getline(file, temp)) mode = temp;
        file.close();
    }
}; 