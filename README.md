# CpuTurboScheduler V3.0
[![C++](https://img.shields.io/badge/language-C++-%23f34b7d.svg?style=plastic)](https://en.wikipedia.org/wiki/C++)
[![Android](https://img.shields.io/badge/platform-Android-0078d7.svg?style=plastic)](https://en.wikipedia.org/wiki/Android_(operating_system)) 
[![AArch64](https://img.shields.io/badge/arch-AArch64-red.svg?style=plastic)](https://en.wikipedia.org/wiki/AArch64)
[![Android Support10-16](https://img.shields.io/badge/Android%2010~16-Support-green)](https://img.shields.io/badge/Android%2010~16-Support-green)
#### 介绍
CPU Turbo Scheduler 是一款基于 C++ 编写的智能 CPU 调度工具 旨在优化 Android 设备的 CPU 性能和功耗表现而设计 通过智能调度算法 它可以根据不同的使用场景动态调整 CPU 频率以达到最佳的性能和能效平衡 <br>
#### 工作条件
1.目前该调度适用于ARM64平台的Android10-16 <br>
2.拥有Root权限

#### 修改启动时的默认模式
1.打开/sdcard/Android/CTS/mode.txt <br>
2.重启后查看/sdcard/Android/CTS/log.txt检查CPU Turbo Scheduler是否正常自启动

### 情景模式
- powersave 省电
- balance 均衡
- performance 性能
- fast 极速

#### 日志等级
- DEBUG 调试
- INFO 信息
- WARNNING 警告
- ERROR 错误
  
#### 情景模式切换
powersave（省电模式）：在保证基本流畅度的同时，尽可能降低功耗。推荐'待机'使用 <br>
balance（均衡模式）：比原厂设置更流畅且更省电。推荐'日常'使用 <br>
performance（性能模式）：在保证较高流畅度的同时，可能会增加功耗。推荐'需要短时间性能的场景'使用 <br>
fast（极速模式）：全力保证游戏时的流畅度，忽略能效比，并没有任何'频率限制'并且所有核心将拉升至最高频率

## 常见问题
Q：是否会对待机功耗产生负面影响？ <br>
A：CPU Turbo Scheduler 做了低功耗优化 由于使用了 C++ 语言 自身运行功耗很低 并不会对设备的待机功耗产生显著影响   <br>
 
Q：为什么使用了 CPU Turbo Scheduler 后功耗仍然很高？  <br> 
A：SOC 的 AP 部分功耗主要取决于计算量和使用的频点 CPU Turbo Scheduler 只能通过控制性能释放和改进频率的方式来降低功耗 如果后台应用的计算量很大 可能无法显著延长续航时间 可以通过 Scene 工具箱的进程管理器来定位问题  PS:建议全局测试时间不低于一小时 否则属于瞬时功耗 瞬时功耗不具有任何参考价值 <br>

Q：何时更新 XXXX 版本？  <br>
A：如果您觉得有需要更新的内容，请发送至邮箱：mowei2077@gmail.com PS:作者每周都会查看谷歌邮箱考虑是否需要实现某些功能 <br>

Q：如何确保我的设备拥有并支持 Feas特性？  <br>
A：开启 CPU Turbo Scheduler 的 Feas 开关并切换到极速模式 CPU Turbo Scheduler将会自动识别内核的 Feas 接口 如果设备没有 Feas 功能接口 将会在日志中抛出错误 目前CPU Turbo Scheduler 已接入大多数内核的 Feas 接口  <br>

Q：是否还需要关闭系统的performance boost？  <br>
A：CPU Turbo Scheduler在初始化阶段就已经关闭了大部分主流的用户态和内核态升频 如果有非常规的升频需要用户自己关闭  <br>

Q：CPU Turbo Scheduler和Scene工具箱是什么关系？  <br>
A：这两个软件独立运作没有互相依赖 CPU Turbo Scheduler实现了接口可供Scene工具箱调用 例如性能模式切换以及分APP性能模式 如果不安装Scene工具箱也可以实现性能模式切换 <br>

Q：为什么在使用Scene工具箱接管CPU Turbo Scheduler时开机需要重新开启开关 这是否意味着CPU Turbo Scheduler没有正常工作？  <br>
A：CPU Turbo Scheduler在系统解锁后就会正常自启动 CPU Turbo Scheduler不依赖于Scene工具箱 至于开机时需要重新打开调度开关请询问Scene工具箱的开发人员<br>

Q：为什么在使用Scene工具箱接管CPU Turbo Scheduler后 会出现一堆切换模式的日志？  <br>
A：因为Scene工具箱会一直监听屏幕是否亮屏和息屏 当亮屏时Scene工具箱会切换一次模式 CPU Turbo Scheduler监听到模式更改后就会输出一次日志并写入一些相关的参数 PS:虽然性能开销很低 但我个人认为这样会造成不必要的性能开销 所以我本人并不推荐大家去使用Scene工具箱去接管任何调度 <br>

Q：支不支持XXX内核？<br>
A：目前CpuTurboScheduler支持大部分内核 举例4.4及以上的内核 <br> 

Q：切换情景模式后是否需要重启？<br>
A：不需要 目前CpuTurboScheduler会监听情景模式的配置变化进行切换 <br> 

Q: 调速器参数是什么东西?
A: 这是Scheduler中的一些参数 这些设置将影响你的流畅度体验或续航 它的路径位于'/sys/devices/system/cpu/cpufreq/policyX/scheduler/' 
## 配置文件说明
### （一）元信息（meta）

```json
    "meta": {
        "name": "7+Gen2",
        "version": 1,
        "author": "MoWei",
        "loglevel": "INFO"
    },
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| name     | string   | 配置文件的名称                                 |
| version   | int   |  配置文件的版本号                            |
| author | string   |  配置文件的作者信息 |
| loglevel | string   | 日志等级，可选值为 DEBUG、INFO、WARNING、ERROR |

### (二) CPU簇配置 （Policy）
```json
    "Policy": {
        "c0": 0,
        "c1": 4,
        "c2": 7,
        "c3": -1
    },
```

如果CPU簇都配置完毕了 在后面的CPU簇可以使用-1 CTS会自动跳过 没有必要进行删除否则可能导致进程异常 <br>
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| c0 | int   | 配置CPU0簇 | 
| c1 | int   | 配置CPU1簇 |
| c2 | int   | 配置CPU2簇 |
| c3 | int   | 配置CPU3簇 |


### (三) 附加功能 (Function)
#### Cpuset设置
```json
    "Cpuset": {
        "enable": true,
        "top_app": "0-7",
        "foreground": "0-6",
        "restricted": "0-5",
        "system_background": "1-2",
        "background": "0-2"
    },
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| enable | bool   | cpuset总开关 | 
| top_app | string   | 顶层应用前台可用核心范围 |
| foreground | string   | 前台应用前台可用核心范围 |
| restricted | string   | 前台应用加速可用核心范围 |
| system_background | string | 系统后台应用加速可用核心范围 |
| background | string | 系统后台应用加速可用核心范围 |

#### Boost设置
```json
    "Boost": {
        "boost_rate_limit_ms": 48,
        "BoostFreq": {
            "c0": "1670400",
            "c1": "1996800",
            "c2": "2246400",
            "c3": ""
        }
    },
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| boost_rate_limit_ms | int   | Boost延续时间 单位:毫秒 | 
| c0 | string   | Boost最大频率 |
| c1 | string   | Boost最大频率 |
| c2 | string   | Boost最大频率 |
| c3 | string | Boost最大频率 |

#### LoadBalancing设置
```json
    "LoadBalancing": {
        "enable": false
    },
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| enable | bool | 负载均衡总开关 原理:通过调整sched_load_balance和sched_relax_domain_level达到负载均衡 | 

#### LaunchBoost设置
```json
    "LaunchBoost": {
        "enable": false
    },
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| enable | bool | LaunchBoost总开关 PS:通过监听应用切换事件 进行临时升频 详细设置可看Boost节点 | 

#### DisableGpuBoost设置
```json
    "DisableGpuBoost": {
        "enable": false
    },
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| enable | bool | 关闭高通GPUBoost总开关 PS:即使不开启此功能也会对GPU设置进行优化 | 

#### IO_Optimization设置
```json
    "IO_Optimization": {
        "enable": true,
        "scheduler": "none",
        "iostats": "0",
        "nomerges": "2",
        "read_ahead_kb": "128"
    },
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| enable | bool | I/O优化总开关 | 
| scheduler | string | I/O调速器选择 PS:emmc设备如果有cfq尽量使用cfq UFS4.0推荐使用none作为I/O调速器 UFS3.0-3.1如果拥有SSG 尽量优先使用SSG其次bfq | 
| iostats | string | I/O统计信息 PS:关闭后能优化I/O性能 | 
| nomerges | string | I/O合并 设置"0"开启I/O合并 "1"禁用简单的单校I/O合并 "2"禁用所有类型的I/O合并 | 


#### Scheduler设置
```json
    "Scheduler": {
        "enable": true,
        "sched_energy_aware": true,
        "sched_schedstats": false,
        "sched_latency_ns": "10000000",
        "sched_migration_cost_ns": "100000",
        "sched_min_granularity_ns": "15000000",
        "sched_wakeup_granularity_ns": "2000000",
        "sched_nr_migrate": "32",
        "sched_util_clamp_min": "0",
        "sched_util_clamp_max": "1024"
    }
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| enable | bool   | 调度器设置总开关 | 
| sched_energy_aware | bool   | 开启EAS调度器 |
| sched_schedstats | bool   | 开启调度器统计信息 |
| sched_latency_ns | string   | 调度周期的目标延迟（单位：ns）值越小任务切换越频繁，响应更快但调度开销更大 |
| sched_migration_cost_ns | string | 任务迁移成本估计（单位：ns）值越大调度器越倾向于减少 CPU 迁移 |
| sched_min_granularity_ns | string   | 单个任务的最小运行时间片（单位：ns）防止任务被过度抢占 |
| sched_wakeup_granularity_ns | string | 唤醒抢占阈值（单位：ns）控制新唤醒任务是否抢占当前运行任务 |
| sched_nr_migrate | string | 单次负载均衡中允许迁移的最大任务数 |
| sched_util_clamp_min | string   | 任务最小 Util Clamp 值 用于限制调度器感知的最低 CPU 利用率 |
| sched_util_clamp_max | string | 任务最大 Util Clamp 值 用于限制调度器感知的最高 CPU 利用率（1024 为上限） |

###  (四)频率核心开关调整
```ini
 "balance": {
    "MinFreq": {
        "c0": "0",
        "c1": "0",
        "c2": "0",
        "c3": ""
    },
    "MaxFreq": {
        "c0": "1670400",
        "c1": "2227200",
        "c2": "2476800",
        "c3": ""
    },
    "governor": {
        "c0": "walt",
        "c1": "walt",
        "c2": "walt",
        "c3": ""
    },
    "SchedulerParam": {
        "cX": {
            "Path1": "adaptive_high_freq",
            "value1": "0",
            "Path2": "adaptive_low_freq",
            "value2": "0",
            "Path3": "boost",
            "value3": "0",
            "Path4": "down_rate_limit_us",
            "value4": "0",
            "Path5": "hispeed_freq",
            "value5": "1228800",
            "Path6": "hispeed_load",
            "value6": "90",
            "Path7": "pl",
            "value7": "1",
            "Path8": "rtg_boost_freq",
            "value8": "1000000",
            "Path9": "target_load_shift",
            "value9": "2",
            "Path10": "target_load_thresh",
            "value10": "1024",
            "Path11": "target_loads",
            "value11": "-3 1548398:-13",
            "Path12": "up_rate_limit_us",
            "value12": "2000"
        }
    },
    "CoreOnline": {
        "Core0": 1,
        "Core1": 1,
        "Core2": 1,
        "Core3": 1,
        "Core4": 1,
        "Core5": 1,
        "Core6": 1,
        "Core7": 1
    }
},
```
| 字段名   | 数据类型 | 描述                                           |
| -------- | -------- | ---------------------------------------------- |
| MinFreq | string   | 最小频率 |
| MaxFreq | string   | 最大频率 |
| governor | string   | CPU调速器 |
| SchedulerParam | string   | 调速器参数 |
| PathX | string   | 为调速器参数名称 PS:当你不需要多余参数时 请留空CTS会自动跳过 而不是选择删节点 |
| valueX | string   | 为调速器参数值 |
| CoreOnline | int   | 单独开关CPU核心 |


### 情景模式的切换
```
在CPU Turbo Scheduler启动时会读取配置文件获取情景模式,在日志以如下方式体现：  
2026-01-13 13:40:00 信息 -> 已启用powersave模式
```
`switchInode`对应路径的文件，监听新模式名称的写入完成模式切换：  
```shell
echo "powersave" > /sdcard/Android/MW_CpuSpeedController/config.txt
```


## 外围改进
本模块除了CS调度本体的优化，还配合一些外围的改进共同提升用户使用体验。
  - 高通GPU优化
  - Cpuset优化
  - Cpuctl优化
  - 负载均衡
  - 禁用大多数内核态和用户态boost、热插拔
  - 调度器参数优化
  - I/O优化
  - 核心进程调度优化
  
# 致谢 （排名不分前后）
感谢以下用户对本项目的帮助：  
- nullptr

# 使用的开源项目
[作者:qinyonghang 项目:JSON](https://github.com/qinyonghang/json/tree/master) <br>
[作者:MoWei 项目:LibUtils](https://github.com/MoWei-2077/FastStdLib) <br>

### 该文档更新于:2026/01/03
- 感谢所有用户的测试反馈 这将推进CPU Turbo Scheduler的开发
