#pragma once 

#include "LibUtils.hpp"
#include "Function.hpp"
#include "vpopen.hpp"

class Control{
private:
    static constexpr const char* confPath = "/sdcard/Android/CTS/perapp_powermode.txt";
    
    static constexpr const size_t GET_VISIBLE_BUF_SIZE = 256 * 1024;
    static constexpr int SDK_INT_VER = 36;
    unique_ptr<char[]> getVisibleAppBuff = make_unique<char[]>(GET_VISIBLE_BUF_SIZE);

    std::string curForegroundApp;

public:

/*
    通过提取使用指针计算':''/'的位置 来获取前台APP包名
    只要提取中间就行

    marble:/ # dumpsys activity lru
    ACTIVITY MANAGER LRU PROCESSES (dumpsys activity lru)
    Activities:
    #114: fg     TOP  LCMNFUAT 14234:com.android.settings/1000 act:activities|recents
    #113: vis    BFGS ---NFU-T 24553:com.miui.home/u0a134 act:activities|recents
    #112: vis    BFGS LCMNFUAT 4649:com.miui.miwallpaper/u0a168 act:client
    #111: vis    BFGS ---NFU-T 8066:com.miui.personalassistant/u0a127 act:client
    #109: prev   CEM  -------- 25390:com.tencent.mobileqq/u0a282 act:activities|recents
    #108: svc    CEM  -------- 26000:com.tencent.mobileqq:MSF/u0a282 act:client
    #107: cch+ 5 CEM  -------- 25082:com.tencent.mobileqq:video/u0a282 act:client
    #106: hvy    CEM  -------T 30870:com.tencent.mm/u0a281 act:activities|recents|client
    #105: hvy    CAC  -------- 17142:com.android.camera/u0a133 act:activities|recents
    #104: svc    CEM  -------T 28655:com.tencent.mm:push/u0a281 act:client

*/
    void init() {

    }

    void getVisibleAppByShellLRU() {

		const char* cmdList[] = { "/system/bin/dumpsys", "dumpsys", "activity", "lru", nullptr };
		VPOPEN::vpopen(cmdList[0], cmdList + 1, getVisibleAppBuff.get(), GET_VISIBLE_BUF_SIZE);

		stringstream ss;
		ss << getVisibleAppBuff.get();

		// 以下耗时仅 0.08-0.14ms, VPOPEN::vpopen 15-60ms
		std::string line;
		getline(ss, line);

        
        if (SDK_INT_VER >= 29) { //Android 11 Android 12+

			/* SDK 31-32-33
            Activities:
            #128: fg     TOP  LCMNFUAT 11721:com.tencent.mobileqq/u0a282 act:activities|recents
			*/
			auto getForegroundLevel = [](const char* ptr) {
				// const char level[][8] = {
				// // 0, 1,   2顶层,   3, 4常驻状态栏, 5, 6悬浮窗
				// "PER ", "PERU", "TOP ", "BTOP", "FGS ", "BFGS", "IMPF",
				// };
				// for (int i = 2; i < sizeof(level) / sizeof(level[0]); i++) {
				//   if (!strncmp(ptr, level[i], 4))
				//     return i;
				// }

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
		
					const size_t pkgLen = end - ptr;
				
					char pkgName[256];

					memcpy(pkgName, ptr + 1, pkgLen); // +1 跳过':' 字符串
					pkgName[pkgLen] = '\0'; // 必须手动复制 否则将多出一个'\'字符

                    curForegroundApp = pkgName;		
				}
			}
		}
	}
};