#include "JsonConfig.hpp"
#include "Logger.hpp"


int main(void) {
    Logger logger;
    JsonConfig config;
    logger.clear_log();
    if (!config.readConfig()) {
        return -1;
    }
}