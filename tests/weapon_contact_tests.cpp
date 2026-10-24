#include "weapon_contact.hpp"
#include "beam_math.hpp"
#undef NDEBUG
#include <cassert>
#include <iostream>
using namespace copvr;
int main() {
    const float s=std::sqrt(.5f);
    // A Touch grip can have a large pitch relative to its aim frame. Pointing
    // forward must still point the barrel forward, for either contact side.
    for(float side:{-1.f,1.f}) for(auto aimRotation:{Quaternion{},Quaternion{0,s,0,s},Quaternion{s,0,0,s}}) {
        Pose aim{{},aimRotation,true};
        for(auto gripRotation:{Quaternion{},Quaternion{s,0,0,s},Quaternion{0,0,s,s}}) {
            Pose grip{{side*.25f,1.3f,.6f},gripRotation,true};
            for(auto localPalm:{Vec3{.02f,-.1f,-.07f},Vec3{-.04f,-.08f,.02f}}) {
                auto object=weaponAtPalm(aim,grip,localPalm); assert(object.valid);
                assert((rotate(object.orientation,{0,0,1})-rotate(aimRotation,{0,0,1})).length()<.0001f);
                assert((object.position+rotate(object.orientation,localPalm)-grip.position).length()<.0001f);
                // The authored wrist may rotate by 90 degrees: composing the
                // attachment changes the wrist, never the object's barrel.
                Vec3 wristPalm{0,0,.04f}; Quaternion attachment{s,0,0,s};
                auto wristOrigin=localPalm-rotate(attachment,wristPalm);
                auto worldPalm=object.position+rotate(object.orientation,wristOrigin+rotate(attachment,wristPalm));
                assert((worldPalm-grip.position).length()<.0001f);
            }
        }
    }
    assert(!weaponAtPalm(Pose{},Pose{{},{},true},{}).valid);
    assert(!weaponAtPalm(Pose{{},{},true},Pose{},{}).valid);
    std::cout<<"PASS: weapon forward independent of grip/wrist pitch, both-hand palm contact, inactive tracking\n";
}
