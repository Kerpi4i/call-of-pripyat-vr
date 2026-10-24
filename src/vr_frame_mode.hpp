#pragma once
namespace copvr {
// 1: spatial screen; 2: interactive overlay; 3: passive overlay; 0: stereo.
inline int frameMode(bool loading,bool screen,int ui) { return loading||screen?1:ui==2?3:ui?2:0; }
}
