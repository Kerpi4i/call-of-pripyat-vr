#include "magazine_reload.hpp"
#include <cassert>
int main() {
    copvr::MagazineReload reload;
    assert(reload.canFire());
    assert(reload.rack(30,true)); // A jam clears by racking the existing magazine.
    assert(reload.canFire());
    assert(!reload.insert(30));
    assert(reload.remove()); assert(!reload.remove());
    assert(!reload.canFire()); assert(!reload.rack(30));
    assert(!reload.insert(0)); assert(reload.insert(30));
    assert(!reload.canFire()); assert(!reload.insert(30));
    assert(reload.rack(30,true)); assert(reload.canFire());
    copvr::MagazineReload jammedEmpty;
    assert(jammedEmpty.remove());assert(jammedEmpty.rack(0,true));
    assert(!jammedEmpty.present&&jammedEmpty.needsChamber&&!jammedEmpty.canFire());
    assert(jammedEmpty.insert(5));assert(jammedEmpty.needsChamber);
    assert(jammedEmpty.rack(5));assert(jammedEmpty.canFire());
    copvr::MagazineReload jammedUnchambered;
    assert(jammedUnchambered.remove());assert(jammedUnchambered.insert(2));
    assert(jammedUnchambered.rack(2,true));assert(jammedUnchambered.canFire());
    copvr::MagazineReload gauss;
    assert(gauss.remove());assert(gauss.insert(5,false));assert(gauss.canFire());
    // Tactical reload always requires another rack; empty rack cannot clear it.
    assert(reload.remove()); assert(reload.insert(1));
    assert(!reload.rack(0)); assert(!reload.canFire());
    assert(reload.rack(1)); assert(reload.canFire());
}
