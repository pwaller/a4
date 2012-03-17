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
    std::stack<Times> start_times;
    std::map<const char*, Durations> duration_map;
public:
    void start();
    /// Non-dynamic strings only please!
    void stop(const char* const name);
};


class PerformanceInfoStore {
public:

    ~PerformanceInfoStore();

    static boost::thread_specific_ptr<ThreadLocalPerformanceInfo> tss;
    
    std::vector<ThreadLocalPerformanceInfo*> threads_data;
    
    ThreadLocalPerformanceInfo& get() {
        auto* tlpi = tss.get();
        if (unlikely(!tlpi)) {
            tlpi = new ThreadLocalPerformanceInfo;
            threads_data.push_back(tlpi);
            tss.reset(tlpi);
        }
        return *tlpi;
    }
};

extern PerformanceInfoStore performance_store;

// Assumption: These should only ever exist on the stack, no rly.
class ScopePerformanceMonitor {
    const char* _name;
    ThreadLocalPerformanceInfo& _store;
public:
    ScopePerformanceMonitor(const char* name);
    ~ScopePerformanceMonitor();
};

#define A4PERF_MONITOR(name) ScopePerformanceMonitor a4perf_ ## __LINE__ ## _(name)


} // namespace a4

#endif // _A4_PERF_H_
