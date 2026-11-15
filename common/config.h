#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

namespace Config
{
    constexpr int LOAN_PERIOD_DAYS = 7;
    constexpr double FINE_PER_DAY = 5.0;
    constexpr int MAX_ACTIVE_LOANS = 3;
    constexpr int RECENT_HISTORY_COUNT = 10;
    constexpr const char* DATA_DIR = "data";
}

#endif
