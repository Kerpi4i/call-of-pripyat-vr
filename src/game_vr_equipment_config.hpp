#pragma once
namespace copvr_game {
// A separate optional file avoids modifying shipped weapon configs or saves.
inline CInifile* equipmentConfig() {
    static CInifile* value=[]() -> CInifile* {
        string_path path; FS.update_path(path,"$game_config$","copvr_equipment.ltx");
        return FS.exist(path)?xr_new<CInifile>(path):nullptr;
    }();
    return value;
}
inline Fvector equipmentVector(LPCSTR section,LPCSTR key,const Fvector& fallback) {
    auto ini=equipmentConfig();
    if(!ini||!ini->section_exist(section)||!ini->line_exist(section,key)) return fallback;
    auto value=ini->r_fvector3(section,key); return _valid(value)?value:fallback;
}
inline Fmatrix equipmentPose(LPCSTR section,LPCSTR prefix,const Fmatrix& source) {
    string128 positionKey,rotationKey;
    xr_sprintf(positionKey,"%s_position",prefix); xr_sprintf(rotationKey,"%s_rotation",prefix);
    Fvector zero; zero.set(0,0,0);
    auto offset=equipmentVector(section,positionKey,zero);
    auto angles=equipmentVector(section,rotationKey,zero); angles.mul(PI/180.f);
    Fmatrix adjustment,result; adjustment.setHPB(angles.x,angles.y,angles.z); adjustment.c=offset;
    result.mul_43(source,adjustment); return result;
}
}
