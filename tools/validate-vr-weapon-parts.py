"""Offline checks of extracted magazine models and authored interaction points."""
import configparser, json, math, struct
from collections import Counter
from pathlib import Path

ROOT=Path(__file__).resolve().parent.parent

def chunks(data):
    offset=0
    while offset+8<=len(data):
        kind,size=struct.unpack_from('<II',data,offset)
        assert offset+8+size<=len(data)
        yield kind,data[offset+8:offset+8+size]
        offset+=8+size
    assert offset==len(data)

def bone_names(data):
    count=struct.unpack_from('<I',data)[0];offset=4;names=[]
    for _ in range(count):
        end=data.index(0,offset);names.append(data[offset:end].decode())
        offset=data.index(0,end+1)+61
    assert offset==len(data)
    return names

ini=configparser.ConfigParser()
ini.read(ROOT/'game-vr/gamedata/configs/copvr_equipment.ltx',encoding='utf-8')
magazines=json.loads((ROOT/'reports/magazine-models.json').read_text())
handles=json.loads((ROOT/'reports/weapon-handle-models.json').read_text())
for profile in magazines:
    section='magazine_vr_'+profile['hud']
    path=ROOT/'game-vr/gamedata/meshes'/(ini[section]['model'].replace('\\','/')+'.ogf')
    data=dict(chunks(path.read_bytes()))
    assert data[1][1]==10 and bone_names(data[13])==['copvr_magazine']
    # One identity rest pose; no inherited weapon bone/visibility state.
    ik=data[16];offset=4;offset=ik.index(0,offset)+1+112+76
    assert len(ik)==offset+40
    assert struct.unpack_from('<6f',ik,offset)==(0,0,0,0,0,0)
    points=[]
    for _,child in chunks(data[9]):
        mesh=dict(chunks(child));assert mesh[1][1]==5
        fmt,count=struct.unpack_from('<II',mesh[3]);assert fmt==1 and len(mesh[3])==8+count*60
        for i in range(count):
            values=struct.unpack_from('<14fI',mesh[3],8+i*60)
            assert all(math.isfinite(v) for v in values)
            assert values[-1]==0
            points.append(values[:3])
        count=struct.unpack_from('<I',mesh[4])[0]
        indices=struct.unpack_from('<'+'H'*count,mesh[4],4)
        assert count>0 and count%3==0 and max(indices)<(len(mesh[3])-8)//60
    center=[(min(p[i] for p in points)+max(p[i] for p in points))/2 for i in range(3)]
    assert all(abs(a-b)<.000002 for a,b in zip(center,profile['center']))
    socket=list(map(float,ini[section]['socket_center'].split(',')))
    for axis in range(3):
        expected=center[axis]-(.04 if profile['pistol'] and axis==1 else 0)
        assert abs(socket[axis]-expected)<.000002
    assert ini[section].getboolean('manual_rack') == (profile['model']!='wpn_gauss_hud')
    if profile['pistol']:
        original=next((ROOT/'reports/archive-inspection/meshes').rglob(profile['model']+'.ogf'))
        override=ROOT/'game-vr/gamedata/meshes'/original.relative_to(ROOT/'reports/archive-inspection/meshes')
        before=dict(chunks(original.read_bytes()));after=dict(chunks(override.read_bytes()))
        assert {k:v for k,v in before.items() if k!=9}=={k:v for k,v in after.items() if k!=9}
        wanted=bone_names(before[13]).index(profile['bone'])
        def triangles(model):
            result=Counter();isolated=0
            for _,child in chunks(model[9]):
                mesh=dict(chunks(child));vb=mesh.get(3);ib=mesh.get(4)
                if not vb or not ib:continue
                fmt,count=struct.unpack_from('<II',vb);assert fmt==1
                stride=(len(vb)-8)//count;vertices=[vb[8+i*stride:8+(i+1)*stride] for i in range(count)]
                owners={struct.unpack_from('<I',v,56)[0] for v in vertices}
                if owners=={wanted}:isolated+=1
                size=struct.unpack_from('<I',ib)[0];indices=struct.unpack_from('<'+'H'*size,ib,4)
                for i in range(0,size,3):
                    result[(mesh[2],tuple(vertices[j] for j in indices[i:i+3]))]+=1
            return result,isolated
        original_faces,_=triangles(before);split_faces,isolated=triangles(after)
        assert original_faces==split_faces and isolated>0

for profile in handles:
    path=next((ROOT/'reports/archive-inspection/meshes/dynamics/weapons').rglob(profile['model']+'.ogf'))
    data=dict(chunks(path.read_bytes()));names=bone_names(data[13]);bone=names.index(profile['bone'])
    points=[]
    for _,child in chunks(data[9]):
        vb=dict(chunks(child)).get(3)
        if not vb:continue
        fmt,count=struct.unpack_from('<II',vb);stride=(len(vb)-8)//count
        for i in range(count):
            offset=8+i*stride
            if fmt==1:
                owner=struct.unpack_from('<I',vb,offset+56)[0];position=offset
            elif fmt==2:
                a,b=struct.unpack_from('<HH',vb,offset);weight=struct.unpack_from('<f',vb,offset+52)[0]
                owner=a if a==b or weight<.001 else b if weight>.999 else -1;position=offset+4
            else:continue
            if owner==bone:points.append(struct.unpack_from('<3f',vb,position))
    assert points,profile
    for axis in range(3):
        assert min(p[axis] for p in points)-.001<=profile['contact'][axis]<=max(p[axis] for p in points)+.001,profile
    assert .015<=profile['travel']<=.18
    assert 'handle_vr_'+profile['hud'] in ini

print(f'PASS: {len(magazines)} magazines; 10 lossless pistol mesh splits; lowered sockets; Gauss; {len(handles)} part contacts.')
