"""Extract native detached magazine surfaces, including pistol cartridge/joint4 bones.

Inputs are locally extracted Steam assets. No original game archive is modified.
"""
from pathlib import Path
import struct, math, re, json

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / 'reports/archive-inspection'
OUTPUT = ROOT / 'game-vr/gamedata/meshes/copvr/magazines'
OUTPUT.mkdir(parents=True, exist_ok=True)

def chunks(data):
    offset = 0
    while offset + 8 <= len(data):
        kind, size = struct.unpack_from('<II', data, offset)
        yield kind, data[offset+8:offset+8+size]
        offset += 8 + size

def chunk(kind, data):
    return struct.pack('<II', kind, len(data)) + data

def header(kind, points):
    lo = [min(p[i] for p in points) for i in range(3)]
    hi = [max(p[i] for p in points) for i in range(3)]
    mid = [(a+b)/2 for a,b in zip(lo,hi)]
    radius = max(math.dist(p,mid) for p in points)
    return chunk(1,struct.pack('<BBH10f',4,kind,0,*lo,*hi,*mid,radius))

def save(name, surfaces, skeleton):
    points=[v[:3] for _,vertices,_ in surfaces for v in vertices]
    center=[(min(p[i] for p in points)+max(p[i] for p in points))/2 for i in range(3)]
    children=[]
    for index,(material,vertices,indices) in enumerate(surfaces):
        pts=[v[:3] for v in vertices]
        child=header(5,pts)+chunk(2,material)
        child+=chunk(3,struct.pack('<II',1,len(vertices))+b''.join(struct.pack('<14fI',*v[:14],0) for v in vertices))
        child+=chunk(4,struct.pack('<I',len(indices))+struct.pack('<'+'H'*len(indices),*indices))
        children.append(chunk(index,child))
    lo=[min(p[i] for p in points) for i in range(3)]
    hi=[max(p[i] for p in points) for i in range(3)]
    half=[max(.001,(hi[i]-lo[i])/2) for i in range(3)]
    obb=struct.pack('<15f',1,0,0,0,1,0,0,0,1,*center,*half)
    bones=struct.pack('<I',1)+b'copvr_magazine\0\0'+obb
    shape=struct.pack('<HH27f',0,0,*([0.]*27))
    joint=struct.pack('<I12f2fI3f',0,*([0.,0.,1.,1.]*3),1.,1.,0,0.,0.,0.)
    ik=struct.pack('<I',1)+b'default_object\0'+shape+joint+struct.pack('<10f',0,0,0,0,0,0,.15,*center)
    data=header(10,points)+chunk(9,b''.join(children))+chunk(13,bones)+chunk(16,ik)
    (OUTPUT/(name+'.ogf')).write_bytes(data)
    return center

