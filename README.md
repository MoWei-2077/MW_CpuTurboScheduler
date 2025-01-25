# CpuTurboScheduler V2.0
[![C++](https://img.shields.io/badge/language-C++-%23f34b7d.svg?style=plastic)](https://en.wikipedia.org/wiki/C++)
[![Android](https://img.shields.io/badge/platform-Android-0078d7.svg?style=plastic)](https://en.wikipedia.org/wiki/Android_(operating_system)) 
[![AArch64](https://img.shields.io/badge/arch-AArch64-red.svg?style=plastic)](https://en.wikipedia.org/wiki/AArch64)
[![Android Support8-15](https://img.shields.io/badge/Android%208~15-Support-green)](https://img.shields.io/badge/Android%208~15-Support-green)
#### 介绍
CPU Turbo Scheduler 是一款基于 C++ 编写的智能 CPU 调度工具 旨在优化 Android 设备的 CPU 性能和功耗表现而设计 通过智能调度算法 它可以根据不同的使用场景动态调整 CPU 频率以达到最佳的性能和能效平衡 <br>
#### 工作条件
1.目前该调度适用于Android8-15 <br>
2.拥有Root权限

#### 修改启动时的默认模式
1.打开/sdcard/Android/MW_CpuSpeedController/config.txt <br>
2.可选择的模式包括 powersave（省电）、balance（均衡）、performance（性能）和 fast（极速）<br>
3.重启后查看/sdcard/Android/MW_CpuSpeedController/log.txt检查CPU Turbo Scheduler是否正常自启动

#### 情景模式切换
powersave（省电模式）：在保证基本流畅度的同时，尽可能降低功耗。推荐日常使用 <br>
balance（均衡模式）：比原厂设置更流畅且更省电。推荐日常使用 <br>
performance（性能模式）：在保证较高流畅度的同时，可能会增加功耗。推荐游戏使用 <br>
fast（极速模式）：全力保证游戏时的流畅度，忽略能效比 

## 负载采样
通过对 /proc/stat 进行负载采样 获取以 jiffies（时钟滴答）为单位的负载 并通过计算转换为百分比负载 

## 监听前台APP切换
通过使用 inotify 对 /dev/cpuset/top-app/cgroup.procs 进行监听来获取顶层APP的切换 

#### 公式说明：
声明变量：
- cerrFreq：当前频率
- cerrLoad：当前负载
- Margin：余量
- ReferenceFreq：基准频率
- MaxFreq：最大频率
- LoadBoostValue：Boost频率的临界值 <br>
公式:<br>
LoadBoostValue=(cerrFreq/cerrLoad)+Magrin <br>
当 LoadBoostValue > ReferenceFreq 且 LoadBoostValue < MaxFreq 时，进行频率提升 <br>
注意:ReferenceFreq为基准频率并不是最小频率！！！

## 常见问题
Q：是否会对待机功耗产生负面影响？ <br>
A：CPU Turbo Scheduler 做了低功耗优化 由于使用了 C++ 语言 自身运行功耗很低 并不会对设备的待机功耗产生显著影响   <br>
 
Q：为什么使用了 CPU Turbo Scheduler 后功耗仍然很高？  <br> 
A：SOC 的 AP 部分功耗主要取决于计算量和使用的频点 CPU Turbo Scheduler 只能通过控制性能释放和改进频率的方式来降低功耗 如果后台应用的计算量很大 可能无法显著延长续航时间 可以通过 Scene 工具箱的进程管理器来定位问题  <br>

Q：何时更新 XXXX 版本？  <br>
A：如果您觉得有需要更新的内容，请发送至邮箱：mowei2077@gmail.com   <br>

Q：如何确保我的设备拥有并支持 Feas特性？  <br>
A：开启 CPU Turbo Scheduler 的 Feas 开关并切换到极速模式 CPU Turbo Scheduler将会自动识别内核的 Feas 接口 如果设备没有 Feas 功能接口 将会在日志中抛出错误 目前CPU Turbo Scheduler 已接入大多数内核的 Feas 接口  <br>

Q：是否还需要关闭系统的performance boost？  <br>
A：CPU Turbo Scheduler在初始化阶段就已经关闭了大部分主流的用户态和内核态升频 如果有非常规的升频需要用户自己关闭   <br>

Q：CPU Turbo Scheduler和Scene工具箱是什么关系？  <br>
A：这两个软件独立运作没有互相依赖 CPU Turbo Scheduler实现了接口可供Scene工具箱调用 例如性能模式切换以及分APP性能模式 如果不安装Scene工具箱也可以实现性能模式切换 <br>

Q：为什么在使用Scene工具箱接管CPU Turbo Scheduler时开机需要重新开启开关 这是否意味着CPU Turbo Scheduler没有正常工作？  <br>
A：CPU Turbo Scheduler在系统解锁后就会正常自启动 CPU Turbo Scheduler不依赖于Scene工具箱 至于开机时需要重新打开调度开关请询问Scene工具箱的开发人员<br>

Q：为什么在使用Scene工具箱接管CPU Turbo Scheduler后 会出现一堆切换模式的日志？  <br>
A：因为Scene工具箱会一直监听屏幕是否亮屏和息屏 当亮屏时Scene工具箱会切换一次模式 CPU Turbo Scheduler监听到模式更改后就会输出一次日志并写入一些相关的参数 PS:我个人认为这样会造成不必要的性能资源消耗 所以我本人并不推荐大家使用Scene工具箱去接管任何调度 <br>

## 配置文件说明
### （一）元信息（meta）

```ini
[meta]
name = "骁龙7+ Gen2"
author = MoWei
configVersion = 5
loglevel = "INFO"
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| name     | string   | 配置文件的名称，通常为设备型号                                 |
| author   | string   | 配置文件的作者信息                             |
| configVersion | string   | 配置文件的版本号 |
| loglevel | string   | 日志等级，可选值为 Debug、INFO、Warning、Error |

### (二) 附加功能 （function）
```ini
[function]
DisableQcomGpu = false
AffintySetter = false
cpuctl = false
ufsClkGate = false
CpuIdleScaling_Governor = false
EasScheduler = false
cpuset = false
LoadBalancing = false
EnableFeas = false
AdjIOScheduler = false

```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| DisableQcomGpu | bool   | 禁用高通 GPU Boost 防止 GPU 频率无序升高 | 
| AffintySetter | bool   | 对系统和传感器关键进程和线程进行绑核操作 |
| cpuctl | bool   | CPU 使用率控制功能 |
| ufsClkGate | bool   | 关闭UFS 时钟门功能（性能模式和极速模式下关闭 UFS 时钟门） |
| CpuIdleScaling_Governor | bool   | 自定义 CPUIdle 调度器。 |
| EasScheduler | bool   | EAS 调度器优化 |
| cpuset | bool | CPUSet功能 调整应用的核心分配 |
| LoadBalancing | bool | 通过优化CFS调度器的参数达到负载均衡的效果 |
| EnableFeas | bool |  FEAS 功能（仅限极速模式）|
| AdjIOScheduler | bool |  I/O 调度器调整以及I/O优化总开关 |

### （三）核心分配 (CoreAllocation）
```ini
[CoreAllocation]
cpusetCore = "4-7"
cpuctlUclampBoostMin = "10"
cpuctlUclampBoostMax = "70"
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| cpusetCore | string   | 指定 CPUSet核心 用于核心分配 |
| cpuctlUclampBoostMin | string   | CPU使用率控制的最小值 |
| cpuctlUclampBoostMax | string   | CPU使用率控制的最大值 |

### （四）I/O 设置（IO_Settings）
```ini
[IO_Settings]
Scheduler = "ssg"
IO_optimization = false
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| Scheduler | string   | 指定 I/O 调度器类型，如 ssg、bfq 等 |
| IO_optimization | bool   | 启用 I/O 优化功能 |

### （五）EAS 调度器参数（EasSchedulerVaule）
```ini
[EasSchedulerVaule]
sched_min_granularity_ns = "2000000" 
sched_nr_migrate = "30"
sched_wakeup_granularity_ns = "3200000"
sched_schedstats = "0"
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| sched_min_granularity_ns | string   | EAS 调度器的最小调度粒度 单位为纳秒（ns） |
| sched_nr_migrate | string   | 控制任务在 CPU 核心之间迁移的次数 |
| sched_wakeup_granularity_ns | string   | EAS 调度器调整任务唤醒时间的粒度 单位为纳秒（ns） |
| sched_schedstats | string   | 是否启用调度统计信息收集(0表示禁用) |

### （六）CpuIdle 调度器（CpuIdle）
```ini
[CpuIdle]
current_governor = "qcom-cpu-lpm"
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| current_governor | string   | 设置使用的 CpuIdle 调度器模式 如高通推荐:qcom-cpu-lpm 联发科推荐:menu |

### （七）CPUSet 配置（Cpuset）
```ini
[Cpuset]
top_app = "0-7"
foreground = "0-7"
restricted = "0-5"
system_background = "1-2"
background = "0-2"
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| top_app | string   | 顶层应用可使用的 CPU 核心范围 |
| foreground | string   | 前台应用可使用的 CPU 核心范围 |
| restricted | string   | 前台任务加速时可使用的 CPU 核心范围 |
| system_background | string   | 系统后台进程可使用的 CPU 核心范围 |
| backgroundd | string   | 后台进程可使用的 CPU 核心范围 |

###  (八)功耗模型开发 (这里使用性能模式举例)
```ini
[performance]
LoadBoost = false
RefreshTopAppBoost = true
Margin = "300000"
up_rate_limit_ms = "3000"
down_rate_limit_ms = "1000"
modelType0 = "policy0"
ReferenceFreq0 = "1804000"
BoostFreq0 = "1804000"
MaxFreq0 = "1804000"
modelType1 = "policy4"
ReferenceFreq1 = "2496000"
BoostFreq1 = "2496000"
MaxFreq1 = "2496000"
modelType2 = "policy7"
ReferenceFreq2 = "2476000"
BoostFreq2 = "2702000" 
MaxFreq2 = "2702000"
scaling_governor = "walt"
UclampTopAppMin = "0"
UclampTopAppMax = "100"
UclampTopApplatency_sensitive = true
UclampForeGroundMin = "0"
UclampForeGroundMax = "80"
UclampBackGroundMin = "0"
UclampBackGroundMax =  "50"
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| LoadBoost | bool   | 负载升频 |
| RefreshTopAppBoost | bool   | 应用冷、热启动升频 |
| Margin | string   | 负载升频的余量（单位：Hz） |
| up_rate_limit_ms | int   | 下一次升频的间隔时间（单位：毫秒） |
| down_rate_limit_ms | int   | 下一次降频的间隔时间（单位：毫秒） |
| modelTypeX | string   | CPU X 簇的定义（例如 policy0、policy4 等） |
| ReferenceFreqX | string   | CPU X 簇的常规最大频率（单位：Hz）） |
| BoostFreqX | string   | CPU X 簇在负载达到临界值时提升到的频率（单位：Hz） |
| MaxFreqX | string   | CPU X 簇的最大频率（单位：Hz），用于负载采样） |
| scaling_governor | string   | CPU0-7核心的调速器模式 |
| UclampTopAppMin | string   | 用于设置顶层APP可使用的CPU频率下限 （范围0-100）|
| UclampTopAppMax | string   | 用于设置顶层APP可使用的CPU频率上限 （范围0-100）|
| UclampTopApplatency_sensitive | bool   | 标记应用或进程对延迟敏感性的参数 设置此参数可以告知调度器前台应用对延迟非常敏感 需要优先处理以减少响应时间 |
| UclampForeGroundMin | string   | 用于设置前台APP可使用的CPU频率下限 （范围0-100）|
| UclampForeGroundMax | string   | 用于设置前台APP可使用的CPU频率上限 （范围0-100）|
| UclampBackGroundMin | string   | 用于设置后台APP可使用的CPU频率下限 （范围0-100）|
| UclampBackGroundMax | string   | 用于设置后台APP可使用的CPU频率上限 （范围0-100）|

## TODO: 以下为重点内容 
### 关于 CPU 簇的参数
- modelTypeX：表示 CPU X 簇的定义 例如，modelType0 表示 CPU 0 簇
- ReferenceFreqX：表示 CPU X 簇的常规最大频率 这是在正常负载下的目标频率
- BoostFreqX：表示 CPU X 簇在负载达到临界值时提升到的频率 这是在高负载场景下为了保证性能而设置的频率
- MaxFreqX：表示 CPU X 簇的最大频率 这是用于负载采样的频率上限 CPU Turbo Scheduler 不会将频率提升到此值
### 参数拼接示例
- CPU Turbo Scheduler 会根据 modelTypeX 的值动态拼接路径 例如:对于 CPU 2 簇：
"/sys/devices/system/cpu/cpufreq/" + modelType2
这样可以动态访问和调整不同 CPU 簇的频率参数

### 情景模式的切换
```
在CPU Turbo Scheduler启动时会读取配置文件获取情景模式,在日志以如下方式体现：  
[2025-01-05 17:43:28] INFO:均衡模式已启用
```
`switchInode`对应路径的文件，监听新模式名称的写入完成模式切换：  
```shell
echo "powersave" > /sdcard/Android/MW_CpuSpeedController/config.txt
```
在日志以如下方式体现：
[2025-01-05 17:43:28] INFO:均衡模式已启用

## 外围改进
本模块除了CS调度本体的优化，还配合一些外围的改进共同提升用户使用体验。
  - Mi FEAS功能
  - 调整系统和传感器的关键进程线程的CPU核心
  - 关闭高通GpuBoost
  - 调整CPUset
  - 调整CPUctl
  - 负载均衡
  - UFS时钟门开关
  - CPUIdle调度器调整
  - 关闭foreground boost
  - I/O优化
  - 禁用大多数内核态和用户态boost、热插拔
  - schedtune.Boost置零
  - EAS调度器参数优化
  - 顶层应用冷、热启动速度加快

# 致谢 （排名不分前后）
感谢以下用户对本项目的帮助：  
- [CoolAPK@hfdem](https://github.com/hfdem) <br>
- [CoolAPK@ztc1997](https://github.com/ztc1997) <br>
- [CoolAPK@XShe](https://github.com/XShePlus) <br>
- [CoolAPK@Timeline](https://github.com/nep-Timeline) <br>
- [CoolAPK@shrairo](https://github.com/shrairo) <br>
- QQ@长虹久奕
  
# 使用的开源项目
[作者:wme7 项目:INIreader](https://github.com/wme7/INIreader) <br>
感谢所有用户的测试反馈 这将推进CPU Turbo Scheduler的开发
### 该文档更新于:2025/01/25 13:31
