#include "a4/perf.h"

#include <a4/debug.h>
#include <a4/types.h>


#include <boost/chrono.hpp>
namespace chrono = boost::chrono;
typedef chrono::duration<double> duration;

namespace a4 {

PerformanceInfoStore performance_store;
boost::thread_specific_ptr<ThreadLocalPerformanceInfo> PerformanceInfoStore::tss;

PerformanceInfoStore::~PerformanceInfoStore() {
    foreach (auto* thread_data, threads_data)
        delete thread_data;
    VERBOSE("PerformanceInfoStore cleanup..");
}

void ThreadLocalPerformanceInfo::start() {
    start_times.push(Times());
}

void ThreadLocalPerformanceInfo::stop(const char* const name) {
    duration_map[name] += Times() - start_times.top();
    start_times.pop();
}

ScopePerformanceMonitor::ScopePerformanceMonitor(const char* name) 
  : _name(name), _store(performance_store.get()) {
    _store.start();
}

ScopePerformanceMonitor::~ScopePerformanceMonitor() {
    _store.stop(_name);
}


} // namespace a4

