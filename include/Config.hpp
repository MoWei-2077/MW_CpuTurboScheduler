#pragma once 

#include <iostream>
#include <fstream>
#include <string>
#include "Json/string.hpp"

using string_t = qlib::string_t;
using bool_t = qlib::bool_t;

using std::ifstream;

namespace Config {
    namespace Meta {
        string_t name;
        int version = -1;
        string_t author;
        string_t loglevel;
    }
    
    namespace Policy {
        int CpuPolicy [4] = { -1, -1, -1, -1 }; 
    }

    namespace Cpuset {
        bool enable = false;
        string_t top_app;
        string_t foreground;
        string_t background;
        string_t system_background;
        string_t restricted;
    }

    namespace LaunchBoost {
        bool enable = false;
        int boost_rate_limit_ms;
        string_t BoostFreq[4];  
    }

    namespace OfficialMode{
        bool enable = false;
    } 

    namespace LoadBanlace {
        bool enable = false;
    }

    namespace DisableGpuBoost {
        bool enable = false;
    }

    namespace Scheduler {
        bool enable = false;
        bool Sched_energy_aware = false;
        bool Sched_schedstats = false;
        string_t Sched_latency_ns;
        string_t Sched_migration_cost_ns;
        string_t Sched_min_granularity_ns;
        string_t Sched_wakeup_granularity_ns;
        string_t Sched_nr_migrate;
        string_t Sched_util_clamp_min;
        string_t Sched_util_clamp_max;
    }

    namespace Performances {
        int Online[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
        string_t MinFreq[4];
        string_t MaxFreq[4];
        string_t CpuGovernor[4];
    }

    struct SchedParam {
        string_t Name[24];
        string_t Value[24];
    };
}; 