// Read runtime recommendations only: no graphics device, session or game launch.
#include "../src/openxr_d3d11.hpp"
int main() {
    try {
        copvr::Host host;
        if(!host.initialize(false))return 2;
        auto enumerate=host.fn<PFN_xrEnumerateViewConfigurationViews>("xrEnumerateViewConfigurationViews");
        uint32_t count=0;
        copvr::xr(enumerate(host.instance,host.system,XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,0,&count,nullptr),"view count");
        std::vector<XrViewConfigurationView> views(count,{XR_TYPE_VIEW_CONFIGURATION_VIEW});
        copvr::xr(enumerate(host.instance,host.system,XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,count,&count,views.data()),"view sizes");
        for(uint32_t i=0;i<count;++i)std::cout<<"Recommended eye "<<i<<": "<<views[i].recommendedImageRectWidth<<"x"<<views[i].recommendedImageRectHeight
            <<"; maximum "<<views[i].maxImageRectWidth<<"x"<<views[i].maxImageRectHeight<<"\n";
        std::cout<<"No XR session or swapchains created.\n";
        return 0;
    }catch(const std::exception& e) {std::cerr<<e.what()<<"\n";return 1;}
}
