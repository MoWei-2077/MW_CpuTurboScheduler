# CpuTurboScheduler V2.0
[![C++](https://img.shields.io/badge/language-C++-%23f34b7d.svg?style=plastic)](https://en.wikipedia.org/wiki/C++)
[![Android](https://img.shields.io/badge/platform-Android-0078d7.svg?style=plastic)](https://en.wikipedia.org/wiki/Android_(operating_system)) 
[![AArch64](https://img.shields.io/badge/arch-AArch64-red.svg?style=plastic)](https://en.wikipedia.org/wiki/AArch64)
[![Android Support8-15](https://img.shields.io/badge/Android%208~15-Support-green)](https://img.shields.io/badge/Android%208~15-Support-green)
#### 介绍
CPU Turbo Scheduler 是一款基于 C++ 编写的智能 CPU 调度工具，旨在优化 Android 设备的 CPU 性能和功耗表现。通过智能调度算法，它可以根据不同的使用场景动态调整 CPU 频率以达到最佳的性能和能效平衡。 <br>

#### 工作条件
1.目前该调度适用于Android8-15 <br>
2.拥有Root权限

#### 修改启动时的默认模式
1.打开/sdcard/Android/MW_CpuSpeedController/config.txt <br>
2.可选的挡位有powersave balance performance fast四个挡位 <br>
2.重启后查看/sdcard/Android/MW_CpuSpeedController/log.txt检查CS调度是否正常自启动

#### 情景模式切换
powersave（省电模式）：在保证基本流畅度的同时，尽可能降低功耗。推荐日常使用 <br>
balance（均衡模式）：比原厂设置更流畅且更省电。推荐日常使用 <br>
performance（性能模式）：在保证较高流畅度的同时，可能会增加功耗。推荐游戏使用 <br>
fast（极速模式）：全力保证游戏时的流畅度，忽略能效比 

## 负载采样
通过对/proc/stat进行负载采样,获取以jiffies为单位的负载再通过计算转换为百分比的负载。 <br>
TODO:  <br>
声明:cerrFreq为当前频率 cerrLoad为当前负载 Magrin为余量 ReferenceFreq为基准频率 MaxFreq为最大频率 <br>
公式:LoadBoostValue=(cerrFreq/cerrLoad)+Magrin 当LoadBoostValue>ReferenceFreq且LoadBoostValue<MaxFreq时 进行Boost频率 <br>
警告:ReferenceFreq为基准频率并不是最小频率！！！

## 常见问题
Q：是否会对待机功耗产生负面影响？
A：CPU Turbo Scheduler 做了低功耗优化 由于使用了 C++ 语言 自身运行功耗很低 并不会对设备的待机功耗产生显著影响  <br>
Q：为什么使用了 CPU Turbo Scheduler 后功耗仍然很高？ <br>
A：SOC 的 AP 部分功耗主要取决于计算量和使用的频点 CPU Turbo Scheduler 只能通过控制性能释放和改进频率的方式来降低功耗 如果后台应用的计算量很大 可能无法显著延长续航时间 可以通过 Scene 工具箱的进程管理器来定位问题 <br>
Q：何时更新 XXXX 版本？ <br>
A：如果您需要更新的内容，请发送至邮箱：mowei2077@gmail.com <br>
Q：如何确保设备拥有 Perfmgr 内核模块？ <br>
A：开启 CPU Turbo Scheduler 的 Feas 开关并切换到极速模式 调度器会自动识别内核的 Feas 接口 如果设备没有 Feas 功能接口 将会在日志中抛出错误 目前CPU Turbo Scheduler 已接入大多数内核的 Feas 接口 <br>

## 配置文件说明
### （一）元信息（meta）

```ini
[meta]
name = "骁龙7+ Gen2"
author = MoWei
configVersion = 1
loglevel = "Debug"
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
| Disable_qcom_GpuBoost | bool   | 禁用高通 GPU Boost 防止 GPU 频率无序升高 | 
| AffintySetter | bool   | 对系统和传感器关键进程和线程进行绑核操作 |
| cpuctl | bool   | CPU 使用率控制功能 |
| ufsClkGate | bool   | 关闭UFS 时钟门功能（性能模式和极速模式下关闭 UFS 时钟门） |
| CpuIdleScaling_Governor | bool   | 自定义 CPUIdle 调度器。 |
| EasScheduler | bool   | EAS 调度器优化 |
| cpuset | bool | CPUSet功能 调整应用的核心分配 |
| LoadBalancing | bool | 通过优化CFS调度器的参数达到负载均衡的效果 |
| EnableFeas | bool |  FEAS 功能（仅限极速模式）|
| AdjIOScheduler | bool |  I/O 调度器调整以及I/O优化总开关 |

### （三）核心分配CoreAllocation）
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
| cpuctlUclampBoostMax | bool   | CPU使用率控制的最大值 |

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

### （六）CPUIdle 调度器（CpuIdle）
```ini
[CpuIdle]
current_governor = "qcom-cpu-lpm"
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| current_governor | string   | 设置使用的 CPUIdle 调度器模式，如高通推荐:qcom-cpu-lpm 联发科推荐:menu |

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

###  功耗模型开发(🕊)

在CS启动时会读取配置文件获取情景模式,在日志以如下方式体现：  
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
  - schedtune.Boost置零
  - EAS调度器参数优化
  - 应用冷、热速度加快

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
感谢所有用户的测试反馈 这将推进CS调度的开发
### 该文档更新于:2025/01/24 22:54
