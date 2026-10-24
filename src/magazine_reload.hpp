#pragma once
namespace copvr {
struct MagazineReload {
    bool present=true;
    bool needsChamber=false;
    bool remove() {
        if(!present) return false;
        present=false; needsChamber=true; return true;
    }
    bool insert(int rounds,bool requireRack=true) {
        if(present||rounds<=0) return false;
        present=true; needsChamber=requireRack; return true;
    }
    bool rack(int rounds,bool jammed=false) {
        // A complete charging stroke clears a jam independently of magazine
        // replacement. Without usable ammunition, preserve chamber readiness.
        if(!present||rounds<=0) return jammed;
        needsChamber=false; return true;
    }
    bool canFire() const { return present&&!needsChamber; }
};
}
