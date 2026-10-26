#pragma once
#include <cstddef>
namespace copvr {
// Choose ready work without waiting on an earlier unfinished task. When none
// is ready, the caller waits on the oldest task and guarantees progress.
template<class Queue,class Ready>
std::size_t readyShadowTask(const Queue& queue,Ready ready) {
    for(std::size_t i=0;i<queue.size();++i)if(ready(queue[i]))return i;
    return 0;
}
}
