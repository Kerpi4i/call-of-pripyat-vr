#pragma once

namespace copvr_game {
// World-space motion, including player locomotion. Tracking discontinuities
// reset the estimate instead of imparting an impulse to a released object.
struct MotionHistory {
    bool valid=false;
    Fmatrix previous;
    Fvector linear{},angular{};
    struct Sample { Fvector velocity{}; float age=1.f; };
    Sample history[8]; unsigned cursor=0;
    void reset() { valid=false; linear.set(0,0,0); angular.set(0,0,0); for(auto& sample:history) sample.age=1.f; }
    Fvector throwLinear() const {
        Fvector peak=linear;
        for(const auto& sample:history) if(sample.age<=.08f&&sample.velocity.square_magnitude()>peak.square_magnitude()) peak=sample.velocity;
        Fvector result; result.mul(peak,.75f); result.mad(linear,.25f); result.mul(1.7f);
        const float speed=result.magnitude(); if(speed>24.f) result.mul(24.f/speed);
        return result;
    }
    void update(const Fmatrix& pose,float dt) {
        Fvector velocity,spin; velocity.set(0,0,0); spin.set(0,0,0);
        bool continuous=false;
        if(valid&&dt>.001f&&dt<.1f) {
            Fvector delta; delta.sub(pose.c,previous.c);
            const float cosine=_max(-1.f,_min(1.f,(previous.i.dotproduct(pose.i)+
                previous.j.dotproduct(pose.j)+previous.k.dotproduct(pose.k)-1.f)*.5f));
            if(delta.magnitude()<.25f&&cosine>0.f) {
                continuous=true;
                velocity=delta; velocity.div(dt);
                const float speed=velocity.magnitude(); if(speed>18.f) velocity.mul(18.f/speed);
                Fvector cross; spin.crossproduct(previous.i,pose.i);
                cross.crossproduct(previous.j,pose.j); spin.add(cross);
                cross.crossproduct(previous.k,pose.k); spin.add(cross);
                const float angle=acosf(cosine),sine=sinf(angle);
                spin.mul(sine>.0001f?angle/(2.f*sine*dt):.5f/dt);
                const float rate=spin.magnitude(); if(rate>12.f) spin.mul(12.f/rate);
            }
        }
        for(auto& sample:history) sample.age+=dt;
        if(!continuous) for(auto& sample:history) sample.age=1.f;
        history[cursor].velocity=velocity; history[cursor].age=0.f; cursor=(cursor+1)%8;
        linear=velocity; angular=spin; previous=pose; valid=true;
    }
};
}
