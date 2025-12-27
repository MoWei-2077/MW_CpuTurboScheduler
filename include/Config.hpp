#pragma once 

#include <iostream>
#include <fstream>
#include <string>
#include "Json/string.hpp"

using string_t = qlib::string_t;
using std::ifstream;

namespace Config {
    int log;
    string_t name;
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