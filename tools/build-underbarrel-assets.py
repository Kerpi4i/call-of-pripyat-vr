"""Native underbarrel muzzle contacts, single rounds and final reload audio."""
from pathlib import Path
import struct,re,json,subprocess,importlib.util,math
ROOT=Path(__file__).resolve().parent.parent
helpers={'__file__':str(ROOT/'tools/build-magazine-models.py')};text=(ROOT/'tools/build-magazine-models.py').read_text();exec(text[:text.index('profiles=[]')],helpers)
chunks=helpers['chunks'];helpers['OUTPUT']=ROOT/'game-vr/gamedata/meshes/copvr/rounds'
spec=importlib.util.spec_from_file_location('vr_sound',ROOT/'tools/build-vr-mechanical-sounds.py');sound=importlib.util.module_from_spec(spec);spec.loader.exec_module(sound)
sections={};parents={}
for p in (ROOT/'reports/archive-inspection/configs/weapons').glob('*.ltx'):
 current=None
 for line in p.read_text(encoding='cp1251').splitlines():
  line=line.split(';')[0].strip();m=re.match(r'\[([^]]+)\](?::(.*))?',line)
  if m:current=m[1];sections.setdefault(current,{});parents[current]=[v.strip() for v in (m[2] or '').split(',') if v.strip()]
  elif current and '=' in line:k,v=line.split('=',1);sections[current][k.strip()]=v.strip()
def resolve(name,seen=()):
 result={}
 if name in seen:return result
 for parent in parents.get(name,[]):result.update(resolve(parent,seen+(name,)))
 result.update(sections.get(name,{}));return result
config=ROOT/'game-vr/gamedata/configs/copvr_equipment.ltx';text=config.read_text(encoding='utf-8');text=re.sub(r'\n\[(?:launcher_vr_|launcher_grip_|launcher_round_)[^[]*(?=\n\[|\Z)','',text)
manifest=[]
for hud,weapon in [('wpn_ak74_hud','wpn_ak74'),('wpn_abakan_hud','wpn_abakan'),('wpn_lr300_hud','wpn_lr300'),('wpn_groza_hud','wpn_groza'),('wpn_fn2000_hud','wpn_fn2000'),('wpn_sig550_hud','wpn_sig550')]:
 data=dict(chunks(next((ROOT/'reports/archive-inspection/meshes').rglob(hud+'.ogf')).read_bytes()));d=data[13];at=4;names=[]
 for _ in range(struct.unpack_from('<I',d)[0]):
  e=d.index(0,at);names.append(d[at:e].decode());at=d.index(0,e+1)+61
 bone=names.index('grenade');surfaces=[]
 for _,child in chunks(data[9]):
  mesh=dict(chunks(child));vb=mesh[3];fmt,n=struct.unpack_from('<II',vb);stride=(len(vb)-8)//n;vertices=[];owners=[]
  for i in range(n):
   a=8+i*stride
   if fmt==1:off=0;owner=struct.unpack_from('<I',vb,a+56)[0]
   elif fmt==2:
    off=4;x,y=struct.unpack_from('<HH',vb,a);weight=struct.unpack_from('<f',vb,a+52)[0];owner=x if x==y or weight<.001 else y if weight>.999 else -1
   else:raise ValueError(fmt)
   values=struct.unpack_from('<12f',vb,a+off)+struct.unpack_from('<2f',vb,a+(48 if fmt==1 else 56))
   if not all(math.isfinite(v) for v in values[9:12]):
    nrm,tan=values[3:6],values[6:9];cross=(nrm[1]*tan[2]-nrm[2]*tan[1],nrm[2]*tan[0]-nrm[0]*tan[2],nrm[0]*tan[1]-nrm[1]*tan[0]);length=math.sqrt(sum(v*v for v in cross))
    values=values[:9]+tuple(v/max(length,1e-8) for v in cross)+values[12:]
   if owner==bone:assert all(math.isfinite(v) for v in values),(hud,values)
   vertices.append(values+(0,));owners.append(owner)
  ib=mesh[4];n=struct.unpack_from('<I',ib)[0];indices=struct.unpack_from('<'+'H'*n,ib,4)
  faces=[indices[i:i+3] for i in range(0,n,3) if all(owners[v]==bone for v in indices[i:i+3])]
  if not faces:continue
  used=sorted({v for face in faces for v in face});mapping={v:i for i,v in enumerate(used)}
  surfaces.append((mesh[2],[vertices[v] for v in used],[mapping[v] for face in faces for v in face]))
 assert surfaces,hud
 center=helpers['save']('launcher_'+hud,surfaces,data);round_profile='launcher_round_'+hud
 h=resolve(hud);w=resolve(weapon);port=[float(v) for v in h['fire_point2'].split(',')];port_bone=h.get('fire_bone2','wpn_body')
 source=w['snd_reload_grenade'].split(',')[0].strip().split('\\')[-1];source_path=ROOT/'reports/archive-inspection/sounds/weapons'/(source+'.ogg')
 duration=float(json.loads(subprocess.check_output(['ffprobe','-v','quiet','-show_format','-of','json',str(source_path)]))['format']['duration']);start=duration*.65
 output=ROOT/'game-vr/gamedata/sounds/copvr/mechanical'/('launcher_'+hud+'_finish.ogg')
 subprocess.run(['ffmpeg','-v','error','-y','-i',str(source_path),'-af',f'atrim=start={start}:end={duration},asetpts=PTS-STARTPTS,afade=t=in:d=0.005,afade=t=out:st={duration-start-.005}:d=0.005','-ac','1','-ar','44100','-c:a','libvorbis','-q:a','5','-map_metadata','-1',str(output)],check=True);sound.native_metadata(output,source_path)
 path='copvr\\mechanical\\'+output.stem
 text+=f'\n[{round_profile}]\nmodel = copvr\\rounds\\launcher_{hud}\ncenter = '+', '.join(map(str,center))+'\n'
 text+=f'\n[launcher_grip_{hud}]\nbone = grenade\nalias = anm_reload_g\ncenter = '+', '.join(map(str,center))+'\n'
 text+=f'\n[launcher_vr_{hud}]\nbone = {port_bone}\nport = '+', '.join(map(str,port))+f'\nround_profile = {round_profile}\nfinish_fraction = 0.65\nfinish_sound = {path}\n'
 manifest.append(dict(hud=hud,weapon=weapon,port=port,bone=port_bone,center=center,audio=str(output.relative_to(ROOT)),source=source,start=start,end=duration))
for name,save in [('underbarrel_test','copvr_underbarrel_test'),('jam_test','copvr_jam_test')]:
 text=re.sub(r'\n\['+name+r'\][^[]*(?=\n\[|\Z)','',text);text+=f'\n[{name}]\nenabled = true\nsave_name = {save}\nunlimited_weight = true\n'
config.write_text(text,encoding='utf-8');(ROOT/'reports/underbarrel-profiles.json').write_text(json.dumps(manifest,indent=2));print('Built 6 native underbarrel round, grip, muzzle and final-audio profiles.')
