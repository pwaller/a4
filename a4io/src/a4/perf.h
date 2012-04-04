#ifndef _A4_PERF_H_
#define _A4_PERF_H_

#include <a4/config.h>

#ifdef HAVE_SYSTEMTAP

#include <map>
#include <stack>
#include <vector>

#include <boost/thread/tss.hpp>

#include <boost/chrono.hpp>
namespace chrono = boost::chrono;
typedef chrono::duration<double> Duration;

#include <a4/types.h>

#include <sys/sdt.h>


namespace a4 {


// Assumption: These should only ever exist on the stack, no rly.
class ScopePerformanceMonitor {
    const char* _name;
public:
    ScopePerformanceMonitor(const char* name) : _name(name) {
        
        STAP_PROBE1("a4", entry, name);
        
    }
    ~ScopePerformanceMonitor() {
        STAP_PROBE1("a4", exit, _name);
    }
};


} // namespace a4

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define MAKE_UNIQUE(x) CONCATENATE(x, __COUNTER__)

#define A4PERF_MONITOR(name) a4::ScopePerformanceMonitor MAKE_UNIQUE(a4perf)(name)

#else // (!HAVE_SYSTEMTAP)

#define A4PERF_MONITOR(name) 

#endif // HAVE_SYSTEMTAP

#endif // _A4_PERF_H_
