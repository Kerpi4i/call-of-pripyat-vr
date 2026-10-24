"""Local native shotgun round and non-detachable loading profiles."""
from pathlib import Path
import struct, json, re
ROOT=Path(__file__).resolve().parent.parent
helpers={'__file__':str(ROOT/'tools/build-magazine-models.py')}
text=(ROOT/'tools/build-magazine-models.py').read_text()
exec(text[:text.index('profiles=[]')],helpers)
chunks=helpers['chunks']

def model(name):
    path=next((ROOT/'reports/archive-inspection/meshes').rglob(name+'.ogf'))
    data=dict(chunks(path.read_bytes()));names=[];offset=4
    for _ in range(struct.unpack_from('<I',data[13])[0]):
        end=data[13].index(0,offset);names.append(data[13][offset:end].decode());offset=data[13].index(0,end+1)+61
    return data,names

def surfaces(data,names,bone):
    wanted=names.index(bone);result=[];points=[]
    for _,child in chunks(data[9]):
        mesh=dict(chunks(child));vb=mesh.get(3);ib=mesh.get(4)
        if not vb or not ib:continue
        fmt,count=struct.unpack_from('<II',vb);assert fmt==1
        vertices=[struct.unpack_from('<14fI',vb,8+i*60) for i in range(count)]
        count=struct.unpack_from('<I',ib)[0];indices=struct.unpack_from('<'+'H'*count,ib,4)
        faces=[indices[i:i+3] for i in range(0,count,3) if all(vertices[j][-1]==wanted for j in indices[i:i+3])]
        if not faces:continue
        used=sorted({j for f in faces for j in f});remap={j:i for i,j in enumerate(used)}
        result.append((mesh[2],[vertices[j] for j in used],[remap[j] for f in faces for j in f]))
        points.extend(vertices[j][:3] for j in used)
    assert result,bone
    return result,points

data,names=model('wpn_toz34_hud');shell,_=surfaces(data,names,'cartridgel')
helpers['OUTPUT']=ROOT/'game-vr/gamedata/meshes/copvr/rounds';helpers['OUTPUT'].mkdir(parents=True,exist_ok=True)
center=helpers['save']('shell12g',shell,data)
rocket=ROOT/'reports/archive-inspection/meshes/dynamics/weapons/wpn_rpg7/wpn_rpg7_grenade.ogf'
header=dict(chunks(rocket.read_bytes()))[1]
lohi=struct.unpack_from('<6f',header,4);rocket_center=[(lohi[i]+lohi[i+3])/2 for i in range(3)]

data,names=model('wpn_rg6_hud');vog,vog_points=surfaces(data,names,'grenade')
vog_center=helpers['save']('vog25',vog,data)
vog_port=list(vog_center);vog_port[2]=max(p[2] for p in vog_points)+.035

profiles={
 'wpn_winchester1300_hud':dict(kind=1,port0=[0,.045,.19],pump_bone='zatvor',bolt_bone='zatvor',feed_bone='cartridger',travel=.10),
 'wpn_spas12_hud':dict(kind=1,port0=[0,.005,.16],pump_bone='joint1',bolt_bone='zatvor',feed_bone='joint2',travel=.09,load_requires_open=1),
 'wpn_bm16_hud':dict(kind=2,hinge='bend_reload',chamber0='cartridgell',chamber1='cartridgerr'),
 'wpn_toz34_hud':dict(kind=2,hinge='bend_reload',chamber0='cartridgel',chamber1='cartridger'),
 'wpn_rg-6_hud':dict(kind=5,port0=vog_port,latch=[-.0001,-.0471,.114],hinge='reload',drum='rotategrenades',feed_bone='grenade',open_motion='rg6_reload_start'),
 'wpn_protecta_hud':dict(kind=4,port0=[.0342,.125,-.010]),
 'wpn_rpg7_hud':dict(kind=3,port0=[0,.054,.216]),
}
for hud,profile in profiles.items():
    if profile['kind']!=2:continue
    data,names=model(hud)
    for i in range(2):
        _,pts=surfaces(data,names,profile['chamber'+str(i)])
        profile['port'+str(i)]=[(min(p[j] for p in pts)+max(p[j] for p in pts))/2 for j in range(3)]
        profile['port'+str(i)][2]=min(p[2] for p in pts)-.012

path=ROOT/'game-vr/gamedata/configs/copvr_equipment.ltx'
text=re.sub(r'\n\[(?:internal_vr_|internal_round_)[^[]*(?=\n\[|\Z)','',path.read_text(encoding='utf-8'))
text+='\n[internal_round_12g]\nmodel = copvr\\rounds\\shell12g\ncenter = '+', '.join(map(str,center))+'\n'
text+='\n[internal_round_rocket]\nmodel = dynamics\\weapons\\wpn_rpg7\\wpn_rpg7_grenade\ncenter = '+', '.join(map(str,rocket_center))+'\n'
text+='\n[internal_round_vog25]\nmodel = copvr\\rounds\\vog25\ncenter = '+', '.join(map(str,vog_center))+'\n'
for hud,profile in profiles.items():
    text+=f'\n[internal_vr_{hud}]\nround_profile = internal_round_{"rocket" if profile["kind"]==3 else "vog25" if profile["kind"]==5 else "12g"}\n'
    for key,value in profile.items():text+=f'{key} = '+(', '.join(map(str,value)) if isinstance(value,list) else str(value))+'\n'
    if profile['kind']==3:text+='finish_fraction = 0.60\n'
    if profile['kind']==2:text+='finish_fraction = 0.90\nmax_angle = 0.65\n'
path.write_text(text,encoding='utf-8')
(ROOT/'reports/internal-reload-profiles.json').write_text(json.dumps(profiles,indent=2))
print('Built native single 12g shell and 7 loading profiles; rocket length:',round(lohi[5]-lohi[2],3),'m')
