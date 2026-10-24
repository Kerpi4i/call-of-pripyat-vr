#pragma once
namespace copvr_game {
struct ReleaseCollision { u16 item=u16(-1),actor=u16(-1); u32 after=0; };
inline ReleaseCollision* releaseCollisions() { static ReleaseCollision value[64]; return value; }
inline bool suppressItemCamera(u16 id) {
    for(int i=0;i<64;++i) if(releaseCollisions()[i].item==id) return true;
    return false;
}
}
