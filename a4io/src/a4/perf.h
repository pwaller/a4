#ifndef _A4_PERF_H_
#define _A4_PERF_H_

#include <map>
#include <stack>
#include <vector>

#include <boost/thread/tss.hpp>

#include <boost/chrono.hpp>
namespace chrono = boost::chrono;
typedef chrono::duration<double> Duration;

#include <a4/types.h>

namespace a4 {


class Durations {
public:
    Duration _cpu_duration, _wall_duration;
    Durations() {}
    Durations(const Duration& cpu_duration, const Duration& wall_duration) 
        : _cpu_duration(cpu_duration), _wall_duration(wall_duration)
    {
    }
    
    Durations& operator+=(const Durations& rhs) {
        _cpu_duration += rhs._cpu_duration;
        _wall_duration += rhs._wall_duration;
        return *this;
    }
};

class Times {
    chrono::thread_clock::time_point _cpu_time;
    chrono::steady_clock::time_point _wall_time;
    friend class Durations;
public:
    Times() : _cpu_time(chrono::thread_clock::now()), 
              _wall_time(chrono::steady_clock::now()) {}
              
    Durations operator-(const Times& rhs) const {
        return Durations(_cpu_time - rhs._cpu_time, _wall_time - rhs._wall_time);
    }
};

class ThreadLocalPerformanceInfo {
    std::stack<Times> _start_times;
    std::map<const char*, Durations> _duration_map;
public:
    ~ThreadLocalPerformanceInfo();

    void start();
    /// Non-dynamic strings only please!
    void stop(const char* const name);
    void dump();
    void insert(const char* n, const Durations& d) { _duration_map[n] = d; }
    
    ThreadLocalPerformanceInfo& operator+=(const ThreadLocalPerformanceInfo& rhs);
};

// Assumption: These should only ever exist on the stack, no rly.
class ScopePerformanceMonitor {
    const char* _name;
    ThreadLocalPerformanceInfo& _store;
public:
    ScopePerformanceMonitor(const char* name);
    ~ScopePerformanceMonitor();
};

class PerformanceInfoStore {
    Times _program_start;
    ThreadLocalPerformanceInfo _total_info;
public:
    ~PerformanceInfoStore();

    static boost::thread_specific_ptr<ThreadLocalPerformanceInfo> tss;
    
    void update_total(const ThreadLocalPerformanceInfo& rhs) {
        _total_info += rhs;
    }
    
    ThreadLocalPerformanceInfo& get() {
        auto* tlpi = tss.get();
        if (unlikely(!tlpi)) {
            tlpi = new ThreadLocalPerformanceInfo;
            tss.reset(tlpi);
            //threads_data.push_back(tlpi);
        }
        return *tlpi;
    }
};

extern PerformanceInfoStore performance_store;

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define MAKE_UNIQUE(x) CONCATENATE(x, __COUNTER__)

#define A4PERF_MONITOR(name) a4::ScopePerformanceMonitor MAKE_UNIQUE(a4perf)(name)


} // namespace a4

#endif // _A4_PERF_H_
