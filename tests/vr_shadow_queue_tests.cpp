#include "vr_shadow_queue.hpp"
#include <vector>
#undef NDEBUG
#include <cassert>
int main(){
    struct Work {int id;bool ready;};
    std::vector<Work> queue{{0,false},{1,true},{2,false},{3,true}};
    auto ready=[](const Work& work){return work.ready;};
    assert(copvr::readyShadowTask(queue,ready)==1);
    std::vector<int> completed;
    while(!queue.empty()) {
        const auto i=copvr::readyShadowTask(queue,ready);
        completed.push_back(queue[i].id);queue.erase(queue.begin()+i);
    }
    assert((completed==std::vector<int>{1,3,0,2}));
    std::vector<Work> allPending{{0,false},{1,false}};
    assert(copvr::readyShadowTask(allPending,ready)==0);
    std::vector<Work> synchronous{{0,true}};
    assert(copvr::readyShadowTask(synchronous,ready)==0);
}
