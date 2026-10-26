#include "vr_profile.hpp"
#undef NDEBUG
#include <cassert>
#include <fstream>
#include <iterator>
int main() {
    const char* dir=std::getenv("COPVR_PROFILE_DIR"); assert(dir);
    {
        copvr::ProfileTrace trace("profile-test");
        assert(trace.enabled());
        trace.record("known",7,100,2.5);
        {copvr::ProfileSpan span(trace,"span",8);}
    }
    std::ifstream f(std::string(dir)+"/profile-test.csv");
    const std::string data((std::istreambuf_iterator<char>(f)),{});
    assert(data.find("frame,stage,start_qpc_ms,duration_ms")!=std::string::npos);
    assert(data.find("7,known,100.000000,2.500000")!=std::string::npos);
    assert(data.find("8,span,")!=std::string::npos);
    const double a=copvr::profileClockMs(),b=copvr::profileClockMs();assert(b>=a);
}
