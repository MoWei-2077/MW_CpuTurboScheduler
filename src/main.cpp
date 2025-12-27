#include "JsonConfig.hpp"
#include "Logger.hpp"

std::string level = "DEBUG";
int main(void) {
    Logger logger;
    JsonConfig config;
    logger.setLogLevel(level);
    if (!config.readConfig()) {
        return -1;
    }
}