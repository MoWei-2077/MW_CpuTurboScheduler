#pragma once 

#include "LibUtils.hpp"
#include "Function.hpp"
#include "vpopen.hpp"

class Control{
private:
    static constexpr const char* modePath = "/sdcard/Android/CTS/mode.txt";
    static constexpr const char* cpusetEventPath = "/dev/cpuset/top-app/cgroup.procs";
    static constexpr const char* confPath = "/sdcard/Android/CTS/perapp_powermode.txt";

	static constexpr const size_t GET_VISIBLE_BUF_SIZE = 256 * 1024;

	std::vector<thread> threads;
	unordered_map <std::string, std::string> modeConfig;
	unique_ptr<char[]> getVisibleAppBuff = make_unique<char[]>(GET_VISIBLE_BUF_SIZE);
	
	int SDK_INT_VER = -1;

    std::string curForegroundApp; // 一读一写 没必要使用互斥锁
	std::string defaultMode;       // 默认模式


	Utils utils;
	Logger logger;

	std::string HomePackage;

	char tmp[1024];
public:
	Control() {
		threads.emplace_back(thread(&Control::init, this));
		threads.emplace_back(thread(&Control::TopappMonitor, this));
		threads.emplace_back(thread(&Control::ConfigMonitor, this));	
	}

	void TopappMonitor() {
		// 后续考虑整合到一起
		sleep(2);
		while (true) {
			utils.InotifyMain(cpusetEventPath, IN_MODIFY); // 防止多余事件造成不必要的资源开销
			utils.sleep_ms(500); // 防抖
			getVisibleAppByShellLRU();
			WriteModes();
		}
	} 

	void ConfigMonitor() {
		sleep(2);
		while (true) {
			utils.InotifyMain(confPath, IN_MODIFY);
			updaterConfig();
		}
	}

    void init() {
		sleep(2); 

		SDK_INT_VER = __system_property_get("ro.build.version.sdk", tmp) > 0 ? Fastatoi(tmp) : 0;
		if (SDK_INT_VER < 29) {
			logger.Error("安卓SDK版本过低 初始化失败");
			return;
		}
		logger.Info("调度控制器初始化成功");
		updaterConfig();
		getHomePackageByShell();
	}

	void WriteModes() {
		auto it = modeConfig.find(curForegroundApp);
		std::string newMode;

		if (it != modeConfig.end()) 
			newMode = it->second;                  	                    
		else if (!defaultMode.empty()) 
			newMode = defaultMode;          	
		else 
			return;                                   

		utils.WriteFile(modePath, newMode.c_str());
		logger.Debug("调度控制器: 已切换 " + newMode + " 模式");
	}

	size_t updaterConfig() {
		ifstream file;
		std::string line;
		file.open(confPath);

		if (!file.is_open()) { 
            fprintf(stderr, "无法打开配置文件: %s\n", confPath);
            return -1;
        }

		if (!modeConfig.empty()) modeConfig.clear(); 
		defaultMode.clear();

		char package[256] = {};
    	char mode[256] = {};

        while (getline(file, line)) {
			if (line.starts_with("#") || line.empty()) continue;

			if (line.starts_with("*")) {
				const char* ptr = strchr(line.c_str(), '*') + 2;
				if (!ptr) continue;

				size_t len = strcspn(ptr, "\r\n");          
				defaultMode.assign(ptr, len);                
				logger.Info("默认模式: " + defaultMode);
				continue;
			}

			sscanf(line.c_str(), "%s %s", package, mode);
			if ((strlen(package) == 0) || (strlen(mode) == 0)) {
                logger.Error("调度控制器配置文件异常 请检查");
                return -1;
			}

			modeConfig.emplace(std::string(package), std::string(mode));
		}

        file.close();
		logger.Info("更新配置 " + std::to_string(modeConfig.size()));
		return modeConfig.size() > 0;
	}

	void getHomePackageByShell() {
		const char* cmdShell = "pm  resolve-activity --brief -c android.intent.category.HOME -a android.intent.action.MAIN | grep '/'";
		const char* cmdList[] = { "/system/bin/sh", "sh", "-c", cmdShell, nullptr };
		VPOPEN::vpopen(cmdList[0], cmdList + 1, getVisibleAppBuff.get(), GET_VISIBLE_BUF_SIZE);
		stringstream ss;
		ss << getVisibleAppBuff.get();
		std::string line;
		getline(ss, line);

		const char* ptr = strchr(line.c_str(), '/');
		if (!ptr) return;

		std::string temp(line.c_str(), ptr);

		if (temp.empty()) return;
		logger.Info("已获取系统桌面: " + temp);
		HomePackage = std::move(temp);	
	}

    void getVisibleAppByShellLRU() {
		const char* cmdList[] = { "/system/bin/dumpsys", "dumpsys", "activity", "lru", nullptr };
		VPOPEN::vpopen(cmdList[0], cmdList + 1, getVisibleAppBuff.get(), GET_VISIBLE_BUF_SIZE);

		stringstream ss;
		ss << getVisibleAppBuff.get();

		// 以下耗时仅 0.08-0.14ms, VPOPEN::vpopen 15-60ms
		std::string line;
		getline(ss, line);

		/* SDK 31-32-33
		Activities:
		#128: fg     TOP  LCMNFUAT 11721:com.tencent.mobileqq/u0a282 act:activities|recents
		*/
		auto getForegroundLevel = [](const char* ptr) {
			constexpr uint32_t levelInt[7] = { 0x20524550, 0x55524550, 0x20504f54, 0x504f5442,
												0x20534746, 0x53474642, 0x46504d49 };
			const uint32_t target = *((uint32_t*)ptr);
			for (int i = 2; i < 7; i++) {
				if (target == levelInt[i])
					return i;
			}
			return 16;
		};

		int offset = SDK_INT_VER == 29 ? 5 : 3; // 行首 空格加#号 数量
		auto startStr = SDK_INT_VER == 29 ? "    #" : "  #";
		getline(ss, line);
		if (!strncmp(line.c_str(), "  Activities:", 4)) {
			while (getline(ss, line)) {
				// 此后每行必需以 "  #"、"    #" 开头，否则就是 Service: Other:需跳过
				if (strncmp(line.c_str(), startStr, offset)) break;

				auto linePtr = line.c_str() + offset; // 偏移已经到数字了

				auto ptr = linePtr + (linePtr[2] == ':' ? 11 : 12); //11: # 1 ~ 99   12: #100+
				int level = getForegroundLevel(ptr);
				if (level != 2) continue;
				ptr = strchr(line.c_str(), ':');
				ptr = strstr(ptr + 1, ":");

				if (!ptr) continue;

				const char* end = strchr(ptr, '/');
				if (!end) continue;
	
				const size_t pkgLen = end - (ptr + 1);
			
				char pkgName[256];

				memcpy(pkgName, ptr + 1, pkgLen); // +1 跳过':' 字符串
				
				pkgName[pkgLen] = '\0'; 
				
				if (strcmp(pkgName, HomePackage.c_str()) == 0) continue;
				curForegroundApp = pkgName;	
				logger.Debug("前台应用:" + curForegroundApp);	
				break;
			}
		}	
	}
};