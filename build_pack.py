from datetime import datetime
import subprocess
import random
import os

now = datetime.now()
data = now.strftime("%Y%m%d")
time = now.strftime("%Y %m %d: %H:%M:%S")
clang = "D:/Android-NDK/toolchains/llvm/prebuilt/windows-x86_64/bin/clang++.exe"
sysroot = "--sysroot=D:/Android-NDK/toolchains/llvm/prebuilt/windows-x86_64/sysroot"
cppFlags = "--target=aarch64-linux-android27 -std=c++23 -static -s -O3 -flto -fno-exceptions -ffast-math -funroll-loops -frtti -fexceptions -finline-functions -fomit-frame-pointer -Wall -Wextra -Wshadow -fPIE"
cppFlags_list = cppFlags.split()
prop = "./magisk/module.prop"
Project = os.path.dirname(os.path.abspath(__file__))
new_version_code = random.randint(20250308, 20991231)  

def log (LogMessage) :
    print(time, LogMessage)

def build ():
    command = [clang, sysroot] + cppFlags_list + ["-I.", Project + "/src/main.cpp", "-o", Project + "/magisk/MW_CpuSpeedController"]
    if (subprocess.run(command)) :
        log("Release 版本编译成功!")
    else :
        log("Release 版本编译失败!")

def pack ():
    with open(prop, "r", encoding="utf-8") as file:
        lines = file.readlines()


    with open(prop, "w", encoding="utf-8") as file:
        for line in lines:
            if line.startswith("versionCode="):
                file.write("versionCode" + "=" + data + "\n")  
            else:
                file.write(line)
    
build()
pack()