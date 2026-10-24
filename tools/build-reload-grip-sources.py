from pathlib import Path
import json,struct,re
root=Path.cwd(); ns={'__file__':str(root/'tools/build-magazine-models.py')}; t=(root/'tools/build-magazine-models.py').read_text();exec(t[:t.index('profiles=[]')],ns)
profiles=[]
for m in json.loads((root/'reports/magazine-models.json').read_text()):profiles.append((m['hud'],m['bone'],m['center'],'anm_reload'))
for hud,bone,alias in [('wpn_winchester1300_hud','cartridger','anm_add_cartridge'),('wpn_spas12_hud','joint2','anm_add_cartridge'),('wpn_bm16_hud','cartridgell','anm_reload_2'),('wpn_toz34_hud','cartridgel','anm_reload_2'),('wpn_rpg7_hud','grenade','anm_reload'),('wpn_rg6_hud','grenade','anm_add_cartridge')]:
 p=next((root/'reports/archive-inspection/meshes').rglob(hud+'.ogf'));data=dict(ns['chunks'](p.read_bytes()));d=data[13];names=[];at=4
 for _ in range(struct.unpack_from('<I',d)[0]):
  end=d.index(0,at);names.append(d[at:end].decode());at=d.index(0,end+1)+61
 pts=[];bid=names.index(bone)
 for _,child in ns['chunks'](data[9]):
  mesh=dict(ns['chunks'](child));vb=mesh.get(3)
  if not vb:continue
  fmt,count=struct.unpack_from('<II',vb);stride=(len(vb)-8)//count
  for i in range(count):
   start=8+i*stride
   if fmt==1: owner=struct.unpack_from('<I',vb,start+56)[0];off=0
   elif fmt==2:
    a,b=struct.unpack_from('<HH',vb,start);w=struct.unpack_from('<f',vb,start+52)[0];owner=a if a==b or w<.001 else b if w>.999 else -1;off=4
   else:raise ValueError(fmt)
   if owner==bid:pts.append(struct.unpack_from('<3f',vb,start+off))
 assert pts,(hud,bone)
 center=[(min(p[i] for p in pts)+max(p[i] for p in pts))/2 for i in range(3)]
 profiles.append(("wpn_rg-6_hud" if hud=="wpn_rg6_hud" else hud,bone,center,alias))
p=root/'game-vr/gamedata/configs/copvr_equipment.ltx';text=p.read_text(encoding='utf-8');text=re.sub(r'\n\[reload_grip_[^[]*(?=\n\[|\Z)','',text)
for hud,bone,center,alias in profiles:text+=f'\n[reload_grip_{hud}]\nbone = {bone}\ncenter = '+', '.join(map(str,center))+f'\nalias = {alias}\n'
p.write_text(text,encoding='utf-8');(root/'reports/reload-grip-sources.json').write_text(json.dumps(profiles,indent=2));print('Validated geometry sources:',len(profiles))
