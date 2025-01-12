#include <iostream>
class Config {
private:
    const std::string confpath = "/sdcard/Android/MW_CpuSpeedController/config.txt";
public:
    std::ifstream Getconfig() {
        std::ifstream file(confpath);
        return file;
    }
};