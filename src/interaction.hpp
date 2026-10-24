#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

namespace copvr {
struct Vec3 {
    float x{}, y{}, z{};
    Vec3 operator+(Vec3 b) const { return {x+b.x,y+b.y,z+b.z}; }
    Vec3 operator-(Vec3 b) const { return {x-b.x,y-b.y,z-b.z}; }
    Vec3 operator*(float s) const { return {x*s,y*s,z*s}; }
    float length() const { return std::sqrt(x*x+y*y+z*z); }
};
struct Quaternion { float x{}, y{}, z{}, w{1}; };
struct Pose { Vec3 position; Quaternion orientation; bool valid{}; };
inline Vec3 rotate(Quaternion q, Vec3 v) {
    const Vec3 u{q.x,q.y,q.z};
    const auto cross=[](Vec3 a,Vec3 b) { return Vec3{a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; };
    return v+cross(u,v)*(2*q.w)+cross(u,cross(u,v))*2;
}
// OpenXR: right handed, forward -Z. X-Ray: left handed, forward +Z.
inline Vec3 to_xray(Vec3 v) { return {v.x,v.y,-v.z}; }
inline Quaternion to_xray(Quaternion q) { return {-q.x,-q.y,q.z,q.w}; }
struct Hand { Pose grip, aim; Vec3 velocity; bool velocityValid{}; float squeeze{},trigger{}; float stickX{},stickY{}; uint32_t buttons{}; };
struct Frame { Pose head; std::array<Hand,2> hands; bool focused{}; float floorEyeHeight{}; bool floorValid{}; };
enum class Location { World, Held, Slot };
struct Item {
    std::uint32_t id{}; Pose pose; Location location{Location::World};
    int owner{-1}; bool weapon{};
};
struct Slot { Vec3 position; float radius{0.16f}; std::optional<std::uint32_t> item; };
enum class EventType { Grab, Drop, Stow, FireStart, FireStop };
struct Event { EventType type; std::uint32_t item; int hand; Pose pose; Vec3 velocity; int slot{-1}; };

// Prototype domain logic only: the host must apply events to game entities.
// Slot positions and all poses use the same metre-based reference space.
class Interaction {
    std::array<bool,2> gripping_{}, firing_{}, armed_{};
    std::array<std::optional<std::uint32_t>,2> held_;
    Item* find(std::vector<Item>& items,std::uint32_t id) {
        for(auto& i:items) if(i.id==id) return &i;
        return nullptr;
    }
public:
    const auto& held() const { return held_; }
    std::vector<Event> update(const Frame& f,std::vector<Item>& items,std::vector<Slot>& slots) {
        std::vector<Event> out;
        for(int h=0;h<2;++h) {
            const auto& hand=f.hands[h];
            Item* item=held_[h]?find(items,*held_[h]):nullptr;
            if(held_[h]&&!item) { held_[h].reset(); firing_[h]=false; }
            const bool usable=f.focused&&f.head.valid&&hand.grip.valid;
            if(!usable) {
                if(firing_[h]&&item) out.push_back({EventType::FireStop,item->id,h,{},{}});
                firing_[h]=false; armed_[h]=false;
                // Keep ownership on tracking loss; do not throw an unseen item.
                gripping_[h]=hand.squeeze>0.25f;
                continue;
            }
            if(hand.trigger<0.2f) armed_[h]=true;
            const bool grip=gripping_[h]?hand.squeeze>0.25f:hand.squeeze>=0.65f;
            if(grip&&!gripping_[h]&&!item) {
                float nearest=0.18f;
                for(auto& candidate:items) {
                    if(candidate.location==Location::Held||!candidate.pose.valid) continue;
                    Vec3 position=candidate.pose.position;
                    if(candidate.location==Location::Slot) {
                        if(candidate.owner<0||static_cast<size_t>(candidate.owner)>=slots.size()) continue;
                        position=slots[candidate.owner].position;
                    }
                    float distance=(position-hand.grip.position).length();
                    if(distance<nearest) { nearest=distance; item=&candidate; }
                }
                if(item) {
                    if(item->location==Location::Slot) slots[item->owner].item.reset();
                    held_[h]=item->id; item->location=Location::Held; item->owner=h;
                    out.push_back({EventType::Grab,item->id,h,hand.grip,{}});
                }
            }
            if(item) {
                item->pose=hand.grip;
                bool fire=grip&&item->weapon&&hand.aim.valid&&armed_[h]&&
                    (firing_[h]?hand.trigger>0.2f:hand.trigger>=0.75f);
                if(fire!=firing_[h]) out.push_back({fire?EventType::FireStart:EventType::FireStop,item->id,h,hand.aim,{}});
                firing_[h]=fire;
                if(!grip) {
                    int target=-1; float distance=1e9f;
                    for(size_t s=0;s<slots.size();++s) {
                        float d=(slots[s].position-hand.grip.position).length();
                        if(!slots[s].item&&d<slots[s].radius&&d<distance) { target=static_cast<int>(s); distance=d; }
                    }
                    item->owner=target;
                    item->location=target<0?Location::World:Location::Slot;
                    if(target>=0) { slots[target].item=item->id; item->pose.position=slots[target].position; }
                    Vec3 velocity=hand.velocityValid?hand.velocity:Vec3{};
                    out.push_back({target<0?EventType::Drop:EventType::Stow,item->id,h,item->pose,velocity,target});
                    held_[h].reset(); armed_[h]=false;
                }
            }
            gripping_[h]=grip;
        }
        return out;
    }
};
}
