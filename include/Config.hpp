#pragma once 

#include <string>
#include <fstream>
#include <iostream>

using std::ifstream;
using std::string;


namespace Config {
    int log;
    string name;



}

class SwitchConfig {
private:
    static constexpr const char* configPath = "";
public:
    string mode;

    void LoadConfig() {
        ifstream file;
        string temp; 
        file.open(configPath);
        if (!file.is_open()) {  
            printf("无法打开配置文件: %s\n", configPath);
            return;
        }
        while (getline(file, temp)) Mode = temp;
        file.close();
    }
}; 