profiles=[]
for model in sorted((SOURCE/'meshes/dynamics/weapons').rglob('*_hud.ogf')):
    data=dict(chunks(model.read_bytes()));d=data.get(13)
    if not d:continue
    names=[];at=4
    for _ in range(struct.unpack_from('<I',d)[0]):
        end=d.index(0,at);names.append(d[at:end].decode());at=d.index(0,end+1)+61
    pistol=model.stem in {'wpn_'+n+'_hud' for n in ['beretta92fs','colt1911','desert_eagle','fort','hpsa','pb','pm','sig220','usp45','walter99']}
    magazine_bone=('joint4' if model.stem=='wpn_hpsa_hud' else 'cartridge') if pistol else 'magazin'
    if magazine_bone not in names:continue
    bone=names.index(magazine_bone);surfaces=[]
    for _,child in chunks(data[9]):
        mesh=dict(chunks(child));vb=mesh.get(3);ib=mesh.get(4)
        if not vb or not ib:continue
        fmt,count=struct.unpack_from('<II',vb);stride=(len(vb)-8)//count
        assert fmt in (1,2),(model,fmt)
        vertices=[];owners=[]
        for i in range(count):
            start=8+i*stride
            if fmt==1:
                offset=0;owner=struct.unpack_from('<I',vb,start+56)[0]
            else:
                a,b=struct.unpack_from('<HH',vb,start);weight=struct.unpack_from('<f',vb,start+52)[0];owner=a if a==b or weight<.001 else b if weight>.999 else -1;offset=4
            pos=struct.unpack_from('<3f',vb,start+offset)
            rest=struct.unpack_from('<9f',vb,start+offset+12)+struct.unpack_from('<2f',vb,start+(48 if fmt==1 else 56))
            if not all(math.isfinite(x) for x in rest[6:9]):
                n,t=rest[:3],rest[3:6]
                b=(n[1]*t[2]-n[2]*t[1],n[2]*t[0]-n[0]*t[2],n[0]*t[1]-n[1]*t[0])
                length=math.sqrt(sum(x*x for x in b));assert length>1e-8
                rest=rest[:6]+tuple(x/length for x in b)+rest[9:]
            vertices.append(pos+rest+(owner,));owners.append(owner)
        n=struct.unpack_from('<I',ib)[0];indices=struct.unpack_from('<'+'H'*n,ib,4)
        selected=[indices[i:i+3] for i in range(0,n,3) if all(owners[v]==bone for v in indices[i:i+3])]
        if not selected:continue
        used=sorted({i for face in selected for i in face});mapping={v:i for i,v in enumerate(used)}
        surfaces.append((mesh[2],[vertices[i] for i in used],[mapping[v] for face in selected for v in face]))
    assert surfaces,model
    center=save(model.stem,surfaces,data)
    if pistol:
        # Isolate the native magazine child so empty-slide animations cannot
        # expose its geometry through a mixed body/material skin batch.
        split=[]
        for _,child in chunks(data[9]):
            mesh=dict(chunks(child));vb=mesh.get(3);ib=mesh.get(4)
            if not vb or not ib:continue
            fmt,count=struct.unpack_from('<II',vb);assert fmt==1
            stride=(len(vb)-8)//count
            owners=[struct.unpack_from('<I',vb,8+i*stride+56)[0] for i in range(count)]
            n=struct.unpack_from('<I',ib)[0];ix=struct.unpack_from('<'+'H'*n,ib,4)
            groups=[[],[]]
            for i in range(0,n,3):
                face=ix[i:i+3];hits=sum(owners[v]==bone for v in face)
                assert hits in (0,3),(model,'mixed magazine triangle')
                groups[1 if hits else 0].append(face)
            for faces in groups:
                if not faces:continue
                used=sorted({v for face in faces for v in face});remap={v:i for i,v in enumerate(used)}
                raw=[vb[8+i*stride:8+(i+1)*stride] for i in used]
                points=[struct.unpack_from('<3f',v) for v in raw]
                indices=[remap[v] for face in faces for v in face]
                part=header(5,points)+chunk(2,mesh[2])+chunk(3,struct.pack('<II',fmt,len(used))+b''.join(raw))
                part+=chunk(4,struct.pack('<I',len(indices))+struct.pack('<'+'H'*len(indices),*indices))
                split.append(chunk(len(split),part))
        replacement=b''.join(chunk(k,b''.join(split) if k==9 else v) for k,v in chunks(model.read_bytes()))
        override=ROOT/'game-vr/gamedata/meshes'/model.relative_to(SOURCE/'meshes')
        override.parent.mkdir(parents=True,exist_ok=True);override.write_bytes(replacement)
    bolt='joint2' if model.stem=='wpn_hpsa_hud' else next((b for b in ('breachblock','zatvor','wpn_bolt','bolt','lock') if b in names),'')
    profiles.append(dict(model=model.stem,center=center,bone=magazine_bone,bolt=bolt,pistol=pistol))

# Resolve actual HUD section names (Beretta's model filename differs).
mapping={}
for p in (SOURCE/'configs/weapons').glob('w_*.ltx'):
    section=None
    for line in p.read_text(encoding='cp1251').splitlines():
        m=re.match(r'\s*\[([^]]+)\]',line)
        if m:section=m[1]
        m=re.match(r'\s*item_visual\s*=\s*(\S+)',line)
        if m and section:mapping[Path(m[1].replace('\\','/')).stem]=section

