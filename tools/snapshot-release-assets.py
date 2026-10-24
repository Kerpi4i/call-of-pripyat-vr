"""Freeze the tested overlay and graphics, excluding private data and test cheats."""
from pathlib import Path
import hashlib, json, re, shutil
root=Path(__file__).resolve().parent.parent
out=root/'release'
allowed={'.ltx','.xml','.script','.dds','.ogf','.omf','.ogg','.ps','.vs','.s','.h','.anm','.anms','.thm'}
copied=[]
for path in sorted((root/'game-vr/gamedata').rglob('*')):
    if not path.is_file() or path.suffix.lower() not in allowed: continue
    relative=path.relative_to(root/'game-vr/gamedata')
    target=out/'gamedata'/relative;target.parent.mkdir(parents=True,exist_ok=True)
    data=path.read_bytes()
    if str(relative).replace('\\','/')=='configs/copvr_equipment.ltx':
        text=data.decode('cp1251');section='';lines=[]
        for line in text.splitlines():
            if line.startswith('['): section=line.strip('[]').strip()
            if section in {'scope_test','integrated_scope_test','all_weapons_test','underbarrel_test','jam_test'} and re.match(r'\s*(enabled|unlimited_weight)\s*=',line):
                line=line.split('=')[0]+'= false'
            if re.match(r'\s*debug_zones\s*=',line):line='debug_zones = false'
            lines.append(line)
        data=('\r\n'.join(lines)+'\r\n').encode('cp1251')
    target.write_bytes(data)
    copied.append({'path':relative.as_posix(),'sha256':hashlib.sha256(data).hexdigest(),'bytes':len(data)})
# Full graphics snapshot, not identities, sound devices, bindings, cheats or saves.
source=root/'game-vr/appdata/user.ltx';graphics=[]
for line in source.read_text(encoding='cp1251').splitlines():
    key=line.split(maxsplit=1)[0] if line.strip() else ''
    if key.startswith(('r1_','r2_','r3_','r4_','r__','rs_','vid_')) or key in {'_preset','texture_lod','ph_iterations'}:
        
        if key != 'vid_monitor': graphics.append(re.sub(r'\s+\(\d+Hz\)', ' (0Hz)', line) if key == 'vid_mode' else line)
graphics += ['renderer renderer_r4','copvr_show_interaction_zones off']
(out/'defaults/user.ltx').write_text('\n'.join(graphics)+'\n',encoding='ascii')
template=(root/'game-vr/fsgame.ltx').read_text(encoding='ascii')
game='C:\\Program Files (x86)\\Steam\\steamapps\\common\\Stalker Call of Pripyat\\'
template=template.replace(str(root/'game-vr/appdata')+'\\','@DATA@').replace(str(root/'game-vr/gamedata')+'\\','@ROOT@gamedata\\').replace(game,'@GAME@')
if str(root) in template or 'C:\\' in template:raise ValueError('Unresolved machine-specific fs path')
(out/'defaults/fsgame.template.ltx').write_text(template,encoding='ascii')
(out/'asset-manifest.json').write_text(json.dumps({'version':'0.1.0-alpha.1','overlay':copied,'graphics_source_sha256':hashlib.sha256(source.read_bytes()).hexdigest()},indent=2)+'\n')
print(f'Frozen {len(copied)} overlay files and {len(graphics)} graphics settings; no saves copied.')
