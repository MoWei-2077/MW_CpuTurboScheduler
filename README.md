# CpuTurboScheduler V2.0
[![C++](https://img.shields.io/badge/language-C++-%23f34b7d.svg?style=plastic)](https://en.wikipedia.org/wiki/C++)
[![Android](https://img.shields.io/badge/platform-Android-0078d7.svg?style=plastic)](https://en.wikipedia.org/wiki/Android_(operating_system)) 
[![AArch64](https://img.shields.io/badge/arch-AArch64-red.svg?style=plastic)](https://en.wikipedia.org/wiki/AArch64)
[![Android Support8-15](https://img.shields.io/badge/Android%208~15-Support-green)](https://img.shields.io/badge/Android%208~15-Support-green)
#### 介绍
CPU Turbo Scheduler 是一款基于 C++ 编写的智能 CPU 调度工具 旨在优化 Android 设备的 CPU 性能和功耗表现而设计 通过智能调度算法 它可以根据不同的使用场景动态调整 CPU 频率以达到最佳的性能和能效平衡 <br>
#### 工作条件
1.目前该调度适用于Android9-15 <br>
2.拥有Root权限

#### 修改启动时的默认模式
1.打开/sdcard/Android/MW_CpuSpeedController/config.txt <br>
2.可选择的模式包括 powersave（省电）、balance（均衡）、performance（性能）和 fast（极速）<br>
3.重启后查看/sdcard/Android/MW_CpuSpeedController/log.txt检查CPU Turbo Scheduler是否正常自启动

#### 情景模式切换
powersave（省电模式）：在保证基本流畅度的同时，尽可能降低功耗。推荐'待机'使用 <br>
balance（均衡模式）：比原厂设置更流畅且更省电。推荐'日常'使用 <br>
performance（性能模式）：在保证较高流畅度的同时，可能会增加功耗。推荐'需要短时间性能的场景'使用 <br>
fast（极速模式）：全力保证游戏时的流畅度，忽略能效比，并没有任何'频率限制'并且所有核心将拉升至最高频率

## 负载采样
通过对 /proc/stat 进行负载采样 获取以 jiffies（时钟滴答）为单位的负载 并通过计算转换为百分比负载 

## SchedTurbo调速器
通过使用C++制造的用户态Boost调速器 通过对负载进行采样进行智能升降频
#### SchedTurbo智能频率调节策略：
1.高负载响应机制:
- 当核心负载 ≥ HighLoad阈值时：
- ① 立即将核心频率提升至HighLoadFreq基准档位
- ② 维持up_rate_limit_ms时长后
- ③ 升级至BoostFreq增强档位
- ④ 再次维持up_rate_limit_ms时长

2.频率回退条件：
- 若在BoostFreq阶段检测到负载回落至[LowLoad, HighLoad)区间时立即降频至BasicFreq基础档位
  
3.低负载优化机制:
- 当核心负载 ≤ LowLoad阈值时：
- ① 降频至LowLoadFreq节能档位
- ② 持续保持down_rate_limit_ms时长
- ③ 期间持续监测负载变化

4.常规负载策略:
- 当负载处于(LowLoad, HighLoad)中间区间时
- 保持BasicFreq基础运行频率
- 实时监测系统负载变化

该方案通过500ms级细粒度负载采样 实现三级频率档位智能切换 在保证响应速度的同时优化能效表现 频率切换过程采用双参数控制机制 通过up_rate_limit_ms/down_rate_limit_ms独立设置升降频延迟 确保频率切换稳定性

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
name = "CpuTurboScheduler"
author = MoWei
configVersion = 7
loglevel = "INFO"
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| name     | string   | 配置文件的名称                                 |
| author   | string   | 配置文件的作者信息                             |
| configVersion | string   | 配置文件的版本号 |
| loglevel | string   | 日志等级，可选值为 Debug、INFO、Warning、Error |

### (二) 附加功能 （function）
```ini
[function]
DisableQcomGpu = false
AffintySetter = true
ufsClkGate = false
CpuIdleScaling_Governor = true
EasScheduler = false
cpuset = true
LoadBalancing = true
EnableFeas = false
AdjIOScheduler = true

```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| DisableQcomGpu | bool   | 禁用高通 GPU Boost 防止 GPU 频率无序升高 | 
| AffintySetter | bool   | 对系统和传感器关键进程和线程进行绑核操作 |
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
cpuctlUclampBoostMin = "0"
cpuctlUclampBoostMax = "70"
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| cpusetCore | string   | 指定 CPUSet核心 用于核心分配 |
| cpuctlUclampBoostMin | string   | CPU使用率控制的最小值 （范围0-100） |
| cpuctlUclampBoostMax | string   | CPU使用率控制的最大值 （范围0-100） |

### （四）I/O 设置（IO_Settings）
```ini
[IO_Settings]
Scheduler = ""
IO_optimization = false
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| Scheduler | string   | 指定 I/O 调度器类型，如 ssg、bfq 等 PS:该值为空时 将不会修改I/O调度器 |
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
current_governor = "menu"
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
SchedTurbo_governor = true
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
| SchedTurbo_governor | bool   | 在指定模式中开启SchedTurbo调速器 PS:该开关目前并无任何作用 |
| UclampTopAppMin | string   | 用于设置顶层APP可使用的CPU频率下限 （范围0-100）|
| UclampTopAppMax | string   | 用于设置顶层APP可使用的CPU频率上限 （范围0-100）|
| UclampTopApplatency_sensitive | bool   | 标记应用或进程对延迟敏感性的参数 设置此参数可以告知调度器前台应用对延迟非常敏感 需要优先处理以减少响应时间 |
| UclampForeGroundMin | string   | 用于设置前台APP可使用的CPU频率下限 （范围0-100）|
| UclampForeGroundMax | string   | 用于设置前台APP可使用的CPU频率上限 （范围0-100）|
| UclampBackGroundMin | string   | 用于设置后台APP可使用的CPU频率下限 （范围0-100）|
| UclampBackGroundMax | string   | 用于设置后台APP可使用的CPU频率上限 （范围0-100）|

###  (九)SchedTurbo调参
```ini
[SchedTurboScheduler]
BasicFreq = "2000000"
LowLoadFreq = "1800000"
HighLoadFreq = "2400000"
BoostFreq = "2700000"
HighLoad = 80
LowLoad = 30
up_rate_limit_ms = 500
down_rate_limit_ms = 1000
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| BasicFreq | string   | 常规频率 |
| LowLoadFreq | string   | 低负载频率 |
| HighLoadFreq | string   | 高负载频率 |
| BoostFreq | string   | 临时高频 |
| HighLoad | int   | 高负载阈值 |
| LowLoad | int   | 低负载阈值 |
| up_rate_limit_ms | int   | 下一次升频时间 单位:MS |
| down_rate_limit_ms | int   | 下一次降频时间 单位:MS |

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
- QQ@Microsoft
- 各位酷友以及CS调度的所有用户
# 使用的开源项目
- 暂无 <br>

### 该文档更新于:2025/02/06 22:01
- 感谢所有用户的测试反馈 这将推进CPU Turbo Scheduler的开发
