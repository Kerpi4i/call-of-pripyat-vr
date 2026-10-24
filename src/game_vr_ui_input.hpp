#pragma once
namespace copvr_ui {
inline bool& mouseHeld() { static bool value=false; return value; }
}