handles=[]
for model in sorted((SOURCE/'meshes/dynamics/weapons').rglob('*_hud.ogf')):
    data=dict(chunks(model.read_bytes()));d=data.get(13)
    if not d:continue
    names=[];at=4
    for _ in range(struct.unpack_from('<I',d)[0]):
        end=d.index(0,at);names.append(d[at:end].decode());at=d.index(0,end+1)+61
    bolt='joint2' if model.stem=='wpn_hpsa_hud' else next((b for b in ('breachblock','zatvor','zatvor_jnt','wpn_bolt','bolt','lock') if b in names),'')
    if not bolt:continue
    bone=names.index(bolt);points=[]
    for _,child in chunks(data[9]):
        vb=dict(chunks(child)).get(3)
        if not vb:continue
        fmt,count=struct.unpack_from('<II',vb);stride=(len(vb)-8)//count
        for i in range(count):
            off=8+i*stride
            if fmt==1:owner=struct.unpack_from('<I',vb,off+56)[0];position=off
            elif fmt==2:
                a,b=struct.unpack_from('<HH',vb,off);w=struct.unpack_from('<f',vb,off+52)[0]
                owner=a if a==b or w<.001 else b if w>.999 else -1;position=off+4
            else:continue
            if owner==bone:points.append(struct.unpack_from('<3f',vb,position))
    assert points,(model,bolt)
    pistol=bolt in ('breachblock','joint2')
    lo=[min(p[i] for p in points) for i in range(3)];hi=[max(p[i] for p in points) for i in range(3)]
    if pistol:
        contact=[(lo[0]+hi[0])/2,hi[1]-.005,lo[2]+min(.025,(hi[2]-lo[2])*.25)]
    else:
        side=-1 if abs(lo[0])>abs(hi[0]) else 1
        extreme=min(p[0]*side for p in points) # use outer side, not receiver centre
        outer=[p for p in points if p[0]*side>=max(q[0]*side for q in points)-.004]
        contact=[(min(p[i] for p in outer)+max(p[i] for p in outer))/2 for i in range(3)]
    handles.append(dict(hud=mapping.get(model.stem,model.stem),model=model.stem,bone=bolt,contact=contact,travel=.035 if pistol else .075))

config=ROOT/'game-vr/gamedata/configs/copvr_equipment.ltx'
text=config.read_text(encoding='utf-8');text=re.sub(r'\n\[(?:magazine_vr_|handle_vr_)[^[]*(?=\n\[|\Z)','',text)
for p in profiles:
    hud=mapping.get(p['model'],p['model']);p['hud']=hud
    text+=f"\n[magazine_vr_{hud}]\nmodel = copvr\\magazines\\{p['model']}\n"
    text+='center = '+', '.join(f'{x:.6f}' for x in p['center'])+'\n'
    text+=f"bone = {p['bone']}\nbolt_bone = {p['bolt']}\npistol = {'true' if p['pistol'] else 'false'}\n"
    socket=list(p['center'])
    if p['pistol']:socket[1]-=.04
    text+='socket_center = '+', '.join(f'{x:.6f}' for x in socket)+'\n'
    text+=f"manual_rack = {'false' if p['model']=='wpn_gauss_hud' else 'true'}\n"
for p in handles:
    text+=f"\n[handle_vr_{p['hud']}]\nbone = {p['bone']}\n"
    text+='contact = '+', '.join(f'{x:.6f}' for x in p['contact'])+'\n'
    text+=f"axis = 0, 0, -1\ntravel = {p['travel']:.3f}\n"
config.write_text(text,encoding='utf-8')
(ROOT/'reports/weapon-handle-models.json').write_text(json.dumps(handles,indent=2),encoding='utf-8')
(ROOT/'reports/magazine-models.json').write_text(json.dumps(profiles,indent=2),encoding='utf-8')
print(f'Generated {len(profiles)} magazine profiles and {len(list(OUTPUT.glob("*.ogf")))} models.')
