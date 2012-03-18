#include "a4/perf.h"

#include <a4/debug.h>
#include <a4/types.h>


#include <boost/chrono.hpp>
namespace chrono = boost::chrono;
typedef chrono::duration<double> duration;

namespace a4 {

PerformanceInfoStore performance_store;
boost::thread_specific_ptr<ThreadLocalPerformanceInfo> PerformanceInfoStore::tss;

~ThreadLocalPerformanceInfo() {
    performance_store.update_total(*this);
}

PerformanceInfoStore::~PerformanceInfoStore() {
    ThreadLocalPerformanceInfo total_info;
    total_info.insert("Whole Program", Times() - _program_start);
    
    total_info.dump();
}

void ThreadLocalPerformanceInfo::start() {
    _start_times.push(Times());
}

void ThreadLocalPerformanceInfo::stop(const char* const name) {
    _duration_map[name] += Times() - _start_times.top();
    _start_times.pop();
}

ThreadLocalPerformanceInfo& ThreadLocalPerformanceInfo::operator+=(ThreadLocalPerformanceInfo& rhs) {
    foreach (auto i, rhs._duration_map) {
        _duration_map[i.first] += i.second;
    }
    return *this;
}

void ThreadLocalPerformanceInfo::dump() {
    VERBOSE("Performance Information:");
    foreach (auto i, _duration_map) {
        VERBOSE(" func: ", i.first, " : ", i.second._cpu_duration, " -- ", i.second._wall_duration);
    }
}

ScopePerformanceMonitor::ScopePerformanceMonitor(const char* name) 
  : _name(name), _store(performance_store.get()) {
    _store.start();
}

ScopePerformanceMonitor::~ScopePerformanceMonitor() {
    _store.stop(_name);
}


} // namespace a4

