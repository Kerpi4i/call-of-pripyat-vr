#include "trigger_gate.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>
void check(bool condition,const char* reason) { if(!condition) { std::cerr<<reason<<'\n'; std::exit(1); } }
int main() {
    copvr::TriggerGate gate;
    check(gate.update(true,1)==0,"must release after startup");
    gate.update(true,0);
    check(gate.update(true,.8f)==1,"trigger starts");
    check(gate.update(true,.5f)==0,"hysteresis maintains hold");
    check(gate.update(false,1)==-1,"tracking/UI inhibition releases");
    check(gate.update(true,1)==0,"focus return cannot auto fire");
    gate.update(true,0); check(gate.update(true,1)==1,"release rearms");
    check(gate.update(true,std::numeric_limits<float>::quiet_NaN())==-1,"nonfinite input releases");
    check(gate.reset()==0,"reset is idempotent");
    gate.update(true,0); gate.update(true,1);
    check(gate.reset()==-1,"weapon switch releases old weapon");
    check(gate.update(true,1)==0,"weapon switch cannot auto fire new weapon");
    std::cout<<"PASS: trigger hysteresis, startup/focus/tracking/UI inhibition, weapon switch, nonfinite input\n";
}
